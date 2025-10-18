/**
 * @file utils_base.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief SoC基本实用函数部分单元测试
 */

import "test_framework.hpp";
import SoC.unit_test;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("utils_base/" NAME)

namespace SoC::test
{
    /**
     * @brief 导出SoC::log_device_t中的符号用于测试
     *
     */
    extern "C++" struct log_device_t : ::SoC::log_device_t
    {
        using ::SoC::log_device_t::device;
        using ::SoC::log_device_t::log_device_t;
        using ::SoC::log_device_t::write_callback;
        using ::SoC::log_device_t::write_callback_t;
    };
}  // namespace SoC::test

/// @test SoC基本实用函数部分单元测试
TEST_SUITE("utils_base" * ::doctest::description{"SoC基本实用函数部分单元测试"})
{
    /// @test 测试is_build_mode能否正确判断构建模式
    REGISTER_TEST_CASE("is_build_mode" * ::doctest::description{"测试is_build_mode能否正确判断构建模式"})
    {
        using enum ::SoC::build_mode;

        CHECK_EQ(::SoC::is_build_mode(debug), current == debug);
        CHECK_EQ(::SoC::is_build_mode(release), current == release);
        CHECK_EQ(::SoC::is_build_mode(releasedbg), current == releasedbg);
        CHECK_EQ(::SoC::is_build_mode(minsizerel), current == minsizerel);
        CHECK_EQ(::SoC::is_build_mode(coverage), current == coverage);
        CHECK_EQ(::SoC::is_build_mode(current), true);
    }

    /// @test 测试wait_until能否正确等待条件成立
    REGISTER_TEST_CASE("wait_until" * ::doctest::description{"测试wait_until能否正确等待条件成立"})
    {
        ::std::atomic_flag flag{};
        ::std::jthread thread{[&flag]
                              {
                                  ::std::this_thread::sleep_for(::std::chrono::microseconds{1});
                                  flag.test_and_set();
                              }};
        // NOLINTNEXTLINE(cert-err58-cpp)
        ::SoC::wait_until([&flag]() { return flag.test(); });
        CHECK(flag.test());
    }

    /// @test 测试assert_failed能否正确抛出异常
    REGISTER_TEST_CASE("assert_failed" * ::doctest::description{"测试assert_failed能否正确抛出异常"})
    {
        constexpr auto* message{::SoC::detail::default_assert_message.data()};
        CHECK_THROWS_WITH_AS(::SoC::assert_failed(message, ::std::source_location::current()),
                             ::doctest::Contains{message},
                             ::SoC::assert_failed_exception);
    }

    /// @test 测试assert能否正确处理断言
    REGISTER_TEST_CASE("assert" * ::doctest::description{"测试assert能否正确处理断言"})
    {
        constexpr auto* message{::SoC::detail::default_assert_message.data()};

        CHECK_NOTHROW(::SoC::assert(true));
        CHECK_THROWS_WITH_AS(::SoC::assert(false), ::doctest::Contains{message}, ::SoC::assert_failed_exception);
    }

    // @test 测试always_assert能否正确处理断言
    REGISTER_TEST_CASE("always_assert" * ::doctest::description{"测试always_assert能否正确处理断言"})
    {
        constexpr auto* message{::SoC::detail::default_assert_message.data()};

        CHECK_NOTHROW(::SoC::always_assert(true));
        CHECK_THROWS_WITH_AS(::SoC::always_assert(false), ::doctest::Contains{message}, ::SoC::assert_failed_exception);
    }

    /// @test 测试fast_fail能否通过异常终止程序
    REGISTER_TEST_CASE("fast_fail" * ::doctest::description{"测试fast_fail能否通过异常终止程序"})
    {
        CHECK_THROWS_WITH_AS(::SoC::fast_fail(),
                             ::doctest::Contains{::SoC::detail::fast_fail_message.data()},
                             ::SoC::fast_fail_exception);
    }

    /// @test 测试always_check能否正确处理断言
    REGISTER_TEST_CASE("always_check" * ::doctest::description{"测试always_check能否正确处理断言"})
    {
        constexpr auto* message{::SoC::detail::default_assert_message.data()};

        CHECK_NOTHROW(::SoC::always_check(true));
        CHECK_THROWS_WITH_AS(::SoC::always_check(false), ::doctest::Contains{message}, ::SoC::assert_failed_exception);
    }

    /// @test 测试log_device_t能否正确处理日志设备
    REGISTER_TEST_CASE("log_device_t" * ::doctest::description{"测试log_device_t能否正确处理日志设备"})
    {
        ::SoC::test::log_device_t log_device{};

        struct device_t
        {
            const void* buffer{};
            const void* end{};

            static void write(void* device, const void* buffer, const void* end) noexcept
            {
                auto&& self{*static_cast<device_t*>(device)};
                self.buffer = buffer;
                self.end = end;
            }
        } device{};

        SUBCASE("set")
        {
            log_device.set(device.write, &device);
            CHECK_EQ(log_device.device, &device);
            CHECK_EQ(log_device.write_callback, device.write);
        }

        SUBCASE("get")
        {
            using pair = ::std::pair<::SoC::test::log_device_t::write_callback_t, void*>;

            // 未设置时，返回空指针
            CHECK_EQ(log_device.get(), pair{nullptr, nullptr});
            log_device.set(device.write, &device);
            // 设置后，返回正确的回调函数和设备指针
            CHECK_EQ(log_device.get(), pair{device.write, &device});
        }

        SUBCASE("write")
        {
            constexpr ::std::string_view buffer{"test"};
            constexpr auto* begin{buffer.begin()};
            constexpr auto* end{buffer.end()};

            // 未设置时，返回false
            CHECK_EQ(log_device.write(begin, end), false);
            CHECK_EQ(device.buffer, nullptr);
            CHECK_EQ(device.end, nullptr);

            log_device.set(device.write, nullptr);
            // 只设置了回调函数，未设置设备指针，返回false
            CHECK_EQ(log_device.write(begin, end), false);
            CHECK_EQ(device.buffer, nullptr);
            CHECK_EQ(device.end, nullptr);

            log_device.set(device.write, &device);
            // 正确设置后，返回true，且设备接收到正确的数据
            CHECK_EQ(log_device.write(begin, end), true);
            CHECK_EQ(device.buffer, begin);
            CHECK_EQ(device.end, end);
        }
    }

    /// @test 测试bit_cast能否正确转换数据
    REGISTER_TEST_CASE("bit_cast" * ::doctest::description{"测试bit_cast能否正确转换数据"})
    {
        constexpr auto value{1234.f};
        CHECK_EQ(::SoC::bit_cast<::std::uint32_t>(value), ::std::bit_cast<::std::uint32_t>(value));
    }

    /// @test 测试array_cast能否正确转换数据
    REGISTER_TEST_CASE("array_cast" * ::doctest::description{"测试array_cast能否正确转换数据"})
    {
        constexpr ::std::array<::std::uint32_t, 1> dst{0x01020304};
        if constexpr(::std::endian::native == ::std::endian::little)
        {
            CHECK_EQ(::SoC::array_cast<::std::uint32_t>({0x04, 0x03, 0x02, 0x01}), dst);
        }
        else
        {
            CHECK_EQ(::SoC::array_cast<::std::uint32_t>({0x01, 0x02, 0x03, 0x04}), dst);
        }
    }

    /// @test 测试round能否正确四舍五入浮点数
    REGISTER_TEST_CASE("round" * ::doctest::description{"测试round能否正确四舍五入浮点数"})
    {
        // 测试保留0位小数
        CHECK_EQ(::SoC::round<0>(1.0f), 1.0f);
        CHECK_EQ(::SoC::round<0>(1.1f), 1.0f);
        CHECK_EQ(::SoC::round<0>(1.5f), 2.0f);
        CHECK_EQ(::SoC::round<0>(1.9f), 2.0f);
        CHECK_EQ(::SoC::round<0>(-1.0f), -1.0f);
        CHECK_EQ(::SoC::round<0>(-1.1f), -1.0f);
        CHECK_EQ(::SoC::round<0>(-1.5f), -2.0f);
        CHECK_EQ(::SoC::round<0>(-1.9f), -2.0f);

        // 测试保留1位小数
        CHECK_EQ(::SoC::round<1>(1.0f), 1.0f);
        CHECK_EQ(::SoC::round<1>(1.01f), 1.0f);
        CHECK_EQ(::SoC::round<1>(1.05f), 1.1f);
        CHECK_EQ(::SoC::round<1>(1.09f), 1.1f);
        CHECK_EQ(::SoC::round<1>(-1.0f), -1.0f);
        CHECK_EQ(::SoC::round<1>(-1.01f), -1.0f);
        CHECK_EQ(::SoC::round<1>(-1.05f), -1.1f);
        CHECK_EQ(::SoC::round<1>(-1.09f), -1.1f);

        // 测试保留2位小数
        CHECK_EQ(::SoC::round<2>(1.0f), 1.0f);
        CHECK_EQ(::SoC::round<2>(1.001f), 1.0f);
        CHECK_EQ(::SoC::round<2>(1.005f), 1.01f);
        CHECK_EQ(::SoC::round<2>(1.009f), 1.01f);
        CHECK_EQ(::SoC::round<2>(-1.0f), -1.0f);
        CHECK_EQ(::SoC::round<2>(-1.001f), -1.0f);
        CHECK_EQ(::SoC::round<2>(-1.005f), -1.01f);
        CHECK_EQ(::SoC::round<2>(-1.009f), -1.01f);

        // 测试保留3位小数
        CHECK_EQ(::SoC::round<3>(1.0f), 1.0f);
        CHECK_EQ(::SoC::round<3>(1.0001f), 1.0f);
        CHECK_EQ(::SoC::round<3>(1.0005f), 1.001f);
        CHECK_EQ(::SoC::round<3>(1.0009f), 1.001f);
        CHECK_EQ(::SoC::round<3>(-1.0f), -1.0f);
        CHECK_EQ(::SoC::round<3>(-1.0001f), -1.0f);
        CHECK_EQ(::SoC::round<3>(-1.0005f), -1.001f);
        CHECK_EQ(::SoC::round<3>(-1.0009f), -1.001f);

        // 测试较大的数值
        CHECK_EQ(::SoC::round<0>(1234.5f), 1235.0f);
        CHECK_EQ(::SoC::round<1>(1234.56f), 1234.6f);
        CHECK_EQ(::SoC::round<2>(1234.567f), 1234.57f);
    }

    /// @test 测试to_underlying能否正确转换枚举值
    REGISTER_TEST_CASE("to_underlying" * ::doctest::description{"测试to_underlying能否正确转换枚举值"})
    {
        enum class test_enum : ::std::uint8_t
        {
            a = 0,
            b = 1,
            c = 2,
        };

        CHECK_EQ(::SoC::to_underlying(test_enum::a), 0u);
        CHECK_EQ(::SoC::to_underlying(test_enum::b), 1u);
        CHECK_EQ(::SoC::to_underlying(test_enum::c), 2u);
    }
}
