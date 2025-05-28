#include "../include/io.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    namespace detail
    {
        /// 断言消息起始字符串，将终端配置为红色文字
        constexpr auto assert_msg_start{"\N{ESCAPE}[31m文件: "sv};

        /// 断言消息结束字符串，将终端文字颜色恢复默认
        constexpr auto assert_msg_end{"\N{ESCAPE}[39m"sv};
    }  // namespace detail

    extern "C" [[noreturn]] [[using gnu: noinline, cold]]  void c_assert_failed(const char* file_name, ::std::uint32_t line, const char* function_name) noexcept
    {
        // 断言内容如下：
        // 文件: file_name(line) `function_name`: message

        ::SoC::println(::SoC::log_device,
                       ::SoC::detail::assert_msg_start,
                       ::std::string_view{file_name},
                       "("sv,
                       line,
                       ") `"sv,
                       ::std::string_view{function_name},
                       "`: "sv,
                       "stm32 hal/ll 内部断言失败"sv,
                       ::SoC::detail::assert_msg_end);
        ::SoC::fast_fail();
    }

    [[noreturn]] [[using gnu: noinline, cold]] void assert_failed(::std::string_view message, ::std::source_location location) noexcept
    {
        // 断言内容如下：
        // 文件: file_name(line:column) `function_name`: message

        ::SoC::println(::SoC::log_device,
                       ::SoC::detail::assert_msg_start,
                       ::std::string_view{location.file_name()},
                       "("sv,
                       location.line(),
                       ":"sv,
                       location.column(),
                       ") `"sv,
                       ::std::string_view{location.function_name()},
                       "`: "sv,
                       message,
                       ::SoC::detail::assert_msg_end);
        ::SoC::fast_fail();
    }

    void assert(bool expression, ::std::string_view message, ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }
}  // namespace SoC
