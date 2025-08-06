#include "../include/io.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;
    using namespace ::SoC::literal;

#define ASSERT_FAILED_MESSAGE "\N{ESCAPE}[31m断言失败:\N{ESCAPE}[39m\r\n"

#ifdef USE_FULL_ASSERT
    extern "C" [[noreturn]] [[using gnu: noinline, cold]] void
        c_assert_failed(const char* file_name, ::std::uint32_t line, const char* function_name) noexcept
    {
        ::SoC::println(::SoC::log_device, ASSERT_FAILED_MESSAGE "文件: {}({}) `{}`"_fmt, file_name, line, function_name);
        ::SoC::fast_fail();
    }
#endif

    [[noreturn]] [[using gnu: noinline, cold]] void assert_failed(::std::string_view message,
                                                                  ::std::source_location location) noexcept
    {
        ::SoC::println(::SoC::log_device, ASSERT_FAILED_MESSAGE "{}: {}"_fmt, location, message);
        ::SoC::fast_fail();
    }

#undef MSG_START
#undef MSG_END

    void assert(bool expression, ::std::string_view message, ::std::source_location location) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
        }
    }

    void always_assert(bool expression, ::std::string_view message, ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }
}  // namespace SoC
