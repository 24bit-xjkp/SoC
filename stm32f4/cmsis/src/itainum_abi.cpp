#include "common.hpp"

extern "C"
{
    int __cxa_atexit(::SoC::cxa_atexit_callback_t callback, ::SoC::cxa_atexit_callback_arg_t arg, void*) noexcept
    {
        auto current_index{::SoC::cxa_at_exit_callback_index++};
        if(current_index >= ::SoC::max_cxa_at_exit_callback) [[unlikely]] { while(true); }
        ::SoC::cxa_at_exit_callback_array[current_index] = callback;
        ::SoC::cxa_at_exit_callback_arg_array[current_index] = arg;
        return 0;
    }

    [[noreturn]] void __cxa_pure_virtual() noexcept { while(true); }

    void* __dso_handle;
}
