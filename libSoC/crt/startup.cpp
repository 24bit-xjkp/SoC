/**
 * @file startup.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 平台无关的启动模块
 */

module SoC.crt:startup_impl;
import :common;

namespace SoC
{
    using cursor = ::SoC::cursor_t<::size_t>;
    extern "C"
    {
        extern ::SoC::cursor _sdata;
        extern ::SoC::cursor _edata;
        extern ::SoC::cursor _sidata;

        extern ::SoC::cursor _sbss;
        extern ::SoC::cursor _ebss;

        extern ::SoC::cursor _sccmram;
        extern ::SoC::cursor _eccmram;
        extern ::SoC::cursor _siccmram;

        [[gnu::naked, noreturn]] void Reset_Handler() noexcept { asm volatile("ldr sp, =_estack\n" "b SoC_startup\n"); }
    }

    using no_optimize_cursor = volatile ::size_t*;

    void copy(::SoC::no_optimize_cursor begin, ::SoC::no_optimize_cursor end, ::SoC::no_optimize_cursor from) noexcept
    {
#pragma GCC unroll 0
        while(begin != end) { *begin++ = *from++; }
    }

    void fill(::SoC::no_optimize_cursor begin, ::SoC::no_optimize_cursor end) noexcept
    {
#pragma GCC unroll 0
        while(begin != end) { *begin++ = 0; }
    }

    extern "C" [[noreturn, gnu::used]] void SoC_startup() noexcept
    {
        ::SoC::SystemInit();
        ::SoC::copy(auto(::SoC::_sdata), auto(::SoC::_edata), auto(::SoC::_sidata));
        ::SoC::fill(auto(::SoC::_sbss), auto(::SoC::_ebss));
        ::SoC::copy(auto(::SoC::_sccmram), auto(::SoC::_eccmram), auto(::SoC::_siccmram));

        ::SoC::_init();
        ::main();
        ::SoC::_fini();

        while(true) { ; }
    }
}  // namespace SoC
