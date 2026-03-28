/**
 * @file exti.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 exti外设
 */

module;
#include "pch.hpp"
module SoC:exti_impl;
import :exti;
import :nvic;

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::syscfg::syscfg() noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此系统控制器不应处于使能状态"sv); }
        enable();
    }

    ::SoC::syscfg::~syscfg() noexcept { release(); }

    ::SoC::syscfg::syscfg(syscfg&& other) noexcept : clock_enabled{::std::exchange(other.clock_enabled, false)} {}

    void ::SoC::syscfg::release() noexcept { disable(); }

    void ::SoC::syscfg::enable() noexcept
    {
        if(!clock_enabled)
        {
            ::LL_APB2_GRP1_EnableClock(periph);
            clock_enabled = true;
        }
    }

    void ::SoC::syscfg::disable() noexcept
    {
        if(clock_enabled)
        {
            ::LL_APB2_GRP1_DisableClock(periph);
            clock_enabled = false;
        }
    }

    bool ::SoC::syscfg::is_enabled() noexcept { return ::LL_APB2_GRP1_IsEnabledClock(periph) != 0; }
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
        return (::SoC::to_underlying(value) & ::SoC::to_underlying(mask)) !=
               ::std::underlying_type_t<::SoC::exti_trigger_source>{};
    }

    /// 外部线中断触发源枚举到中断号枚举的映射表
    constexpr inline ::std::array irqn_table_0_15{
        ::IRQn_Type::EXTI0_IRQn,
        ::IRQn_Type::EXTI1_IRQn,
        ::IRQn_Type::EXTI2_IRQn,
        ::IRQn_Type::EXTI3_IRQn,
        ::IRQn_Type::EXTI4_IRQn,
        ::IRQn_Type::EXTI9_5_IRQn,
        ::IRQn_Type::EXTI9_5_IRQn,
        ::IRQn_Type::EXTI9_5_IRQn,
        ::IRQn_Type::EXTI9_5_IRQn,
        ::IRQn_Type::EXTI9_5_IRQn,
        ::IRQn_Type::EXTI15_10_IRQn,
        ::IRQn_Type::EXTI15_10_IRQn,
        ::IRQn_Type::EXTI15_10_IRQn,
        ::IRQn_Type::EXTI15_10_IRQn,
        ::IRQn_Type::EXTI15_10_IRQn,
        ::IRQn_Type::EXTI15_10_IRQn,
    };

    /**
     * @brief 外部线中断触发源枚举到中断号枚举的映射
     *
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::IRQn_Type
        exti_line_enum2irqn(::SoC::exti_line::exti_line_enum line) noexcept
    {
        auto index{::std::countr_zero(::SoC::to_underlying(line))};
        if(index > 15) { return ::IRQn_Type{}; }
        else
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            return ::SoC::irqn_table_0_15[index];
        }
    }

    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line0) == ::IRQn_Type::EXTI0_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line1) == ::IRQn_Type::EXTI1_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line2) == ::IRQn_Type::EXTI2_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line3) == ::IRQn_Type::EXTI3_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line4) == ::IRQn_Type::EXTI4_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line5) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line6) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line7) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line8) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line9) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line10) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line11) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line12) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line13) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line14) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line15) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line16) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line17) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line18) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line19) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line20) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line21) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum2irqn(::SoC::exti_line::exti_line_enum::line22) == ::IRQn_Type{});

    ::SoC::exti_line::exti_line(::SoC::syscfg& syscfg,
                                exti_line_enum line,
                                ::SoC::gpio_port::port_enum gpio_port,
                                ::SoC::exti_trigger_source trigger_source) noexcept :
        line{line}, irqn{::SoC::exti_line_enum2irqn(line)}, gpio_port{static_cast<::SoC::detail::exti_gpio_port>(gpio_port)},
        trigger_source{trigger_source}
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(syscfg.is_enabled(), "使用外部线中断必须使能系统控制器"sv);
            ::SoC::assert(::std::has_single_bit(::SoC::to_underlying(line)), "外部中断线对象只能对应一根中断线"sv);
            ::SoC::assert(!is_enabled(), "初始化前外部中断线不应处于使能状态"sv);
        }

        set_gpio_port(gpio_port);
        // 已断言trigger_source为none，可以直接enable而无需先清空可能已设置的位
        enable();
    }

    ::SoC::exti_line::~exti_line() noexcept { release(); }

    ::SoC::exti_line::exti_line(exti_line&& other) noexcept
    {
        if(this == &other) { return; }
        gpio_port = other.gpio_port;
        line = ::std::exchange(other.line, invalid);
        irqn = other.irqn;
        trigger_source = other.trigger_source;
        is_irq_enabled = other.is_irq_enabled;
    }

    ::SoC::exti_line& ::SoC::exti_line::operator= (exti_line&& other) noexcept
    {
        if(this == &other) { return *this; }
        release();
        gpio_port = other.gpio_port;
        line = ::std::exchange(other.line, invalid);
        irqn = other.irqn;
        trigger_source = other.trigger_source;
        is_irq_enabled = other.is_irq_enabled;
        return *this;
    }

    void ::SoC::exti_line::release() noexcept
    {
        if(line != invalid)
        {
            set_it(false);
            clear_flag_it();
            disable_irq();
            disable();
            clear_gpio_port();
            line = invalid;
        }
    }

    void ::SoC::exti_line::set_trigger_source(::SoC::exti_trigger_source trigger_source) noexcept
    {
        this->trigger_source = trigger_source;
        // 清空可能已设置的位
        disable();
        enable();
    }

    void ::SoC::exti_line::enable() const noexcept
    {
        ::std::size_t trigger_source_value{::SoC::to_underlying(trigger_source)};
        auto line_value{::SoC::to_underlying(line)};
        EXTI->RTSR |= (0zu - (trigger_source_value & 1zu)) & line_value;
        EXTI->FTSR |= (0zu - (trigger_source_value >> 1zu)) & line_value;
    }

    void ::SoC::exti_line::disable() const noexcept
    {
        auto line_mask{~::SoC::to_underlying(line)};
        EXTI->RTSR &= line_mask;
        EXTI->FTSR &= line_mask;
    }

    void ::SoC::exti_line::set_gpio_port(::SoC::gpio_port::port_enum gpio_port) noexcept
    {
        this->gpio_port = static_cast<::SoC::detail::exti_gpio_port>(gpio_port);
        ::LL_SYSCFG_SetEXTISource(::SoC::to_underlying(gpio_port), ::SoC::to_underlying(line));
    }

    void ::SoC::exti_line::clear_gpio_port() noexcept
    {
        constexpr static auto default_port_value{0zu};
        gpio_port = ::SoC::detail::exti_gpio_port{default_port_value};
        ::LL_SYSCFG_SetEXTISource(default_port_value, ::SoC::to_underlying(line));
    }

    bool ::SoC::exti_line::is_enabled() const noexcept
    {
        auto line_mask{::SoC::to_underlying(line)};
        auto is_rising_enabled{EXTI->RTSR & line_mask};
        auto is_falling_enabled{EXTI->FTSR & line_mask};
        constexpr static auto none{::SoC::to_underlying(::SoC::exti_trigger_source::none)};
        return (is_rising_enabled | is_falling_enabled) != none;
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,readability-container-data-pointer)

    /**
     * @brief 将外部中断线枚举转换为NVIC侧中断引用计数器引用
     *
     * @param line 外部中断线枚举
     * @return NVIC侧中断引用计数器
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::uint8_t&
        exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum line) noexcept
    {
        auto index{static_cast<::std::uint8_t>(::std::countr_zero(::SoC::to_underlying(line)))};
        constexpr static auto line5_index{
            static_cast<::std::uint8_t>(::std::countr_zero(::SoC::to_underlying(::SoC::exti_line::line5))),
        };
        constexpr static auto line10_index{
            static_cast<::std::uint8_t>(::std::countr_zero(::SoC::to_underlying(::SoC::exti_line::line10))),
        };
        if(index < line5_index) { return ::SoC::detail::exti_line_irq_reference_counter[index]; }
        else if(index < line10_index) { return ::SoC::detail::exti_line_irq_reference_counter[5]; }
        else
        {
            return ::SoC::detail::exti_line_irq_reference_counter[6];
        }
    }

    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line0) ==
                  &::SoC::detail::exti_line_irq_reference_counter[0]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line1) ==
                  &::SoC::detail::exti_line_irq_reference_counter[1]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line2) ==
                  &::SoC::detail::exti_line_irq_reference_counter[2]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line3) ==
                  &::SoC::detail::exti_line_irq_reference_counter[3]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line4) ==
                  &::SoC::detail::exti_line_irq_reference_counter[4]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line5) ==
                  &::SoC::detail::exti_line_irq_reference_counter[5]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line6) ==
                  &::SoC::detail::exti_line_irq_reference_counter[5]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line7) ==
                  &::SoC::detail::exti_line_irq_reference_counter[5]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line8) ==
                  &::SoC::detail::exti_line_irq_reference_counter[5]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line9) ==
                  &::SoC::detail::exti_line_irq_reference_counter[5]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line10) ==
                  &::SoC::detail::exti_line_irq_reference_counter[6]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line11) ==
                  &::SoC::detail::exti_line_irq_reference_counter[6]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line12) ==
                  &::SoC::detail::exti_line_irq_reference_counter[6]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line13) ==
                  &::SoC::detail::exti_line_irq_reference_counter[6]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line14) ==
                  &::SoC::detail::exti_line_irq_reference_counter[6]);
    static_assert(&::SoC::exti_line_enum2irq_reference_counter(::SoC::exti_line::exti_line_enum::line15) ==
                  &::SoC::detail::exti_line_irq_reference_counter[6]);

    // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,readability-container-data-pointer)

    void ::SoC::exti_line::enable_irq(::std::size_t encoded_priority) noexcept
    {
        if(!is_irq_enabled)
        {
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert(irqn != ::IRQn_Type{}, "该外部中断线不支持NVIC中断，只支持事件"sv);
            }
            ++::SoC::exti_line_enum2irq_reference_counter(line);
            ::SoC::enable_irq(irqn);
            ::SoC::set_priority(irqn, encoded_priority);
            is_irq_enabled = true;
        }
    }

    void ::SoC::exti_line::enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept
    { enable_irq(::SoC::encode_priority(preempt_priority, sub_priority)); }

    void ::SoC::exti_line::disable_irq() noexcept
    {
        if(auto&& ref{::SoC::exti_line_enum2irq_reference_counter(line)}; is_irq_enabled && ref != 0)
        {
            if(--ref == 0) { ::SoC::disable_irq(irqn); }
            is_irq_enabled = false;
        }
    }

    void ::SoC::exti_line::set_it(bool enable) const noexcept
    {
        if(enable) { ::LL_EXTI_EnableIT_0_31(::SoC::to_underlying(line)); }
        else
        {
            ::LL_EXTI_DisableIT_0_31(::SoC::to_underlying(line));
        }
    }

    bool ::SoC::exti_line::get_it() const noexcept
    { return static_cast<bool>(::LL_EXTI_IsEnabledIT_0_31(::SoC::to_underlying(line))); }

    bool ::SoC::exti_line::get_flag_it() const noexcept
    { return static_cast<bool>(::LL_EXTI_IsActiveFlag_0_31(::SoC::to_underlying(line))); }

    void ::SoC::exti_line::clear_flag_it() const noexcept { ::LL_EXTI_ClearFlag_0_31(::SoC::to_underlying(line)); }

    bool ::SoC::exti_line::is_it_enabled() const noexcept { return get_it() && get_flag_it(); }
}  // namespace SoC
