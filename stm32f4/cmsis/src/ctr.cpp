#include <cstddef>
#include <ranges>

namespace SoC
{
    using cxa_atexit_callback_t = void (*)(void*);
    using cxa_atexit_callback_arg_t = void*;
    using init_callback_t = void (*)();
    constexpr inline ::std::size_t max_cxa_at_exit_callback{32};
    ::SoC::cxa_atexit_callback_t cxa_at_exit_callback_array[::SoC::max_cxa_at_exit_callback];
    ::SoC::cxa_atexit_callback_arg_t cxa_at_exit_callback_arg_array[::SoC::max_cxa_at_exit_callback];
    ::std::size_t cxa_at_exit_callback_index{};
}  // namespace SoC

extern "C"
{
    extern ::SoC::init_callback_t __init_array_start[];
    extern ::SoC::init_callback_t __init_array_end[];

    int __cxa_atexit(::SoC::cxa_atexit_callback_t callback, ::SoC::cxa_atexit_callback_arg_t arg, void*) noexcept
    {
        auto current_index{::SoC::cxa_at_exit_callback_index++};
        if(current_index >= ::SoC::max_cxa_at_exit_callback) [[unlikely]] { while(true); }
        ::SoC::cxa_at_exit_callback_array[current_index] = callback;
        ::SoC::cxa_at_exit_callback_arg_array[current_index] = arg;
        return 0;
    }

    void _init() noexcept
    {
#pragma clang loop unroll_count(1)
        for(auto callback: ::std::ranges::subrange{__init_array_start, __init_array_end}) { callback(); }
    }

    void _fini() noexcept
    {
#pragma clang loop unroll_count(1)
        for(auto i{::SoC::cxa_at_exit_callback_index}; i != -1zu; --i)
        {
            (::SoC::cxa_at_exit_callback_array[i])(::SoC::cxa_at_exit_callback_arg_array[i]);
        }
    }
}
