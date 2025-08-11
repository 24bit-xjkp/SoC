/**
 * @file exti.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 exti外设
 */

module;
#include <pch.hpp>
module SoC;

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::syscfg::syscfg() noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此系统控制器不应处于使能状态"sv); }
        enable();
    }

    ::SoC::syscfg::~syscfg() noexcept
    {
        if(need_stop_clock) { disable(); }
    }

    ::SoC::syscfg::syscfg(syscfg&& other) noexcept { other.need_stop_clock = false; }

    void ::SoC::syscfg::enable() const noexcept { ::LL_APB2_GRP1_EnableClock(periph); }

    void ::SoC::syscfg::disable() const noexcept { ::LL_APB2_GRP1_DisableClock(periph); }

    bool ::SoC::syscfg::is_enabled() const noexcept { return ::LL_APB2_GRP1_IsEnabledClock(periph); }
}  // namespace SoC

namespace SoC
{
    /**
     * @brief 萃取外部线中断触发源枚举
     *
     * @param value 外部线中断触发源枚举
     * @param mask 掩码
     * @return 外部线中断触发源枚举是否包含在掩码中
     */
    constexpr inline bool operator& (::SoC::exti_trigger_source value, ::SoC::exti_trigger_source mask) noexcept
    {
        return ::SoC::to_underlying(value) & ::SoC::to_underlying(mask);
    }

    ::SoC::exti_line::exti_line(::SoC::syscfg& syscfg,
                                ::SoC::gpio_port::port_enum gpio_port,
                                exti_line_enum line,
                                ::SoC::exti_trigger_source trigger_source) noexcept :
        gpio_port{static_cast<::SoC::detail::exti_gpio_port>(gpio_port)}, line{line}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(syscfg.is_enabled(), "使用外部线中断必须使能系统控制器"sv); }

        ::LL_SYSCFG_SetEXTISource(::SoC::to_underlying(gpio_port), ::SoC::to_underlying(line));
        set_trigger_source(trigger_source);
        if(line & line0) { irqn = ::IRQn_Type::EXTI0_IRQn; }
        else if(line & line1) { irqn = ::IRQn_Type::EXTI1_IRQn; }
        else if(line & line2) { irqn = ::IRQn_Type::EXTI2_IRQn; }
        else if(line & line3) { irqn = ::IRQn_Type::EXTI3_IRQn; }
        else if(line & line4) { irqn = ::IRQn_Type::EXTI4_IRQn; }
        else if(line & line5_9) { irqn = ::IRQn_Type::EXTI9_5_IRQn; }
        else if(line & line10_15) { irqn = ::IRQn_Type::EXTI15_10_IRQn; }
        else
        {
            ::std::unreachable();
        }
    }

    ::SoC::exti_line::~exti_line() noexcept
    {
        if(line != exti_line_enum{})
        {
            clear_trigger_source();
            disable_irq();
        }
    }

    ::SoC::exti_line::exti_line(exti_line&& other) noexcept
    {
        ::std::memcpy(reinterpret_cast<void*>(this), &other, sizeof(other));
        other.line = exti_line_enum{};
    }

    void ::SoC::exti_line::set_trigger_source(::SoC::exti_trigger_source trigger_source) const noexcept
    {
        if(trigger_source & ::SoC::exti_trigger_source::rising) { ::LL_EXTI_EnableRisingTrig_0_31(::SoC::to_underlying(line)); }
        if(trigger_source & ::SoC::exti_trigger_source::falling) { ::LL_EXTI_EnableFallingTrig_0_31(::SoC::to_underlying(line)); }
    }

    void ::SoC::exti_line::clear_trigger_source() const noexcept
    {
        ::LL_EXTI_DisableRisingTrig_0_31(::SoC::to_underlying(line));
        ::LL_EXTI_DisableFallingTrig_0_31(::SoC::to_underlying(line));
    }

    void ::SoC::exti_line::enable_irq(::std::size_t encoded_priority) const noexcept
    {
        ::SoC::enable_irq(irqn);
        ::SoC::set_priority(irqn, encoded_priority);
    }

    void ::SoC::exti_line::enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) const noexcept
    {
        ::SoC::enable_irq(irqn);
        ::SoC::set_priority(irqn, preempt_priority, sub_priority);
    }

    void ::SoC::exti_line::disable_irq() const noexcept { ::SoC::disable_irq(irqn); }

    ::SoC::exti_line::exti_line_enum(::SoC::exti_line::check_lines)(exti_line_enum lines,
                                                                    ::std::source_location location) const noexcept
    {
        if(lines == default_lines) { return line; }
        else
        {
            auto value{::SoC::to_underlying(lines)};
            auto mask{::SoC::to_underlying(line)};
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert((value & mask) == value, "访问未绑定到当前对象的中断线"sv, location);
            }
            return lines;
        }
    }

    void ::SoC::exti_line::set_it(bool enable, exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        if(enable) { ::LL_EXTI_EnableIT_0_31(::SoC::to_underlying(lines)); }
        else
        {
            ::LL_EXTI_DisableIT_0_31(::SoC::to_underlying(lines));
        }
    }

    bool ::SoC::exti_line::get_it(exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        return ::LL_EXTI_IsEnabledIT_0_31(::SoC::to_underlying(lines));
    }

    bool ::SoC::exti_line::get_flag_it(exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        return ::LL_EXTI_IsActiveFlag_0_31(::SoC::to_underlying(lines));
    }

    void ::SoC::exti_line::clear_flag_it(exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        ::LL_EXTI_ClearFlag_0_31(::SoC::to_underlying(lines));
    }
}  // namespace SoC
