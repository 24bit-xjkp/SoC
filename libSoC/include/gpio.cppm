/**
 * @file gpio.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 gpio外设
 */

module;
#include "pch.hpp"
export module SoC:gpio;
import :utils;

namespace SoC::detail
{
    /**
     * @brief gpio端口
     *
     */
    enum class gpio_port : ::std::uint8_t
    {
        /// 端口A
        pa,
        /// 端口B
        pb,
        /// 端口C
        pc,
        /// 端口D
        pd,
        /// 端口E
        pe,
        /// 端口F
        pf,
        /// 端口G
        pg,
        /// 端口H
        ph,
        /// 端口I
        pi
    };

    /**
     * @brief gpio引脚
     *
     */
    enum class gpio_pin : ::std::uint16_t
    {
        /// 无效引脚
        invalid = 0,
        /// 引脚0
        p0 = LL_GPIO_PIN_0,
        /// 引脚1
        p1 = LL_GPIO_PIN_1,
        /// 引脚2
        p2 = LL_GPIO_PIN_2,
        /// 引脚3
        p3 = LL_GPIO_PIN_3,
        /// 引脚4
        p4 = LL_GPIO_PIN_4,
        /// 引脚5
        p5 = LL_GPIO_PIN_5,
        /// 引脚6
        p6 = LL_GPIO_PIN_6,
        /// 引脚7
        p7 = LL_GPIO_PIN_7,
        /// 引脚8
        p8 = LL_GPIO_PIN_8,
        /// 引脚9
        p9 = LL_GPIO_PIN_9,
        /// 引脚10
        p10 = LL_GPIO_PIN_10,
        /// 引脚11
        p11 = LL_GPIO_PIN_11,
        /// 引脚12
        p12 = LL_GPIO_PIN_12,
        /// 引脚13
        p13 = LL_GPIO_PIN_13,
        /// 引脚14
        p14 = LL_GPIO_PIN_14,
        /// 引脚15
        p15 = LL_GPIO_PIN_15,
        /// 所有引脚
        all = LL_GPIO_PIN_ALL,
    };

    /**
     * @brief 拼接gpio引脚枚举
     *
     * @param lhs gpio引脚枚举
     * @param rhs gpio引脚枚举
     * @return 拼接后的gpio引脚枚举
     */
    export constexpr inline ::SoC::detail::gpio_pin operator| (::SoC::detail::gpio_pin lhs, ::SoC::detail::gpio_pin rhs) noexcept
    {
        return static_cast<::SoC::detail::gpio_pin>(::SoC::to_underlying(lhs) | ::SoC::to_underlying(rhs));
    }

    /**
     * @brief 萃取gpio引脚枚举
     *
     * @param value gpio引脚枚举
     * @param mask 掩码
     * @return gpio引脚枚举是否包含在掩码中
     */
    constexpr inline ::std::size_t operator& (::SoC::detail::gpio_pin lhs, ::SoC::detail::gpio_pin rhs) noexcept
    {
        return ::SoC::to_underlying(lhs) & ::SoC::to_underlying(rhs);
    }
}  // namespace SoC::detail

export namespace SoC
{
    /**
     * @brief gpio端口
     *
     */
    struct gpio_port
    {
        using port_enum = ::SoC::detail::gpio_port;

    private:
        port_enum port{};

        /**
         * @brief 获取gpio在AHB总线上的外设号
         *
         * @return gpio外设号
         */
        [[nodiscard]] ::std::size_t get_periph() const noexcept;

    public:
        using enum port_enum;

        /**
         * @brief 初始化gpio端口并开启时钟
         *
         * @param port 要初始化的端口
         */
        explicit gpio_port(port_enum port) noexcept;

        /**
         * @brief 关闭端口的时钟
         *
         */
        ~gpio_port() noexcept;

        gpio_port(const gpio_port&) noexcept = delete;
        gpio_port& operator= (const gpio_port&) noexcept = delete;
        gpio_port(gpio_port&& other) noexcept;
        gpio_port& operator= (gpio_port&& other) noexcept = delete;

