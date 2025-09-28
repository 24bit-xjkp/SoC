import "test_framework.hpp";
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

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("heap/" NAME)

namespace
{
    /// 测试使用堆空间的大小
    constexpr auto heap_size{128 * 1024zu};

    /**
     * @brief 堆测试用例的夹具，用于提供共用内存
     *
     */
    struct test_fixture
    {
    private:
        // NOLINTNEXTLINE(*-avoid-c-arrays)
        inline static constinit ::std::unique_ptr<::std::uintptr_t[]> origin_ptr{};
        /// 内存区域[begin, end)，已对齐到页大小
        inline static constinit ::std::uintptr_t* begin{};
        inline static constinit ::std::uintptr_t* end{};

        /**
         * @brief 分配共用内存，避免测试用例间重复分配
         *
         */
        static void allocate_once()
        {
            if(begin == nullptr) [[unlikely]]
            {
                auto size{256 * 1024zu};
                // NOLINTNEXTLINE(*-avoid-c-arrays)
                origin_ptr = ::std::make_unique<::std::uintptr_t[]>(size / sizeof(::std::uintptr_t));
                void* ptr{origin_ptr.get()};
                REQUIRE_NE(ptr, nullptr);
                REQUIRE_NE(::std::align(::SoC::heap::page_size, ::heap_size, ptr, size), nullptr);
                begin = static_cast<::std::uintptr_t*>(ptr);
                end = begin + (::heap_size / sizeof(::std::uintptr_t));
            }
        }

    public:
        /**
         * @brief 获取内存区域
         *
         * @return 内存区域[begin, end)
         */
        static ::std::pair<::std::uintptr_t*, ::std::uintptr_t*> get_memory()
        {
            allocate_once();
            return ::std::pair{begin, end};
        }

        /**
         * @brief 获取堆对象
         *
         * @return 堆对象
         */
        static ::SoC::test::heap get_heap()
        {
            allocate_once();
            return ::SoC::test::heap{begin, end};
        }
    };

    using metadata_t = ::std::remove_reference_t<decltype(::SoC::test::heap::metadata.front())>;
    using free_block_list_t = ::std::remove_pointer_t<decltype(::metadata_t::free_block_list)>;
}  // namespace

