/**
 * @file exti.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 exti外设
 */

module;
#include "pch.hpp"
export module SoC:exti;
import :gpio;

namespace SoC::detail
{
    /**
     * @brief 外部中断线枚举
     *
     */
    enum class exti_line : ::std::size_t
    {
        /// 无效外部中断线
        invalid = 0,
        /// 外部中断线0
        line0 = LL_EXTI_LINE_0,
        /// 外部中断线1
        line1 = LL_EXTI_LINE_1,
        /// 外部中断线2
        line2 = LL_EXTI_LINE_2,
        /// 外部中断线3
        line3 = LL_EXTI_LINE_3,
        /// 外部中断线4
        line4 = LL_EXTI_LINE_4,
        /// 外部中断线5
        line5 = LL_EXTI_LINE_5,
        /// 外部中断线6
        line6 = LL_EXTI_LINE_6,
        /// 外部中断线7
        line7 = LL_EXTI_LINE_7,
        /// 外部中断线8
        line8 = LL_EXTI_LINE_8,
        /// 外部中断线9
        line9 = LL_EXTI_LINE_9,
        /// 外部中断线10
        line10 = LL_EXTI_LINE_10,
        /// 外部中断线11
        line11 = LL_EXTI_LINE_11,
        /// 外部中断线12
        line12 = LL_EXTI_LINE_12,
        /// 外部中断线13
        line13 = LL_EXTI_LINE_13,
        /// 外部中断线14
        line14 = LL_EXTI_LINE_14,
        /// 外部中断线15
        line15 = LL_EXTI_LINE_15,
        /// 外部中断线16
        line16 = LL_EXTI_LINE_16,
        /// 外部中断线17
        line17 = LL_EXTI_LINE_17,
        /// 外部中断线18
        line18 = LL_EXTI_LINE_18,
        /// 外部中断线19
        line19 = LL_EXTI_LINE_19,
        /// 外部中断线20
        line20 = LL_EXTI_LINE_20,
        /// 外部中断线21
        line21 = LL_EXTI_LINE_21,
        /// 外部中断线22
        line22 = LL_EXTI_LINE_22,
        /// 所有外部中断线
        all = LL_EXTI_LINE_ALL_0_31,
    };

    /**
     * @brief 外部中断关联的gpio端口枚举
     *
     */
    enum class exti_gpio_port : ::std::uint8_t
    {
        /// GPIO A
        pa = LL_SYSCFG_EXTI_PORTA,
        /// GPIO B
        pb = LL_SYSCFG_EXTI_PORTB,
        /// GPIO C
        pc = LL_SYSCFG_EXTI_PORTC,
        /// GPIO D
        pd = LL_SYSCFG_EXTI_PORTD,
        /// GPIO E
        pe = LL_SYSCFG_EXTI_PORTE,
        /// GPIO F
        pf = LL_SYSCFG_EXTI_PORTF,
        /// GPIO G
        pg = LL_SYSCFG_EXTI_PORTG,
        /// GPIO H
        ph = LL_SYSCFG_EXTI_PORTH,
        /// GPIO I
        pi = LL_SYSCFG_EXTI_PORTI,
    };

    /**
     * @brief 拼接外部中断线枚举
     *
     * @param lhs 外部中断线枚举
     * @param rhs 外部中断线枚举
     * @return 拼接后的外部中断线枚举
     */
    constexpr inline ::SoC::detail::exti_line operator| (::SoC::detail::exti_line lhs, ::SoC::detail::exti_line rhs) noexcept
    { return ::SoC::detail::exti_line{::SoC::to_underlying(lhs) | ::SoC::to_underlying(rhs)}; }

    /**
     * @brief 萃取外部中断线枚举
     *
     * @param value 外部中断线枚举
     * @param mask 掩码
     * @return 外部中断线枚举是否包含在掩码中
     */
    constexpr inline bool operator& (::SoC::detail::exti_line value, ::SoC::detail::exti_line mask) noexcept
    { return (::SoC::to_underlying(value) & ::SoC::to_underlying(mask)) != ::std::underlying_type_t<::SoC::detail::exti_line>{}; }

