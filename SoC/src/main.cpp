#include "../include/init.hpp"
#include "../include/gpio.hpp"

int main()
{
    using namespace ::SoC::literal;
    ::SoC::system_clock_init();
    auto gpio_f{::SoC::gpio_port(::SoC::gpio_port::gpio_f)};
    auto gpio_f10{::SoC::gpio_pin(gpio_f, ::SoC::gpio_pin::pin_10, ::SoC::gpio_mode::output, ::SoC::gpio_speed::low)};
    gpio_f10.set();

    while(true)
    {
        ::SoC::wait_for(1_s);
        gpio_f10.toggle();
    }
}
