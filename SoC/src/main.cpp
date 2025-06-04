#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"
#include "../include/tim.hpp"
#include "../include/io.hpp"
#include "../include/fmt.hpp"
#include "../include/adc.hpp"

struct adc_test
{
    inline static float coefficient{0.0008f};
    inline static ::SoC::dma_stream* dma_stream;
    inline static ::std::array<::std::array<::std::uint16_t, 2>, 32> buffer;
    inline static ::SoC::text_ofile<::SoC::dma_stream>* file;
};

extern "C" void DMA2_Stream0_IRQHandler() noexcept
{
    if(adc_test::dma_stream->is_it_tc())
    {
        ::std::uint32_t p0_measure{}, p1_measure{};
        for(auto&& [ch0, ch1]: adc_test::buffer)
        {
            p0_measure += ch0;
            p1_measure += ch1;
        }
        p0_measure >>= 5;
        p1_measure >>= 5;
        auto v_p0{p0_measure * adc_test::coefficient};
        auto v_p1{p1_measure * adc_test::coefficient};
        ::SoC::println<"PC0电压: {}, PC1电压: {}", true>(*adc_test::file, v_p0, v_p1);
        adc_test::dma_stream->read(::adc_test::buffer.begin(), ::adc_test::buffer.end());
    }
}

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

    ::SoC::gpio_port gpio_c{::SoC::gpio_port::pc};
    ::SoC::gpio_pin tim8_ch1_pin{gpio_c, ::SoC::gpio_pin::p6, ::SoC::gpio_mode::alternate, ::SoC::gpio_af::af3};
    ::SoC::gpio_pin adc_pin{gpio_c,
                            ::SoC::gpio_pin::p0 | ::SoC::gpio_pin::p1 | ::SoC::gpio_pin::p2 | ::SoC::gpio_pin::p3,
                            ::SoC::gpio_mode::analog};
    // constexpr auto pwm_freq{100_K};
    // constexpr auto arr{::SoC::rcc::apb2_tim_freq / pwm_freq / 2 - 1};
    // constexpr auto repeat_cnt{10};
    constexpr auto prescaler{::SoC::rcc::apb2_tim_freq / 32_K};
    constexpr auto arr{100};
    constexpr auto repeat_cnt{10};
    ::SoC::tim tim8{::SoC::tim::tim8,
                    prescaler - 1,
                    arr,
                    ::SoC::tim_mode::center_up_down,
                    ::SoC::tim_clock_div::div1,
                    repeat_cnt - 1};
    tim8.set_trigger_output(::SoC::tim_trigger_output::update);
    ::SoC::tim_channel tim8_ch1{tim8, ::SoC::tim_channel::ch1, ::SoC::tim_oc_mode::pwm1, static_cast<::std::uint32_t>(arr * 0.5)};
    tim8.enable();

    ::SoC::dma dma2{::SoC::dma::dma2};
    auto usart1_dma_write{usart1.enable_dma_write(dma2, ::SoC::dma_fifo_threshold::full, ::SoC::dma_memory_burst::inc16)};
    ::SoC::text_ofile file{usart1_dma_write, {}};
    ::adc_test::file = &file;
    // auto cnt{0zu};

    ::SoC::adc adc1{::SoC::adc::adc1, true};
    auto&& [coefficient, _]{
        ::SoC::adc_calibrator{adc1, dma2}
        .get_result()
    };
    ::adc_test::coefficient = coefficient;

    ::SoC::adc_regular_group adc_sample{
        adc1,
        ::SoC::adc_regular_trigger_source::tim8_trgo,
        false,
        ::SoC::adc_regular_dma_mode::unlimited,
        {{::SoC::adc_channel::ch10, ::SoC::adc_sampling_time::cycles112},
                                              {::SoC::adc_channel::ch11, ::SoC::adc_sampling_time::cycles112}}
    };
    auto adc_dma{adc_sample.enable_dma(dma2,
                                       ::SoC::dma_mode::normal,
                                       ::SoC::dma_fifo_threshold::disable,
                                       ::SoC::dma_memory_burst::single,
                                       ::SoC::dma_priority::high)};
    adc1.enable();
    ::adc_test::dma_stream = &adc_dma;
    adc_dma.read(::adc_test::buffer.begin(), ::adc_test::buffer.end());
    adc_dma.enable_irq(2, 0);
    adc_dma.set_it_tc(true);
    adc_sample.enable(::SoC::adc_trig_edge::rising);

#pragma GCC unroll 0
    while(true)
    {
        ::SoC::wait_for(0.5_s);
        green_led.toggle();
    }
}
