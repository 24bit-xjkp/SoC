#include "../include/utils.hpp"

namespace SoC
{
    extern "C" [[noreturn]] void c_assert_failed() noexcept { ::SoC::fast_fail(); }

    /**
     * @brief 将消息写入日志设备
     *
     * @param message 要写入的消息
     */
    void write_string(const ::std::string_view message) noexcept { ::SoC::log_device.write(message.begin(), message.end()); }

    [[noreturn, gnu::noinline]] void assert_failed(const ::std::string_view message, ::std::source_location location) noexcept
    {
        // 断言失败:
        // 文件: file(line:column) `function`: message

        using namespace ::std::string_view_literals;
        ::SoC::write_string("\N{ESCAPE}[31m断言失败:\r\n文件: "sv);
        // 填入文件名
        ::SoC::write_string(location.file_name());
        // 填入行号和列号
        char buffer[32];
        buffer[0] = '(';
        auto end{::std::to_chars(buffer + 1, buffer + sizeof(buffer), location.line()).ptr};
        *end++ = ':';
        end = ::std::to_chars(end, buffer + sizeof(buffer), location.column()).ptr;
        // 填入分隔符
        ::std::memcpy(end, ") `", 4);
        end += 3;
        log_device.write(buffer, end);
        // 填入函数名
        ::SoC::write_string(location.function_name());
        ::SoC::write_string("`: "sv);
        // 填入消息
        ::SoC::write_string(message);
        // 填入后缀
        ::SoC::write_string("\r\n\N{ESCAPE}[39m"sv);

        ::SoC::fast_fail();
    }

    void assert(bool expression, const ::std::string_view message, ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }
}  // namespace SoC
