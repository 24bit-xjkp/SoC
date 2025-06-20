#include "../include/heap.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::heap::heap(::std::uintptr_t* begin, ::std::uintptr_t* end) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(reinterpret_cast<::std::uintptr_t>(end) % page_size == 0, "堆结束地址必须对齐到页边界"sv);
        }
        auto bytes{(end - begin) * ptr_size};
        auto pages{bytes / (page_size + sizeof(::SoC::detail::heap_page_metadata))};
        auto metadata_begin{reinterpret_cast<::SoC::detail::heap_page_metadata*>(begin)};
        auto metadata_end{metadata_begin + pages};
        metadata = ::std::ranges::subrange{metadata_begin, metadata_end};

        auto ptr{reinterpret_cast<::std::uintptr_t>(metadata_end)};
        constexpr auto shift{::std::countr_zero(page_size)};
        ptr = (ptr + page_size - 1) & (-1zu << shift);
        data = reinterpret_cast<::SoC::detail::free_block_list_t*>(ptr);

#pragma GCC unroll(2)
        for(auto&& page: metadata)
        {
            page = ::SoC::detail::heap_page_metadata{&page + 1, reinterpret_cast<::SoC::detail::free_block_list_t*>(ptr), 0};
            ptr += page_size;
        }
        metadata.back().next_page = nullptr;

        free_page_list.back() = metadata.begin();
    }

    ::SoC::detail::free_block_list_t* ::SoC::heap::make_block_in_page(::std::size_t free_list_index) noexcept
    {
        auto&& free_page{free_page_list.back()};
        if(free_page == nullptr) [[unlikely]] { free_page = page_gc(true); }
        // 空闲页基址
        auto page_begin{free_page->free_block_list};
        // 从空闲页表里取出一页
        free_page = free_page->next_page;

        auto heap_block_size{1zu << (free_list_index + min_block_shift)};
        auto page_ptr{page_begin};
        auto step{heap_block_size / ptr_size};
#pragma GCC unroll(0)
        while(page_ptr != page_begin + page_size / ptr_size)
        {
            for(auto i{0zu}; i != 4; ++i)
            {
                *page_ptr = ::SoC::detail::free_block_list_t{page_ptr + step};
                page_ptr += step;
            }
        }
        *(page_ptr - step) = ::SoC::detail::free_block_list_t{};

        auto&& block_metadata_ptr{free_page_list[free_list_index]};
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(block_metadata_ptr == nullptr, "仅在块空闲链表为空时调用此函数"sv); }
        auto metadata_index{get_metadata_index(page_begin)};
        metadata[metadata_index].next_page = nullptr;
        metadata[metadata_index].free_block_list = page_begin;
        // 从空闲链表里取出的页使用计数为0，不需要设置
        block_metadata_ptr = &metadata[metadata_index];
        return page_begin;
    }

    ::SoC::detail::heap_page_metadata* ::SoC::heap::insert_block_into_page_list(
        ::SoC::detail::heap_page_metadata* page_metadata) noexcept
    {
        auto old_head{::std::exchange(page_metadata, page_metadata->next_page)};
        old_head->free_block_list = (old_head - metadata.begin()) * page_size / ptr_size + data;
        auto old_page_head{::std::exchange(free_page_list.back(), old_head)};
        old_head->next_page = old_page_head;
        return page_metadata;
    }

    ::SoC::detail::heap_page_metadata* ::SoC::heap::page_gc(bool assert) noexcept
    {
#pragma GCC unroll(0)
        for(auto&& block_list: ::std::ranges::subrange{free_page_list.begin(), free_page_list.end() - 1})
        {
            while(block_list != nullptr && block_list->used_block == 0) { block_list = insert_block_into_page_list(block_list); }
            // 一次回收未完全完成，继续通过游标遍历链表
            if(block_list != nullptr)
            {
                auto block_list_cursor{block_list};
                while(block_list_cursor != nullptr)
                {
                    if(block_list_cursor->used_block == 0) { insert_block_into_page_list(block_list_cursor); }
                    else
                    {
                        block_list_cursor = block_list_cursor->next_page;
                    }
                }
            }
        }
        auto free_page{free_page_list.back()};
        if(assert)
        {
            if constexpr(::SoC::use_full_assert) { ::SoC::assert(free_page != nullptr, "剩余堆空间不足"sv); }
            else
            {
                if(free_page == nullptr) [[unlikely]] { ::SoC::fast_fail(); }
            }
        }
        return free_page;
    }

    void* ::SoC::heap::remove_pages(::SoC::detail::free_block_list_t* range_begin,
                                    ::SoC::detail::free_block_list_t* range_end) noexcept
    {
        auto&& head{free_page_list.back()};
        // 移除除了head外所有范围内的页
        for(auto ptr{head}; ptr->next_page != nullptr;)
        {
            auto&& next_page{ptr->next_page};
            if(auto page_ptr{next_page->free_block_list}; page_ptr >= range_begin && page_ptr <= range_end)
            {
                next_page->used_block = 1;
                next_page = next_page->next_page;
            }
            else
            {
                ptr = ptr->next_page;
            }
        }
        // 若head也在范围内则删除
        if(auto page_ptr{head->free_block_list}; page_ptr >= range_begin && page_ptr <= range_end)
        {
            head->used_block = 1;
            head = head->next_page;
        }
        return range_begin;
    }

    void* ::SoC::heap::allocate_pages(::std::size_t page_cnt) noexcept
    {
        // 分配一个页的快速路径
        if(page_cnt == 1) [[likely]]
        {
            auto free_page{free_page_list.back()};
            if(free_page == nullptr) [[unlikely]] { free_page = page_gc(true); }
            free_page_list.back() = free_page->next_page;
            free_page->used_block = 1;
            return free_page->free_block_list;
        }
        else
        {
            page_gc(false);

            for(auto metadata_ptr{metadata.begin()}, metadata_end{metadata.end()}; metadata_ptr != metadata_end;)
            {
                if(metadata_ptr->used_block == 0)
                {
                    ::std::size_t continuous_page_cnt{1};
                    // 闭区间[range_begin, range_end]表示连续页范围
                    auto range_begin = metadata_ptr->free_block_list;
                    auto range_end = metadata_ptr->free_block_list;
                    // 向后搜索
                    for(auto&& [_, free_block_list, used_block]: ::std::ranges::subrange{metadata_ptr + 1, metadata.end()})
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
                    auto delta{reinterpret_cast<::std::uintptr_t>(range_end) - reinterpret_cast<::std::uintptr_t>(range_begin)};
                    // 转化为连续页数，由于是闭区间，因此+1
                    auto advance{delta / page_size + 1};
                    metadata_ptr += advance;
                }
                else
                {
                    ++metadata_ptr;
                }
            }

            if constexpr(::SoC::use_full_assert) { ::SoC::assert(false, "堆中剩余连续分页数量不足"sv); }
            else
            {
                ::SoC::fast_fail();
            }
            return nullptr;
        }
    }

    void ::SoC::heap::deallocate_pages(void* ptr, ::std::size_t size) noexcept
    {
        auto page_cnt{(size + page_size - 1) / page_size};
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(reinterpret_cast<::std::uintptr_t>(ptr) % page_size == 0, "释放范围首指针不满足页对齐"sv);
        }
        auto metadata_index{get_metadata_index(reinterpret_cast<::SoC::detail::free_block_list_t*>(ptr))};
        auto page_ptr{reinterpret_cast<::std::uintptr_t>(ptr)};
        auto&& head{free_page_list.back()};
        for(auto&& metadata: ::std::ranges::subrange{&metadata[metadata_index], &metadata[metadata_index + page_cnt]})
        {
            auto&& [next_page, free_block_list, used_block]{metadata};
            used_block = 0;
            free_block_list = reinterpret_cast<::SoC::detail::free_block_list_t*>(page_ptr);
            page_ptr += page_size;
            next_page = ::std::exchange(head, &metadata);
        }
    }

    void* ::SoC::heap::allocate_cold_path(::std::size_t size) noexcept
    {
        auto actual_size{get_actual_allocate_size(size)};
        auto free_page_list_index{::std::countr_zero(actual_size) - min_block_shift};
        if(actual_size >= page_size) { return allocate_pages((size + page_size - 1) / page_size); }
        else
        {
            auto step{actual_size / ptr_size};
            auto&& free_list{free_page_list[free_page_list_index]};
            auto page_begin{make_block_in_page(free_page_list_index)};
            free_list->free_block_list = free_list->free_block_list + step;
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

    void* ::SoC::heap::allocate(::std::size_t size) noexcept
    {
        auto actual_size{get_actual_allocate_size(size)};
        auto free_page_list_index{::std::countr_zero(actual_size) - min_block_shift};
        if(actual_size < page_size && free_page_list[free_page_list_index] != nullptr) [[likely]]
        {
            auto&& free_list{free_page_list[free_page_list_index]};
            auto&& [next_page, free_block_list, used_block]{*free_list};
            // 由于空页会移除空闲链表，因此free_block_list不为nullptr
            void* result{free_block_list};
            ++used_block;
            free_block_list = free_block_list->next;
            if(free_block_list == nullptr) [[unlikely]]
            {
                if(next_page == nullptr) [[unlikely]]
                {
                    // 所有块全部耗尽，将块的空闲链表设置为空
                    free_list = nullptr;
                }
                else
                {
                    // 进入下一页进行分配
                    free_block_list = next_page->free_block_list;
                    next_page = next_page->next_page;
                }
            }
            return result;
        }
        else
        {
            return allocate_cold_path(size);
        }
    }

    void ::SoC::heap::deallocate(void* ptr, ::std::size_t size) noexcept
    {
        auto page_ptr{reinterpret_cast<::SoC::detail::free_block_list_t*>(ptr)};
        auto actual_size{get_actual_allocate_size(size)};
        auto size_align{::std::countr_zero(actual_size)};
        if constexpr(::SoC::use_full_assert)
        {
            auto ptr_align{::std::countr_zero(reinterpret_cast<::std::uintptr_t>(page_ptr))};
            ::SoC::assert(ptr_align >= size_align, "非法块指针"sv);
        }
        if(actual_size >= page_size) [[unlikely]] { return deallocate_pages(ptr, size); }
        auto metadata_index{get_metadata_index(page_ptr)};
        auto&& metadata_ref{metadata[metadata_index]};
        auto&& [next_page, free_block_list, used_block]{metadata_ref};
        auto old_head{::std::exchange(free_block_list, page_ptr)};
        *page_ptr = ::SoC::detail::free_block_list_t{old_head};
        --used_block;
        if(old_head == nullptr) [[unlikely]]
        {
            // 原先页是满的，不在空闲链表里，现在将其插入链表
            auto free_page_list_index{size_align - 5};
            next_page = ::std::exchange(free_page_list[free_page_list_index], &metadata_ref);
        }
    }

    extern "C"
    {
        /// 主内存堆起始地址
        extern ::std::uintptr_t _user_heap_start[];
        /// 主内存堆终止地址
        extern ::std::uintptr_t _user_heap_end[];
        /// ccmram堆起始地址
        extern ::std::uintptr_t _ccmram_heap_start[];
        /// ccmram堆终止地址
        extern ::std::uintptr_t _ccmram_heap_end[];
    }

    ::SoC::heap make_ram_heap() noexcept { return ::SoC::heap{::SoC::_user_heap_start, ::SoC::_user_heap_end}; }

    ::SoC::heap make_ccmram_heap() noexcept { return ::SoC::heap{::SoC::_ccmram_heap_start, ::SoC::_ccmram_heap_end}; }
}  // namespace SoC
