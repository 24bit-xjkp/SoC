#include "doctest_pch.hpp"
import SoC.unit_test;

namespace SoC::test
{
    /**
     * @brief 导出SoC::heap中的符号用于测试
     *
     */
    struct heap : ::SoC::heap
    {
        using ::SoC::heap::allocate_cold_path;
        using ::SoC::heap::allocate_pages;
        using ::SoC::heap::data;
        using ::SoC::heap::deallocate_pages;
        using ::SoC::heap::free_list_t;
        using ::SoC::heap::free_page_list;
        using ::SoC::heap::get_metadata_index;
        using ::SoC::heap::heap;
        using ::SoC::heap::insert_block_into_page_list;
        using ::SoC::heap::make_block_in_page;
        using ::SoC::heap::metadata;
        using ::SoC::heap::min_block_shift;
        using ::SoC::heap::page_gc;
        using ::SoC::heap::page_shift;
        using ::SoC::heap::ptr_size;
        using ::SoC::heap::remove_pages;
    };
}  // namespace SoC::test

TEST_SUITE("heap")
{
    namespace
    {
        /// 测试使用堆空间的大小
        constexpr auto heap_size{128 * 1024zu};

        /**
         * @brief 堆测试用例的夹具，用于提供共用内存
         *
         */
        struct heap_test_fixture
        {
        private:
            /// malloc得到的原始指针，用于在析构时进行free操作
            void* origin_ptr{};
            /// 内存区域[begin, end)，已对齐到页大小
            ::std::uintptr_t* begin{};
            ::std::uintptr_t* end{};

            /**
             * @brief 分配共用内存，避免测试用例间重复分配
             *
             */
            void allocate_once()
            {
                if(begin == nullptr) [[unlikely]]
                {
                    auto size{256 * 1024zu};
                    auto ptr{::std::malloc(size)};
                    REQUIRE_NE(ptr, nullptr);
                    origin_ptr = ptr;
                    REQUIRE_NE(::std::align(::SoC::heap::page_size, heap_size, ptr, size), nullptr);
                    begin = static_cast<::std::uintptr_t*>(ptr);
                    end = begin + (heap_size / sizeof(::std::uintptr_t));
                }
            }

        public:
            /**
             * @brief 释放共用内存
             *
             */
            ~heap_test_fixture() noexcept { ::std::free(origin_ptr); }

            /**
             * @brief 获取内存区域
             *
             * @return ::std::pair<::std::uintptr_t*, ::std::uintptr_t*> 内存区域[begin, end)
             */
            ::std::pair<::std::uintptr_t*, ::std::uintptr_t*> get_memory()
            {
                allocate_once();
                return ::std::pair{begin, end};
            }

            /**
             * @brief 获取堆对象
             *
             * @return SoC::test::heap
             */
            ::SoC::test::heap get_heap()
            {
                allocate_once();
                return ::SoC::test::heap{begin, end};
            }
        }
        /// 测试夹具，用于提供共用内存
        constinit heap_fixture{};
    }  // namespace

    /// @test 测试堆的构造函数能否检出输入内存范围错误
    TEST_CASE("invalid_initialize")
    {
        auto [begin, end]{heap_fixture.get_memory()};

        /// 堆结束地址未对齐到页大小
        SUBCASE("unaligned_heap_end")
        {
            REQUIRE_THROWS_AS_MESSAGE((::SoC::test::heap{begin, end + 1}),
                                      ::SoC::assert_failed_exception,
                                      "堆结束地址未对齐到页大小的情况下应触发断言失败");
        }

        /// 堆大小不足一页
        SUBCASE("heap_too_small")
        {
            REQUIRE_THROWS_AS_MESSAGE((::SoC::test::heap{begin, begin + 1}),
                                      ::SoC::assert_failed_exception,
                                      "堆大小不足一页的情况下应触发断言失败");
        }
    }

    using metadata_t = ::std::remove_reference_t<decltype(::SoC::test::heap::metadata.front())>;
    using free_block_list_t = ::std::remove_pointer_t<decltype(metadata_t::free_block_list)>;

    /// @test 测试堆的构造函数能否正常工作
    TEST_CASE("initialize")
    {
        auto heap{heap_fixture.get_heap()};

        auto page_num{heap.metadata.size()};

        /// 测试页数量是否正确
        SUBCASE("page_num")
        {
            constexpr auto size_per_page{::SoC::test::heap::page_size + sizeof(metadata_t)};
            REQUIRE_EQ(page_num, heap_size / size_per_page);
        }

        /// 测试metadata初始化是否正确
        SUBCASE("metadata")
        {
            void* page_begin{heap.metadata.end()};
            auto space_left{heap_size - page_num * sizeof(metadata_t)};
            REQUIRE_NE(::std::align(heap.page_size, page_num * heap.page_size, page_begin, space_left), nullptr);
            auto current_page_address{reinterpret_cast<::std::uintptr_t>(page_begin)};
            for(auto&& metadata: heap.metadata)
            {
                // 除了最后一页，其他页的next_page都指向后一页的metadata
                auto next_page{&metadata != &heap.metadata.back() ? &metadata + 1 : nullptr};
                REQUIRE_EQ(metadata.next_page, next_page);
                // 每一页的free_block_list都指向当前页的首地址
                REQUIRE_EQ(metadata.free_block_list, reinterpret_cast<free_block_list_t*>(current_page_address));
                // 页为空，因此每一页的used_block都为0
                REQUIRE_EQ(metadata.used_block, 0);
                current_page_address += heap.page_size;
            }
        }

        /// 测试空闲页链表初始化是否正确
        SUBCASE("free_page_list") { REQUIRE_EQ(heap.free_page_list.back(), heap.metadata.begin()); }
    }

    /**
     * @brief 为测试堆的插入块函数准备堆
     *
     * @param heap 堆对象
     * @return std::array<metadata_t*, 2> 空闲页链表的第一页、第二页
     */
    ::std::array<metadata_t*, 2> make_heap_for_insert_block_into_page_list_test(::SoC::test::heap & heap)
    {
        auto&& free_page_list{heap.free_page_list.back()};
        auto first_page{free_page_list};
        auto second_page{first_page->next_page};
        // 将空闲页链表的头指针指向第三页，将第一页插入空闲块链表
        // 即将空闲页链表的前两页放入空闲块链表
        heap.free_page_list.front() = ::std::exchange(free_page_list, second_page->next_page);
        // 将第二页设为空闲块链表的尾节点
        second_page->next_page = nullptr;
        return {first_page, second_page};
    }

    /**
     * @brief 测试堆的插入块函数能否正常工作
     *
     * @param heap 堆对象
     * @param page_ptr 要插入的页的元数据指针
     */
    void do_insert_block_into_page_list_test(::SoC::test::heap & heap, metadata_t * page_ptr)
    {
        auto&& free_page_list{heap.free_page_list.back()};
        // 记录空闲页链表的头指针的期望值
        auto free_page_list_head_gt{free_page_list};
        // 记录下一个页的元数据指针的期望值
        auto next_page_gt{page_ptr->next_page};
        auto next_page{heap.insert_block_into_page_list(page_ptr)};

        // 检查返回值是否正确，即下一个页的元数据指针
        CHECK_EQ(next_page, next_page_gt);
        // 检查页是否插入空闲页链表的头部
        CHECK_EQ(page_ptr, free_page_list);
        // 检查新插入的页的next_page是否指向了原空闲页链表的头指针，即是否成功穿成链表
        CHECK_EQ(page_ptr->next_page, free_page_list_head_gt);
        auto metadata_index{::std::distance(heap.metadata.begin(), page_ptr)};
        auto data_ptr{reinterpret_cast<::std::uint8_t*>(heap.data) + metadata_index * heap.page_size};
        // 检查页的空闲块指针是否指向数据块首地址，即完成对于页操作的初始化
        CHECK_EQ(page_ptr->free_block_list, reinterpret_cast<free_block_list_t*>(data_ptr));
    }

    /// @test 测试堆的插入块函数能否在链表元素数大于1时正常工作
    TEST_CASE("insert_block_into_page_list/first_element")
    {
        auto heap{heap_fixture.get_heap()};
        auto [page_ptr, _]{make_heap_for_insert_block_into_page_list_test(heap)};
        do_insert_block_into_page_list_test(heap, page_ptr);
    }

    /// @test 测试堆的插入块函数能否在链表元素数为1时正常工作
    TEST_CASE("insert_block_into_page_list/last_element")
    {
        auto heap{heap_fixture.get_heap()};
        auto [_, page_ptr]{make_heap_for_insert_block_into_page_list_test(heap)};
        do_insert_block_into_page_list_test(heap, page_ptr);
    }
}
