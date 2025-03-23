#include "common.hpp"

int main();

namespace SoC
{
    extern "C"
    {
        [[noreturn]] void startup() noexcept;
        using cursor = ::SoC::cursor_t<::std::size_t>;

        extern ::SoC::cursor _sdata;
        extern ::SoC::cursor _edata;
        extern ::SoC::cursor _sidata;

        extern ::SoC::cursor _sbss;
        extern ::SoC::cursor _ebss;

        extern ::SoC::cursor _sccmram;
        extern ::SoC::cursor _eccmram;
        extern ::SoC::cursor _siccmram;
    }

    void copy(::SoC::cursor begin, ::SoC::cursor end, ::SoC::cursor from) noexcept
    {
#ifdef __clang__
    #pragma clang loop unroll_count(1)
#else
    #pragma GCC unroll 1
#endif
        while(begin != end) { *begin++ = *from++; }
    }

    void fill(::SoC::cursor begin, ::SoC::cursor end) noexcept
    {
#ifdef __clang__
    #pragma clang loop unroll_count(1)
#else
    #pragma GCC unroll 1
#endif
        while(begin != end) { *begin++ = 0; }
    }

    void startup() noexcept
    {
        ::SoC::SystemInit();
        ::SoC::copy(::SoC::_sdata, ::SoC::_edata, ::SoC::_sidata);
        ::SoC::fill(::SoC::_sbss, ::SoC::_ebss);
        ::SoC::copy(::SoC::_sccmram, ::SoC::_eccmram, ::SoC::_siccmram);

        ::SoC::_init();
        ::main();
        ::SoC::_fini();

        while(true);
    }
}  // namespace SoC
