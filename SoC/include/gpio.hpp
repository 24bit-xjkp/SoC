#pragma once
#include "utils.hpp"

namespace SoC
{
    namespace detail
    {
        /**
         * @brief gpio端口
         *
         */
        enum class gpio_port : ::std::size_t
        {
            pa,
            pb,
            pc,
            pd,
            pe,
            pf,
            pg,
            ph,
            pi
        };

        /**
         * @brief 非法的gpio端口，用于实现gpio端口类的移动
         *
         */
        constexpr inline static auto gpio_port_invalid{static_cast<::SoC::detail::gpio_port>(-1zu)};

        /**
         * @brief gpio引脚
         *
         */
        enum class gpio_pin : ::std::size_t
        {
            p0 = ::SoC::mask_single_one<0>,
            p1 = ::SoC::mask_single_one<1>,
            p2 = ::SoC::mask_single_one<2>,
            p3 = ::SoC::mask_single_one<3>,
            p4 = ::SoC::mask_single_one<4>,
            p5 = ::SoC::mask_single_one<5>,
            p6 = ::SoC::mask_single_one<6>,
            p7 = ::SoC::mask_single_one<7>,
            p8 = ::SoC::mask_single_one<8>,
            p9 = ::SoC::mask_single_one<9>,
            p10 = ::SoC::mask_single_one<10>,
            p11 = ::SoC::mask_single_one<11>,
            p12 = ::SoC::mask_single_one<12>,
            p13 = ::SoC::mask_single_one<13>,
            p14 = ::SoC::mask_single_one<14>,
            p15 = ::SoC::mask_single_one<15>,
            all = ::SoC::mask_all_one<16>,
        };

        constexpr inline ::SoC::detail::gpio_pin operator| (::SoC::detail::gpio_pin lhs, ::SoC::detail::gpio_pin rhs) noexcept
        {
            return static_cast<::SoC::detail::gpio_pin>(::std::to_underlying(lhs) | ::std::to_underlying(rhs));
        }

        constexpr inline ::std::size_t operator& (::SoC::detail::gpio_pin lhs, ::SoC::detail::gpio_pin rhs) noexcept
        {
            return ::std::to_underlying(lhs) & ::std::to_underlying(rhs);
        }
    }  // namespace detail

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
        ::std::size_t get_periph() const noexcept;

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
        constexpr inline ::SoC::gpio_port::port_enum get_port_enum() const noexcept { return port; }

        /**
         * @brief 获取gpio端口指针
         *
         * @return gpio端口指针
         */
        inline ::GPIO_TypeDef* get_port() const noexcept
        {
            return reinterpret_cast<::GPIO_TypeDef*>(AHB1PERIPH_BASE + 0x0400zu * ::std::to_underlying(port));
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
        bool is_enabled() const noexcept;
    };

    /**
     * @brief gpio引脚工作模式
     *
     */
    enum class gpio_mode : ::std::size_t
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
    enum class gpio_speed : ::std::size_t
    {
        /// 低速 ~2MHz
        low,
        /// 中速 ~25MHz
        medium,
        /// 高速 ~50MHz
        high,
        /// 超高速 ~100MHz
        very_high,
        /// 默认速度为低速
        default_speed = low
    };

    /**
     * @brief gpio引脚输出状态
     *
     */
    enum class gpio_output_type : ::std::size_t
    {
        /// 推挽输出
        push_pull,
        /// 开漏输出
        open_drain,
        /// 默认输出模式为推挽输出
        default_type = push_pull
    };

    /**
     * @brief gpio引脚上下拉电阻
     *
     */
    enum class gpio_pull : ::std::size_t
    {
        /// 无上下拉电阻
        no_pull,
        /// 上拉电阻
        pull_up,
        /// 下拉电阻
        pull_down,
        /// 默认无上下拉电阻
        default_pull = no_pull
    };

    /**
     * @brief gpio引脚复用功能
     *
     */
    enum class gpio_af : ::std::size_t
    {
        af0,
        af1,
        af2,
        af3,
        af4,
        af5,
        af6,
        af7,
        af8,
        af9,
        af10,
        af11,
        af12,
        af13,
        af14,
        af15,
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
        pin_enum check_pin(pin_enum pin_in, ::std::source_location location = ::std::source_location::current()) const noexcept;

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
        constexpr inline ::GPIO_TypeDef* get_port() const noexcept { return gpio; }

        /**
         * @brief 获取gpio端口枚举
         *
         * @return gpio端口枚举
         */
        inline ::SoC::gpio_port::port_enum get_port_enum() const noexcept
        {
            return ::SoC::gpio_port::port_enum{(reinterpret_cast<::std::uintptr_t>(gpio) - AHB1PERIPH_BASE) / 0x0400zu};
        }

        /**
         * @brief 获取所有引脚
         *
         */
        constexpr inline pin_enum get_pin() const noexcept { return pin; }

        /**
         * @brief 获取引脚的工作模式
         *
         */
        constexpr inline ::SoC::gpio_mode get_mode() const noexcept { return mode; }

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
        bool read(pin_enum pin_in = default_pins) const noexcept;
    };
}  // namespace SoC
