#pragma once
#include "allocator.hpp"

namespace SoC
{
    namespace detail
    {
        /**
         * @brief 堆块大小类型
         *
         */
        enum class heap_block_size_type : ::std::uint8_t
        {
            byte32,
            byte64,
            byte128,
            byte256,
            byte512,
            page = byte512
        };

        /**
         * @brief 从堆块大小枚举获取堆块大小
         *
         * @param type 堆块大小枚举
         * @return 堆块大小
         */
        constexpr inline ::std::size_t get_heap_block_size(::SoC::detail::heap_block_size_type type) noexcept
        {
            return 1zu << (::std::to_underlying(type) + 5);
        }

        /**
         * @brief 空闲块链表
         *
         */
        struct free_block_list_t
        {
            free_block_list_t* next;
        };

        /**
         * @brief 堆页元数据
         *
         */
        struct heap_page_metadata
        {
            ::SoC::detail::heap_page_metadata* next_page;
            ::SoC::detail::free_block_list_t* free_block_list;
            ::std::size_t used_block;
        };
    }  // namespace detail

    /**
     * @brief 基于空闲链表和slab的堆
     *
     */
    struct heap
    {
    public:
        using block_size_enum = ::SoC::detail::heap_block_size_type;
        using enum block_size_enum;

        /// 元数据区
        ::std::ranges::subrange<::SoC::detail::heap_page_metadata*> metadata;

        using free_list_t = ::std::array<::SoC::detail::heap_page_metadata*, 5>;

        /// 空闲链表
        free_list_t free_page_list{};

        /// 数据区
        ::SoC::detail::free_block_list_t* data;

        /**
         * @brief 寻找一个空闲页并在其中划分出内存块，将其移除空闲页链表
         *
         * @param block_size 内存块大小枚举
         * @return 页起始地址
         */
        ::SoC::detail::free_block_list_t* make_block_in_page(block_size_enum block_size) noexcept;

        /**
         * @brief 将已分块的页从块空闲链表中删除，插入页空闲链表中
         *
         * @param page 页元数据指针
         * @return 下一个元数据的指针
         */
        ::SoC::detail::heap_page_metadata* insert_block_into_page_list(::SoC::detail::heap_page_metadata* page_metadata) noexcept;

        /**
         * @brief 从空闲的已分块页中回收页到空闲页链表
         *
         */
        [[using gnu: noinline, cold]] ::SoC::detail::heap_page_metadata* page_gc() noexcept;

        /**
         * @brief 获取页内指针所在页对应的元数据数组索引
         *
         * @param page_ptr 页内指针
         * @return 元数据数组索引
         */
        [[using gnu: always_inline, hot]] ::std::size_t
            get_metadata_index(::SoC::detail::free_block_list_t* page_ptr) const noexcept;

        /**
         * @brief 分配指定大小的块，在块空闲链表耗尽时的慢速路径
         *
         * @param size 实际块大小
         * @param free_page_list_index 空闲块链表索引
         * @return void* 块起始地址
         */
        [[using gnu: noinline, cold]] void* allocate_slow(::std::size_t actual_size, ::std::size_t free_page_list_index) noexcept;

        /**
         * @brief 分配多个页，慢速路径
         *
         * @param actual_size 实际分配大小
         */
        [[using gnu: noinline, cold]] void* allocate_pages(::std::size_t actual_size) noexcept;

        /**
         * @brief 释放多个页，慢速路径
         *
         * @param ptr 块指针
         * @param actual_size 实际分配大小
         */
        [[using gnu: noinline, cold]] void free_pages(void* ptr, ::std::size_t actual_size) noexcept;

        /// 指针大小
        constexpr inline static auto ptr_size{sizeof(void*)};

    public:
        /// 堆页大小
        constexpr inline static auto page_size{512zu};

        /**
         * @brief 初始化堆
         *
         * @param begin 堆起始地址
         * @param end 堆结束地址
         * @note end必须对齐到页边界
         */
        explicit heap(::std::uintptr_t* begin, ::std::uintptr_t* end) noexcept;

        inline ~heap() noexcept = default;

        inline heap(const heap&) noexcept = delete;
        inline heap& operator= (const heap&) = delete;
        inline heap(heap&&) noexcept = delete;
        inline heap& operator= (heap&&) noexcept = delete;

        /**
         * @brief 获取实际分配的大小
         *
         * @param size 需要分配的大小
         * @return 实际分配的大小
         */
        [[using gnu: always_inline, artificial, hot]] constexpr inline static ::std::size_t
            get_actual_allocate_size(::std::size_t size) noexcept
        {
            return ::std::bit_ceil(size);
        }

        /**
         * @brief 分配指定大小的块
         *
         * @param size 块大小
         * @return void* 块起始地址
         */
        [[gnu::noinline]] void* allocate(::std::size_t size) noexcept;

        /**
         * @brief 释放指定块
         *
         * @param ptr 块起始地址
         * @param size 块大小
         */
        [[gnu::noinline]] void free(void* ptr, ::std::size_t size) noexcept;
    };

    /**
     * @brief 从主内存创建堆
     *
     * @return 用户堆
     */
    ::SoC::heap make_user_heap() noexcept;

    /**
     * @brief 从ccmram创建堆
     *
     * @return ccmram堆
     */
    ::SoC::heap make_ccmram_heap() noexcept;
}  // namespace SoC
