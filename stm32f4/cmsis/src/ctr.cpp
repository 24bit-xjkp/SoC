#include "../include/startup.hpp"

namespace SoC
{
    using cxa_atexit_callback_t = void (*)(void*);
    using cxa_atexit_callback_arg_t = void*;
    using init_fini_callback_t = void (*)();

    constexpr inline ::std::size_t max_cxa_at_exit_callback{32};
    ::SoC::cxa_atexit_callback_t cxa_at_exit_callback_array[::SoC::max_cxa_at_exit_callback];
    ::SoC::cxa_atexit_callback_arg_t cxa_at_exit_callback_arg_array[::SoC::max_cxa_at_exit_callback];
    ::std::size_t cxa_at_exit_callback_index{};
    using cursor = ::SoC::cursor_t<::SoC::init_fini_callback_t>;

    void do_init_fini(::SoC::cursor begin, ::SoC::cursor end) noexcept
    {
#pragma GCC unroll 0
        while(begin != end) { (*begin++)(); }
    }
}  // namespace SoC

extern "C"
{
    extern ::SoC::cursor __preinit_array_start;
    extern ::SoC::cursor __preinit_array_end;
    extern ::SoC::cursor __init_array_start;
    extern ::SoC::cursor __init_array_end;
    extern ::SoC::cursor __fini_array_start;
    extern ::SoC::cursor __fini_array_end;

    int __cxa_atexit(::SoC::cxa_atexit_callback_t callback, ::SoC::cxa_atexit_callback_arg_t arg, void*) noexcept
    {
        auto current_index{::SoC::cxa_at_exit_callback_index++};
        if(current_index >= ::SoC::max_cxa_at_exit_callback) [[unlikely]] { while(true); }
        ::SoC::cxa_at_exit_callback_array[current_index] = callback;
        ::SoC::cxa_at_exit_callback_arg_array[current_index] = arg;
        return 0;
    }
}

namespace SoC
{
    void _init() noexcept
    {
        ::SoC::do_init_fini(::__preinit_array_start, ::__preinit_array_end);
        ::SoC::do_init_fini(::__init_array_start, ::__init_array_end);
    }

    void _fini() noexcept
    {
#pragma GCC unroll 0
        for(auto i{::SoC::cxa_at_exit_callback_index}; i != 0zu; --i)
        {
            (::SoC::cxa_at_exit_callback_array[i])(::SoC::cxa_at_exit_callback_arg_array[i]);
        }

        ::SoC::do_init_fini(::__fini_array_start, ::__fini_array_end);
    }
}  // namespace SoC
