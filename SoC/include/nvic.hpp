#pragma once
#include "pch.hpp"

namespace SoC
{
    /**
     * @brief 中断优先级分组
     *
     */
    enum class nvic_priority_group
    {
        /// 0bit抢占优先级，4bit响应优先级
        g0 = 7,
        /// 1bit抢占优先级，3bit响应优先级
        g1 = 6,
        /// 2bit抢占优先级，2bit响应优先级
        g2 = 5,
        /// 3bit抢占优先级，1bit响应优先级
        g3 = 4,
        /// 4bit抢占优先级，0bit响应优先级
        g5 = 3
    } inline constexpr default_priority_group{::SoC::nvic_priority_group::g2};

    /**
     * @brief 设置中断优先级分组
     *
     * @param group 中断优先级分组
     */
    void set_priority_group(::SoC::nvic_priority_group group = ::SoC::default_priority_group) noexcept;

    /**
     * @brief 获取中断优先级分组
     *
     * @return 中断优先级分组
     */
    ::SoC::nvic_priority_group get_priority_group() noexcept;

    /**
     * @brief 根据中断优先级分组进行编码
     *
     * @param preempt_priority 抢占优先级
     * @param sub_priority 响应优先级
     * @return 编码后的中断优先级
     */
    ::std::size_t encode_priority(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept;

    /**
     * @brief 根据中断优先级分组进行解码
     *
     * @param encoded_priority 编码后的中断优先级
     * @return std::pair{抢占优先级, 响应优先级}
     */
    ::std::pair<::std::size_t, ::std::size_t> decode_priority(::std::size_t encoded_priority) noexcept;

    /**
     * @brief 设置中断优先级
     *
     * @param irqn 中断枚举
     * @param encoded_priority 编码后的中断优先级
     */
    void set_priority(::IRQn_Type irqn, ::std::size_t encoded_priority) noexcept;

    /**
     * @brief 设置中断优先级
     *
     * @param irqn 中断枚举
     * @param preempt_priority 抢占优先级
     * @param sub_priority 响应优先级
     */
    void set_priority(::IRQn_Type irqn, ::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept;

    /**
     * @brief 获取中断优先级
     *
     * @param irqn 中断枚举
     * @return 编码后的中断优先级
     */
    ::std::size_t get_priority(::IRQn_Type irqn) noexcept;

    /**
     * @brief 获取中断优先级
     *
     * @param irqn 中断枚举
     * @return std::pair{抢占优先级, 响应优先级
     */
    ::std::pair<::std::size_t, ::std::size_t> get_priority_decoded(::IRQn_Type irqn) noexcept;

    /**
     * @brief 使能中断
     *
     * @param irqn 中断枚举
     */
    void enable_irqn(::IRQn_Type irqn) noexcept;

    /**
     * @brief 失能中断
     *
     * @param irqn 中断枚举
     */
    void disable_irqn(::IRQn_Type irqn) noexcept;

    /**
     * @brief 使能全局中断
     *
     */
    [[using gnu: always_inline, artificial]] inline void enable_irq() noexcept { ::__enable_irq(); }

    /**
     * @brief 失能全局中断
     *
     */
    [[using gnu: always_inline, artificial]] inline void disable_irq() noexcept { ::__disable_irq(); }

    /**
     * @brief 设置全局中断状态
     *
     * @param enable 是否使能中断
     */
    inline void set_irq(bool enable) noexcept
    {
        if(enable) { ::SoC::enable_irq(); }
        else { ::SoC::disable_irq(); }
    }
}  // namespace SoC
