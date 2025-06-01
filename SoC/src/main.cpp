#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"
#include "../include/tim.hpp"
#include "../include/io.hpp"
#include "../include/fmt.hpp"
#include "../include/nvic.hpp"

int main()
{
    using namespace ::SoC::literal;
    using namespace ::std::string_view_literals;

    ::SoC::system_clock_init();
    ::SoC::set_priority_group();
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

    ::SoC::dma dma2{::SoC::dma::dma2};
    auto usart1_dma_write{usart1.enable_dma_write(dma2, ::SoC::dma_fifo_threshold::full, ::SoC::dma_memory_burst::inc16)};
    ::SoC::text_ofile file{usart1_dma_write, {}};
    auto cnt{::std::bit_cast<::std::uint32_t>(::std::numbers::sqrt2_v<float>)};

#pragma GCC unroll 0
    while(true)
    {
        ::SoC::wait_for(0.5_s);
        green_led.toggle();
        auto temp{static_cast<::std::uint64_t>(cnt) * cnt};
        cnt = (temp << 16) >> 32;
        ::SoC::println<"{{当前计数器: {}}}", true>(file, cnt);
    }
}
