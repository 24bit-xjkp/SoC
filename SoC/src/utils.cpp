#include "../include/utils.hpp"

namespace SoC
{
    extern "C" void SysTick_Handler() noexcept {}
}  // namespace SoC

namespace SoC::detail
{
    void wait_for(::SoC::cycles cycles) noexcept
    {
        // 每次循环约4周期
        for(auto i{0zu}; i < cycles.rep / 4; i++);
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
            __wfi();
            if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
            {
                ::SoC::wait_until([start_value] noexcept { return SysTick->VAL < start_value; });
                ms--;
            }
        }
    }
}  // namespace SoC::detail
