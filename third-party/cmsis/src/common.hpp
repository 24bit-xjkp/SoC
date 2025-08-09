#pragma once
#include "../include/startup.hpp"

namespace SoC
{
    using cxa_atexit_callback_t = void (*)(void*);
    using cxa_atexit_callback_arg_t = void*;

    constexpr inline ::std::size_t max_cxa_at_exit_callback{32};
    extern ::SoC::cxa_atexit_callback_t cxa_at_exit_callback_array[::SoC::max_cxa_at_exit_callback];
    extern ::SoC::cxa_atexit_callback_arg_t cxa_at_exit_callback_arg_array[::SoC::max_cxa_at_exit_callback];
    extern ::std::size_t cxa_at_exit_callback_index;
}  // namespace SoC
