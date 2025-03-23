#pragma once
#include <arm_acle.h>
#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_cortex.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_system.h>
#include <stm32f4xx_ll_tim.h>
#include <stm32f4xx_ll_usart.h>
#include <stm32f4xx_ll_utils.h>
#include <stm32f4xx_ll_rcc.h>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <charconv>
#include <cmath>
#include <concepts>
#include <ratio>
#include <functional>
#include <source_location>
#include <bit>
#include <span>
#include <ranges>
#include <string_view>
#include <cstring>
#include <stm32_assert.h>

namespace SoC
{
    /**
     * @brief 当前构建模式
     *
     */
    enum class build_mode : ::std::size_t
    {
        debug,
        release,
        releasedbg,
        minsizerel
    };

#ifdef SOC_BUILD_MODE_DEBUG
    inline constexpr auto current_build_mode{::SoC::build_mode::debug};
#elifdef SOC_BUILD_MODE_RELEASE
    inline constexpr auto current_build_mode{::SoC::build_mode::release};
#elifdef SOC_BUILD_MODE_RELEASEDBG
    inline constexpr auto current_build_mode{::SoC::build_mode::releasedbg};
#else
    inline constexpr auto current_build_mode{::SoC::build_mode::minsizerel};
#endif
}  // namespace SoC
