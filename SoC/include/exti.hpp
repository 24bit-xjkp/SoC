#pragma once
#include "syscfg.hpp"
#include "gpio.hpp"

namespace SoC
{
    namespace detail
    {
        /**
         * @brief 外部中断线枚举
         *
         */
        enum class exti_line : ::std::size_t
        {
            line0 = LL_EXTI_LINE_0,
            line1 = LL_EXTI_LINE_1,
            line2 = LL_EXTI_LINE_2,
            line3 = LL_EXTI_LINE_3,
            line4 = LL_EXTI_LINE_4,
            line5 = LL_EXTI_LINE_5,
            line6 = LL_EXTI_LINE_6,
            line7 = LL_EXTI_LINE_7,
            line8 = LL_EXTI_LINE_8,
            line9 = LL_EXTI_LINE_9,
            line10 = LL_EXTI_LINE_10,
            line11 = LL_EXTI_LINE_11,
            line12 = LL_EXTI_LINE_12,
            line13 = LL_EXTI_LINE_13,
            line14 = LL_EXTI_LINE_14,
            line15 = LL_EXTI_LINE_15,
            line16 = LL_EXTI_LINE_16,
            line17 = LL_EXTI_LINE_17,
            line18 = LL_EXTI_LINE_18,
            line19 = LL_EXTI_LINE_19,
            line20 = LL_EXTI_LINE_20,
            line21 = LL_EXTI_LINE_21,
            line22 = LL_EXTI_LINE_22,
            all = LL_EXTI_LINE_ALL_0_31
        };

        /**
         * @brief 外部中断关联的gpio端口枚举
         *
         */
        enum class exti_gpio_port : ::std::size_t
        {
            pa = LL_SYSCFG_EXTI_PORTA,
            pb = LL_SYSCFG_EXTI_PORTB,
            pc = LL_SYSCFG_EXTI_PORTC,
            pd = LL_SYSCFG_EXTI_PORTD,
            pe = LL_SYSCFG_EXTI_PORTE,
            pf = LL_SYSCFG_EXTI_PORTF,
            pg = LL_SYSCFG_EXTI_PORTG,
            ph = LL_SYSCFG_EXTI_PORTH,
            pi = LL_SYSCFG_EXTI_PORTI,
            none = -1zu
        };

        /**
         * @brief 拼接外部中断线枚举
         *
         * @param lhs 外部中断线枚举
         * @param rhs 外部中断线枚举
         * @return 拼接后的外部中断线枚举
         */
        constexpr inline ::SoC::detail::exti_line operator| (::SoC::detail::exti_line lhs, ::SoC::detail::exti_line rhs) noexcept
        {
            return ::SoC::detail::exti_line{::std::to_underlying(lhs) | ::std::to_underlying(rhs)};
        }

        /**
         * @brief 萃取外部中断线枚举
         *
         * @param value 外部中断线枚举
         * @param mask 掩码
         * @return 外部中断线枚举是否包含在掩码中
         */
        constexpr inline bool operator& (::SoC::detail::exti_line value, ::SoC::detail::exti_line mask) noexcept
        {
            return ::std::to_underlying(value) & ::std::to_underlying(mask);
        }
    }  // namespace detail

    /**
     * @brief 外部线中断触发源
     *
     */
    enum class exti_trigger_source : ::std::size_t
    {
        rising = 1,
        falling = 2,
        rising_falling = rising | falling
    };

    /**
     * @brief 萃取外部线中断触发源枚举
     *
     * @param value 外部线中断触发源枚举
     * @param mask 掩码
     * @return 外部线中断触发源枚举是否包含在掩码中
     */
    constexpr inline bool operator& (::SoC::exti_trigger_source value, ::SoC::exti_trigger_source mask) noexcept
    {
        return ::std::to_underlying(value) & ::std::to_underlying(mask);
    }

    /**
     * @brief 外部线中断
     *
     */
    struct exti_line
    {
        using exti_line_enum = ::SoC::detail::exti_line;
        using enum exti_line_enum;

    private:
        ::SoC::detail::exti_gpio_port gpio_port;
        exti_line_enum line;
        ::IRQn_Type irqn;

        /// 线枚举默认值
        constexpr inline static exti_line_enum default_lines{-1zu};

        /// 5-9线掩码
        constexpr inline static exti_line_enum line5_9{line5 | line6 | line7 | line8 | line9};

        /// 10-15线掩码
        constexpr inline static exti_line_enum line10_15{line10 | line11 | line12 | line13 | line14 | line15};

        /**
         * @brief 检查中断线枚举是否合法
         *
         * @param lines 中断线枚举
         * @param location 源代码位置
         * @return 若lines为default_lines则返回当前对象管理的中断线，否则返回lines
         */
        exti_line_enum check_lines(exti_line_enum lines, ::std::source_location location = ::std::source_location::current()) const noexcept;

    public:
        /**
         * @brief 根据gpio引脚配置线中断
         *
         * @param syscfg 系统控制器
         * @param gpio_port gpio端口
         * @param trigger_source 触发源
         */
        explicit exti_line(::SoC::syscfg& syscfg,
                           ::SoC::gpio_port::port_enum gpio_port,
                           exti_line_enum line,
                           ::SoC::exti_trigger_source trigger_source) noexcept;

        /**
         * @brief 获取gpio端口枚举
         *
         * @return gpio端口枚举
         */
        inline ::SoC::detail::exti_gpio_port get_gpio_port() const noexcept { return gpio_port; }

        /**
         * @brief 获取中断线枚举
         *
         * @return 中断线枚举
         */
        inline exti_line_enum get_lines() const noexcept
        {
            return line;
        }

        /**
         * @brief 清除中断线的触发源
         *
         */
        ~exti_line() noexcept;

        inline exti_line(const exti_line&) noexcept = delete;
        inline exti_line& operator= (const exti_line&) = delete;
        exti_line(exti_line&&) noexcept;
        inline exti_line& operator= (exti_line&&) = delete;

        /**
         * @brief 设置线触发源
         *
         * @param trigger_source 触发源
         */
        void set_trigger_source(::SoC::exti_trigger_source trigger_source) const noexcept;

        /**
         * @brief 清除线触发源
         *
         * @param lines 要清除的线，默认使用当前对象中全部中断线
         * @note 线需要在当前对象中初始化
         */
        void clear_trigger_source() const noexcept;

        /**
         * @brief 使能外部中断
         *
         * @param encoded_priority 编码后的优先级
         */
        void enable_irq(::std::size_t encoded_priority) const noexcept;

        /**
         * @brief 使能外部中断
         *
         * @param preempt_priority 抢占优先级
         * @param sub_priority 响应优先级
         */
        void enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) const noexcept;

        /**
         * @brief 失能外部中断
         *
         */
        void disable_irq() const noexcept;

        /**
         * @brief 设置中断源是否使能
         *
         * @param enable 是否使能中断
         * @param lines 要设置的线
         */
        void set_it(bool enable, exti_line_enum lines = default_lines) const noexcept;

        /**
         * @brief 判断中断源是否使能
         *
         * @param lines 要判断的线
         * @return 中断源是否使能
         */
        bool get_it(exti_line_enum lines = default_lines) const noexcept;

        /**
         * @brief 获取中断线标志
         *
         * @param lines 要获取标志的线
         * @return true
         * @return false
         */
        bool get_flag_it(exti_line_enum lines = default_lines) const noexcept;

        /**
         * @brief 清除中断线标志
         *
         * @param lines 要清除标志的线
         */
        void clear_flag_it(exti_line_enum lines = default_lines) const noexcept;
    };
}  // namespace SoC
