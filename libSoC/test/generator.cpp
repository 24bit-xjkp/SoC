import "test_framework.hpp";
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
    using generator = ::SoC::test::generator<::std::size_t, ::SoC::std_allocator>;
}  // namespace

/// @test SoC::generator单元测试
TEST_SUITE("generator" * ::doctest::description{"SoC::generator单元测试"})
{
    /// @test 测试自定义分配器是否能正常工作
    REGISTER_TEST_CASE("custom_allocator" * ::doctest::description{"测试自定义分配器是否能正常工作"})
    {
        ::SoC::std_allocator::reset();
        auto&& allocate_cnt{::SoC::std_allocator::allocate_cnt};
        auto&& deallocate_cnt{::SoC::std_allocator::deallocate_cnt};

        {
            auto gen{[] static -> ::generator
                     {
                         for(auto i{0zu}; i != 3; ++i) { co_yield i; }
                     }};
            REQUIRE_EQ(allocate_cnt, 0);
            REQUIRE_EQ(deallocate_cnt, 0);
            auto generator{gen()};
            CHECK_EQ(allocate_cnt, 1);
            CHECK_EQ(deallocate_cnt, 0);
            auto iterator = generator.begin();
            auto sentinel{generator.end()};
            CHECK_EQ(allocate_cnt, 1);
            CHECK_EQ(deallocate_cnt, 0);
            while(iterator != sentinel) { ++iterator; }
        }
        CHECK_EQ(allocate_cnt, 1);
        CHECK_EQ(deallocate_cnt, 1);
    }

    /// @test 测试co_yield能否正常工作
    REGISTER_TEST_CASE("co_yield" * ::doctest::description{"测试co_yield能否正常工作"})
    {
        SUBCASE("single generator")
        {
            auto gen{[] static -> ::generator
                     {
                         for(auto i{0zu}; i != 3; ++i) { co_yield i; }
                     }};
            auto result{0};
            for(auto&& value: gen()) { CHECK_EQ(value, result++); }
            CHECK_EQ(result, 3);
        }

        SUBCASE("cascading generator")
        {
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-reference-coroutine-parameters, misc-no-recursion)
            auto gen{[](this auto&& self, int i) -> ::generator
                     {
                         if(i == 0 || i == 1) { co_yield 1zu; }
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
            auto gen{[] static -> ::generator
                     {
                         ::SoC::assert(false, assert_message);
                         co_yield 0zu;
                     }};
            CHECK_THROWS_WITH_AS_MESSAGE(gen().begin(),
                                         ::doctest::Contains{assert_message},
                                         ::SoC::assert_failed_exception,
                                         check_message);
        }

        SUBCASE("exception after yield")
        {
            auto gen{[] static -> ::generator
                     {
                         co_yield 0zu;
                         ::SoC::assert(false, assert_message);
                     }};
            auto generator{gen()};
            ::generator::iterator iterator;
            CHECK_NOTHROW_MESSAGE(iterator = generator.begin(), "第一次迭代不应该抛出异常");
            CHECK_THROWS_WITH_AS_MESSAGE(++iterator,
                                         ::doctest::Contains{assert_message},
                                         ::SoC::assert_failed_exception,
                                         check_message);
        }

        SUBCASE("exception between yield")
        {
            auto gen{[] static -> ::generator
                     {
                         co_yield 0zu;
                         ::SoC::assert(false, assert_message);
                         co_yield 0zu;
                     }};
            auto generator{gen()};
            ::generator::iterator iterator;
            CHECK_NOTHROW_MESSAGE(iterator = generator.begin(), "第一次迭代不应该抛出异常");
            CHECK_THROWS_WITH_AS_MESSAGE(++iterator,
                                         ::doctest::Contains{assert_message},
                                         ::SoC::assert_failed_exception,
                                         check_message);
        }
    }
}
