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
        for(auto& page: metadata)
        {
            page.next_page = &page + 1;
            page.free_block_list = reinterpret_cast<::SoC::detail::free_block_list_t*>(ptr);
            page.used_block = 0;
            ptr += page_size;
        }
        metadata.back().next_page = nullptr;

        free_page_list[::std::to_underlying(page)] = metadata.begin();
    }

    ::std::size_t(::SoC::heap::get_metadata_index)(::SoC::detail::free_block_list_t* page_ptr) const noexcept
    {
        return (page_ptr - data) * ptr_size / page_size;
    }

    ::SoC::detail::free_block_list_t* ::SoC::heap::make_block_in_page(block_size_enum block_size) noexcept
    {
        auto&& free_page{free_page_list[::std::to_underlying(page)]};
        if(free_page == nullptr) [[unlikely]]
        {
            free_page = page_gc();
            if constexpr(::SoC::use_full_assert) { ::SoC::assert(free_page != nullptr, "剩余堆空间不足"sv); }
            else
            {
                if(free_page == nullptr) [[unlikely]] { ::SoC::fast_fail(); }
            }
        }
        // 空闲页基址
        auto page_begin{free_page->free_block_list};
        // 从空闲页表里取出一页
        free_page = free_page->next_page;

        auto heap_block_size{::SoC::detail::get_heap_block_size(block_size)};
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

        auto&& block_metadata_ptr{free_page_list[::std::to_underlying(block_size)]};
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
        auto old_page_head{::std::exchange(free_page_list[::std::to_underlying(page)], old_head)};
        old_head->next_page = old_page_head;
        return page_metadata;
    }

    ::SoC::detail::heap_page_metadata* ::SoC::heap::page_gc() noexcept
    {
#pragma GCC unroll(0)
        for(auto i{0zu}; i != 4; ++i)
        {
            auto&& block_list{free_page_list[i]};
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
        return free_page_list[::std::to_underlying(page)];
    }

    void* ::SoC::heap::allocate_slow(::std::size_t actual_size, ::std::size_t free_page_list_index) noexcept
    {
        auto step{actual_size / ptr_size};
        auto&& free_list{free_page_list[free_page_list_index]};
        if(actual_size != page_size) [[likely]]
        {
            auto page_begin{make_block_in_page(static_cast<block_size_enum>(free_page_list_index))};
            free_list->free_block_list = free_list->free_block_list + step;
            ++free_list->used_block;
            return page_begin;
        }
        else
        {
            auto free_page{page_gc()};
            if constexpr(::SoC::use_full_assert) { ::SoC::assert(free_page != nullptr, "剩余堆空间不足"sv); }
            else
            {
                if(free_page == nullptr) [[unlikely]] { ::SoC::fast_fail(); }
            }
            auto&& [next_page, free_list, used_block]{*free_page};
            ++used_block;
            auto result = free_list;
            next_page = next_page->next_page;
            return result;
        }
    }

    void* ::SoC::heap::allocate_pages(::std::size_t actual_size [[maybe_unused]]) noexcept
    {
        ::SoC::always_assert(false, "尚未实现多页分配"sv);
        return nullptr;
    }

    void ::SoC::heap::deallocate_pages(void* ptr [[maybe_unused]], ::std::size_t actual_size [[maybe_unused]]) noexcept
    {
        ::SoC::always_assert(false, "尚未实现多页释放"sv);
    }

    void* ::SoC::heap::allocate(::std::size_t size) noexcept
    {
        auto actual_size{get_actual_allocate_size(size)};
        auto free_page_list_index{::std::countr_zero(actual_size) - 5};
        auto&& free_list{free_page_list[free_page_list_index]};
        if(actual_size >= page_size) [[unlikely]] { return allocate_pages(actual_size); }
        else if(free_list == nullptr) [[unlikely]] { return allocate_slow(actual_size, free_page_list_index); }
        else
        {
            auto&& [next_page, free_block_list, used_block] = *free_list;
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
        if(actual_size >= page_size) [[unlikely]] { deallocate_pages(ptr, actual_size); }
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
            auto& free_list{free_page_list[free_page_list_index]};
            auto old_head{::std::exchange(free_list, &metadata_ref)};
            next_page = old_head;
        }
    }

    extern "C"
    {
        extern ::std::uintptr_t _user_heap_start[];
        extern ::std::uintptr_t _user_heap_end[];
        extern ::std::uintptr_t _ccmram_heap_start[];
        extern ::std::uintptr_t _ccmram_heap_end[];
    }

    ::SoC::heap make_ram_heap() noexcept { return ::SoC::heap{::SoC::_user_heap_start, ::SoC::_user_heap_end}; }

    ::SoC::heap make_ccmram_heap() noexcept { return ::SoC::heap{::SoC::_ccmram_heap_start, ::SoC::_ccmram_heap_end}; }
}  // namespace SoC
