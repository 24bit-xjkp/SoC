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
#include <stm32f4xx_ll_adc.h>
#include <stm32f4xx_ll_dma.h>
#include <stm32f4xx_ll_exti.h>
#include <stm32_assert.h>
#include "startup.hpp"

#include <atomic>
#include <version>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <charconv>
#include <cmath>
#include <numbers>
#include <concepts>
#include <ratio>
#include <memory>
#include <source_location>
#include <bit>
#include <span>
#include <ranges>
#include <string_view>
#include <cstring>
#include <compare>
#include <array>
#include <functional>
#include <vector>
#include <optional>
#include <initializer_list>

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
        minsizerel,

#ifdef SOC_BUILD_MODE_DEBUG
        current = debug
#elifdef SOC_BUILD_MODE_RELEASE
        current = release
#elifdef SOC_BUILD_MODE_RELEASEDBG
        current = releasedbg,
#elifdef SOC_BUILD_MODE_MINSIZEREL
        current = minsizerel
#else
#error Unknown build mode
#endif
    };

#ifdef USE_FULL_ASSERT
    /// 是否启用全部的断言
    constexpr inline auto use_full_assert{true};
#else
    /// 是否启用全部的断言
    constexpr inline auto use_full_assert{false};
#endif
}  // namespace SoC
