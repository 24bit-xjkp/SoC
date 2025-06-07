#include "../include/exti.hpp"
#include "../include/nvic.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::exti_line::exti_line(::SoC::syscfg& syscfg,
                                ::SoC::gpio_port::port_enum gpio_port,
                                exti_line_enum line,
                                ::SoC::exti_trigger_source trigger_source) noexcept :
        gpio_port{static_cast<::SoC::detail::exti_gpio_port>(gpio_port)}, line{line}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(syscfg.is_enabled(), "使用外部线中断必须使能系统控制器"sv); }

        ::LL_SYSCFG_SetEXTISource(::std::to_underlying(gpio_port), ::std::to_underlying(line));
        set_trigger_source(trigger_source);
        if(line & line0) { irqn = ::IRQn_Type::EXTI0_IRQn; }
        else if(line & line1) { irqn = ::IRQn_Type::EXTI1_IRQn; }
        else if(line & line2) { irqn = ::IRQn_Type::EXTI2_IRQn; }
        else if(line & line3) { irqn = ::IRQn_Type::EXTI3_IRQn; }
        else if(line & line4) { irqn = ::IRQn_Type::EXTI4_IRQn; }
        else if(line & line5_9) { irqn = ::IRQn_Type::EXTI9_5_IRQn; }
        else if(line & line10_15) { irqn = ::IRQn_Type::EXTI15_10_IRQn; }
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
        if(trigger_source & ::SoC::exti_trigger_source::rising) { ::LL_EXTI_EnableRisingTrig_0_31(::std::to_underlying(line)); }
        if(trigger_source & ::SoC::exti_trigger_source::falling) { ::LL_EXTI_EnableFallingTrig_0_31(::std::to_underlying(line)); }
    }

    void ::SoC::exti_line::clear_trigger_source() const noexcept
    {
        ::LL_EXTI_DisableRisingTrig_0_31(::std::to_underlying(line));
        ::LL_EXTI_DisableFallingTrig_0_31(::std::to_underlying(line));
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

    ::SoC::exti_line::exti_line_enum(::SoC::exti_line::check_lines)(exti_line_enum lines) const noexcept
    {
        if(lines == default_lines) { return line; }
        else
        {
            auto value{::std::to_underlying(lines)};
            auto mask{::std::to_underlying(line)};
            if constexpr(::SoC::use_full_assert) { ::SoC::assert((value & mask) == value, "访问未绑定到当前对象的中断线"sv); }
            return lines;
        }
    }

    void ::SoC::exti_line::set_it(bool enable, exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        if(enable) { ::LL_EXTI_EnableIT_0_31(::std::to_underlying(lines)); }
        else
        {
            ::LL_EXTI_DisableIT_0_31(::std::to_underlying(lines));
        }
    }

    bool ::SoC::exti_line::get_it(exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        return ::LL_EXTI_IsEnabledIT_0_31(::std::to_underlying(lines));
    }

    bool ::SoC::exti_line::get_flag_it(exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        return ::LL_EXTI_IsActiveFlag_0_31(::std::to_underlying(lines));
    }

    void ::SoC::exti_line::clear_flag_it(exti_line_enum lines) const noexcept
    {
        lines = check_lines(lines);
        ::LL_EXTI_ClearFlag_0_31(::std::to_underlying(lines));
    }
}  // namespace SoC
