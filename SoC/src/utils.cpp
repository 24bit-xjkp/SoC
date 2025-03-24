#include "../include/utils.hpp"

namespace SoC
{
    extern "C" void SysTick_Handler() noexcept {}
}  // namespace SoC

namespace SoC::detail
{
    [[gnu::noinline]] void wait_for(::SoC::cycles cycles) noexcept
    {
        if(cycles.rep <= 1) { return; }
        else
        {
            auto cnt{cycles.rep >> 1};
            asm volatile (
                "SoC_detail_wait_for_loop:\n"
                "subs %[counter], #1\n"
                "bne SoC_detail_wait_for_loop\n"
                : [counter] "+r"(cnt)
                :
                : "cc");
        }
    }

    void wait_for(::SoC::systicks ticks) noexcept
    {
        auto tick{ticks.rep};
        if(tick == 0) [[unlikely]] { return; }
        auto start_value{SysTick->VAL};
        // 清除溢出标记
        volatile auto _{SysTick->CTRL};

#ifdef __clang__
    #pragma clang loop unroll_count(1)
#else
    #pragma GCC unroll 1
#endif
        while(tick != 0)
        {
#ifdef __clang__
            ::__wfi();
#else
            __WFI();
#endif
            if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
            {
                ::SoC::wait_until([start_value] noexcept { return SysTick->VAL < start_value || SysTick->CTRL; });
                tick--;
            }
        }
    }
}  // namespace SoC::detail
