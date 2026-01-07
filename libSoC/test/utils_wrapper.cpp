/**
 * @file utils_wrapper.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief SoC实用包装体部分单元测试
 */

import "test_framework.hpp";
import SoC.unit_test;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("utils_wrapper/" NAME)

namespace SoC::test
{
    /**
     * @brief 导出SoC::optional中的符号用于测试
     *
     */
    extern "C++" template <typename type>
    struct optional : ::SoC::optional<type>
    {
        using base_t = ::SoC::optional<type>;
        using base_t::optional;
        using base_t::ptr;
        using base_t::operator=;
    };
}  // namespace SoC::test

/// @test SoC实用包装体部分单元测试
TEST_SUITE("utils_wrapper" * ::doctest::description{"SoC实用包装体部分单元测试"})
{
    /// @test 测试dtor_close_clock_callback_t能否正确关闭时钟
    REGISTER_TEST_CASE("dtor_close_clock_callback_t" * ::doctest::description{"测试dtor_close_clock_callback_t能否正确关闭时钟"})
    {
        constexpr static ::std::uint32_t clock_id{0x12345678};
        static constinit bool clock_closed{false};
        auto callback = [](::std::uint32_t input_id)
        {
            CHECK_EQ(input_id, clock_id);
            clock_closed = true;
        };

        ::SoC::detail::dtor_close_clock_callback_t callback_object{callback, clock_id};
        callback_object();

        CHECK_EQ(clock_closed, true);
    }

    /// @test 测试optional能否正确存储和访问值
    REGISTER_TEST_CASE("optional" * ::doctest::description{"测试optional能否正确存储和访问值"})
    {
        struct test_struct  // NOLINT(cppcoreguidelines-virtual-class-destructor)
        {
            int value;

            test_struct(int value) noexcept : value{value} {}

            virtual void operator() () noexcept {}

            virtual void operator[] (::std::size_t index [[maybe_unused]]) noexcept {}

            bool operator== (const test_struct& other) const = default;
        };

        ::SoC::test::optional<test_struct&> opt{};

        SUBCASE("without value")
        {
            CHECK_EQ(static_cast<bool>(opt), false);
            CHECK_EQ(opt.has_value(), false);
            test_struct other{1};
            CHECK_EQ(opt.value_or(other), other);
            CHECK_THROWS_WITH_AS(opt.value(), ::doctest::Contains{"尝试获取空optional的值"}, ::SoC::assert_failed_exception);
        }

        SUBCASE("with value")
        {
            test_struct self{0};
            test_struct other{1};
            ::fakeit::Mock mock{self};
            const auto method_call{Method(mock, operator())};
            const auto method_index{Method(mock, operator[])};
            ::fakeit::Fake(method_call, method_index);
            auto&& mocked_self{mock.get()};

            opt = mocked_self;
            CHECK_EQ(opt.ptr, &mocked_self);
            CHECK_EQ(static_cast<bool>(opt), true);
            CHECK_EQ(opt.has_value(), true);
            CHECK_EQ(opt.value_or(other), mocked_self);
            CHECK_EQ(opt.value(), mocked_self);
            CHECK_EQ(*opt, mocked_self);
            CHECK_EQ(opt->value, mocked_self.value);
            opt(), ::fakeit::Verify(method_call).Once();
            opt[0], ::fakeit::Verify(method_index).Once();
            CHECK_EQ(&static_cast<test_struct&>(opt), &mocked_self);
            CHECK_EQ(&static_cast<const test_struct&>(opt), &mocked_self);
            {
                auto opt_copy{opt};
                opt_copy.reset();
                CHECK_EQ(opt_copy.ptr, nullptr);
            }
            {
                auto opt_copy{opt};
                opt_copy = nullptr;
                CHECK_EQ(opt_copy.ptr, nullptr);
            }
        }
    }

    /// @test 测试union_wrapper能否正确包装以实现不自动构造析构
    REGISTER_TEST_CASE("union_wrapper" * ::doctest::description{"测试union_wrapper能否正确包装以实现不自动构造析构"})
    {
        static constinit auto ctor_cnt{0zu};
        static constinit auto copy_ctor_cnt{0zu};
        static constinit auto copy_assign_cnt{0zu};
        static constinit auto move_ctor_cnt{0zu};
        static constinit auto move_assign_cnt{0zu};
        static constinit auto dtor_cnt{0zu};
        static constinit auto three_way_compare_cnt{0zu};
        static constinit auto equal_compare_cnt{0zu};

        struct test_struct
        {
            test_struct() noexcept { ++ctor_cnt; }

            ~test_struct() noexcept { ++dtor_cnt; }

            test_struct(const test_struct& other [[maybe_unused]]) noexcept { ++copy_ctor_cnt; }

            test_struct(test_struct&& other [[maybe_unused]]) noexcept { ++move_ctor_cnt; }

            test_struct& operator= (const test_struct& other [[maybe_unused]]) noexcept
            {
                ++copy_assign_cnt;
                return *this;
            }

            test_struct& operator= (test_struct&& other [[maybe_unused]]) noexcept
            {
                ++move_assign_cnt;
                return *this;
            }

            auto operator<=> (const test_struct& other [[maybe_unused]]) const noexcept
            {
                ++three_way_compare_cnt;
                return ::std::strong_ordering::equal;
            }

            bool operator== (const test_struct& other [[maybe_unused]]) const noexcept
            {
                ++equal_compare_cnt;
                return true;
            }
        };

        ::SoC::union_wrapper<test_struct> wrapper{};
        SUBCASE("constructor and destructor")
        {
            CHECK_EQ(ctor_cnt, 0zu);
            CHECK_EQ(dtor_cnt, 0zu);
            ::new(&wrapper.value) test_struct{};
            CHECK_EQ(ctor_cnt, 1zu);
            CHECK_EQ(dtor_cnt, 0zu);
            wrapper.value.~test_struct();
            CHECK_EQ(ctor_cnt, 1zu);
            CHECK_EQ(dtor_cnt, 1zu);
        }

        // NOLINTBEGIN(clang-analyzer-cplusplus.Move,bugprone-use-after-move,hicpp-invalid-access-moved)

        SUBCASE("copy constructor")
        {
            // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
            ::SoC::union_wrapper _{wrapper};
            CHECK_EQ(copy_ctor_cnt, 1zu);
        }

        SUBCASE("copy assignment operator")
        {
            ::SoC::union_wrapper<test_struct> _{};
            _ = wrapper;
            CHECK_EQ(copy_assign_cnt, 1zu);
        }

        SUBCASE("move constructor")
        {
            ::SoC::union_wrapper _{::std::move(wrapper)};
            CHECK_EQ(move_ctor_cnt, 1zu);
        }

        SUBCASE("move assignment operator")
        {
            ::SoC::union_wrapper<test_struct> _{};
            _ = ::std::move(wrapper);
            CHECK_EQ(move_assign_cnt, 1zu);
        }

        SUBCASE("three-way comparison operator")
        {
            // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
            ::SoC::union_wrapper<test_struct> other{wrapper};
            wrapper <=> other;
            CHECK_EQ(three_way_compare_cnt, 1zu);
        }

        SUBCASE("equal comparison operator")
        {
            // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
            ::SoC::union_wrapper<test_struct> other{wrapper};
            auto _{wrapper == other};
            CHECK_EQ(equal_compare_cnt, 1zu);
        }

        // NOLINTEND(clang-analyzer-cplusplus.Move,bugprone-use-after-move,hicpp-invalid-access-moved)
    }

    /// @test 测试moveable_value能否正确在移动时清空原始值
    REGISTER_TEST_CASE("moveable_value" * ::doctest::description{"测试moveable_value能否正确在移动时清空原始值"})
    {
        struct test_struct
        {
            int value;

            bool operator== (const test_struct& other) const = default;
        };

        test_struct value{1};
        using moveable_value_t = ::SoC::moveable_value<test_struct*>;

        SUBCASE("default construct")
        {
            moveable_value_t moveable_value{};
            CHECK_EQ(moveable_value.value, nullptr);
        }

        SUBCASE("assign value")
        {
            moveable_value_t moveable_value{};
            moveable_value = &value;
            CHECK_EQ(moveable_value.value, &value);
        }

        moveable_value_t moveable_value{&value};
        SUBCASE("general operator")
        {
            CHECK_EQ(moveable_value.value, &value);
            CHECK_EQ(static_cast<test_struct*>(moveable_value), &value);
            CHECK_EQ(*moveable_value, value);
            CHECK_EQ(moveable_value->value, value.value);
            CHECK_EQ(moveable_value[0], value);
        }

        // 由于doctest框架下SUBCASE会重新初始化moveable_value，所以这里需要忽略clang-analyzer-cplusplus.Move警告
        // NOLINTBEGIN(clang-analyzer-cplusplus.Move,bugprone-use-after-move,hicpp-invalid-access-moved)

        SUBCASE("move construct")
        {
            moveable_value_t other{::std::move(moveable_value)};
            CHECK_EQ(other.value, &value);
            CHECK_EQ(moveable_value.value, nullptr);
        }

        SUBCASE("move assign")
        {
            moveable_value_t other{};
            other = ::std::move(moveable_value);
            CHECK_EQ(other.value, &value);
            CHECK_EQ(moveable_value.value, nullptr);
        }

        SUBCASE("copy construct")
        {
            moveable_value_t other{moveable_value};
            CHECK_EQ(other.value, &value);
            CHECK_EQ(moveable_value.value, &value);
        }

        SUBCASE("copy assign")
        {
            moveable_value_t other{};
            other = moveable_value;
            CHECK_EQ(other.value, &value);
            CHECK_EQ(moveable_value.value, &value);
        }

        // NOLINTEND(clang-analyzer-cplusplus.Move,bugprone-use-after-move,hicpp-invalid-access-moved)
    }
}
