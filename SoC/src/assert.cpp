#include "../include/utils.hpp"

namespace SoC
{
    [[noreturn]] void assert_failed(const char* file, ::std::uint_least32_t line, const char* func, ::std::uint_least32_t column)
    {
        // 断言失败:
        // 文件: file(line:column) `function`: 断言失败

        using namespace ::std::string_view_literals;
        constexpr auto assert_prefix{"\N{ESCAPE}[31m断言失败:\r\n文件: "sv};
        log_device.write(assert_prefix.begin(), assert_prefix.end());
        log_device.write(file, file + ::std::strlen(file));
        // 填入行号和列号
        char buffer[32];
        buffer[0] = '(';
        auto end{::std::to_chars(buffer + 1, buffer + sizeof(buffer), line).ptr};
        *end++ = ':';
        end = ::std::to_chars(end, buffer + sizeof(buffer), column).ptr;
        // 填入分隔符
        ::std::memcpy(end, ") `", 4);
        end += 3;
        log_device.write(buffer, end);
        // 写入函数名
        log_device.write(func, func + ::std::strlen(func));
        constexpr auto assert_suffix{"`: 断言失败\r\n\N{ESCAPE}[39m"sv};
        log_device.write(assert_suffix.begin(), assert_suffix.end());

        while(true) { __WFI(); }
    }

    void assert(bool expression, ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]]
        {
            ::SoC::assert_failed(location.file_name(), location.line(), location.function_name(), location.column());
        }
    }
}  // namespace SoC
