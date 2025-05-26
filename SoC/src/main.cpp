#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"
#include "../include/tim.hpp"
#include "../include/io.hpp"

int main()
{
    using namespace ::SoC::literal;
    using namespace ::std::string_view_literals;

    ::SoC::system_clock_init();
    ::SoC::gpio_port gpio_a{::SoC::gpio_port::pa};
    ::SoC::gpio_pin _{gpio_a, ::SoC::gpio_pin::p9 | ::SoC::gpio_pin::p10, ::SoC::gpio_mode::alternate, ::SoC::gpio_af::af7};
    ::SoC::usart usart1{::SoC::usart::usart1, 115.2_K};
    ::SoC::log_device.set(usart1.write_wrapper, &usart1);

    ::SoC::gpio_port gpio_f{::SoC::gpio_port::pf};
    ::SoC::gpio_pin green_led{gpio_f, ::SoC::gpio_pin::p10, ::SoC::gpio_mode::output};
    ::SoC::gpio_pin shutdown{gpio_f, ::SoC::gpio_pin::p11, ::SoC::gpio_mode::output};
    green_led.set();
    shutdown.set();

    ::SoC::gpio_port gpio_e{::SoC::gpio_port::pe};
    ::SoC::gpio_pin _{gpio_e, ::SoC::gpio_pin::p9, ::SoC::gpio_mode::alternate, ::SoC::gpio_af::af1};

    constexpr auto arr{::SoC::rcc::apb2_tim_freq / 100_K - 1};
    ::SoC::tim tim1{::SoC::tim::tim1, 0, arr};
    ::SoC::tim_channel tim1_ch1{tim1, ::SoC::tim_channel::ch1, ::SoC::tim_oc_mode::pwm1, static_cast<::std::uint32_t>(arr * 0.8)};
    tim1.enable();

    while(true)
    {
        ::SoC::wait_for(1_s);
        green_led.toggle();
    }
}