        /**
         * @brief 获取gpio端口枚举
         *
         * @return gpio端口枚举
         */
        [[nodiscard]] constexpr inline ::SoC::gpio_port::port_enum get_port_enum() const noexcept { return port; }

        /**
         * @brief 获取gpio端口指针
         *
         * @return gpio端口指针
         */
        [[nodiscard]] inline ::GPIO_TypeDef* get_port() const noexcept
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return reinterpret_cast<::GPIO_TypeDef*>(AHB1PERIPH_BASE + 0x0400zu * ::SoC::to_underlying(port));
        }

        /**
         * @brief 使能gpio端口时钟
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能gpio端口时钟
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断gpio端口时钟是否使能
         *
         * @return gpio端口时钟是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;
    };

    /**
     * @brief gpio引脚工作模式
     *
     */
    enum class gpio_mode : ::std::uint8_t
    {
        /// 输入模式
        input,
        /// 输出模式
        output,
        /// 功能复用模式
        alternate,
        /// 模拟模式
        analog
    };

    /**
     * @brief gpio引脚输出速度
     *
     */
    enum class gpio_speed : ::std::uint8_t
    {
        /// 低速 ~2MHz
        low = 0,
        /// 中速 ~25MHz
        medium = 1,
        /// 高速 ~50MHz
        high = 2,
        /// 超高速 ~100MHz
        very_high = 3,
        /// 默认速度为低速
        default_speed = low
    };

    /**
     * @brief gpio引脚输出状态
     *
     */
    enum class gpio_output_type : ::std::uint8_t
    {
        /// 推挽输出
        push_pull = 0,
        /// 开漏输出
        open_drain = 1,
        /// 默认输出模式为推挽输出
        default_type = push_pull
    };

    /**
     * @brief gpio引脚上下拉电阻
     *
     */
    enum class gpio_pull : ::std::uint8_t
    {
        /// 无上下拉电阻
        no_pull = 0,
        /// 上拉电阻
        pull_up = 1,
        /// 下拉电阻
        pull_down = 2,
        /// 默认无上下拉电阻
        default_pull = no_pull
    };

    /**
     * @brief gpio引脚复用功能
     *
     */
    enum class gpio_af : ::std::size_t
    {
        /// 复用功能0
        af0 = LL_GPIO_AF_0,
        /// 复用功能1
        af1 = LL_GPIO_AF_1,
        /// 复用功能2
        af2 = LL_GPIO_AF_2,
        /// 复用功能3
        af3 = LL_GPIO_AF_3,
        /// 复用功能4
        af4 = LL_GPIO_AF_4,
        /// 复用功能5
        af5 = LL_GPIO_AF_5,
        /// 复用功能6
        af6 = LL_GPIO_AF_6,
        /// 复用功能7
        af7 = LL_GPIO_AF_7,
        /// 复用功能8
        af8 = LL_GPIO_AF_8,
        /// 复用功能9
        af9 = LL_GPIO_AF_9,
        /// 复用功能10
        af10 = LL_GPIO_AF_10,
        /// 复用功能11
        af11 = LL_GPIO_AF_11,
        /// 复用功能12
        af12 = LL_GPIO_AF_12,
        /// 复用功能13
        af13 = LL_GPIO_AF_13,
        /// 复用功能14
        af14 = LL_GPIO_AF_14,
        /// 复用功能15
        af15 = LL_GPIO_AF_15,
        /// 默认复用号为非法值，用于检查输入
        default_af = -1zu
    };

    /**
     * @brief gpio引脚
     *
     */
    struct gpio_pin
    {
    private:
        using pin_enum = ::SoC::detail::gpio_pin;
        ::GPIO_TypeDef* gpio{};
        pin_enum pin{};
        ::SoC::gpio_mode mode{};

