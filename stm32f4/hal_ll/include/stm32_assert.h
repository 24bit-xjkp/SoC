#pragma once

#ifdef __cplusplus
extern "C"
#endif
[[noreturn]] void assert_failed(const char* file, int line, const char* func);
#define assert_param(expr) ((expr) ? (void)0U : assert_failed(__FILE__, __LINE__, __func__))
