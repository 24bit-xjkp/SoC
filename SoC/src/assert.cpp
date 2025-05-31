#include "../include/io.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

#define MSG_START "\N{ESCAPE}[31m"
#define MSG_END "\N{ESCAPE}[39m"

#ifdef USE_FULL_ASSERT
    extern "C" [[noreturn]] [[using gnu: noinline, cold]] void
        c_assert_failed(const char* file_name, ::std::uint32_t line, const char* function_name) noexcept
    {
        ::SoC::println<MSG_START "文件: {}({}) `{}`" MSG_END>(::SoC::log_device, file_name, line, function_name);
        ::SoC::fast_fail();
    }
#endif

    [[noreturn]] [[using gnu: noinline, cold]] void assert_failed(::std::string_view message,
                                                                  ::std::source_location location) noexcept
    {
        ::SoC::println<MSG_START "文件: {}({}:{}) `{}`: {}" MSG_END>(::SoC::log_device,
                                                                     location.file_name(),
                                                                     location.line(),
                                                                     location.column(),
                                                                     location.function_name(),
                                                                     message);
        ::SoC::fast_fail();
    }

#undef MSG_START
#undef MSG_END

    void assert([[maybe_unused]] bool expression,
                [[maybe_unused]] ::std::string_view message,
                [[maybe_unused]] ::std::source_location location) noexcept
    {
#ifdef USE_FULL_ASSERT
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
#endif
    }

    void always_assert([[maybe_unused]] bool expression,
                       [[maybe_unused]] ::std::string_view message,
                       [[maybe_unused]] ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }
}  // namespace SoC
