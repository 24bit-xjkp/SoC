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
            gpio_a,
            gpio_b,
            gpio_c,
            gpio_d,
            gpio_e,
            gpio_f,
            gpio_g,
            gpio_h,
            gpio_i,
            gpio_invalid
        };

        /**
         * @brief gpio引脚
         *
         */
        enum class gpio_pin : ::std::size_t
        {
            pin_0 = ::SoC::mask_single_one<0>,
            pin_1 = ::SoC::mask_single_one<1>,
            pin_2 = ::SoC::mask_single_one<2>,
            pin_3 = ::SoC::mask_single_one<3>,
            pin_4 = ::SoC::mask_single_one<4>,
            pin_5 = ::SoC::mask_single_one<5>,
            pin_6 = ::SoC::mask_single_one<6>,
            pin_7 = ::SoC::mask_single_one<7>,
            pin_8 = ::SoC::mask_single_one<8>,
            pin_9 = ::SoC::mask_single_one<9>,
            pin_10 = ::SoC::mask_single_one<10>,
            pin_11 = ::SoC::mask_single_one<11>,
            pin_12 = ::SoC::mask_single_one<12>,
            pin_13 = ::SoC::mask_single_one<13>,
            pin_14 = ::SoC::mask_single_one<14>,
            pin_15 = ::SoC::mask_single_one<15>,
            pin_all = ::SoC::mask_all_one<16>,
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

    struct gpio_port
    {
    private:
        using port_enum = ::SoC::detail::gpio_port;
        port_enum port{};

        struct view
        {
            friend gpio_port;
            const port_enum port;

        private:
            constexpr inline view(port_enum port) noexcept : port{port} {}
        };

    public:
        using enum port_enum;
        using port_view = view;

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
        gpio_port& operator= (gpio_port&& other) noexcept;

        /**
         * @brief 转化为不具有raii的端口视图
         *
         * @return 端口视图
         */
        operator port_view () const& noexcept { return port; }
    };

    /**
     * @brief gpio引脚工作模式
     *
     */
    enum class gpio_mode : ::std::size_t
    {
        input,
        output,
        alternate,
        analog
    };

    /**
     * @brief gpio引脚输出速度
     *
     */
    enum class gpio_speed : ::std::size_t
    {
        low,
        medium,
        high,
        very_high,
        default_speed = low
    };

    /**
     * @brief gpio引脚输出状态
     *
     */
    enum class gpio_output_type : ::std::size_t
    {
        push_pull,
        open_drain,
        default_type = push_pull
    };

    /**
     * @brief gpio引脚上下拉电阻
     *
     */
    enum class gpio_pull : ::std::size_t
    {
        no_pull,
        pull_up,
        pull_down,
        default_pull = no_pull
    };

    /**
     * @brief gpio引脚复用功能
     *
     */
    enum class gpio_alternate_function : ::std::size_t
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
        default_af = af0
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
         * @return pin_enum 映射后的引脚列表
         */
        pin_enum check_pin(pin_enum pin_in) noexcept;

        /**
         * @brief 断言当前对象中的引脚工作在指定模式下
         *
         * @param mode_in 指定模式
         */
        void check_mode(::SoC::gpio_mode mode_in) noexcept;

    public:
        using enum pin_enum;

        /**
         * @brief 初始化gpio引脚
         *
         * @param gpio_port gpio端口
         * @param pin gpio引脚
         * @param mode gpio工作模式
         * @param speed gpio输出速度，仅输出模式下可设置此参数
         * @param pull gpio上下拉电阻 仅开漏输出下可设置此参数
         * @param output_type gpio输出类型，仅输出模式下可配置此参数
         * @param alternate_function gpio复用功能，仅复用模式下可设置此参数
         */
        explicit gpio_pin(
            ::SoC::gpio_port::port_view gpio_port,
            pin_enum pin,
            ::SoC::gpio_mode mode,
            ::SoC::gpio_speed speed = ::SoC::gpio_speed::default_speed,
            ::SoC::gpio_pull pull = ::SoC::gpio_pull::default_pull,
            ::SoC::gpio_output_type output_type = ::SoC::gpio_output_type::default_type,
            ::SoC::gpio_alternate_function alternate_function = ::SoC::gpio_alternate_function::default_af) noexcept;

        /**
         * @brief 获取gpio结构体指针
         *
         */
        constexpr inline ::GPIO_TypeDef* get_gpio() noexcept { return gpio; }

        /**
         * @brief 获取所有引脚
         *
         */
        constexpr inline pin_enum get_pin() noexcept { return pin; }

        /**
         * @brief 获取引脚的工作模式
         *
         */
        constexpr inline ::SoC::gpio_mode get_mode() noexcept { return mode; }

        /**
         * @brief 切换指定引脚的状态
         *
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void toggle(pin_enum pin_in = default_pins) noexcept;

        /**
         * @brief 设置指定引脚的状态为高电平
         *
         * @note 开漏输出下为高阻态
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void set(pin_enum pin_in = default_pins) noexcept;

        /**
         * @brief 设置指定引脚的状态为低电平
         *
         * @note 开漏输出下为接地
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void reset(pin_enum pin_in = default_pins) noexcept;

        /**
         * @brief 设置指定引脚的状态为指定电平
         *
         * @note 开漏输出下level为true设置引脚为高阻态，为false设置引脚为接地
         * @note 引脚需要在当前对象中初始化
         * @param level 为true设置引脚为高电平，为false设置引脚为低电平
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         */
        void write(bool level, pin_enum pin_in = default_pins) noexcept;

        /**
         * @brief 读取指定引脚的状态
         *
         * @note 支持输入和输出模式，根据对象中引脚的状态自动选择
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表，默认使用当前对象中全部引脚
         * @return true 所有指定引脚均为高电平
         * @return false 有引脚为低电平
         */
        bool read(pin_enum pin_in = default_pins) noexcept;
    };
}  // namespace SoC
