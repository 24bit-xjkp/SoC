#include "../include/init.hpp"

namespace SoC
{
    /**
     * @brief 初始化系统时钟为168MHz
     *
     */
    void system_clock_init() noexcept
    {
        using namespace ::SoC::literal;
        ::LL_RCC_HSE_Enable();
        ::SoC::wait_until(::LL_RCC_HSE_IsReady);
        ::LL_SetFlashLatency(::SoC::rcc::sys_clock_freq);
        // PLL = HSE * N / M / R
        constexpr auto pll_pr{(::SoC::rcc::pll_pr - 2) / 2 << 16};
        ::LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, ::SoC::rcc::pll_m, ::SoC::rcc::pll_n, pll_pr);
        ::LL_RCC_PLL_Enable();
        ::SoC::wait_until(::LL_RCC_PLL_IsReady);
        // 以PLL为系统时钟
        ::LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
        ::SoC::wait_until([]() static noexcept { return ::LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL; });
        // AHB时钟=168MHz
        ::LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
        // APB1时钟=42MHz
        ::LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
        // APB2时钟=84MHz
        ::LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
        constexpr auto systick_cnt{::SoC::rcc::sys_clock_freq / ::SoC::rcc::sys_tick_freq};
        // 设置SysTick
        ::SysTick_Config(systick_cnt);
        ::SystemCoreClock = ::SoC::rcc::sys_clock_freq;
    }
}  // namespace SoC
