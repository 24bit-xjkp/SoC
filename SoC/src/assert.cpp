#include "../include/io.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    extern "C" [[noreturn]] [[using gnu: noinline, cold]] void
        c_assert_failed(const char* file_name, ::std::uint32_t line, const char* function_name) noexcept
    {
        ::SoC::println<"\N{ESCAPE}[31m文件: {}({}) `{}`\N{ESCAPE}[39m">(::SoC::log_device,
                                                                        ::std::string_view{file_name},
                                                                        line,
                                                                        ::std::string_view{function_name});
        ::SoC::fast_fail();
    }

    [[noreturn]] [[using gnu: noinline, cold]] void assert_failed(::std::string_view message,
                                                                  ::std::source_location location) noexcept
    {
        ::SoC::println<"\N{ESCAPE}[31m文件: {}({}:{}) `{}`: {}\N{ESCAPE}[39m">(::SoC::log_device,
                                                                               ::std::string_view{location.file_name()},
                                                                               location.line(),
                                                                               location.column(),
                                                                               ::std::string_view{location.function_name()},
                                                                               message);
        ::SoC::fast_fail();
    }

    void assert(bool expression, ::std::string_view message, ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }
}  // namespace SoC