        /**
         * @brief 函数默认使用的引脚列表，对应到当前对象中全部的引脚
         *
         */
        constexpr inline static auto default_pins{static_cast<pin_enum>(-1zu)};

        /**
         * @brief 断言输入引脚列表在已经初始化的引脚列表内，并返回映射后的引脚列表
         *
         * @param pin_in 输入引脚列表，若为default_pins则映射到this->pin
         * @param location 源代码位置
         * @return pin_enum 映射后的引脚列表
         */
        [[nodiscard]] pin_enum check_pin(pin_enum pin_in,
                                         ::std::source_location location = ::std::source_location::current()) const noexcept;

        /**
         * @brief 断言当前对象中的引脚工作在输出模式下
         *
         * @param location 源代码位置
         */
        void check_output_mode(::std::source_location location = ::std::source_location::current()) const noexcept;

    public:
        using enum pin_enum;

        /**
         * @brief 初始化gpio引脚
         *
         * @param gpio_port gpio端口
         * @param pin gpio引脚
         * @param mode gpio工作模式
         * @param af gpio复用功能，仅复用模式下可设置此参数
         * @param speed gpio输出速度，仅输出模式下可设置此参数
         * @param pull gpio上下拉电阻，仅开漏输出下可设置此参数
         * @param output_type gpio输出类型，仅输出模式下可配置此参数
         */
        explicit gpio_pin(::SoC::gpio_port& gpio_port,
                          pin_enum pin,
                          ::SoC::gpio_mode mode,
                          ::SoC::gpio_af af = ::SoC::gpio_af::default_af,
                          ::SoC::gpio_speed speed = ::SoC::gpio_speed::default_speed,
                          ::SoC::gpio_pull pull = ::SoC::gpio_pull::default_pull,
                          ::SoC::gpio_output_type output_type = ::SoC::gpio_output_type::default_type) noexcept;

        /**
         * @brief 获取gpio结构体指针
         *
         */
        [[nodiscard]] constexpr inline ::GPIO_TypeDef* get_port() const noexcept { return gpio; }

        /**
         * @brief 获取gpio端口枚举
         *
         * @return gpio端口枚举
         */
        [[nodiscard]] inline ::SoC::gpio_port::port_enum get_port_enum() const noexcept
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return static_cast<::SoC::gpio_port::port_enum>((reinterpret_cast<::std::uintptr_t>(gpio) - AHB1PERIPH_BASE) /
                                                            0x0400zu);
        }

        /**
         * @brief 获取所有引脚
         *
         */
        [[nodiscard]] constexpr inline pin_enum get_pin() const noexcept { return pin; }

        /**
         * @brief 获取引脚的工作模式
         *
         */
        [[nodiscard]] constexpr inline ::SoC::gpio_mode get_mode() const noexcept { return mode; }

        /**
         * @brief 切换指定引脚的状态
         *
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void toggle(pin_enum pin_in = default_pins) const noexcept;

        /**
         * @brief 设置指定引脚的状态为高电平
         *
         * @note 开漏输出下为高阻态
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void set(pin_enum pin_in = default_pins) const noexcept;

        /**
         * @brief 设置指定引脚的状态为低电平
         *
         * @note 开漏输出下为接地
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void reset(pin_enum pin_in = default_pins) const noexcept;

        /**
         * @brief 设置指定引脚的状态为指定电平
         *
         * @note 开漏输出下level为true设置引脚为高阻态，为false设置引脚为接地
         * @note 引脚需要在当前对象中初始化
         * @param level 为true设置引脚为高电平，为false设置引脚为低电平
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void write(bool level, pin_enum pin_in = default_pins) const noexcept;

        /**
         * @brief 读取指定引脚的状态
         *
         * @note 支持除模拟外的模式，根据对象中引脚的状态自动选择
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         * @return true 所有指定引脚均为高电平
         * @return false 有引脚为低电平
         */
        [[nodiscard]] bool read(pin_enum pin_in = default_pins) const noexcept;
    };
}  // namespace SoC
