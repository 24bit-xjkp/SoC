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

        /// @brief 系统时钟频率
        constexpr inline auto sys_clock_freq{::SoC::seconds{1}.duration_cast<::SoC::cycles>().rep};

        /// @brief 系统时刻频率
        constexpr inline auto sys_tick_freq{::SoC::seconds{1}.duration_cast<::SoC::systicks>().rep};

        /// @brief 外部高速晶振频率
        constexpr inline ::std::size_t hse_freq{HSE_VALUE};

        /// @brief 分频后输入pll的频率
        constexpr inline auto pll_input_freq{1_M};

        /// @brief pll输入分频数
        constexpr inline auto pll_m{::SoC::rcc::hse_freq / ::SoC::rcc::pll_input_freq};

        /// @brief pll输出分频数
        constexpr inline auto pll_pr{2zu};

        /// @brief pll倍频数
        constexpr inline auto pll_n{::SoC::rcc::sys_clock_freq / ::SoC::rcc::pll_input_freq * ::SoC::rcc::pll_pr};

        /// @brief AHB总线频率
        constexpr inline auto ahb_freq{::SoC::rcc::sys_clock_freq};

        /// @brief APB1总线频率
        constexpr inline auto apb1_freq{::SoC::rcc::sys_clock_freq / 4};

        /// @brief APB2总线频率
        constexpr inline auto apb2_freq{::SoC::rcc::sys_clock_freq / 2};
    }  // namespace rcc
}  // namespace SoC
