#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"
#include "../include/tim.hpp"
#include "../include/io.hpp"
#include "../include/fmt.hpp"
#include "../include/adc.hpp"

int main()
{
    using namespace ::SoC::literal;
    using namespace ::std::string_view_literals;

    ::SoC::system_clock_init();
    ::SoC::enable_prefetch_cache();
    ::SoC::set_priority_group();

    ::SoC::gpio_port gpio_a{::SoC::gpio_port::pa};
    ::SoC::gpio_pin usart1_pin{gpio_a,
                               ::SoC::gpio_pin::p9 | ::SoC::gpio_pin::p10,
                               ::SoC::gpio_mode::alternate,
                               ::SoC::gpio_af::af7};
    ::SoC::usart usart1{::SoC::usart::usart1, 115.2_K};
    ::SoC::log_device.set(usart1.write_wrapper, &usart1);

    ::SoC::gpio_port gpio_f{::SoC::gpio_port::pf};
    ::SoC::gpio_pin green_led{gpio_f, ::SoC::gpio_pin::p10, ::SoC::gpio_mode::output};
    ::SoC::gpio_pin shutdown{gpio_f, ::SoC::gpio_pin::p11, ::SoC::gpio_mode::output};
    green_led.set();
    shutdown.set();

    ::SoC::gpio_port gpio_c{::SoC::gpio_port::pe};
    ::SoC::gpio_pin tim8_ch1_pin{gpio_c, ::SoC::gpio_pin::p6, ::SoC::gpio_mode::alternate, ::SoC::gpio_af::af3};
    constexpr auto pwm_freq{100_K};
    constexpr auto arr{::SoC::rcc::apb2_tim_freq / pwm_freq / 2 - 1};
    constexpr auto repeat_cnt{10};
    ::SoC::tim tim8{::SoC::tim::tim8, 0, arr, ::SoC::tim_mode::center_up_down, ::SoC::tim_clock_div::div1, repeat_cnt - 1};
    tim8.set_trigger_output(::SoC::tim_trigger_output::update);
    ::SoC::tim_channel tim8_ch1{tim8, ::SoC::tim_channel::ch1, ::SoC::tim_oc_mode::pwm1, static_cast<::std::uint32_t>(arr * 0.5)};
    tim8.enable();

    ::SoC::dma dma2{::SoC::dma::dma2};
    auto usart1_dma_write{usart1.enable_dma_write(dma2, ::SoC::dma_fifo_threshold::full, ::SoC::dma_memory_burst::inc16)};
    ::SoC::text_ofile file{usart1_dma_write, {}};
    auto cnt{0zu};

    ::SoC::adc adc1{::SoC::adc::adc1, true};
    auto&& [coefficient, temp]{
        ::SoC::adc_calibrator{adc1, dma2}
        .get_result()
    };

#pragma GCC unroll 0
    while(true)
    {
        ::SoC::wait_for(0.5_s);
        green_led.toggle();
        ::SoC::println<"{{Vcc: {}, 结温: {}}}">(file, coefficient * 4095.f, temp);
        ::SoC::println<"{{当前计数器: {}}}", true>(file, cnt++);
    }
}
