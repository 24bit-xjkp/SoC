#include "../include/io.hpp"

namespace SoC
{
    extern "C" [[noreturn]] void c_assert_failed() noexcept { ::SoC::fast_fail(); }

    [[noreturn, gnu::noinline]] void assert_failed(const ::std::string_view message, ::std::source_location location) noexcept
    {
        // 断言内容如下：
        // 文件: file_name(line:column) `function_name`: message

        using namespace ::std::string_view_literals;
        ::SoC::println(::SoC::log_device,
                       "\N{ESCAPE}[31m文件: "sv,
                       ::std::string_view{location.file_name()},
                       "("sv,
                       location.line(),
                       ":"sv,
                       location.column(),
                       ") `"sv,
                       ::std::string_view{location.function_name()},
                       "`: "sv,
                       message,
                       "\N{ESCAPE}[39m"sv);
        ::SoC::fast_fail();
    }

    void assert(bool expression, const ::std::string_view message, ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }
}  // namespace SoC
