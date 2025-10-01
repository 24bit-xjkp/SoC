import "test_framework.hpp";
import SoC.unit_test.heap;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("heap_deallocate/" NAME)

/// @test SoC::heap释放函数单元测试
TEST_SUITE("heap_deallocate" * ::doctest::description{"SoC::heap释放函数单元测试"})
{  /// @test 测试页释放函数
    REGISTER_TEST_CASE("deallocate_pages" * ::doctest::description{"测试页释放函数"})
    {
        auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};
        auto&& free_page_list{heap.free_page_list.back()};

        SUBCASE("invalid page ptr")
        {
            auto* first_page{free_page_list};
            auto* page_ptr{first_page->free_block_list};

            SUBCASE("unaligned page ptr")
            {
                first_page->used_block = 1;
                CHECK_THROWS_WITH_AS_MESSAGE(heap.deallocate_pages(page_ptr + 1, heap.page_size),
                                             ::doctest::Contains{"释放范围首指针不满足页对齐"},
                                             ::SoC::assert_failed_exception,
                                             "页指针未对齐，应该断言失败");
            }

            SUBCASE("used_block not 1")
            {
                first_page->used_block = 0;
                CHECK_THROWS_WITH_AS_MESSAGE(heap.deallocate_pages(page_ptr, heap.page_size),
                                             ::doctest::Contains{"要释放的页使用计数不为1"},
                                             ::SoC::assert_failed_exception,
                                             "used_block不为1，应该断言失败");
            }

            SUBCASE("block_size not page_size")
            {
                first_page->used_block = 1;
                first_page->block_size_shift = heap.page_shift - 1;
                CHECK_THROWS_WITH_AS_MESSAGE(heap.deallocate_pages(page_ptr, heap.page_size),
                                             ::doctest::Contains{"释放块大小与申请块大小不匹配"},
                                             ::SoC::assert_failed_exception,
                                             "释放块大小与申请块大小不匹配，应该断言失败");
            }
        }

        SUBCASE("deallocate pages")
        {
            auto* first_page{::std::exchange(free_page_list, free_page_list->next_page)};
            first_page->used_block = 1;
            void* page_ptr{first_page->free_block_list};

            SUBCASE("deallocate 1 page")
            {
                auto* second_page{::std::exchange(first_page->next_page, nullptr)};
                CHECK_NOTHROW_MESSAGE(heap.deallocate_pages(page_ptr, heap.page_size), "释放已分配的1页，不应当断言失败");

                // 检查页指针是否正确插入free_page_list中
                CHECK_EQ(free_page_list, first_page);
                // 检查second_page是否正确串在first_page后面形成链表
                CHECK_EQ(first_page->next_page, second_page);
                // 检查释放后first_page使用计数是否为0
                CHECK_EQ(first_page->used_block, 0);
                // 检查second_page是否未被错误修改
                CHECK_EQ(second_page->used_block, 0);
            }

            SUBCASE("deallocate 2 pages")
            {
                first_page->next_page = nullptr;
                auto* second_page{::std::exchange(free_page_list, free_page_list->next_page)};
                second_page->used_block = 1;
                auto* third_page{::std::exchange(second_page->next_page, nullptr)};
                CHECK_NOTHROW_MESSAGE(heap.deallocate_pages(page_ptr, heap.page_size * 2), "释放已分配的2页，不应当断言失败");

                // 由于按页地址顺序释放，而free_page_list是FILO，所以second_page在first_page前
                CHECK_EQ(free_page_list, second_page);
                // 检查first_page是否正确串在second_page后面形成链表
                CHECK_EQ(second_page->next_page, first_page);
                // 检查释放后second_page使用计数是否为0
                CHECK_EQ(second_page->used_block, 0);
                // 检查third_page是否正确串在first_page后面形成链表
                CHECK_EQ(first_page->next_page, third_page);
                // 检查释放后first_page使用计数是否为0
                CHECK_EQ(first_page->used_block, 0);
                // 检查third_page是否未被错误修改
                CHECK_EQ(third_page->used_block, 0);
            }
        }
    }

    /// @test 测试块释放函数
    REGISTER_TEST_CASE("deallocate" * ::doctest::description{"测试块释放函数"})
    {
        auto heap{::SoC::unit_test::heap::test_fixture::get_heap()};
        constexpr auto* depend_on_allocate_message{"该测试用例依赖allocate函数"};

        // 这些功能在deallocate_pages中已经测试过
        SUBCASE("deallocate pages")
        {
            ::fakeit::Mock mock{heap};
            const auto method{Method(mock, deallocate_pages)};
            ::fakeit::Fake(method);
            auto&& heap{mock.get()};
            constexpr auto message{"deallocate_pages已mock为空实现，deallocate不应断言失败"};

            CHECK_NOTHROW_MESSAGE(heap.deallocate(nullptr, heap.page_size), message);
            CHECK_NOTHROW_MESSAGE(heap.deallocate(nullptr, heap.page_size * 2), message);

            ::fakeit::Verify(method).Exactly(2);
        }

        SUBCASE("invalid block ptr")
        {
            INFO(depend_on_allocate_message);

            constexpr auto block_size{16zu};
            ::std::byte* ptr{};
            // 分配一个16字节的块以便测试释放函数
            REQUIRE_NOTHROW_MESSAGE(ptr = static_cast<::std::byte*>(heap.allocate(block_size)),
                                    "申请16字节，allocate函数不应当断言失败");
            auto&& metadata{*heap.free_page_list.front()};

            CHECK_THROWS_WITH_AS_MESSAGE(heap.deallocate(ptr, block_size + 1),
                                         ::doctest::Contains{"释放块大小与申请块大小不匹配"},
                                         ::SoC::assert_failed_exception,
                                         "释放块大小与申请块大小不匹配，应该断言失败");

            CHECK_THROWS_WITH_AS_MESSAGE(heap.deallocate(ptr + 1, block_size),
                                         ::doctest::Contains{"释放页指针不满足块对齐"},
                                         ::SoC::assert_failed_exception,
                                         "释放页指针不满足块对齐，应该断言失败");

            const ::doctest::Contains exception_string{"要释放的块所在页使用计数不在[1, block_size]范围内"};
            constexpr auto* message{"要释放的块所在页使用计数不在[1, block_size]范围内"};
            metadata.used_block = 0;
            CHECK_THROWS_WITH_AS_MESSAGE(heap.deallocate(ptr, block_size),
                                         exception_string,
                                         ::SoC::assert_failed_exception,
                                         message);
            metadata.used_block = block_size + 1;
            CHECK_THROWS_WITH_AS_MESSAGE(heap.deallocate(ptr, block_size),
                                         exception_string,
                                         ::SoC::assert_failed_exception,
                                         message);
        }

        SUBCASE("hot path")
        {
            INFO(depend_on_allocate_message);
            constexpr auto* allocate_message{"allocate函数不应当断言失败"};
            constexpr auto* deallocate_message{"deallocate函数不应当断言失败"};

            SUBCASE("deallocate 1 block in unfull page")
            {
                // 测试每个块大小是否能正确释放
                for(auto block_size_shift{heap.min_block_shift}; block_size_shift != heap.page_shift; ++block_size_shift)
                {
                    auto block_size{1zu << block_size_shift};
                    CAPTURE(block_size);
                    auto&& metadata{heap.free_page_list[block_size_shift - heap.min_block_shift]};

                    void* block_ptr{};
                    REQUIRE_NOTHROW_MESSAGE(block_ptr = heap.allocate(block_size), allocate_message);
                    auto* next_block{metadata->free_block_list};

                    CHECK_NOTHROW_MESSAGE(heap.deallocate(block_ptr, block_size), deallocate_message);
                    // 检查block_ptr是否被正确添加到空闲块链表
                    CHECK_EQ(metadata->free_block_list, block_ptr);
                    // 检查next_block是否正确串在block_ptr后面形成链表
                    CHECK_EQ(metadata->free_block_list->next, next_block);
                    // 检查释放后使用计数是否为0
                    CHECK_EQ(metadata->used_block, 0);
                }
            }

            SUBCASE("deallocate full page")
            {
                // 使用256字节块以便于测试
                constexpr auto block_size{256zu};
                auto&& metadata{heap.free_page_list[heap.block_size_cnt - 2]};
                // 分配2个256字节块以耗尽该页
                void* block_ptr{};
                REQUIRE_NOTHROW_MESSAGE(block_ptr = heap.allocate(block_size), allocate_message);
                auto* page_ptr{metadata};
                REQUIRE_NOTHROW_MESSAGE(block_ptr = heap.allocate(block_size), allocate_message);
                // 检查页是否耗尽并移除空闲链表
                REQUIRE_EQ(metadata, nullptr);

                CHECK_NOTHROW_MESSAGE(heap.deallocate(block_ptr, block_size), deallocate_message);
                // 检查页是否被正确添加回空闲链表
                CHECK_EQ(metadata, page_ptr);
                // 检查释放1页后使用计数是否为1
                CHECK_EQ(metadata->used_block, 1);
                // 检查block_ptr是否被正确添加到空闲块链表
                CHECK_EQ(metadata->free_block_list, block_ptr);
                // 检查next_block是否为nullptr，因为释放前页内没有空闲块
                CHECK_EQ(metadata->free_block_list->next, nullptr);
                // 检查next_page是否为nullptr，因为释放前空闲链表为空
                CHECK_EQ(metadata->next_page, nullptr);
            }
        }
    }
}
