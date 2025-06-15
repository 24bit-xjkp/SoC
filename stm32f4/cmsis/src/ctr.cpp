#include "common.hpp"

namespace SoC
{
    using cxa_atexit_callback_t = void (*)(void*);
    using cxa_atexit_callback_arg_t = void*;
    using init_fini_callback_t = void (*)();

    ::SoC::cxa_atexit_callback_t cxa_at_exit_callback_array[::SoC::max_cxa_at_exit_callback];
    ::SoC::cxa_atexit_callback_arg_t cxa_at_exit_callback_arg_array[::SoC::max_cxa_at_exit_callback];
    constinit ::std::size_t cxa_at_exit_callback_index{};
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
