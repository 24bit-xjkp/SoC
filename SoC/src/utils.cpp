#include "../include/utils.hpp"

namespace SoC
{
    extern "C" void SysTick_Handler() noexcept {}
}  // namespace SoC

namespace SoC::detail
{
    void wait_for(::SoC::cycles cycles) noexcept
    {
        if(cycles.rep <= 1) [[unlikely]] { return; }
        else
        {
            auto cnt{cycles.rep >> 1};
            if constexpr(::SoC::current_build_mode == ::SoC::build_mode::debug)
            {
                // 可能是lld的bug，在使用#embed的情况下，使用汇编会导致SoC_detail_wait_for_loop符号冲突
                asm volatile (
                "SoC_detail_wait_for_loop:\n"
                "subs %[counter], #1\n"
                "bne SoC_detail_wait_for_loop\n"
                : [counter] "+r"(cnt)
                :
                : "cc");
            }
            else
            {
                while(cnt-- != 0) { asm volatile(""); }
            }
        }
    }

    void wait_for(::SoC::microseconds microseconds) noexcept
    {
        auto ms{microseconds.rep};
        if(ms == 0) [[unlikely]] { return; }
        auto start_value{SysTick->VAL};
        // 清除溢出标记
        volatile auto _{SysTick->CTRL};
        while(ms != 0)
        {
#ifdef __clang__
            ::__wfi();
#else
            __WFI();
#endif
            if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
            {
                ::SoC::wait_until([start_value] noexcept { return SysTick->VAL < start_value; });
                ms--;
            }
        }
    }
}  // namespace SoC::detail
