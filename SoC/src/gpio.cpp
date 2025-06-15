#include "gpio.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::gpio_port::gpio_port(port_enum port) noexcept : port{port}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此gpio端口不应处于使能状态"sv); }
        enable();
    }

    ::SoC::gpio_port::~gpio_port() noexcept
    {
        if(port != ::SoC::detail::gpio_port_invalid) { disable(); }
    }

    ::SoC::gpio_port::gpio_port(gpio_port&& other) noexcept : port{::std::exchange(other.port, ::SoC::detail::gpio_port_invalid)}
    {
    }

    ::std::size_t(::SoC::gpio_port::get_periph)() const noexcept { return 1 << ::std::to_underlying(port); }

    void ::SoC::gpio_port::enable() const noexcept { ::LL_AHB1_GRP1_EnableClock(get_periph()); }

    void ::SoC::gpio_port::disable() const noexcept { ::LL_AHB1_GRP1_DisableClock(get_periph()); }

    bool ::SoC::gpio_port::is_enabled() const noexcept { return ::LL_AHB1_GRP1_IsEnabledClock(get_periph()); }
}  // namespace SoC

namespace SoC
{
    ::SoC::gpio_pin::gpio_pin(::SoC::gpio_port& gpio_port,
                              pin_enum pin,
                              ::SoC::gpio_mode mode,
                              ::SoC::gpio_af af,
                              ::SoC::gpio_speed speed,
                              ::SoC::gpio_pull pull,
                              ::SoC::gpio_output_type output_type) noexcept : gpio{gpio_port.get_port()}, pin{pin}, mode{mode}
    {
        // 非复用模式下，复用号为非法值；复用模式下，复用号不能为非法值
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert((mode == ::SoC::gpio_mode::alternate && af != ::SoC::gpio_af::default_af) ||
                              af == ::SoC::gpio_af::default_af,
                          "当且仅当引脚为复用模式时需要设置功能复用"sv);
        }
        switch(mode)
        {
            case ::SoC::gpio_mode::alternate: break;
            case ::SoC::gpio_mode::output:
                // 推挽不应该设置上下拉电阻，pull保持默认值
                if constexpr(::SoC::use_full_assert)
                {
                    ::SoC::assert(output_type == ::SoC::gpio_output_type::open_drain || pull == ::SoC::gpio_pull::default_pull,
                                  "推挽输出不应设置上下拉电阻"sv);
                }
                break;
            default:
                if constexpr(::SoC::use_full_assert)
                {
                    // 非输出模式下，output_type保持默认值
                    ::SoC::assert(output_type == ::SoC::gpio_output_type::default_type, "非输出模式不应设置输出类型"sv);
                    // 非输出模式下，speed保持默认值
                    ::SoC::assert(speed == ::SoC::gpio_speed::default_speed, "非输出模式不应设置输出速度"sv);
                }
                break;
        }

        auto pin_mask{::std::to_underlying(pin)};
        while(pin_mask != 0)
        {
            auto pin_pos{::std::countr_zero(pin_mask)};
            auto current_pin{pin_mask & 1 << pin_pos};
            if(mode == ::SoC::gpio_mode::output || mode == ::SoC::gpio_mode::alternate)
            {
                ::LL_GPIO_SetPinSpeed(gpio, current_pin, ::std::to_underlying(speed));
                ::LL_GPIO_SetPinOutputType(gpio, current_pin, ::std::to_underlying(output_type));
            }

            ::LL_GPIO_SetPinPull(gpio, current_pin, ::std::to_underlying(pull));

            if(mode == ::SoC::gpio_mode::alternate)
            {
                if(pin_pos < 8) { ::LL_GPIO_SetAFPin_0_7(gpio, current_pin, ::std::to_underlying(af)); }
                else
                {
                    ::LL_GPIO_SetAFPin_8_15(gpio, current_pin, ::std::to_underlying(af));
                }
            }

            ::LL_GPIO_SetPinMode(gpio, current_pin, ::std::to_underlying(mode));

            pin_mask ^= current_pin;
        }
    }

    auto ::SoC::gpio_pin::check_pin(pin_enum pin_in, ::std::source_location location) const noexcept -> pin_enum
    {
        if(pin_in == default_pins) { return pin; }
        else
        {
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert((pin_in & pin) == ::std::to_underlying(pin_in), "访问未绑定到当前对象的引脚"sv, location);
            }
            return pin_in;
        }
    }

    void ::SoC::gpio_pin::check_output_mode(::std::source_location location) const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(mode == ::SoC::gpio_mode::output, "当前引脚模式不支持此操作"sv, location);
        }
    }

    void ::SoC::gpio_pin::toggle(pin_enum pin_in) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_output_mode(); }
        pin_in = check_pin(pin_in);
        ::LL_GPIO_TogglePin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::set(pin_enum pin_in) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_output_mode(); }
        pin_in = check_pin(pin_in);
        ::LL_GPIO_SetOutputPin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::reset(pin_enum pin_in) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_output_mode(); }
        pin_in = check_pin(pin_in);
        ::LL_GPIO_ResetOutputPin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::write(bool level, pin_enum pin_in) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_output_mode(); }
        pin_in = check_pin(pin_in);
        // 低16位置1，高16位清零
        gpio->BSRR = ::std::to_underlying(pin) << !level * 16;
    }

    bool ::SoC::gpio_pin::read(pin_enum pin_in) const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(mode != ::SoC::gpio_mode::analog, "模拟模式下不支持读取数据寄存器"sv);
        }
        pin_in = check_pin(pin_in);
        if(mode == ::SoC::gpio_mode::output) { return ::LL_GPIO_IsOutputPinSet(gpio, ::std::to_underlying(pin_in)); }
        else
        {
            return ::LL_GPIO_IsInputPinSet(gpio, ::std::to_underlying(pin_in));
        }
    }
}  // namespace SoC
