#pragma once

#ifdef __cplusplus
#include <cstdint>
namespace SoC
{
    /**
     * @brief 适用于stm32 hal/ll库的断言接口
     *
     * @param file_name 文件名
     * @param line 行号
     * @param function_name 函数名
     */
    extern "C" [[noreturn]] [[using gnu: noinline, cold]] void
        c_assert_failed(const char* file_name, ::std::uint32_t line, const char* function_name) noexcept;
}  // namespace SoC
#else
    #include <stdint.h>
[[noreturn]] [[gnu::noinline]] [[gnu::cold]] void c_assert_failed(const char* file_name, uint32_t line, const char* function_name);
#endif
#define assert_param(expr) ((expr) ? (void)0U : c_assert_failed(__FILE__, __LINE__, __func__))
