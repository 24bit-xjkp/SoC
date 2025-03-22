#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"

int main()
{
    using namespace ::SoC::literal;
    ::SoC::system_clock_init();
    ::SoC::gpio_port gpio_f{::SoC::gpio_port::gpio_f};
    ::SoC::gpio_pin gpio_f10{gpio_f, ::SoC::gpio_pin::pin_10, ::SoC::gpio_mode::output};
    gpio_f10.set();
    auto cnt{0zu};
    char buffer[32];
    ::SoC::gpio_port gpio_a{::SoC::gpio_port::gpio_a};
    ::SoC::gpio_pin _{gpio_a,
                      ::SoC::gpio_pin::pin_9 | ::SoC::gpio_pin::pin_10,
                      ::SoC::gpio_mode::alternate,
                      ::SoC::gpio_alternate_function::af7,
                      ::SoC::gpio_speed::high};

    ::SoC::usart usart1{::SoC::usart::usart1, 115.2_K};
    const char prefix[]{"当前计数器："};

    while(true)
    {
        ::SoC::wait_for(1_s);
        auto&& [end, _]{::std::to_chars(buffer, buffer + 32, cnt++)};
        *end++ = '\r';
        *end++ = '\n';
        gpio_f10.toggle();
        usart1.write(prefix, prefix + sizeof(prefix) - 1);
        usart1.write(buffer, end);
    }
}
