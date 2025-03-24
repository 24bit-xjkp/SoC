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
        constexpr auto frequency{::SoC::seconds{1}.duration_cast<::SoC::cycles>().rep};
        ::LL_SetFlashLatency(frequency);
        // PLL = HSE * N / M / R
        constexpr auto hse_frequency{HSE_VALUE};
        constexpr auto pll_m{8zu};
        constexpr auto pll_r{2zu};
        constexpr auto pll_in_frequency{hse_frequency / pll_m};
        constexpr auto pll_n{frequency / pll_in_frequency * pll_r};
        constexpr auto pllp_r{(pll_r - 2) / 2 << 16};
        ::LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, pll_m, pll_n, pllp_r);
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
        constexpr auto systick_cnt{::SoC::systicks{1}.duration_cast<::SoC::cycles>().rep};
        // 设置SysTick
        ::SysTick_Config(systick_cnt);
        ::SystemCoreClock = frequency;
    }
}  // namespace SoC
