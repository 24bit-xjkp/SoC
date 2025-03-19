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
         * @param location 用于定位构造函数调用位置以便提供更准确的报错信息
         */
        explicit gpio_port(port_enum port, ::std::source_location location = ::std::source_location::current()) noexcept;

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
        very_high
    };

    /**
     * @brief gpio引脚输出状态
     *
     */
    enum class gpio_output_type : ::std::size_t
    {
        push_pull,
        open_drain
    };

    /**
     * @brief gpio引脚上下拉电阻
     *
     */
    enum class gpio_pull : ::std::size_t
    {
        no_pull,
        pull_up,
        pull_down
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
        af15
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
         * @brief 断言输入引脚列表在已经初始化的引脚列表内
         *
         * @param pin_in 输入引脚列表
         * @param location 源代码位置
         */
        void check_pin(pin_enum pin_in, ::std::source_location location) noexcept;

        /**
         * @brief 断言当前对象中的引脚工作在指定模式下
         *
         * @param mode_in 指定模式
         * @param location 源代码位置
         */
        void check_mode(::SoC::gpio_mode mode_in, ::std::source_location location) noexcept;

    public:
        using enum pin_enum;

        /**
         * @brief 初始化gpio引脚
         *
         * @param gpio_port gpio端口
         * @param pin gpio引脚
         * @param mode gpio工作模式
         * @param speed gpio输出速度
         * @param pull gpio上下拉电阻
         * @param alternate_function gpio复用功能
         * @param location 源代码位置
         */
        gpio_pin(::SoC::gpio_port::port_view gpio_port,
                 pin_enum pin,
                 ::SoC::gpio_mode mode,
                 ::SoC::gpio_speed speed,
                 ::SoC::gpio_pull pull = ::SoC::gpio_pull::no_pull,
                 ::SoC::gpio_output_type output_type = ::SoC::gpio_output_type::push_pull,
                 ::SoC::gpio_alternate_function alternate_function = ::SoC::gpio_alternate_function::af0,
                 ::std::source_location location = ::std::source_location::current()) noexcept;

        /**
         * @brief 切换当前对象中所有引脚的状态
         *
         * @param location 源代码位置
         */
        void toggle(::std::source_location location = ::std::source_location::current()) noexcept;
        /**
         * @brief 切换指定引脚的状态
         *
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表
         * @param location 源代码位置
         */
        void toggle(pin_enum pin_in, ::std::source_location location = ::std::source_location::current()) noexcept;

        /**
         * @brief 设置指定引脚的状态为高电平
         *
         * @note 开漏输出下为高阻态
         * @param location 源代码位置
         */
        void set(::std::source_location location = ::std::source_location::current()) noexcept;
        /**
         * @brief 设置指定引脚的状态为高电平
         *
         * @note 开漏输出下为高阻态
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表
         * @param location 源代码位置
         */
        void set(pin_enum pin_in, ::std::source_location location = ::std::source_location::current()) noexcept;

        /**
         * @brief 设置指定引脚的状态为低电平
         *
         * @note 开漏输出下为接地
         * @param location 源代码位置
         */
        void reset(::std::source_location location = ::std::source_location::current()) noexcept;
        /**
         * @brief 设置指定引脚的状态为低电平
         *
         * @note 开漏输出下为接地
         * @note 引脚需要在当前对象中初始化
         * @param pin_in 输入引脚列表
         * @param location 源代码位置
         */
        void reset(pin_enum pin_in, ::std::source_location location = ::std::source_location::current()) noexcept;

        /**
         * @brief 设置指定引脚的状态为指定电平
         *
         * @note 开漏输出下high_level为true设置引脚为高阻态，为false设置引脚为接地
         * @param high_level 为true设置引脚为高电平，为false设置引脚为低电平
         * @param location 源代码位置
         */
        void write(bool high_level, ::std::source_location location = ::std::source_location::current()) noexcept;
        /**
         * @brief 设置指定引脚的状态为指定电平
         *
         * @note 开漏输出下high_level为true设置引脚为高阻态，为false设置引脚为接地
         * @note 引脚需要在当前对象中初始化
         * @param high_level 为true设置引脚为高电平，为false设置引脚为低电平
         * @param pin_in 输入引脚列表
         * @param location 源代码位置
         */
        void
            write(bool high_level, pin_enum pin_in, ::std::source_location location = ::std::source_location::current()) noexcept;
    };
}  // namespace SoC
