#include "gpio.hpp"

namespace SoC
{
    ::SoC::gpio_port::gpio_port(port_enum port, ::std::source_location location) noexcept : port{port}
    {
        ::SoC::assert(port != gpio_invalid, location);
        ::LL_AHB1_GRP1_EnableClock(1 << ::std::to_underlying(port));
    }

    ::SoC::gpio_port::~gpio_port() noexcept
    {
        if(port != gpio_invalid) { ::LL_AHB1_GRP1_DisableClock(1 << ::std::to_underlying(port)); }
    }

    ::SoC::gpio_port::gpio_port(gpio_port&& other) noexcept : port{::std::exchange(other.port, gpio_invalid)} {}

    ::SoC::gpio_port& ::SoC::gpio_port::operator= (gpio_port&& other) noexcept
    {
        port = ::std::exchange(other.port, gpio_invalid);
        return *this;
    }

    ::SoC::gpio_pin::gpio_pin(::SoC::gpio_port::port_view gpio_port,
                              pin_enum pin,
                              ::SoC::gpio_mode mode,
                              ::SoC::gpio_speed speed,
                              ::SoC::gpio_pull pull,
                              ::SoC::gpio_output_type output_type,
                              ::SoC::gpio_alternate_function alternate_function,
                              ::std::source_location location) noexcept :
        gpio{reinterpret_cast<::GPIO_TypeDef*>(AHB1PERIPH_BASE + 0x0400u * ::std::to_underlying(gpio_port.port))}, pin{pin},
        mode{mode}
    {
        // 断言端口有效
        ::SoC::assert(gpio_port.port != ::SoC::gpio_port::gpio_invalid, location);
        // 非复用模式下，复用号为0
        ::SoC::assert(mode != ::SoC::gpio_mode::alternate || alternate_function == ::SoC::gpio_alternate_function::af0, location);
        if(mode == ::SoC::gpio_mode::output)
        {
            // 推挽不应该有上下拉电阻
            ::SoC::assert(output_type == ::SoC::gpio_output_type::open_drain || pull == ::SoC::gpio_pull::no_pull, location);
        }
        else
        {
            // 非输出模式下，output_type保持默认值
            ::SoC::assert(output_type == ::SoC::gpio_output_type::push_pull, location);
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

    void ::SoC::gpio_pin::check_pin(pin_enum pin_in, ::std::source_location location) noexcept
    {
        ::SoC::assert((pin_in & pin) == ::std::to_underlying(pin_in), location);
    }

    void ::SoC::gpio_pin::check_mode(::SoC::gpio_mode mode_in, ::std::source_location location) noexcept
    {
        ::SoC::assert(mode == mode_in, location);
    }

    void ::SoC::gpio_pin::toggle(::std::source_location location) noexcept
    {
        check_mode(::SoC::gpio_mode::output, location);
        ::LL_GPIO_TogglePin(gpio, ::std::to_underlying(pin));
    }

    void ::SoC::gpio_pin::toggle(pin_enum pin_in, ::std::source_location location) noexcept
    {
        check_mode(::SoC::gpio_mode::output, location);
        check_pin(pin_in, location);
        ::LL_GPIO_TogglePin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::set(::std::source_location location) noexcept
    {
        check_mode(::SoC::gpio_mode::output, location);
        ::LL_GPIO_SetOutputPin(gpio, ::std::to_underlying(pin));
    }

    void ::SoC::gpio_pin::set(pin_enum pin_in, ::std::source_location location) noexcept
    {
        check_mode(::SoC::gpio_mode::output, location);
        check_pin(pin_in, location);
        ::LL_GPIO_SetOutputPin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::reset(::std::source_location location) noexcept
    {
        check_mode(::SoC::gpio_mode::output, location);
        ::LL_GPIO_ResetOutputPin(gpio, ::std::to_underlying(pin));
    }

    void ::SoC::gpio_pin::reset(pin_enum pin_in, ::std::source_location location) noexcept
    {
        check_mode(::SoC::gpio_mode::output, location);
        check_pin(pin_in, location);
        ::LL_GPIO_ResetOutputPin(gpio, ::std::to_underlying(pin_in));
    }

    void ::SoC::gpio_pin::write(bool high_level, ::std::source_location location) noexcept
    {
        if(high_level) { set(location); }
        else { reset(location); }
    }

    void ::SoC::gpio_pin::write(bool high_level, pin_enum pin_in, ::std::source_location location) noexcept
    {
        if(high_level) { set(pin_in, location); }
        else { reset(pin_in, location); }
    }
}  // namespace SoC
