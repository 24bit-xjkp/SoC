/**
 * @file crt.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief crt实现
 */

module SoC.crt:crt_impl;
import :common;

namespace SoC
{
    using cxa_atexit_callback_t = void (*)(void*);
    using cxa_atexit_callback_arg_t = void*;
    using init_fini_callback_t = void (*)();

    ::SoC::cxa_atexit_callback_t cxa_at_exit_callback_array[::SoC::max_cxa_at_exit_callback];
    ::SoC::cxa_atexit_callback_arg_t cxa_at_exit_callback_arg_array[::SoC::max_cxa_at_exit_callback];
    constinit ::size_t cxa_at_exit_callback_index{};
    using cursor = ::SoC::cursor_t<::SoC::init_fini_callback_t>;

    /**
     * @brief 执行初始化或析构回调函数
     *
     * @param begin 回调函数数组首指针
     * @param end 回调函数数组尾哨位
     */
    void do_init_fini(::SoC::cursor begin, ::SoC::cursor end) noexcept
    {
#pragma GCC unroll 0
        while(begin != end) { (*begin++)(); }
    }
}  // namespace SoC

extern "C"
{
    // NOLINTBEGIN(bugprone-reserved-identifier)
    extern ::SoC::cursor __preinit_array_start;
    extern ::SoC::cursor __preinit_array_end;
    extern ::SoC::cursor __init_array_start;
    extern ::SoC::cursor __init_array_end;
    extern ::SoC::cursor __fini_array_start;
    extern ::SoC::cursor __fini_array_end;
    // NOLINTEND(bugprone-reserved-identifier)
}

namespace SoC
{
    /**
     * @brief 执行初始化回调函数
     *
     */
    void _init() noexcept
    {
        ::SoC::do_init_fini(auto(::__preinit_array_start), auto(::__preinit_array_end));
        ::SoC::do_init_fini(auto(::__init_array_start), auto(::__init_array_end));
    }

    /**
     * @brief 执行析构回调函数
     *
     */
    void _fini() noexcept
    {
#pragma GCC unroll 0
        // 逆序调用注册的析构函数
        // 遍历从最后一个注册的回调开始，直到索引 0
        for(auto i{::SoC::cxa_at_exit_callback_index - 1}; i != -1zu; --i)
        {
            // 执行带参数的析构回调
            (::SoC::cxa_at_exit_callback_array[i])(::SoC::cxa_at_exit_callback_arg_array[i]);
        }

        // 执行.fini_array段中的终止回调
        ::SoC::do_init_fini(auto(::__fini_array_start), auto(::__fini_array_end));
    }

    /**
     * @brief 在异常情况下终止程序
     *
     * @note 使用trap指令实现以最小化代码大小
     */
    extern "C" void abort() { __builtin_trap(); }
}  // namespace SoC
