#include "../include/init.hpp"
#include "../include/gpio.hpp"

int main()
{
    using namespace ::SoC::literal;
    ::SoC::system_clock_init();
    auto gpio_f{::SoC::gpio_port(::SoC::gpio_port::gpio_f)};
    auto gpio_f10{::SoC::gpio_pin(gpio_f, ::SoC::gpio_pin::pin_10, ::SoC::gpio_mode::output, ::SoC::gpio_speed::low)};
    gpio_f10.reset();

    auto gpio_e{::SoC::gpio_port(::SoC::gpio_port::gpio_e)};
    auto gpio_e4{::SoC::gpio_pin(gpio_e,
                                 ::SoC::gpio_pin::pin_4,
                                 ::SoC::gpio_mode::input,
                                 ::SoC::gpio_speed::low,
                                 ::SoC::gpio_pull::pull_up)};

    while(true)
    {
        ::SoC::wait_for(20_ms);
        gpio_f10.write(!gpio_e4.read());
    }
}
