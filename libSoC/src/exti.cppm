/**
 * @file exti.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 exti外设
 */

module;
#include <pch.hpp>
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

    ::SoC::syscfg::~syscfg() noexcept
    {
        if(need_stop_clock) { disable(); }
    }

    ::SoC::syscfg::syscfg(syscfg&& other) noexcept { other.need_stop_clock = false; }

    // NOLINTNEXTLINE
    void ::SoC::syscfg::enable() const noexcept { ::LL_APB2_GRP1_EnableClock(periph); }

    // NOLINTNEXTLINE
    void ::SoC::syscfg::disable() const noexcept { ::LL_APB2_GRP1_DisableClock(periph); }

    // NOLINTNEXTLINE
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

    /// 外部线中断触发源枚举到中断号枚举的映射表
    constexpr inline ::IRQn_Type irqn_table_0_15[]{::IRQn_Type::EXTI0_IRQn,
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
                                                   ::IRQn_Type::EXTI15_10_IRQn};

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

    /**
     * @brief 检查外部线中断触发源枚举是否有效
     *
     * @note 若该函数和SoC::exti_line_enum2irqn的返回值不同且该函数返回值不为空，则枚举值非法
     * @param line 外部线中断触发源枚举
     * @return 最高位中断线对应的枚举
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::IRQn_Type
        exti_line_enum_check(::SoC::exti_line::exti_line_enum line) noexcept
    {
        auto index{31 - ::std::countl_zero(::SoC::to_underlying(line))};
        if(index > 15) { return ::IRQn_Type{}; }
        else
        {
            return ::SoC::irqn_table_0_15[index];
        }
    }

    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line0) == ::IRQn_Type::EXTI0_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line1) == ::IRQn_Type::EXTI1_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line2) == ::IRQn_Type::EXTI2_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line3) == ::IRQn_Type::EXTI3_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line4) == ::IRQn_Type::EXTI4_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line5) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line6) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line7) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line8) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line9) == ::IRQn_Type::EXTI9_5_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line10) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line11) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line12) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line13) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line14) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line15) == ::IRQn_Type::EXTI15_10_IRQn);
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line16) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line17) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line18) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line19) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line20) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line21) == ::IRQn_Type{});
    static_assert(::SoC::exti_line_enum_check(::SoC::exti_line::exti_line_enum::line22) == ::IRQn_Type{});

    namespace
    {
        /**
         * @brief 外部线中断枚举单元测试
         *
         * @param line 外部线中断触发源枚举
         * @return 外部线中断触发源枚举是否有效
         */
        consteval inline auto exti_line_enum_unit_test(::SoC::exti_line::exti_line_enum line) noexcept
        {
            auto irqn{::SoC::exti_line_enum2irqn(line)};
            auto check{::SoC::exti_line_enum_check(line)};
            return check == irqn || check == ::IRQn_Type{};
        }

        using enum ::SoC::exti_line::exti_line_enum;
        static_assert(!::SoC::exti_line_enum_unit_test(line0 | line1));
        static_assert(!::SoC::exti_line_enum_unit_test(line0 | line5));
        static_assert(!::SoC::exti_line_enum_unit_test(line0 | line10));
        static_assert(!::SoC::exti_line_enum_unit_test(line5 | line10));
        static_assert(::SoC::exti_line_enum_unit_test(line5 | line6));
        static_assert(::SoC::exti_line_enum_unit_test(line15 | line10));
        static_assert(::SoC::exti_line_enum_unit_test(line0 | line16));
        static_assert(::SoC::exti_line_enum_unit_test(line5 | line16));
        static_assert(::SoC::exti_line_enum_unit_test(line10 | line16));
        static_assert(::SoC::exti_line_enum_unit_test(line22 | line16));
    }  // namespace

    ::SoC::exti_line::exti_line(::SoC::syscfg& syscfg,
                                ::SoC::gpio_port::port_enum gpio_port,
                                exti_line_enum line,
                                ::SoC::exti_trigger_source trigger_source) noexcept :
        gpio_port{static_cast<::SoC::detail::exti_gpio_port>(gpio_port)}, line{line}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(syscfg.is_enabled(), "使用外部线中断必须使能系统控制器"sv); }

        ::LL_SYSCFG_SetEXTISource(::SoC::to_underlying(gpio_port), ::SoC::to_underlying(line));
        set_trigger_source(trigger_source);
        irqn = ::SoC::exti_line_enum2irqn(line);
        if constexpr(::SoC::use_full_assert)
        {
            auto check{::SoC::exti_line_enum_check(line)};
            ::SoC::assert(check == irqn || check == ::IRQn_Type{}, "一个外部中断线对象只能对应一个中断向量"sv);
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

    void ::SoC::exti_line::set_trigger_source(::SoC::exti_trigger_source trigger_source) const noexcept
    {
        if(trigger_source & ::SoC::exti_trigger_source::rising) { ::LL_EXTI_EnableRisingTrig_0_31(::SoC::to_underlying(line.value)); }
        if(trigger_source & ::SoC::exti_trigger_source::falling) { ::LL_EXTI_EnableFallingTrig_0_31(::SoC::to_underlying(line.value)); }
    }

    void ::SoC::exti_line::clear_trigger_source() const noexcept
    {
        ::LL_EXTI_DisableRisingTrig_0_31(::SoC::to_underlying(line.value));
        ::LL_EXTI_DisableFallingTrig_0_31(::SoC::to_underlying(line.value));
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
            auto mask{::SoC::to_underlying(line.value)};
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
