#include "../include/pch.hpp"
#include "../include/utils.hpp"

namespace SoC
{
    [[noreturn]] void assert_failed(const char* file [[maybe_unused]],
                                    ::std::uint_least32_t line [[maybe_unused]],
                                    const char* func [[maybe_unused]],
                                    ::std::uint_least32_t column [[maybe_unused]])
    {
        while(true);
    }

    void assert(bool expression, ::std::source_location location) noexcept
    {
        if(!expression) [[unlikely]]
        {
            ::SoC::assert_failed(location.file_name(), location.line(), location.function_name(), location.column());
        }
    }
}  // namespace SoC
