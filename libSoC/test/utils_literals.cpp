/**
 * @file utils_literals.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief SoC实用字面量部分单元测试
 */

import "test_framework.hpp";
import SoC.unit_test;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("utils_literals/" NAME)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE_TEMPLATE(NAME, TYPE, ...) TEST_CASE_TEMPLATE("utils_literals/" NAME, TYPE, __VA_ARGS__)

namespace
{
    /// 每个微秒的时钟周期数
    constexpr auto cycles_per_us{::SoC::cycles::ratio::den};
}  // namespace

/// @test SoC实用字面量部分单元测试
TEST_SUITE("utils_literals")
{
    using namespace ::SoC::literal;

    /// @test 测试同单位时间段的运算符
    REGISTER_TEST_CASE_TEMPLATE("duration_same_unit_operator" * ::doctest::description{"测试同单位时间段的运算符"},
                                duration,
                                ::SoC::seconds,
                                ::SoC::systicks,
                                ::SoC::milliseconds,
                                ::SoC::microseconds,
                                ::SoC::cycles)
    {
        duration one{1};
        duration other{2};

        CHECK_EQ(one + other, duration{3});
        CHECK_EQ(other - one, duration{1});
        CHECK_LT(one, other);
        CHECK_GT(other, one);
        CHECK_NE(one, other);
        CHECK_EQ(one.template duration_cast<duration>(), one);
        CHECK_EQ(other.template duration_cast<duration>(), other);
    }

    /// @test 测试duration字面量是否正确
    REGISTER_TEST_CASE("duration_literal" * ::doctest::description{"测试duration字面量是否正确"})
    {
        CHECK_EQ(1000_cycle, ::SoC::cycles{1000});
        CHECK_EQ(1e3_cycle, ::SoC::cycles{1000});
        CHECK_EQ(1.4_cycle, ::SoC::cycles{1});
        CHECK_EQ(1.5_cycle, ::SoC::cycles{2});
        CHECK_EQ(1.6_cycle, ::SoC::cycles{2});
        CHECK_EQ(1_Kcycle, ::SoC::cycles{1000});
        CHECK_EQ(1._Kcycle, ::SoC::cycles{1000});
        CHECK_EQ(1_Mcycle, ::SoC::cycles{1'000'000});
        CHECK_EQ(1._Mcycle, ::SoC::cycles{1'000'000});
        CHECK_EQ(1e-3_Mcycle, ::SoC::cycles{1000});

        CHECK_EQ(1_us, ::SoC::microseconds{1});
        CHECK_EQ(1._us, ::SoC::cycles{::cycles_per_us});

        CHECK_EQ(1_ms, ::SoC::milliseconds{1});
        CHECK_EQ(1._ms, ::SoC::microseconds{1000});

        CHECK_EQ(1_s, ::SoC::seconds{1});
        CHECK_EQ(1._s, ::SoC::milliseconds{1000});
    }

    /// @test 测试duration不同单位下的运算符是否正确
    REGISTER_TEST_CASE("duration_different_unit_operator" * ::doctest::description{"测试duration不同单位下的运算符是否正确"})
    {
        // 测试duration_cast和相等性比较运算符
        CHECK_EQ(1._cycle, ::SoC::cycles{1});
        CHECK_EQ(1._us, ::SoC::microseconds{1});
        CHECK_EQ(1._ms, ::SoC::milliseconds{1});
        CHECK_EQ(1._s, ::SoC::seconds{1});

        // 测试加法运算符
        CHECK_EQ(1_us + 1_cycle, ::SoC::cycles{::cycles_per_us + 1});
        CHECK_EQ(1_ms + 1_us, ::SoC::microseconds{1001});
        CHECK_EQ(1_s + 1_ms, ::SoC::milliseconds{1001});

        // 测试减法运算符
        CHECK_EQ(1_us - 1_cycle, ::SoC::cycles{::cycles_per_us - 1});
        CHECK_EQ(1_ms - 1_us, ::SoC::microseconds{999});
        CHECK_EQ(1_s - 1_ms, ::SoC::milliseconds{999});

        // 测试三路比较运算符
        CHECK_EQ(1_us <=> 1_cycle, ::std::strong_ordering::greater);
        CHECK_EQ(1_ms <=> 1_us, ::std::strong_ordering::greater);
        CHECK_EQ(1_s <=> 1_ms, ::std::strong_ordering::greater);
        CHECK_EQ(1_cycle <=> 1_us, ::std::strong_ordering::less);
        CHECK_EQ(1_us <=> 1_ms, ::std::strong_ordering::less);
        CHECK_EQ(1_ms <=> 1_s, ::std::strong_ordering::less);
    }

    /// @test 测试K和M字面量是否正确
    REGISTER_TEST_CASE("KM_literal" * ::doctest::description{"测试K和M字面量是否正确"})
    {
        CHECK_EQ(1_K, 1000);
        CHECK_EQ(1._K, 1000);
        CHECK_EQ(1.001_K, 1001);
        CHECK_EQ(1.0004_K, 1000);
        CHECK_EQ(1.0005_K, 1001);
        CHECK_EQ(1.0006_K, 1001);

        CHECK_EQ(1_M, 1'000'000);
        CHECK_EQ(1._M, 1'000'000);
        CHECK_EQ(1.001_M, 1'001'000);
        CHECK_EQ(1.e-3_M, 1000);
        CHECK_EQ(9.9999e-4_M, 1000);
    }
}