    /// 中断线NVIC侧中断引用计数器
    /// 对应中断线0, 1, 2, 3, 4, 5-9, 10-15的NVIC侧中断
    constinit inline ::std::array<::std::uint8_t, 7> exti_line_irq_reference_counter{};
}  // namespace SoC::detail

export namespace SoC
{
    /**
     * @brief 系统配置控制
     *
     */
    struct syscfg
    {
    private:
        /// 系统控制器时钟是否使能
        bool clock_enabled{};
        /// 外设时钟号
        constexpr inline static auto periph{LL_APB2_GRP1_PERIPH_SYSCFG};

    public:
        /**
         * @brief 开启系统配置控制器时钟
         *
         */
        explicit syscfg() noexcept;

        /**
         * @brief 关闭系统配置控制器时钟
         *
         */
        ~syscfg() noexcept;

        inline syscfg(const syscfg&) noexcept = delete("对象独占外设资源，不能复制构造");
        inline syscfg& operator= (const syscfg&) noexcept = delete("对象独占外设资源，不能复制赋值");
        inline syscfg& operator= (syscfg&&) noexcept = delete("系统配置控制器全局唯一，不能移动赋值");

        /**
         * @brief 转移系统配置控制器的所有权
         *
         * @param other 其他系统配置控制器对象
         */
        syscfg(syscfg&& other) noexcept;

        /**
         * @brief 失能系统控制器时钟，释放系统配置控制器的所有权
         *
         */
        void release() noexcept;

        /**
         * @brief 使能系统控制器时钟
         *
         */
        void enable() noexcept;

        /**
         * @brief 失能系统控制器时钟
         *
         */
        void disable() noexcept;

        /**
         * @brief 判断系统控制器时钟是否使能
         *
         * @return 系统控制器时钟是否使能
         */
        [[nodiscard]] static bool is_enabled() noexcept;
    };

    /**
     * @brief 外部线中断触发源
     *
     */
    enum class exti_trigger_source : ::std::uint8_t
    {
        /// 无触发源
        none = 0,
        /// 上升沿触发
        rising = 1,
        /// 下降沿触发
        falling = 2,
        /// 双边沿触发
        rising_falling = rising | falling,
    };

    /**
     * @brief 外部线中断
     *
     */
    struct exti_line
    {
        using exti_line_enum = ::SoC::detail::exti_line;
        using enum exti_line_enum;

    private:
        /// 中断线枚举
        exti_line_enum line;
        /// 中断向量号
        ::IRQn_Type irqn;
        /// gpio端口枚举
        ::SoC::detail::exti_gpio_port gpio_port;
        /// 触发源
        ::SoC::exti_trigger_source trigger_source;
        /// 中断是否使能
        bool is_irq_enabled{};

        /// 5-9线掩码
        constexpr inline static exti_line_enum line5_9{line5 | line6 | line7 | line8 | line9};
        /// 10-15线掩码
        constexpr inline static exti_line_enum line10_15{line10 | line11 | line12 | line13 | line14 | line15};

    public:
        /**
         * @brief 根据gpio引脚配置线中断
         *
         * @param syscfg 系统控制器
         * @param line 中断线枚举，每个对象管理一根中断线，不能为all
         * @param gpio_port gpio端口
         * @param trigger_source 触发源
         */
        explicit exti_line(::SoC::syscfg& syscfg,
                           exti_line_enum line,
                           ::SoC::gpio_port::port_enum gpio_port,
                           ::SoC::exti_trigger_source trigger_source) noexcept;

        /**
         * @brief 获取gpio端口枚举
         *
         * @return gpio端口枚举
         */
        [[nodiscard]] inline ::SoC::detail::exti_gpio_port get_gpio_port() const noexcept { return gpio_port; }

        /**
         * @brief 获取中断线枚举
         *
         * @return 中断线枚举
         */
        [[nodiscard]] inline exti_line_enum get_line() const noexcept { return line; }

        /**
         * @brief 获取中断线触发源
         *
         * @return 中断线触发源
         */
        [[nodiscard]] inline ::SoC::exti_trigger_source get_trigger_source() const noexcept { return trigger_source; }

