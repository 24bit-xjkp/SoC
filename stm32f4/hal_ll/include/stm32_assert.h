#pragma once

#ifdef __cplusplus
namespace SoC
{
    extern "C" [[noreturn]] void c_assert_failed() noexcept;
}
#else
    #include <stdint.h>
[[noreturn]] void c_assert_failed();
#endif
#define assert_param(expr) ((expr) ? (void)0U : c_assert_failed())
