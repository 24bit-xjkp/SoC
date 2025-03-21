#include "gpio.hpp"

namespace SoC
{
    ::SoC::gpio_port::gpio_port(port_enum port) noexcept : port{port}
    {
        ::LL_AHB1_GRP1_EnableClock(1 << ::std::to_underlying(port));
    }

    ::SoC::gpio_port::~gpio_port() noexcept
    {
        if(port != ::SoC::detail::gpio_port_invalid) { ::LL_AHB1_GRP1_DisableClock(1 << ::std::to_underlying(port)); }
    }

    ::SoC::gpio_port::gpio_port(gpio_port&& other) noexcept : port{::std::exchange(other.port, ::SoC::detail::gpio_port_invalid)}
    {
    }

    ::SoC::gpio_port& ::SoC::gpio_port::operator= (gpio_port&& other) noexcept
    {
        ::std::swap(*this, other);
        return *this;
    }

    ::SoC::gpio_pin::gpio_pin(::SoC::gpio_port::port_view gpio_port,
                              pin_enum pin,
                              ::SoC::gpio_mode mode,
                              ::SoC::gpio_speed speed,
                              ::SoC::gpio_pull pull,
                              ::SoC::gpio_output_type output_type,
                              ::SoC::gpio_alternate_function alternate_function) noexcept :
        gpio{reinterpret_cast<::GPIO_TypeDef*>(AHB1PERIPH_BASE + 0x0400u * ::std::to_underlying(gpio_port.port))}, pin{pin},
        mode{mode}
    {
        // 非复用模式下，复用号为0
        ::SoC::assert(mode != ::SoC::gpio_mode::alternate || alternate_function == ::SoC::gpio_alternate_function::default_af);
        switch(mode)
        {
            case ::SoC::gpio_mode::alternate:
                // 非输出模式下，speed保持默认值
                ::SoC::assert(speed == ::SoC::gpio_speed::default_speed);
                [[fallthrough]];
            case ::SoC::gpio_mode::output:
                // 推挽不应该设置上下拉电阻，pull保持默认值
                ::SoC::assert(output_type == ::SoC::gpio_output_type::open_drain || pull == ::SoC::gpio_pull::default_pull);
                break;
            default:
                // 非输出模式下，output_type保持默认值
                ::SoC::assert(output_type == ::SoC::gpio_output_type::default_type);
                // 非输出模式下，speed保持默认值
                ::SoC::assert(speed == ::SoC::gpio_speed::default_speed);
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
                if(pin_pos < 8) { ::LL_GPIO_SetAFPin_0_7(gpio, current_pin, ::std::to_underlying(alternate_function)); }
                else { ::LL_GPIO_SetAFPin_8_15(gpio, current_pin, ::std::to_underlying(alternate_function)); }
            }

            ::LL_GPIO_SetPinMode(gpio, current_pin, ::std::to_underlying(mode));

            pin_mask ^= current_pin;
        }
    }

    auto ::SoC::gpio_pin::check_pin(pin_enum pin_in) const noexcept -> pin_enum
    {
        if(pin_in == default_pins) { return pin; }
        else
        {
            ::SoC::assert((pin_in & pin) == ::std::to_underlying(pin_in));
            return pin_in;
        }
    }

    void ::SoC::gpio_pin::check_mode(::SoC::gpio_mode mode_in) const noexcept { ::SoC::assert(mode == mode_in); }

    void ::SoC::gpio_pin::toggle(pin_enum pin_in) const noexcept
    {
        check_mode(::SoC::gpio_mode::output);
        pin_in = check_pin(pin_in);
        ::LL_GPIO_TogglePin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::set(pin_enum pin_in) const noexcept
    {
        check_mode(::SoC::gpio_mode::output);
        pin_in = check_pin(pin_in);
        ::LL_GPIO_SetOutputPin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::reset(pin_enum pin_in) const noexcept
    {
        check_mode(::SoC::gpio_mode::output);
        pin_in = check_pin(pin_in);
        ::LL_GPIO_ResetOutputPin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::write(bool level, pin_enum pin_in) const noexcept
    {
        check_mode(::SoC::gpio_mode::output);
        pin_in = check_pin(pin_in);
        // 低16位置1，高16位清零
        gpio->BSRR = ::std::to_underlying(pin) << !level * 16;
    }

    bool ::SoC::gpio_pin::read(pin_enum pin_in) const noexcept
    {
        ::SoC::assert(mode != ::SoC::gpio_mode::alternate);
        pin_in = check_pin(pin_in);
        if(mode == ::SoC::gpio_mode::output) { return ::LL_GPIO_IsOutputPinSet(gpio, ::std::to_underlying(pin_in)); }
        else { return ::LL_GPIO_IsInputPinSet(gpio, ::std::to_underlying(pin_in)); }
    }
}  // namespace SoC