        /**
         * @brief 清除中断线的触发源
         *
         */
        ~exti_line() noexcept;

        inline exti_line(const exti_line&) noexcept = delete("对象独占外设资源，不能复制构造");
        inline exti_line& operator= (const exti_line&) noexcept = delete("对象独占外设资源，不能复制赋值");

        /**
         * @brief 转移中断线的所有权
         *
         * @param other 其他中断线对象
         */
        exti_line(exti_line&& other) noexcept;

        /**
         * @brief 转移中断线的所有权
         *
         * @param other 其他中断线对象
         * @return 本对象
         */
        exti_line& operator= (exti_line&& other) noexcept;

        /**
         * @brief 清除标志，失能中断线，关闭NVIC中断，然后释放中断线的所有权
         *
         */
        void release() noexcept;

        /**
         * @brief 设置中断线触发源
         *
         * @param trigger_source 触发源，设置为none以清空触发源，即失能该中断线
         */
        void set_trigger_source(::SoC::exti_trigger_source trigger_source) noexcept;

        /**
         * @brief 设置gpio端口枚举
         *
         * @param gpio_port gpio端口枚举
         */
        void set_gpio_port(::SoC::gpio_port::port_enum gpio_port) noexcept;

        /**
         * @brief 清除gpio端口枚举
         *
         * @note 将syscfg寄存器和对象内存储的gpio端口枚举还原为syscfg寄存器复位值0，即GPIO A
         */
        void clear_gpio_port() noexcept;

        /**
         * @brief 使能中断线
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能中断线
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断中断线是否使能
         *
         * @return 中断线是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;

        /**
         * @brief 使能外部中断
         *
         * @param encoded_priority 编码后的优先级
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         * @note 在多根中断线使用同一NVIC中断入口时，后续调用设置的优先级将覆盖之前的优先级
         */
        void enable_irq(::std::size_t encoded_priority) noexcept;

        /**
         * @brief 使能NVIC侧外部中断
         *
         * @param preempt_priority 抢占优先级
         * @param sub_priority 响应优先级
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         * @note 在多根中断线使用同一NVIC中断入口时，后续调用设置的优先级将覆盖之前的优先级
         */
        void enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept;

        /**
         * @brief 尝试使能NVIC侧外部中断
         *
         * 当NVIC侧外部中断未使能时使能中断并返回true，否则直接返回false。调用后对象始终获得NVIC侧外部中断所有权。
         * @param encoded_priority 编码后的优先级
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         * @return 是否真正使能NVIC侧外部中断
         */
        [[nodiscard]] bool try_enable_irq(::std::size_t encoded_priority) noexcept;

        /**
         * @brief 尝试使能NVIC侧外部中断
         *
         * 当NVIC侧外部中断未使能时使能中断并返回true，否则直接返回false。调用后对象始终获得NVIC侧外部中断所有权。
         * @param preempt_priority 抢占优先级
         * @param sub_priority 响应优先级
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         * @return 是否真正使能NVIC侧外部中断
         */
        [[nodiscard]] bool try_enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept;

        /**
         * @brief 失能NVIC侧外部中断
         *
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         */
        void disable_irq() noexcept;

        /**
         * @brief 设置中断源是否使能
         *
         * @param enable 是否使能中断
         * @note 外设侧函数是并发安全的，可以在中断上下文中调用
         */
        void set_it(bool enable) const noexcept;

        /**
         * @brief 判断中断源是否使能
         *
         * @return 中断源是否使能
         */
        [[nodiscard]] bool get_it() const noexcept;

        /**
         * @brief 获取中断线标志
         *
         * @return 中断线标志
         */
        [[nodiscard]] bool get_flag_it() const noexcept;

        /**
         * @brief 清除中断线标志
         *
         */
        void clear_flag_it() const noexcept;

        /**
         * @brief 判断是否是本对象管理的中断线上的中断
         *
         * @return 是否是本对象管理的中断线上的中断
         */
        [[nodiscard]] bool is_it_enabled() const noexcept;
    };
}  // namespace SoC
