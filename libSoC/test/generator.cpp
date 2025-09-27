import "doctest.hpp";
import SoC.unit_test;

namespace SoC::test
{
    template <typename value_type, ::SoC::is_allocator allocator_type>
    struct generator : ::SoC::generator<value_type, allocator_type>
    {
        using base_t = ::SoC::generator<value_type, allocator_type>;
        using typename base_t::handle_t;
        using typename base_t::iterator;
        using typename base_t::promise_type;
        using typename base_t::sentinel;

        constexpr inline generator(base_t generator) noexcept : base_t{::std::move(generator)} {}
    };
}  // namespace SoC::test

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("generator/" NAME)

namespace
{
    template <typename type>
    using generator = ::SoC::test::generator<type, ::SoC::std_allocator>;

    /**
     * @brief 自定义分配器，用于测试分配器适配
     *
     */
    struct custom_allocator : ::SoC::std_allocator
    {
        /// 记录是否分配了内存
        inline static bool is_allocated{};
        /// 记录是否释放了内存
        inline static bool is_deallocated{};

        using ::SoC::std_allocator::allocate;
        using ::SoC::std_allocator::deallocate;

        /**
         * @brief 重置分配器状态
         *
         */
        constexpr inline static void reset() noexcept
        {
            is_allocated = false;
            is_deallocated = false;
        }

        /**
         * @brief 分配内存
         *
         * @param size 分配内存大小
         * @return 分配内存区域首指针
         */
        constexpr inline static void* allocate(::std::size_t size)
        {
            is_allocated = true;
            return ::operator new (size);
        }

        /**
         * @brief 释放内存
         *
         * @param ptr 分配内存区域首指针
         * @param size 分配内存大小
         */
        constexpr inline static void deallocate(void* ptr, ::std::size_t size [[maybe_unused]])
        {
            is_deallocated = true;
#if defined(__cpp_sized_deallocation) && __cpp_sized_deallocation >= 201309L
            ::operator delete (ptr, size);
#else
            ::operator delete (ptr);
#endif
        }
    };
}  // namespace

TEST_SUITE("generator" * ::doctest::description{"SoC::generator单元测试"})
{
    /// @test 测试自定义分配器是否能正常工作
    REGISTER_TEST_CASE("custom_allocator" * ::doctest::description{"测试自定义分配器是否能正常工作"})
    {
        custom_allocator::reset();
        {
            auto gen{[] static -> ::SoC::test::generator<int, ::custom_allocator>
                     {
                         for(int i = 0; i != 3; ++i) { co_yield i; }
                     }};
            REQUIRE_FALSE(custom_allocator::is_allocated);
            REQUIRE_FALSE(custom_allocator::is_deallocated);
            auto generator{gen()};
            CHECK(custom_allocator::is_allocated);
            CHECK_FALSE(custom_allocator::is_deallocated);
            auto iterator = generator.begin();
            auto end{generator.end()};
            CHECK(custom_allocator::is_allocated);
            CHECK_FALSE(custom_allocator::is_deallocated);
            while(iterator != end) { ++iterator; }
        }
        CHECK(custom_allocator::is_allocated);
        CHECK(custom_allocator::is_deallocated);
    }

    /// @test 测试co_yield能否正常工作
    REGISTER_TEST_CASE("co_yield" * ::doctest::description{"测试co_yield能否正常工作"})
    {
        SUBCASE("single generator")
        {
            auto gen{[] static -> ::generator<int>
                     {
                         for(int i = 0; i != 3; ++i) { co_yield i; }
                     }};
            auto result{0};
            for(auto&& value: gen()) { CHECK_EQ(value, result++); }
            CHECK_EQ(result, 3);
        }

        SUBCASE("cascading generator")
        {
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters, misc-no-recursion)
            auto gen{[](this auto&& self, int i) -> ::generator<int>
                     {
                         if(i == 0 || i == 1) { co_yield 1; }
                         else
                         {
                             co_yield *self(i - 1).begin() + *self(i - 2).begin();
                         }
                     }};

            constexpr ::std::array table{1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
            for(auto&& [i, result]: ::std::views::zip(::std::views::iota(0), table)) { CHECK_EQ(*gen(i).begin(), result); }
        }
    }

    /// @test 测试未处理的异常是否会导致程序终止
    REGISTER_TEST_CASE("unhandled_exception" * ::doctest::description{"测试未处理的异常是否能正常传播回调用方"})
    {
        constexpr auto* assert_message{"应当断言失败"};
        constexpr auto* check_message{"异常传播失败"};
        SUBCASE("exception before yield")
        {
            auto gen{[] static -> ::generator<int>
                     {
                         ::SoC::assert(false, assert_message);
                         co_yield 0;
                     }};
            CHECK_THROWS_WITH_AS_MESSAGE(gen().begin(),
                                         ::doctest::Contains{assert_message},
                                         ::SoC::assert_failed_exception,
                                         check_message);
        }

        SUBCASE("exception after yield")
        {
            auto gen{[] static -> ::generator<int>
                     {
                         co_yield 0;
                         ::SoC::assert(false, assert_message);
                     }};
            auto generator{gen()};
            ::generator<int>::iterator iterator;
            CHECK_NOTHROW_MESSAGE(iterator = generator.begin(), "第一次迭代不应该抛出异常");
            CHECK_THROWS_WITH_AS_MESSAGE(++iterator,
                                         ::doctest::Contains{assert_message},
                                         ::SoC::assert_failed_exception,
                                         check_message);
        }

        SUBCASE("exception between yield")
        {
            auto gen{[] static -> ::generator<int>
                     {
                         co_yield 0;
                         ::SoC::assert(false, assert_message);
                         co_yield 0;
                     }};
            auto generator{gen()};
            ::generator<int>::iterator iterator;
            CHECK_NOTHROW_MESSAGE(iterator = generator.begin(), "第一次迭代不应该抛出异常");
            CHECK_THROWS_WITH_AS_MESSAGE(++iterator,
                                         ::doctest::Contains{assert_message},
                                         ::SoC::assert_failed_exception,
                                         check_message);
        }
    }
}
