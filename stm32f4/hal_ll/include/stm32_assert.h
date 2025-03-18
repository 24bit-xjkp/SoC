#pragma once

#ifdef __cplusplus
namespace SoC
{
    extern "C" [[noreturn]] void
        assert_failed(const char* file, ::std::uint_least32_t line, const char* func, ::std::uint_least32_t column);
}
#else
    #include <stdint.h>
[[noreturn]] void assert_failed(const char* file, uint_least32_t line, const char* func, uint_least32_t column);
#endif
#define assert_param(expr) ((expr) ? (void)0U : assert_failed(__FILE__, __LINE__, __func__, 0))
