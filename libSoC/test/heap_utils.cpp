import "test_framework.hpp";
import SoC.unit_test.heap;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("heap_utils/" NAME)

/// @test SoC::heap实用函数单元测试
TEST_SUITE("heap_utils" * ::doctest::description{"SoC::heap实用函数单元测试"})
{
    /// @test 测试堆的构造函数能否检出输入内存范围错误
    REGISTER_TEST_CASE("invalid initialize" * ::doctest::description{"测试堆的构造函数能否检出输入内存范围错误"})
    {
        auto [begin, end]{::SoC::unit_test::heap::test_fixture::get_memory()};

        /// 堆结束地址未对齐到页大小
        SUBCASE("unaligned heap end")
        {
            REQUIRE_THROWS_WITH_AS_MESSAGE((::SoC::test::heap{begin, end + 1}),
                                           ::doctest::Contains{"堆结束地址必须对齐到页边界"},
                                           ::SoC::assert_failed_exception,
                                           "堆结束地址未对齐到页大小的情况下应触发断言失败");
        }

        /// 堆大小不足一页
        SUBCASE("heap too small")
        {
            REQUIRE_THROWS_WITH_AS_MESSAGE((::SoC::test::heap{begin, begin}),
                                           ::doctest::Contains{"堆大小必须大于一页"},
                                           ::SoC::assert_failed_exception,
                                           "堆大小不足一页的情况下应触发断言失败");
        }
    }

    /// @test 测试堆的构造函数能否正常工作
    REGISTER_TEST_CASE("initialize" * ::doctest::description{"测试堆的构造函数能否正常工作"})
    {
        auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};

        auto page_num{heap.metadata.size()};

        /// 测试页数量是否正确
        SUBCASE("page num")
        {
            constexpr auto size_per_page{::SoC::test::heap::page_size + sizeof(::SoC::unit_test::heap::metadata_t)};
            CHECK_EQ(page_num, ::SoC::unit_test::heap::heap_size / size_per_page);
        }

        /// 测试metadata初始化是否正确
        SUBCASE("metadata")
        {
            void* page_begin{heap.data};
            auto space_left{::SoC::unit_test::heap::heap_size - page_num * sizeof(::SoC::unit_test::heap::metadata_t)};
            REQUIRE_NE(::std::align(heap.page_size, page_num * heap.page_size, page_begin, space_left), nullptr);
            auto current_page_address{::std::bit_cast<::std::uintptr_t>(page_begin)};
            for(auto&& metadata: heap.metadata)
            {
                // 除了最后一页，其他页的next_page都指向后一页的metadata
                auto* next_page{&metadata != &heap.metadata.back() ? &metadata + 1 : nullptr};
                CHECK_EQ(metadata.next_page, next_page);
                // 每一页的free_block_list都指向当前页的首地址
                CHECK_EQ(metadata.free_block_list,
                         ::std::bit_cast<::SoC::unit_test::heap::free_block_list_t*>(current_page_address));
                // 页为空，因此每一页的used_block都为0
                CHECK_EQ(metadata.used_block, 0);
                current_page_address += heap.page_size;
                // 块大小为页大小
                CHECK_EQ(metadata.block_size_shift, heap.page_shift);
            }
        }

        /// 测试空闲页链表初始化是否正确
        SUBCASE("free_page_list") { CHECK_EQ(heap.free_page_list.back(), heap.metadata.data()); }

        /// 显式使用析构函数以便在生成覆盖率报告时检测到析构函数的调用
        SUBCASE("destructor")
        {
            ::SoC::unit_test::heap::test_fixture::get_heap();
        }
    }

    /// @test 测试堆的获取实际分配大小函数能否正常工作
    REGISTER_TEST_CASE("get_actual_allocate_size" * ::doctest::description{"测试堆的获取实际分配大小函数能否正常工作"})
    {
        // 该函数
        auto&& fun{::SoC::heap::get_actual_allocate_size};
        constexpr auto min_block_size{::SoC::heap::min_block_size};
        constexpr auto page_size{::SoC::heap::page_size};

        SUBCASE("less than min block size")
        {
            CHECK_EQ(fun(0), min_block_size);
            CHECK_EQ(fun(1), min_block_size);
            CHECK_EQ(fun(min_block_size - 1), min_block_size);
        }

        SUBCASE("equal min block size") { CHECK_EQ(fun(min_block_size), min_block_size); }

        SUBCASE("greater than min block size")
        {
            CHECK_EQ(fun(min_block_size + 1), min_block_size * 2);
            CHECK_EQ(fun(min_block_size * 2), min_block_size * 2);
            CHECK_EQ(fun(min_block_size * 2 + 1), min_block_size * 4);
            CHECK_EQ(fun(min_block_size * 3), min_block_size * 4);
        }

        SUBCASE("allocate pages")
        {
            CHECK_EQ(fun(page_size - 1), page_size);
            CHECK_EQ(fun(page_size), page_size);
            CHECK_EQ(fun(page_size + 1), page_size * 2);
            CHECK_EQ(fun(page_size * 2 + 1), page_size * 3);
        }
    }

    /// @test 测试堆的页状态统计系列函数能否正常工作
    REGISTER_TEST_CASE("page status counter" * ::doctest::description{"测试堆的页状态统计系列函数能否正常工作"})
    {
        auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};
        auto total_page_cnt_gt{heap.metadata.size()};

        SUBCASE("get_total_pages") { CHECK_EQ(heap.get_total_pages(), total_page_cnt_gt); }

        constexpr auto using_page_cnt_gt{10zu};
        ::std::uniform_int_distribution<::std::ptrdiff_t> page_index_range{0,
                                                                           static_cast<::std::ptrdiff_t>(total_page_cnt_gt - 1)};
        auto seed{::doctest::getContextOptions()->rand_seed};
        CAPTURE(seed);
        ::std::default_random_engine random_engine{seed};
        for(auto _: ::std::views::iota(0zu, using_page_cnt_gt)) { ++heap.metadata[page_index_range(random_engine)].used_block; }

        SUBCASE("get_using_pages") { CHECK_EQ(heap.get_using_pages(), using_page_cnt_gt); }

        SUBCASE("get_free_pages") { CHECK_EQ(heap.get_free_pages(), total_page_cnt_gt - using_page_cnt_gt); }
    }

    /// @test 测试堆的获取元数据索引函数能否正常工作
    REGISTER_TEST_CASE("get_metadata_index" * ::doctest::description{"测试堆的获取元数据索引函数能否正常工作"})
    {
        auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};

        SUBCASE("valid block pointer in page")
        {
            auto total_page_cnt_gt{static_cast<::std::ptrdiff_t>(heap.metadata.size())};
            auto do_check{
                [&heap](::std::ptrdiff_t page_index_gt)
                {
                    constexpr auto block_size{64zu};
                    for(auto base_address{::std::bit_cast<::std::uintptr_t>(heap.metadata[page_index_gt].free_block_list)},
                        offset{0zu};
                        offset < ::SoC::heap::page_size;
                        offset += block_size)
                    {
                        ::std::ptrdiff_t page_index{};
                        REQUIRE_NOTHROW_MESSAGE(
                            page_index = heap.get_metadata_index(
                                ::std::bit_cast<::SoC::unit_test::heap::free_block_list_t*>(base_address + offset)),
                            "块指针合法，不应该断言失败");
                        CHECK_EQ(page_index, page_index_gt);
                    }
                }};

            SUBCASE("first page") { do_check(0); }

            SUBCASE("last page") { do_check(total_page_cnt_gt - 1); }

            SUBCASE("random page index")
            {
                ::std::uniform_int_distribution<::std::ptrdiff_t> page_index_range{0, total_page_cnt_gt - 1};
                auto seed{::doctest::getContextOptions()->rand_seed};
                CAPTURE(seed);
                ::std::default_random_engine random_engine{seed};
                auto page_index{page_index_range(random_engine)};
                CAPTURE(page_index);
                do_check(page_index);
            }
        }

        SUBCASE("invalid block pointer")
        {
            const ::doctest::Contains exception_string{"页指针超出当前堆范围"};
            CHECK_THROWS_WITH_AS_MESSAGE(heap.get_metadata_index(heap.data - 1),
                                         exception_string,
                                         ::SoC::assert_failed_exception,
                                         "块指针小于堆地址下限，应该断言失败");

            CHECK_THROWS_WITH_AS_MESSAGE(
                heap.get_metadata_index(heap.data + heap.metadata.size() * heap.page_size / heap.ptr_size),
                exception_string,
                ::SoC::assert_failed_exception,
                "块指针大于堆地址上限，应该断言失败");
        }
    }

    /**
     * @brief 为测试堆的插入块函数准备堆
     *
     * @param heap 堆对象
     * @return std::array<metadata_t*, 2> 空闲页链表的第一页、第二页
     */
    ::std::array<::SoC::unit_test::heap::metadata_t*, 2> make_heap_for_insert_block_into_page_list_test(::SoC::test::heap & heap)
    {
        auto&& free_page_list{heap.free_page_list.back()};
        auto* first_page{free_page_list};
        auto* second_page{first_page->next_page};
        // 将空闲页链表的头指针指向第三页，将第一页插入空闲块链表
        // 即将空闲页链表的前两页放入空闲块链表
        heap.free_page_list.front() = ::std::exchange(free_page_list, second_page->next_page);
        // 将第二页设为空闲块链表的尾节点
        second_page->next_page = nullptr;

        // 设置块大小
        first_page->block_size_shift = heap.min_block_shift;
        second_page->block_size_shift = heap.min_block_shift;
        return {first_page, second_page};
    }

    /**
     * @brief 测试堆的插入块函数能否正常工作
     *
     * @param heap 堆对象
     * @param page_ptr 要插入的页的元数据指针
     */
    void do_insert_block_into_page_list_test(::SoC::test::heap & heap, ::SoC::unit_test::heap::metadata_t * page_ptr)
    {
        auto&& free_page_list{heap.free_page_list.back()};
        // 记录空闲页链表的头指针的期望值
        auto* free_page_list_head_gt{free_page_list};
        // 记录下一个页的元数据指针的期望值
        auto* next_page_gt{page_ptr->next_page};
        auto* next_page{heap.insert_block_into_page_list(page_ptr)};

        // 检查返回值是否正确，即下一个页的元数据指针
        CHECK_EQ(next_page, next_page_gt);
        // 检查页是否插入空闲页链表的头部
        CHECK_EQ(page_ptr, free_page_list);
        // 检查新插入的页的next_page是否指向了原空闲页链表的头指针，即是否成功穿成链表
        CHECK_EQ(page_ptr->next_page, free_page_list_head_gt);
        auto metadata_index{::std::distance(heap.metadata.data(), page_ptr)};
        auto data_address{::std::bit_cast<::std::uintptr_t>(heap.data) + metadata_index * heap.page_size};
        // 检查页的空闲块指针是否指向数据块首地址，即完成对于页操作的初始化
        CHECK_EQ(page_ptr->free_block_list, ::std::bit_cast<::SoC::unit_test::heap::free_block_list_t*>(data_address));
        // 检查块大小是否正确设置为页大小
        CHECK_EQ(page_ptr->block_size_shift, heap.page_shift);
    }

    /// @test 测试堆的插入块函数能否在链表元素数大于1时正常工作
    REGISTER_TEST_CASE("insert_block_into_page_list" *
                       ::doctest::description{"测试堆的插入块函数能否在链表元素数大于1时正常工作"})
    {
        SUBCASE("first element")
        {
            auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};
            auto [page_ptr, _]{make_heap_for_insert_block_into_page_list_test(heap)};
            do_insert_block_into_page_list_test(heap, page_ptr);
        }

        SUBCASE("last element")
        {
            auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};
            auto [_, page_ptr]{make_heap_for_insert_block_into_page_list_test(heap)};
            do_insert_block_into_page_list_test(heap, page_ptr);
        }
    }

    /// @test 测试从空闲页链表中移除范围内的页
    REGISTER_TEST_CASE("remove_pages" * ::doctest::description{"测试从空闲页链表中移除范围内的页"})
    {
        auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};
        auto&& free_page_list{heap.free_page_list.back()};
        auto* first_page{free_page_list};
        auto* second_page{first_page->next_page};
        auto* third_page{second_page->next_page};
        auto* fourth_page{third_page->next_page};

        SUBCASE("remove pages with free_page_list head")
        {
            // 检查返回值是否为范围内的首个页
            CHECK_EQ(heap.remove_pages(first_page->free_block_list, third_page->free_block_list), first_page->free_block_list);
            // 检查空闲页表头是否为第四页
            CHECK_EQ(heap.free_page_list.back(), third_page->next_page);
            // 检查范围内的页used_blocks是否为1
            CHECK_EQ(first_page->used_block, 1);
            CHECK_EQ(second_page->used_block, 1);
            CHECK_EQ(third_page->used_block, 1);
            CHECK_EQ(fourth_page->used_block, 0);
        }

        SUBCASE("remove pages without free_page_list head")
        {
            auto do_check{[&]
                          {
                              // 检查返回值是否为范围内的首个页
                              CHECK_EQ(heap.remove_pages(second_page->free_block_list, third_page->free_block_list),
                                       second_page->free_block_list);
                              // 检查空闲页表头是否为第一页
                              CHECK_EQ(heap.free_page_list.back(), first_page);
                              // 检查范围内的页used_blocks是否为1，不在移除范围内的页used_blocks是否为0
                              CHECK_EQ(first_page->used_block, 0);
                              CHECK_EQ(second_page->used_block, 1);
                              CHECK_EQ(third_page->used_block, 1);
                              CHECK_EQ(fourth_page->used_block, 0);
                          }};

            SUBCASE("in order") { do_check(); }

            SUBCASE("out of order")
            {
                // 交换first_page和fourth_page以测试链表穿插的情况
                first_page->next_page = fourth_page->next_page;
                fourth_page->next_page = second_page;
                third_page->next_page = first_page;
                ::std::ranges::swap(first_page, fourth_page);
                free_page_list = first_page;

                do_check();
            }
        }
    }
}