/// @test SoC::heap单元测试
TEST_SUITE("heap" * ::doctest::description{"SoC::heap单元测试"})
{

    /// @test 测试堆的构造函数能否检出输入内存范围错误
    REGISTER_TEST_CASE("invalid initialize" * ::doctest::description{"测试堆的构造函数能否检出输入内存范围错误"})
    {
        auto [begin, end]{::test_fixture::get_memory()};

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
        auto heap{::test_fixture::get_heap()};

        auto page_num{heap.metadata.size()};

        /// 测试页数量是否正确
        SUBCASE("page num")
        {
            constexpr auto size_per_page{::SoC::test::heap::page_size + sizeof(::metadata_t)};
            CHECK_EQ(page_num, heap_size / size_per_page);
        }

        /// 测试metadata初始化是否正确
        SUBCASE("metadata")
        {
            void* page_begin{heap.metadata.end()};
            auto space_left{heap_size - page_num * sizeof(::metadata_t)};
            REQUIRE_NE(::std::align(heap.page_size, page_num * heap.page_size, page_begin, space_left), nullptr);
            auto current_page_address{::std::bit_cast<::std::uintptr_t>(page_begin)};
            for(auto&& metadata: heap.metadata)
            {
                // 除了最后一页，其他页的next_page都指向后一页的metadata
                auto* next_page{&metadata != &heap.metadata.back() ? &metadata + 1 : nullptr};
                CHECK_EQ(metadata.next_page, next_page);
                // 每一页的free_block_list都指向当前页的首地址
                CHECK_EQ(metadata.free_block_list, ::std::bit_cast<::free_block_list_t*>(current_page_address));
                // 页为空，因此每一页的used_block都为0
                CHECK_EQ(metadata.used_block, 0);
                current_page_address += heap.page_size;
                // 块大小为页大小
                CHECK_EQ(metadata.block_size_shift, heap.page_shift);
            }
        }

        /// 测试空闲页链表初始化是否正确
        SUBCASE("free_page_list") { CHECK_EQ(heap.free_page_list.back(), heap.metadata.begin()); }
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
        auto heap{::test_fixture::get_heap()};
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
        auto heap{::test_fixture::get_heap()};

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
                            page_index = heap.get_metadata_index(::std::bit_cast<::free_block_list_t*>(base_address + offset)),
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
    ::std::array<::metadata_t*, 2> make_heap_for_insert_block_into_page_list_test(::SoC::test::heap & heap)
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
    void do_insert_block_into_page_list_test(::SoC::test::heap & heap, ::metadata_t * page_ptr)
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
        auto metadata_index{::std::distance(heap.metadata.begin(), page_ptr)};
        auto data_address{::std::bit_cast<::std::uintptr_t>(heap.data) + metadata_index * heap.page_size};
        // 检查页的空闲块指针是否指向数据块首地址，即完成对于页操作的初始化
        CHECK_EQ(page_ptr->free_block_list, ::std::bit_cast<::free_block_list_t*>(data_address));
        // 检查块大小是否正确设置为页大小
        CHECK_EQ(page_ptr->block_size_shift, heap.page_shift);
    }

    /// @test 测试堆的插入块函数能否在链表元素数大于1时正常工作
    REGISTER_TEST_CASE("insert_block_into_page_list" *
                       ::doctest::description{"测试堆的插入块函数能否在链表元素数大于1时正常工作"})
    {
        SUBCASE("first element")
        {
            auto heap{::test_fixture::get_heap()};
            auto [page_ptr, _]{make_heap_for_insert_block_into_page_list_test(heap)};
            do_insert_block_into_page_list_test(heap, page_ptr);
        }

        SUBCASE("last element")
        {
            auto heap{::test_fixture::get_heap()};
            auto [_, page_ptr]{make_heap_for_insert_block_into_page_list_test(heap)};
            do_insert_block_into_page_list_test(heap, page_ptr);
        }
    }

    /// @test 测试堆的页回收函数能否正常工作
    REGISTER_TEST_CASE("page_gc" * ::doctest::description{"测试堆的页回收函数能否正常工作"})
    {
        auto heap{::test_fixture::get_heap()};
        // 将空闲页链表中的页插入index处的空闲块链表
        auto insert_block_into_page_list{
            [&heap](::std::size_t index, bool is_free = true) noexcept
            {
                auto&& free_page_list{heap.free_page_list.back()};
                auto* old_free_page_head{::std::exchange(free_page_list, free_page_list->next_page)};
                auto* old_free_block_head{::std::exchange(heap.free_page_list[index], old_free_page_head)};
                old_free_page_head->next_page = old_free_block_head;
                old_free_page_head->used_block = !is_free;
                old_free_page_head->block_size_shift = index + heap.min_block_shift;
                return old_free_page_head;
            }};

        // 16字节块链表保持空
        heap.free_page_list[0] = nullptr;
        // 32字节块链表插入1个空闲块
        auto* page_ptr32{insert_block_into_page_list(1)};
        // 64字节块链表插入1个非空闲块
        auto* page_ptr64{insert_block_into_page_list(2, false)};
        // 128字节块链表插入1个空闲块1个非空闲块
        auto* page_ptr128_1{insert_block_into_page_list(3)};
        auto* page_ptr128_2{insert_block_into_page_list(3, false)};
        // 256字节块链表交替插入2个空闲块和2个非空闲块
        auto* page_ptr256_1{insert_block_into_page_list(4)};
        auto* page_ptr256_2{insert_block_into_page_list(4, false)};
        auto* page_ptr256_3{insert_block_into_page_list(4)};
        auto* page_ptr256_4{insert_block_into_page_list(4, false)};
        auto* origin_free_page_list_head{heap.free_page_list.back()};

        SUBCASE("prepare")
        {
            constexpr auto* message{"块插入失败"};
            REQUIRE_MESSAGE(page_ptr32->next_page == nullptr, message);
            REQUIRE_MESSAGE(page_ptr32 == heap.free_page_list[1], message);

            REQUIRE_MESSAGE(page_ptr64->next_page == nullptr, message);
            REQUIRE_MESSAGE(page_ptr64 == heap.free_page_list[2], message);

            REQUIRE_MESSAGE(page_ptr128_1->next_page == nullptr, message);
            REQUIRE_MESSAGE(page_ptr128_2->next_page == page_ptr128_1, message);
            REQUIRE_MESSAGE(page_ptr128_2 == heap.free_page_list[3], message);

            REQUIRE_MESSAGE(page_ptr256_1->next_page == nullptr, message);
            REQUIRE_MESSAGE(page_ptr256_2->next_page == page_ptr256_1, message);
            REQUIRE_MESSAGE(page_ptr256_3->next_page == page_ptr256_2, message);
            REQUIRE_MESSAGE(page_ptr256_4->next_page == page_ptr256_3, message);
            REQUIRE_MESSAGE(page_ptr256_4 == heap.free_page_list[4], message);
        }

        SUBCASE("with free block")
        {
            ::metadata_t* current_free_page_list_head{};
            REQUIRE_NOTHROW_MESSAGE(current_free_page_list_head = heap.page_gc(true), "堆含有空闲块，不应出现空间不足错误");
            // 检查空闲页链表的头指针是否正确
            CHECK_EQ(current_free_page_list_head, page_ptr256_1);
            // 检查16字节块链表是否保持空
            CHECK_EQ(heap.free_page_list[0], nullptr);
            // 检查32字节块链表是否为空，即空闲块被回收
            CHECK_EQ(heap.free_page_list[1], nullptr);
            // 检查64字节块链表是否为原链表头，即非空闲块未被回收
            CHECK_EQ(heap.free_page_list[2], page_ptr64);
            CHECK_EQ(heap.free_page_list[2]->next_page, nullptr);
            // 检查128字节块链表是否为非空闲块，即空闲块被回收而非空闲块未被回收
            CHECK_EQ(heap.free_page_list[3], page_ptr128_2);
            CHECK_EQ(heap.free_page_list[3]->next_page, nullptr);
            // 检查256字节块链表是否为非空闲块，即穿插在非空闲块内的空闲块是否被回收
            CHECK_EQ(heap.free_page_list[4], page_ptr256_4);
            CHECK_EQ(heap.free_page_list[4]->next_page, page_ptr256_2);
            CHECK_EQ(heap.free_page_list[4]->next_page->next_page, nullptr);

            ::std::array free_page_list_gt{page_ptr256_1, page_ptr256_3, page_ptr128_1, page_ptr32, origin_free_page_list_head};
            for(auto* page_ptr{heap.free_page_list.back()};
                auto&& [index_in_free_page_list_gt, page_ptr_gt]: ::std::views::zip(::std::views::iota(0), free_page_list_gt))
            {
                CAPTURE(index_in_free_page_list_gt);
                CHECK_EQ(page_ptr, page_ptr_gt);
                // 检查块大小是否正确设置为页大小
                CHECK_EQ(page_ptr->block_size_shift, heap.page_shift);
                page_ptr = page_ptr->next_page;
            }
        }

        SUBCASE("without free block")
        {
            for(auto&& free_page_list{heap.free_page_list}; auto&& free_page: free_page_list) { free_page = nullptr; }
            // 启用断言的情况下，应当断言失败
            CHECK_THROWS_WITH_AS_MESSAGE(heap.page_gc(true),
                                         ::doctest::Contains{"剩余堆空间不足"},
                                         ::SoC::assert_failed_exception,
                                         "剩余堆空间不足，应该断言失败");
            CHECK_NOTHROW_MESSAGE(heap.page_gc(false), "禁用断言时，剩余堆空间不足不应该产生断言失败");
        }
    }

    /// @test 测试对空闲页进行分块
    REGISTER_TEST_CASE("make_block_in_page" * ::doctest::description{"测试对空闲页进行分块"})
    {
        SUBCASE("free_block_list not empty")
        {
            auto heap{::test_fixture::get_heap()};
            // 模拟空闲页链表非空
            heap.free_page_list.front() = heap.free_page_list.back();
            CHECK_THROWS_WITH_AS_MESSAGE(heap.make_block_in_page(0),
                                         ::doctest::Contains{"仅在块空闲链表为空时调用此函数"},
                                         ::SoC::assert_failed_exception,
                                         "空闲页链表非空，应该断言失败");
        }

        SUBCASE("free_page_list not empty")
        {
            auto heap{::test_fixture::get_heap()};
            auto* current_page{heap.free_page_list.back()};
            auto* next_page{current_page->next_page};
            REQUIRE_MESSAGE(current_page != nullptr, "空闲页链表不应为空");
            auto page_begin{::std::bit_cast<::std::uintptr_t>(current_page->free_block_list)};
            constexpr auto block_index{0zu};
            constexpr auto block_size{1zu << (::SoC::test::heap::min_block_shift + block_index)};
            ::free_block_list_t* free_block_ptr{};
            REQUIRE_NOTHROW_MESSAGE(free_block_ptr = heap.make_block_in_page(block_index), "空闲页链表非空，应该能够成功分块");

            // 首个空闲块地址应当是页起始地址
            CHECK_EQ(free_block_ptr, ::std::bit_cast<void*>(page_begin));
            // 空闲页链表头应该移到下一页
            CHECK_EQ(heap.free_page_list.back(), next_page);
            // 空闲块链表头应该是当前页
            CHECK_EQ(heap.free_page_list[block_index], current_page);
            // 分块后的页是该块大小对应的空闲链表中唯一的一项，因此next_page为nullptr
            CHECK_EQ(current_page->next_page, nullptr);
            // 检查块大小是否正确设置
            CHECK_EQ(current_page->block_size_shift, heap.min_block_shift + block_index);
            constexpr auto page_size{::SoC::heap::page_size};
            // 检查空闲块链表中每个节点是否正确
            for(auto offset{0zu}; offset != page_size;)
            {
                auto next_offset{offset += block_size};
                auto* free_block_ptr_gt{
                    ::std::bit_cast<::free_block_list_t*>(next_offset == page_size ? 0zu : page_begin + next_offset)};
                auto* next_block{free_block_ptr->next};
                CAPTURE(offset);
                CHECK_EQ(next_block, free_block_ptr_gt);
                free_block_ptr = next_block;
            }
        }

        SUBCASE("free_page_list empty")
        {
            auto heap{::test_fixture::get_heap()};
            auto* current_page{::std::exchange(heap.free_page_list.back(), nullptr)};
            // 将堆设置为只有1页，因此next_page为nullptr
            current_page->next_page = nullptr;
            heap.free_page_list.front() = current_page;
            constexpr auto block_index{1zu};

            SUBCASE("no free block")
            {
                current_page->used_block = 1;
                CHECK_THROWS_AS_MESSAGE(heap.make_block_in_page(block_index),
                                        ::SoC::assert_failed_exception,
                                        "无空闲块且空闲链表为空，page_gc应该断言失败");
            }

            SUBCASE("with free block")
            {
                current_page->used_block = 0;
                REQUIRE_NOTHROW_MESSAGE(heap.make_block_in_page(block_index), "空闲页链表为空且有空闲块，应该能够成功分块");
                CHECK_EQ(heap.free_page_list.back(), nullptr);
                CHECK_EQ(heap.free_page_list[block_index], current_page);
                CHECK_EQ(current_page->next_page, nullptr);
                CHECK_EQ(current_page->block_size_shift, heap.min_block_shift + block_index);
            }
        }
    }

    /// @test 测试从空闲页链表中移除范围内的页
    REGISTER_TEST_CASE("remove_pages" * ::doctest::description{"测试从空闲页链表中移除范围内的页"})
    {
        auto heap{::test_fixture::get_heap()};

        SUBCASE("remove pages with free_page_list head")
        {
            auto* first_page{heap.free_page_list.back()};
            auto* second_page{first_page->next_page};
            auto* third_page{second_page->next_page};
            auto* fourth_page{third_page->next_page};

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
            auto* first_page{heap.free_page_list.back()};
            auto* second_page{first_page->next_page};
            auto* third_page{second_page->next_page};
            auto* fourth_page{third_page->next_page};

            // 检查返回值是否为范围内的首个页
            CHECK_EQ(heap.remove_pages(second_page->free_block_list, third_page->free_block_list), second_page->free_block_list);
            // 检查空闲页表头是否为第一页
            CHECK_EQ(heap.free_page_list.back(), first_page);
            // 检查范围内的页used_blocks是否为1，不在移除范围内的页used_blocks是否为0
            CHECK_EQ(first_page->used_block, 0);
            CHECK_EQ(second_page->used_block, 1);
            CHECK_EQ(third_page->used_block, 1);
            CHECK_EQ(fourth_page->used_block, 0);
        }
    }

    /// @test 测试页分配函数
    REGISTER_TEST_CASE("allocate_pages" * ::doctest::description{"测试页分配函数"})
    {
        SUBCASE("allocate a page")
        {
            auto heap{::test_fixture::get_heap()};
            void* page_ptr{};

            SUBCASE("free_page_list not empty")
            {
                auto* current_page{heap.free_page_list.back()};
                auto* next_page{current_page->next_page};
                REQUIRE_NOTHROW_MESSAGE(page_ptr = heap.allocate_pages(1), "堆中空闲页充足，分配不应该失败");
                // 检查分配的页是不是空闲页表头部的页
                CHECK_EQ(page_ptr, current_page->free_block_list);
                // 检查分配后的空闲页表头是不是指向下一页
                CHECK_EQ(heap.free_page_list.back(), next_page);
                // 检查分配后的空闲页表中该页的used_block是否为1
                CHECK_EQ(current_page->used_block, 1);
                CHECK_EQ(next_page->used_block, 0);
                // 检查分配后的空闲页表中该页的块大小是否为页大小
                CHECK_EQ(current_page->block_size_shift, heap.page_shift);
            }

            SUBCASE("free_page_list empty")
            {
                auto* current_page{::std::exchange(heap.free_page_list.back(), nullptr)};
                current_page->next_page = nullptr;
                heap.free_page_list.front() = current_page;

                REQUIRE_NOTHROW_MESSAGE(page_ptr = heap.allocate_pages(1), "可以通过page_gc回收空闲块，分配不应该失败");
                // 检查分配的页是不是空闲页表头部的页
                CHECK_EQ(page_ptr, current_page->free_block_list);
                // 检查分配后的空闲页表头是不是为空
                CHECK_EQ(heap.free_page_list.back(), nullptr);
                // 检查分配后的空闲页表中该页的used_block是否为1
                CHECK_EQ(current_page->used_block, 1);
                CHECK_EQ(current_page->next_page, nullptr);
                // 检查分配后的空闲页表中该页的块大小是否为页大小
                CHECK_EQ(current_page->block_size_shift, heap.page_shift);
                // 检查空闲块是否被page_gc回收
                CHECK_EQ(heap.free_page_list.front(), nullptr);
            }

            SUBCASE("heap empty")
            {
                ::std::ranges::fill(heap.free_page_list, nullptr);

                CHECK_THROWS_AS_MESSAGE(page_ptr = heap.allocate_pages(1),
                                        ::SoC::assert_failed_exception,
                                        "堆中不存在空闲块和空闲页，page_gc应该断言失败");
            }
        }

        SUBCASE("allocate pages")
        {
            SUBCASE("with enough free pages")
            {
                auto heap{::test_fixture::get_heap()};
                constexpr auto* message{"堆中空闲页充足，分配不应该失败"};

                SUBCASE("continuous pages at head")
                {
                    auto* first_page{heap.free_page_list.back()};
                    auto* second_page{first_page->next_page};
                    auto* third_page{second_page->next_page};

                    void* page_ptr{};
                    REQUIRE_NOTHROW_MESSAGE(page_ptr = heap.allocate_pages(2), message);
                    // 检查分配的页是不是空闲页表头部的页
                    CHECK_EQ(page_ptr, first_page->free_block_list);
                    // 检查分配后的空闲页表头是不是指向下一页
                    CHECK_EQ(heap.free_page_list.back(), third_page);
                    // 检查分配后的空闲页表中该页的used_block是否为1
                    CHECK_EQ(first_page->used_block, 1);
                    CHECK_EQ(second_page->used_block, 1);
                    CHECK_EQ(third_page->used_block, 0);
                    // 检查分配后的空闲页表中该页的块大小是否为页大小
                    CHECK_EQ(first_page->block_size_shift, heap.page_shift);
                    CHECK_EQ(second_page->block_size_shift, heap.page_shift);
                }

                SUBCASE("discontinuous pages at head")
                {
                    auto* first_page{heap.free_page_list.back()};
                    auto* second_page{first_page->next_page};
                    auto* third_page{second_page->next_page};
                    auto* fourth_page{third_page->next_page};
                    auto* fifth_page{fourth_page->next_page};
                    auto* sixth_page{fifth_page->next_page};
                    auto* seventh_page{sixth_page->next_page};
                    // 设置第三页不为空，即需要从第四页开始查找连续页
                    third_page->used_block = 1;
                    second_page->next_page = fourth_page;

                    void* page_ptr{};
                    REQUIRE_NOTHROW_MESSAGE(page_ptr = heap.allocate_pages(3), message);
                    // 检查分配的页是不是空闲页表头部的页
                    CHECK_EQ(page_ptr, fourth_page->free_block_list);
                    auto* free_page_list{heap.free_page_list.back()};
                    // 检查分配后的空闲页表头是不是指向第一页，因为该页未被分配
                    CHECK_EQ(free_page_list, first_page);
                    free_page_list = free_page_list->next_page;
                    // 检查后续空闲链表连接是否正确
                    CHECK_EQ(free_page_list, second_page);
                    free_page_list = free_page_list->next_page;
                    CHECK_EQ(free_page_list, seventh_page);
                    // 检查分配后的空闲页表中该页的used_block是否为1
                    CHECK_EQ(first_page->used_block, 0);
                    CHECK_EQ(second_page->used_block, 0);
                    CHECK_EQ(third_page->used_block, 1);
                    CHECK_EQ(fourth_page->used_block, 1);
                    CHECK_EQ(fifth_page->used_block, 1);
                    CHECK_EQ(sixth_page->used_block, 1);
                    CHECK_EQ(seventh_page->used_block, 0);
                    // 检查分配后的空闲页表中该页的块大小是否为页大小
                    CHECK_EQ(first_page->block_size_shift, heap.page_shift);
                    CHECK_EQ(second_page->block_size_shift, heap.page_shift);
                    CHECK_EQ(third_page->block_size_shift, heap.page_shift);
                    CHECK_EQ(fourth_page->block_size_shift, heap.page_shift);
                    CHECK_EQ(fifth_page->block_size_shift, heap.page_shift);
                    CHECK_EQ(sixth_page->block_size_shift, heap.page_shift);
                }
            }

            SUBCASE("not enough free pages")
            {
                auto heap{::test_fixture::get_heap()};
                const ::doctest::Contains exception_string{"堆中剩余连续分页数量不足"};

                SUBCASE("not enough continuous page")
                {
                    auto* first_page{heap.free_page_list.back()};
                    auto* second_page{first_page->next_page};
                    second_page->next_page = nullptr;
                    ::std::ranges::for_each(
                        heap.metadata | ::std::views::filter([=](auto&& metadata) noexcept
                                                             { return &metadata != first_page && &metadata != second_page; }),
                        [](auto&& metadata) static noexcept { metadata.used_block = 1; });

                    CHECK_THROWS_WITH_AS_MESSAGE(heap.allocate_pages(3),
                                                 exception_string,
                                                 ::SoC::assert_failed_exception,
                                                 "堆内没有足够连续页，allocate_pages应该断言失败");
                }

                SUBCASE("no free pages")
                {
                    for(auto&& metadata: heap.metadata) { metadata.used_block = 1; }

                    CHECK_THROWS_WITH_AS_MESSAGE(heap.allocate_pages(2),
                                                 exception_string,
                                                 ::SoC::assert_failed_exception,
                                                 "堆内没有空闲页，allocate_pages应该断言失败");
                }
            }
        }
    }

    /// @test 测试冷路径下的内存分配
    REGISTER_TEST_CASE("allocate_cold_path" * ::doctest::description{"测试冷路径下的内存分配"})
    {
        auto heap{::test_fixture::get_heap()};
        SUBCASE("allocate a block")
        {
            auto do_check{
                [&heap](::std::size_t actual_size, ::std::size_t target_free_block_list_index)
                {
                    auto&& metadata{*heap.free_page_list.back()};
                    auto* page_begin{metadata.free_block_list};
                    auto* next_block{
                        ::std::bit_cast<::free_block_list_t*>(::std::bit_cast<::std::uintptr_t>(page_begin) + actual_size)};

                    void* result{};
                    REQUIRE_NOTHROW_MESSAGE(result = heap.allocate_cold_path(actual_size), "空闲表为空，断言不应该失败");
                    // 检查分配结果是否为页基址
                    CHECK_EQ(result, page_begin);
                    // 检查元数据块是否正确插入空闲表
                    CHECK_EQ(heap.free_page_list[target_free_block_list_index], &metadata);
                    // 检查分配后的空闲块页表头是否为下一个空闲块
                    CHECK_EQ(metadata.free_block_list, next_block);
                    // 检查分配后的元数据中块大小是否为实际分配大小
                    CHECK_EQ(1zu << metadata.block_size_shift, actual_size);
                    // 检查分配后的元数据中used_block是否为1
                    CHECK_EQ(metadata.used_block, 1);
                }};

            SUBCASE("allocate 16 bytes") { do_check(16, 0); }
            SUBCASE("allocate 32 bytes") { do_check(32, 1); }
            SUBCASE("allocate 64 bytes") { do_check(64, 2); }
            SUBCASE("allocate 128 bytes") { do_check(128, 3); }
            SUBCASE("allocate 256 bytes") { do_check(256, 4); }
        }

        // 这些功能在allocate_pages中已经测试过
        SUBCASE("allocate pages")
        {
            ::fakeit::Mock mock{heap};
            ::fakeit::Fake(Method(mock, allocate_pages));
            auto&& heap{mock.get()};

            SUBCASE("allocate 1 page") { CHECK_EQ(heap.allocate_cold_path(heap.page_size), nullptr); }
            SUBCASE("allocate 2 pages") { CHECK_EQ(heap.allocate_cold_path(2 * heap.page_size), nullptr); }
        }
    }

    /// @test 测试内存分配函数
    REGISTER_TEST_CASE("allocate" * ::doctest::description{"测试内存分配函数"})
    {
        auto heap{::test_fixture::get_heap()};

        SUBCASE("cold path")
        {
            ::fakeit::Mock mock{heap};
            auto method{Method(mock, allocate_cold_path)};
            ::fakeit::When(method)
                .Do(
                    [&heap](::std::size_t size)
                    {
                        heap.allocate_cold_path(size);
                        return nullptr;
                    })
                .AlwaysReturn(nullptr);
            auto&& heap{mock.get()};

            SUBCASE("allocate blocks")
            {
                CHECK_EQ(heap.allocate(16), nullptr);
                CHECK_EQ(heap.allocate(32), nullptr);
                CHECK_EQ(heap.allocate(64), nullptr);
                CHECK_EQ(heap.allocate(128), nullptr);
                CHECK_EQ(heap.allocate(256), nullptr);
                ::fakeit::Verify(method).Exactly(5);
            }

            SUBCASE("allocate pages")
            {
                CHECK_EQ(heap.allocate(heap.page_size), nullptr);
                CHECK_EQ(heap.allocate(2 * heap.page_size), nullptr);
                ::fakeit::Verify(method).Exactly(2);
            }
        }

        SUBCASE("hot path")
        {
            // 分配一个块以初始化free_page_list
            for(auto i{heap.min_block_shift}; i < heap.page_shift; ++i) { auto* _{heap.allocate(1zu << i)}; }

            for(::std::span span{heap.free_page_list};
                auto&& [index, metadata]: ::std::views::zip(::std::views::iota(0zu), span.subspan(0, span.size() - 1)))
            {
                CAPTURE(index);
                REQUIRE_NE(metadata, nullptr);
                REQUIRE_NE(metadata->free_block_list, nullptr);
                REQUIRE_EQ(metadata->used_block, 1);
            }

            auto do_check{[&heap](::std::size_t size, ::std::size_t target_free_block_list_index, bool free_list_empty = false)
                          {
                              auto&& free_list{heap.free_page_list[target_free_block_list_index]};
                              auto* block_ptr{free_list->free_block_list};
                              auto* next_block_ptr{block_ptr->next};

                              auto* result{heap.allocate(size)};
                              CHECK_EQ(result, block_ptr);
                              if(free_list_empty) [[unlikely]] { CHECK_EQ(free_list, nullptr); }
                              else
                              {
                                  CHECK_EQ(free_list->used_block, 2);
                                  CHECK_EQ(free_list->free_block_list, next_block_ptr);
                              }
                          }};

            SUBCASE("allocate 16 bytes") { do_check(16, 0); }
            SUBCASE("allocate 32 bytes") { do_check(32, 1); }
            SUBCASE("allocate 64 bytes") { do_check(64, 2); }
            SUBCASE("allocate 128 bytes") { do_check(128, 3); }
            SUBCASE("allocate 256 bytes") { do_check(256, 4, true); }
        }

        SUBCASE("hot path with blocks cached in free_page_list")
        {
            // 256字节块每页数量最少，便于测试
            constexpr auto block_index{4zu};
            constexpr auto block_size{256zu};
            heap.make_block_in_page(block_index);
            auto&& free_block_list{heap.free_page_list[block_index]};
            auto* first_page{::std::exchange(free_block_list, nullptr)};
            auto* first_block{first_page->free_block_list};
            auto* second_block{first_block->next};
            // clang-analyzer无法分析这个函数修改了free_block_list，所以需要NOLINTNEXTLINE
            heap.make_block_in_page(block_index);
            auto* second_page{::std::exchange(free_block_list, first_page)};
            // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
            auto* third_block{second_page->free_block_list};
            auto* fourth_block{third_block->next};
            first_page->next_page = second_page;

            auto* result{heap.allocate(block_size)};
            CHECK_EQ(result, first_block);
            CHECK_EQ(free_block_list->used_block, 1);
            result = heap.allocate(block_size);
            CHECK_EQ(result, second_block);
            // 切换到下一个空闲的页，所以used_block为0
            CHECK_EQ(free_block_list->used_block, 0);
            result = heap.allocate(block_size);
            CHECK_EQ(result, third_block);
            CHECK_EQ(free_block_list->used_block, 1);
            result = heap.allocate(block_size);
            CHECK_EQ(result, fourth_block);
            // 没有空闲页，所以free_block_list为nullptr
            CHECK_EQ(free_block_list, nullptr);
        }
    }
}
