/**
 * @file itanium_abi.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief itanium abi实现
 */

module SoC.crt:itanium_abi_impl;
import :common;

extern "C"
{
    // NOLINTBEGIN(bugprone-reserved-identifier)

    /**
     * @brief 注册程序退出时要执行的函数
     *
     * @param callback 回调函数
     * @param arg 回调函数参数
     * @param dso_handle 动态库柄
     * @return 注册是否成功，0表示成功
     */
    int __cxa_atexit(::SoC::cxa_atexit_callback_t callback,
                     ::SoC::cxa_atexit_callback_arg_t arg,
                     void* dso_handle [[maybe_unused]]) noexcept
    {
        auto current_index{::SoC::cxa_at_exit_callback_index++};
        if(current_index >= ::SoC::max_cxa_at_exit_callback) [[unlikely]]
        {
            while(true) { ; }
        }
        ::SoC::cxa_at_exit_callback_array[current_index] = callback;
        ::SoC::cxa_at_exit_callback_arg_array[current_index] = arg;
        return 0;
    }

    /**
     * @brief 用于填充虚表中纯虚函数对应的表项
     *
     */
    [[noreturn]] void __cxa_pure_virtual() noexcept
    {
        while(true) { ; }
    }

    /**
     * @brief 动态库柄，通过地址实现全局唯一
     *
     */
    extern constexpr const void* __dso_handle{static_cast<const void* const>(&__dso_handle)};

    // NOLINTEND(bugprone-reserved-identifier)
}
