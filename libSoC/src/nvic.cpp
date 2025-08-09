/**
 * @file nvic.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 nvic中断控制器
 */

module SoC;
import :nvic;
import "../include/pch.hpp";

namespace SoC
{
    using namespace ::std::string_view_literals;

    void set_priority_group(::SoC::nvic_priority_group group) noexcept
    {
        ::NVIC_SetPriorityGrouping(::std::to_underlying(group));
    }

    ::SoC::nvic_priority_group get_priority_group() noexcept
    {
        return static_cast<::SoC::nvic_priority_group>(::NVIC_GetPriorityGrouping());
    }

    /**
     * @brief 获取优先级占用位数
     *
     * @return std::pair{抢占优先级占用位数, 响应优先级占用位数}
     */
    auto get_priority_bit() noexcept
    {
        auto preempt_bit{7zu - ::std::to_underlying(::SoC::get_priority_group())};
        auto sub_bit{4 - preempt_bit};
        return ::std::pair{preempt_bit, sub_bit};
    }

    ::std::size_t encode_priority(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept
    {
        auto&& [preempt_bit, sub_bit]{::SoC::get_priority_bit()};
        constexpr auto check{[](::std::size_t bit, ::std::size_t priority) static constexpr noexcept
                             {
                                 auto mask{(1zu << bit) - 1};
                                 return (priority & mask) == priority;
                             }};
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(check(preempt_bit, preempt_priority), "抢占优先级超出范围"sv);
            ::SoC::assert(check(sub_bit, sub_priority), "抢占优先级超出范围"sv);
        }
        return preempt_priority << sub_bit | sub_priority;
    }

    ::std::pair<::std::size_t, ::std::size_t> decode_priority(::std::size_t encoded_priority) noexcept
    {
        auto&& [_, sub_bit]{::SoC::get_priority_bit()};
        return {encoded_priority >> sub_bit, encoded_priority & ((1 << sub_bit) - 1)};
    }

    void set_priority(::IRQn_Type irqn, ::std::size_t encoded_priority) noexcept { ::NVIC_SetPriority(irqn, encoded_priority); }

    void set_priority(::IRQn_Type irqn, ::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept
    {
        ::SoC::set_priority(irqn, ::SoC::encode_priority(preempt_priority, sub_priority));
    }

    ::std::pair<::std::size_t, ::std::size_t> get_priority_decoded(::IRQn_Type irqn) noexcept
    {
        return ::SoC::decode_priority(::SoC::get_priority(irqn));
    }

    void enable_irq(::IRQn_Type irqn) noexcept { ::NVIC_EnableIRQ(irqn); }

    void disable_irq(::IRQn_Type irqn) noexcept { ::NVIC_DisableIRQ(irqn); }
}  // namespace SoC
