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
    ::SoC::gpio_port gpio_a{::SoC::gpio_port::gpio_a};
    ::SoC::gpio_pin _{gpio_a,
                      ::SoC::gpio_pin::pin_9 | ::SoC::gpio_pin::pin_10,
                      ::SoC::gpio_mode::alternate,
                      ::SoC::gpio_alternate_function::af7,
                      ::SoC::gpio_speed::high};
    ::SoC::usart usart1{::SoC::usart::usart1, 115.2_K};
    ::SoC::log_device.set(usart1.write_wrapper, &usart1);

    ::SoC::gpio_port gpio_f{::SoC::gpio_port::gpio_f};
    ::SoC::gpio_pin gpio_f10{gpio_f, ::SoC::gpio_pin::pin_10, ::SoC::gpio_mode::output};
    gpio_f10.set();

    ::SoC::gpio_pin gpio_f9{gpio_f, ::SoC::gpio_pin::pin_9, ::SoC::gpio_mode::alternate, ::SoC::gpio_alternate_function::af9};
    constexpr auto arr{5_K - 1};
    ::SoC::tim tim14{::SoC::tim::tim14, ::SoC::rcc::apb1_freq / 1_M - 1, arr};
    const float table[]{0, 0.5, 1, 0.5};
    constexpr auto size{sizeof(table) / sizeof(float)};
    auto i{0zu};
    ::SoC::tim_channel tim14_ch1{tim14,
                                 ::SoC::tim_channel::ch1,
                                 ::SoC::tim_oc_mode::pwm1,
                                 static_cast<::std::size_t>(arr * table[i++])};
    tim14_ch1.enable_oc_preload();
    tim14.enable();

    ::SoC::text_ofile file{usart1, {}};
    for(auto cnt{0zu};; ++cnt, i = ++i % size)
    {
        ::SoC::wait_for(0.5_s);
        gpio_f10.toggle();
        tim14_ch1.set_compare_value(static_cast<::std::size_t>(arr * table[i]));
        ::SoC::println<true>(file, "当前计数器："sv, cnt);
    }
}
