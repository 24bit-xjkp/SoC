#pragma once
#include "utils.hpp"

namespace SoC
{
    /**
     * @brief 初始化系统时钟
     *
     */
    void system_clock_init() noexcept;

    namespace rcc
    {
        using namespace ::SoC::literal;

        /// 系统时钟频率
        constexpr inline auto sys_clock_freq{::SoC::seconds{1}.duration_cast<::SoC::cycles>().rep};

        /// 系统时刻频率
        constexpr inline auto sys_tick_freq{::SoC::seconds{1}.duration_cast<::SoC::systicks>().rep};

        /// 外部高速晶振频率
        constexpr inline ::std::size_t hse_freq{HSE_VALUE};

        /// 分频后输入pll的频率
        constexpr inline auto pll_input_freq{1_M};

        /// pll输入分频数
        constexpr inline auto pll_m{::SoC::rcc::hse_freq / ::SoC::rcc::pll_input_freq};

        /// pll输出分频数
        constexpr inline auto pll_pr{2zu};

        /// pll倍频数
        constexpr inline auto pll_n{::SoC::rcc::sys_clock_freq / ::SoC::rcc::pll_input_freq * ::SoC::rcc::pll_pr};

        /// AHB总线频率
        constexpr inline auto ahb_freq{::SoC::rcc::sys_clock_freq};

        /// APB1总线频率
        constexpr inline auto apb1_freq{::SoC::rcc::ahb_freq / 4};

        /// APB2总线频率
        constexpr inline auto apb2_freq{::SoC::rcc::ahb_freq / 2};

        namespace detail
        {
            /**
             * @brief 计算定时器频率
             *
             * @tparam bus_freq 总线时钟频率
             * @return 总线上定时器的时钟频率
             */
            template <::std::size_t bus_freq>
            consteval inline auto get_tim_freq() noexcept
            {
                // APB总线预分频系数>1则定时器时钟频率翻倍
                return ::SoC::rcc::ahb_freq / bus_freq > 1 ? bus_freq * 2 : bus_freq;
            }
        }  // namespace detail

        /// APB1总线定时器时钟频率
        constexpr inline auto apb1_tim_freq{::SoC::rcc::detail::get_tim_freq<::SoC::rcc::apb1_freq>()};

        /// APB2总线定时器时钟频率
        constexpr inline auto apb2_tim_freq{::SoC::rcc::detail::get_tim_freq<::SoC::rcc::apb2_freq>()};

        /// ADC时钟频率
        constexpr inline auto adc_freq{::SoC::rcc::apb2_freq / 2};
    }  // namespace rcc
}  // namespace SoC
