#include "../include/utils.hpp"

namespace SoC
{
    extern "C" void SysTick_Handler() noexcept { ::SoC::systick++; }
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
        auto end_tick{::SoC::systick + tick};
        // 清除溢出标记
        volatile auto _{SysTick->CTRL};

#pragma GCC unroll 0
        // 等待直到系统时刻到达预定值
        while(::SoC::systick < end_tick) { ::SoC::wait_for_interpret(); }
        // 自旋以补偿start_value带来的时间误差
        ::SoC::wait_until([start_value, end_tick] noexcept { return SysTick->VAL < start_value || ::SoC::systick > end_tick; });
    }
}  // namespace SoC::detail
