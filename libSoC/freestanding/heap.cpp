/**
 * @file heap.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的堆分配器实现
 */

module SoC.freestanding;
import :heap;
import :utils;

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::heap::heap(::std::uintptr_t* begin, ::std::uintptr_t* end) noexcept(::SoC::optional_noexcept)
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(reinterpret_cast<::std::uintptr_t>(end) % page_size == 0, "堆结束地址必须对齐到页边界"sv);
        }
        auto bytes{(end - begin) * ptr_size};
        auto pages{bytes / (page_size + sizeof(::SoC::detail::heap_page_metadata))};
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(pages > 0, "堆大小必须大于一页"sv); }
        auto* metadata_begin{::std::launder(reinterpret_cast<::SoC::detail::heap_page_metadata*>(begin))};
        auto* metadata_end{metadata_begin + pages};
        metadata = ::std::span{metadata_begin, metadata_end};

        auto ptr{reinterpret_cast<::std::uintptr_t>(metadata_end)};
        ptr = (ptr + page_size - 1) & (-1zu << page_shift);
        data = reinterpret_cast<::SoC::detail::free_block_list_t*>(ptr);

        // 将所有未初始化的内存视为空闲页进行划分，并穿成链表
#pragma GCC unroll(2)
        for(auto&& page: metadata)
        {
            auto* free_block_list{reinterpret_cast<::SoC::detail::free_block_list_t*>(ptr)};
            ::new(&page)::SoC::detail::heap_page_metadata{&page + 1, free_block_list, 0, page_shift};
            ::new(free_block_list)::SoC::detail::free_block_list_t{nullptr};
            ptr += page_size;
        }
        metadata.back().next_page = nullptr;

        // 空闲链表按块大小排序，最后一项为空闲页链表
        // 将首个空闲页放入空闲页链表
        free_page_list.back() = metadata.data();

        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    ::SoC::detail::free_block_list_t* ::SoC::heap::make_block_in_page(::std::size_t free_list_index) noexcept(
        ::SoC::optional_noexcept)
    {
        auto&& block_metadata_ptr{free_page_list[free_list_index]};
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(block_metadata_ptr == nullptr, "仅在块空闲链表为空时调用此函数"sv); }

        auto&& free_page_list_head{free_page_list.back()};
        if(free_page_list_head == nullptr) [[unlikely]] { free_page_list_head = page_gc(true); }
        // 空闲页元数据指针
        auto* free_page_ptr{free_page_list_head};
        // 空闲页基址
        auto* page_begin{free_page_ptr->free_block_list};
        // 从空闲页表里取出一页
        free_page_list_head = ::std::exchange(free_page_list_head->next_page, nullptr);

        auto heap_block_size{1zu << (free_list_index + min_block_shift)};
        auto* page_ptr{page_begin};
        auto step{heap_block_size / ptr_size};
        // 最大分块对应的free_list_index
        constexpr auto max_block_free_list_index{page_shift - min_block_shift - 1};
        if(free_list_index == max_block_free_list_index) [[unlikely]]
        {
            // 一个页只能分出两个最大分块，不满足循环展开4次的要求，因此单独处理
            *page_ptr = ::SoC::detail::free_block_list_t{page_ptr + step};
            page_ptr += step;
            *page_ptr = ::SoC::detail::free_block_list_t{nullptr};
        }
        else
        {
#pragma GCC unroll(0)
            while(page_ptr != page_begin + page_size / ptr_size)
            {
                // 连续进行4次填充，利于循环展开
                for(auto i{0zu}; i != 4; ++i)
                {
                    *page_ptr = ::SoC::detail::free_block_list_t{page_ptr + step};
                    page_ptr += step;
                }
            }
            // 最后一个块的next指针设为nullptr
            *(page_ptr - step) = ::SoC::detail::free_block_list_t{};
        }
        // 从空闲链表里取出的页使用计数为0，不需要设置
        block_metadata_ptr = free_page_ptr;
        // 设置块大小的左移量
        block_metadata_ptr->block_size_shift = free_list_index + min_block_shift;
        return page_begin;
    }

    ::SoC::detail::heap_page_metadata* ::SoC::heap::insert_block_into_page_list(
        ::SoC::detail::heap_page_metadata* page_metadata) noexcept
    {
        auto* old_head{::std::exchange(page_metadata, page_metadata->next_page)};
        // 将空闲块指针指向数据区，对于页来说，完成了空闲链表的重新初始化
        // 对于其他大小的块，需要使用make_block_in_page函数重新初始化空闲链表
        old_head->free_block_list = ((old_head - metadata.data()) * page_size / ptr_size) + data;
        ::new(old_head->free_block_list)::SoC::detail::free_block_list_t{nullptr};
        auto* old_page_head{::std::exchange(free_page_list.back(), old_head)};
        old_head->next_page = old_page_head;
        // 初始化块大小的左移量为页大小左移量
        old_head->block_size_shift = page_shift;
        return page_metadata;
    }

    ::SoC::detail::heap_page_metadata* ::SoC::heap::page_gc(bool assert) noexcept(::SoC::optional_noexcept)
    {
#pragma GCC unroll(0)
        for(auto&& block_list: ::std::span{free_page_list}.first(block_size_cnt - 1))
        {
            auto* block_list_cursor{block_list};
            if(block_list_cursor == nullptr) { continue; }
            // 删除除了第一个块以外的所有空闲块
            for(auto* block_list_next{block_list_cursor->next_page}; block_list_next != nullptr;
                block_list_next = block_list_cursor->next_page)
            {
                if(block_list_next->used_block == 0)
                {
                    block_list_cursor->next_page = insert_block_into_page_list(block_list_next);
                }
                else
                {
                    block_list_cursor = block_list_next;
                }
            }
            // 若第一个块也空闲，则插入到空闲页链表
            if(block_list->used_block == 0) { block_list = insert_block_into_page_list(block_list); }
        }
        auto* free_page{free_page_list.back()};
        if(assert)
        {
            if constexpr(::SoC::is_build_mode(::SoC::build_mode::fuzzer))
            {
                ::SoC::fuzzer_assert(free_page != nullptr, fuzzer_error_code::heap_full);
            }
            else
            {
                ::SoC::always_check(free_page != nullptr, "剩余堆空间不足"sv);
            }
        }
        return free_page;
    }

    void* ::SoC::heap::remove_pages(::SoC::detail::free_block_list_t* range_begin,
                                    ::SoC::detail::free_block_list_t* range_end) noexcept
    {
        auto&& head{free_page_list.back()};
        // 移除除了head外所有范围内的页
        for(auto* ptr{head}; ptr->next_page != nullptr;)
        {
            auto* next_page{ptr->next_page};
            if(auto* page_ptr{next_page->free_block_list}; page_ptr >= range_begin && page_ptr <= range_end)
            {
                next_page->used_block = 1;
                next_page->free_block_list = nullptr;
                ptr->next_page = next_page->next_page;
            }
            else
            {
                ptr = ptr->next_page;
            }
        }
        // 若head也在范围内则删除
        if(auto* page_ptr{head->free_block_list}; page_ptr >= range_begin && page_ptr <= range_end)
        {
            head->used_block = 1;
            head->free_block_list = nullptr;
            head = head->next_page;
        }
        return range_begin;
    }

    void* ::SoC::heap::allocate_pages(::std::size_t page_cnt) noexcept(::SoC::optional_noexcept)
    {
        // 分配一个页的快速路径
        if(page_cnt == 1) [[likely]]
        {
            // 如果空闲页链表非空则已经通过快速路径完成分配
            // 因此进入慢速路径一定需要调用page_gc
            auto* free_page{page_gc(true)};
            free_page_list.back() = free_page->next_page;
            free_page->used_block = 1;
            // 和快速路径保持一致，将空闲块指针设置为空
            return ::std::exchange(free_page->free_block_list, nullptr);
        }
        else
        {
            page_gc(false);

            for(auto metadata_ptr{metadata.begin()}, metadata_end{metadata.end()}; metadata_ptr != metadata_end;)
            {
                if(metadata_ptr->used_block == 0)
                {
                    auto continuous_page_cnt{1zu};
                    // 闭区间[range_begin, range_end]表示连续页范围
                    auto* range_begin = metadata_ptr->free_block_list;
                    auto* range_end = metadata_ptr->free_block_list;
                    // 向后搜索
                    for(auto&& [_, free_block_list, used_block, _]: ::std::ranges::subrange{metadata_ptr + 1, metadata_end})
                    {
                        if(used_block == 0)
                        {
                            range_end = free_block_list;
                            if(++continuous_page_cnt == page_cnt) { return remove_pages(range_begin, range_end); }
                        }
                        else
                        {
                            break;
                        }
                    }
                    // 连续页范围占用的字节数
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    auto delta{reinterpret_cast<::std::uintptr_t>(range_end) - reinterpret_cast<::std::uintptr_t>(range_begin)};
                    // 转化为连续页数，由于是闭区间，因此+1
                    auto advance{static_cast<::std::ptrdiff_t>(delta / page_size + 1)};
                    metadata_ptr += advance;
                }
                else
                {
                    ++metadata_ptr;
                }
            }

            if constexpr(::SoC::is_build_mode(::SoC::build_mode::fuzzer))
            {
                // fuzzer模式下下走异常路径退出，以便和正常分配进行区分
                ::SoC::fuzzer_assert(false, fuzzer_error_code::heap_full);
            }
            else
            {
                ::SoC::always_check(false, "堆中剩余连续分页数量不足"sv);
            }
            return nullptr;
        }
    }

    void ::SoC::heap::deallocate_pages(void* ptr, ::std::size_t actual_size) noexcept(::SoC::optional_noexcept)
    {
        auto page_cnt{static_cast<::std::ptrdiff_t>(actual_size / page_size)};
        if constexpr(::SoC::use_full_assert)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            auto is_aligned{reinterpret_cast<::std::uintptr_t>(ptr) % page_size == 0};
            if constexpr(::SoC::is_build_mode(::SoC::build_mode::fuzzer))
            {
                ::SoC::fuzzer_assert(is_aligned, fuzzer_error_code::pointer_unaligned);
            }
            else
            {
                ::SoC::assert(is_aligned, "释放范围首指针不满足页对齐"sv);
            }
        }
        auto metadata_index{get_metadata_index(static_cast<::SoC::detail::free_block_list_t*>(ptr))};
        auto&& head{free_page_list.back()};
        constexpr auto scaled_page_size{page_size / sizeof(::SoC::detail::free_block_list_t)};
        for(auto&& [index, metadata]:
            ::std::views::zip(::std::views::iota(metadata_index), metadata.subspan(metadata_index, page_cnt)))
        {
            auto&& [next_page, free_block_list, used_block, block_size_shift]{metadata};
            if(::SoC::use_full_assert)
            {
                if constexpr(::SoC::is_build_mode(::SoC::build_mode::fuzzer))
                {
                    ::SoC::fuzzer_assert(block_size_shift == page_shift, fuzzer_error_code::block_size_mismatch);
                }
                else
                {
                    ::SoC::assert(block_size_shift == page_shift, "释放块大小与申请块大小不匹配"sv);
                }
                ::SoC::assert(used_block == 1, "要释放的页使用计数不为1"sv);
            }
            used_block = 0;
            free_block_list = data + index * scaled_page_size;
            ::new(free_block_list)::SoC::detail::free_block_list_t{nullptr};
            next_page = ::std::exchange(head, &metadata);
        }
    }

    void* ::SoC::heap::allocate_cold_path(::std::size_t actual_size) noexcept(::SoC::optional_noexcept)
    {
        if(actual_size >= page_size) { return allocate_pages(actual_size / page_size); }
        else
        {
            auto step{actual_size / ptr_size};
            auto free_page_list_index{::std::countr_zero(actual_size) - min_block_shift};
            auto* page_begin{make_block_in_page(free_page_list_index)};
            // 刚通过make_block_in_page生成的空闲块链表是连续的
            // 不使用next指针以减少一次内存访问
            auto&& free_list{free_page_list[free_page_list_index]};
            free_list->free_block_list += step;
            ++free_list->used_block;
            return page_begin;
        }
    }

    ::std::size_t(::SoC::heap::get_free_pages)() const noexcept
    {
        ::std::size_t cnt{};
#pragma GCC unroll(4)
        for(auto&& metadata: metadata)
        {
            if(metadata.used_block == 0) { ++cnt; }
        }
        return cnt;
    }

    void* ::SoC::heap::allocate(::std::size_t size) noexcept(::SoC::optional_noexcept)
    {
        auto actual_size{get_actual_allocate_size(size)};
        auto free_page_list_index{::std::countr_zero(actual_size) - min_block_shift};
        if(actual_size <= page_size && free_page_list[free_page_list_index] != nullptr) [[likely]]
        {
            auto&& free_list{free_page_list[free_page_list_index]};
            auto&& [next_page, free_block_list, used_block, _]{*free_list};
            // 由于空页会移除空闲链表，因此free_block_list不为nullptr
            void* result{free_block_list};
            ++used_block;
            free_block_list = free_block_list->next;
            if(free_block_list == nullptr) [[unlikely]] { free_list = next_page; }
            return result;
        }
        else
        {
            return allocate_cold_path(actual_size);
        }
    }

    void ::SoC::heap::deallocate(void* ptr, ::std::size_t size) noexcept(::SoC::optional_noexcept)
    {
        auto* page_ptr{static_cast<::SoC::detail::free_block_list_t*>(ptr)};
        auto actual_size{get_actual_allocate_size(size)};
        if(actual_size > page_size) [[unlikely]]
        {
            deallocate_pages(ptr, actual_size);
            return;
        }
        auto metadata_index{get_metadata_index(page_ptr)};
        auto&& metadata_ref{metadata[metadata_index]};
        auto&& [next_page, free_block_list, used_block, block_size_shift]{metadata_ref};
        if constexpr(::SoC::use_full_assert)
        {
            // 输入正确性检查
            auto block_size{1zu << block_size_shift};
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            auto is_aligned{reinterpret_cast<::std::uintptr_t>(ptr) % block_size == 0};
            if constexpr(::SoC::is_build_mode(::SoC::build_mode::fuzzer))
            {
                ::SoC::fuzzer_assert(block_size == actual_size, fuzzer_error_code::block_size_mismatch);
                ::SoC::fuzzer_assert(is_aligned, fuzzer_error_code::pointer_unaligned);
            }
            else
            {
                ::SoC::assert(block_size == actual_size, "释放块大小与申请块大小不匹配"sv);
                ::SoC::assert(is_aligned, "释放页指针不满足块对齐"sv);
            }

            // 堆结构完整性检查
            auto max_block_num{1zu << (page_shift - block_size_shift)};
            ::SoC::assert(used_block >= 1 && used_block <= max_block_num,
                          "要释放的块所在页使用计数不在[1, max_block_num]范围内"sv);
            ::SoC::assert(used_block != max_block_num || free_block_list == nullptr,
                          "要释放的块所在页已完全分配，但其空闲块链表不为空"sv);
            ::SoC::assert(used_block == max_block_num || free_block_list != nullptr,
                          "要释放的块所在页未完全分配，但其空闲块链表为空"sv);
        }
        auto* old_head{::std::exchange(free_block_list, page_ptr)};
        ::new(page_ptr)::SoC::detail::free_block_list_t{old_head};
        --used_block;
        if(old_head == nullptr) [[unlikely]]
        {
            // 原先页是满的，不在空闲链表里，现在将其插入链表
            auto free_page_list_index{block_size_shift - min_block_shift};
            next_page = ::std::exchange(free_page_list[free_page_list_index], &metadata_ref);
        }
    }
}  // namespace SoC
