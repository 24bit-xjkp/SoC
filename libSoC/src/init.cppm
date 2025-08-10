/**
 * @file init.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32系统和和时钟初始化
 */

module;
#include <pch.hpp>
module SoC;
import :init;

namespace SoC
{
    /**
     * @brief 初始化系统时钟为144MHz
     *
     */
    void system_clock_init() noexcept
    {
        using namespace ::SoC::literal;
        ::LL_RCC_HSE_Enable();
        ::SoC::wait_until(::LL_RCC_HSE_IsReady);
        // PLL = HSE * N / M / R
        constexpr auto pll_pr{(::SoC::rcc::pll_pr - 2) / 2 << 16};
        ::LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, ::SoC::rcc::pll_m, ::SoC::rcc::pll_n, pll_pr);
        ::LL_RCC_PLL_Enable();
        ::SoC::wait_until(::LL_RCC_PLL_IsReady);
        ::LL_SetFlashLatency(::SoC::rcc::sys_clock_freq);
        // 以PLL为系统时钟
        ::LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
        ::SoC::wait_until([]() static noexcept { return ::LL_RCC_GetSysClkSource() == LL_RCC_SYS_CLKSOURCE_STATUS_PLL; });

        ::LL_RCC_SetAHBPrescaler(
            [] static consteval noexcept
            {
                switch(::SoC::rcc::sys_clock_freq / ::SoC::rcc::ahb_freq)
                {
                    case 1: return LL_RCC_SYSCLK_DIV_1;
                    case 2: return LL_RCC_SYSCLK_DIV_2;
                    case 4: return LL_RCC_SYSCLK_DIV_4;
                    case 8: return LL_RCC_SYSCLK_DIV_8;
                    case 16: return LL_RCC_SYSCLK_DIV_16;
                    case 64: return LL_RCC_SYSCLK_DIV_64;
                    case 128: return LL_RCC_SYSCLK_DIV_128;
                    case 256: return LL_RCC_SYSCLK_DIV_256;
                    case 512: return LL_RCC_SYSCLK_DIV_512;
                    default: ::std::unreachable();
                }
            }());

        ::LL_RCC_SetAPB1Prescaler(
            [] static consteval noexcept
            {
                switch(::SoC::rcc::ahb_freq / ::SoC::rcc::apb1_freq)
                {
                    case 1: return LL_RCC_APB1_DIV_1;
                    case 2: return LL_RCC_APB1_DIV_2;
                    case 4: return LL_RCC_APB1_DIV_4;
                    case 8: return LL_RCC_APB1_DIV_8;
                    case 16: return LL_RCC_APB1_DIV_16;
                    default: ::std::unreachable();
                }
            }());

        ::LL_RCC_SetAPB2Prescaler(
            [] static consteval noexcept
            {
                switch(::SoC::rcc::ahb_freq / ::SoC::rcc::apb2_freq)
                {
                    case 1: return LL_RCC_APB2_DIV_1;
                    case 2: return LL_RCC_APB2_DIV_2;
                    case 4: return LL_RCC_APB2_DIV_4;
                    case 8: return LL_RCC_APB2_DIV_8;
                    case 16: return LL_RCC_APB2_DIV_16;
                    default: ::std::unreachable();
                }
            }());

        ::LL_ADC_SetCommonClock(ADC,
                                [] static consteval noexcept
                                {
                                    switch(::SoC::rcc::apb2_freq / ::SoC::rcc::adc_freq)
                                    {
                                        case 2: return LL_ADC_CLOCK_SYNC_PCLK_DIV2;
                                        case 4: return LL_ADC_CLOCK_SYNC_PCLK_DIV4;
                                        case 6: return LL_ADC_CLOCK_SYNC_PCLK_DIV6;
                                        case 8: return LL_ADC_CLOCK_SYNC_PCLK_DIV8;
                                        default: ::std::unreachable();
                                    }
                                }());

        constexpr auto systick_cnt{::SoC::rcc::sys_clock_freq / ::SoC::rcc::sys_tick_freq};
        // 设置SysTick
        ::SysTick_Config(systick_cnt);
        ::SystemCoreClock = ::SoC::rcc::sys_clock_freq;
    }

    void enable_prefetch_cache() noexcept
    {
        if constexpr(::SoC::enable_icache) { ::LL_FLASH_EnableInstCache(); }
        if constexpr(::SoC::enable_dcache) { ::LL_FLASH_EnableDataCache(); }
        if constexpr(::SoC::enable_prefetch) { ::LL_FLASH_EnablePrefetch(); }
    }
}  // namespace SoC
