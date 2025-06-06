#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"
#include "../include/tim.hpp"
#include "../include/io.hpp"
#include "../include/heap.hpp"
#include "../include/adc.hpp"
// #include "../include/exti.hpp"
#include "../include/pid.hpp"

using namespace ::SoC::literal;
using namespace ::std::string_view_literals;

constexpr auto prescaler{1};
constexpr auto arr{::SoC::rcc::apb2_tim_freq / 20_K};
constexpr auto actual_arr{arr / 2};

struct adc_test
{
    inline static float coefficient{};
    inline static ::SoC::dma_stream* dma_stream;
    inline static ::std::array<::std::uint16_t, 4> buffer;
    inline static ::SoC::text_ofile<::SoC::dma_stream>* file;
    inline static ::SoC::adc_regular_group* i_sample;
    inline static ::SoC::tim_channel* channel;
    inline static ::SoC::pid* pid;
    inline static float duty{0.8f};
    inline static float i{};
};

extern "C" void ::SoC::DMA2_Stream0_IRQHandler() noexcept
{
    if(adc_test::dma_stream->is_it_tc())
    {
        ::std::uint32_t p0_measure{};
#pragma GCC unroll(4)
        for(auto ch0: adc_test::buffer) { p0_measure += ch0; }
        p0_measure /= 4;
        auto v_p0{p0_measure * adc_test::coefficient};
        // ::SoC::println<"P0: {}", true>(*adc_test::file, ::SoC::round<4>(v_p0));
        auto i{::std::max((v_p0 - 1.66f) / 0.132f, 0.f)};
        adc_test::i = i;
        adc_test::duty = (*adc_test::pid)(i);
        adc_test::channel->set_compare_value(static_cast<::std::uint16_t>(actual_arr * adc_test::duty));
        adc_test::dma_stream->read(::adc_test::buffer.begin(), ::adc_test::buffer.end());
        adc_test::i_sample->reset_dma();
    }
}

struct awd_test
{
    inline static ::SoC::adc_regular_group* awd_sample;
    inline static ::SoC::analog_watchdog* awd;
    inline static ::SoC::gpio_pin* shutdown;
    constexpr inline static auto awd_noise_threshold{20};
};

extern "C" void ::SoC::ADC_IRQHandler() noexcept
{
    if(::awd_test::awd->is_it_awd())
    {
        using namespace ::std::string_view_literals;
        auto sample{::awd_test::awd_sample->get_result()};
        ::SoC::wait_until([] static noexcept { return ::awd_test::awd_sample->get_flag_eocs(); });
        sample += ::awd_test::awd_sample->get_result();
        sample >>= 1;
        auto [_, high_threshold]{::awd_test::awd->get_threshold()};
        if(sample >= high_threshold + ::awd_test::awd_noise_threshold)
        {
            ::awd_test::shutdown->reset();
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::println(::SoC::log_device, "\N{ESCAPE}[31m检测到过压，断开电源\N{ESCAPE}[39m"sv);
            }
        }
        ::awd_test::awd->clear_flag_awd();
        ::awd_test::awd->set_it_awd(false);
    }
}

extern "C" void ::SoC::EXTI2_IRQHandler() noexcept { ::SoC::println(::SoC::log_device, "外部中断触发"); }

int main()
{

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
    auto ram_heap{::SoC::make_ram_heap()};
    ::SoC::ram_allocator.set_heap(ram_heap);

    ::SoC::adc adc1{::SoC::adc::adc1, false};
    ::SoC::dma dma2{::SoC::dma::dma2};
    ::SoC::unique_ptr adc_calibrator{::SoC::ram_allocator.allocate<::SoC::adc_calibrator>()};
    new(adc_calibrator)::SoC::adc_calibrator{adc1, dma2};

    ::SoC::gpio_port gpio_f{::SoC::gpio_port::pf};
    ::SoC::gpio_pin green_led{gpio_f, ::SoC::gpio_pin::p10, ::SoC::gpio_mode::output};
    ::SoC::gpio_pin shutdown{gpio_f, ::SoC::gpio_pin::p11, ::SoC::gpio_mode::output};
    ::awd_test::shutdown = &shutdown;
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
    ::SoC::tim tim8{::SoC::tim::tim8, prescaler - 1, actual_arr, ::SoC::tim_mode::center_up_down};
    tim8.set_trigger_output(::SoC::tim_trigger_output::update);
    ::SoC::tim_channel tim8_ch1{tim8,
                                ::SoC::tim_channel::ch1,
                                ::SoC::tim_oc_mode::pwm1,
                                static_cast<::std::uint32_t>(actual_arr * 0.8)};
    adc_test::channel = &tim8_ch1;
    ::SoC::pid pid{1.05f, 0.2f, 0.02f, 0.02f};
    adc_test::pid = &pid;
    tim8.enable();

    // ::SoC::syscfg syscfg{};
    // ::SoC::gpio_pin exti_pin{gpio_c,
    //                          ::SoC::gpio_pin::p2,
    //                          ::SoC::gpio_mode::input,
    //                          ::SoC::gpio_af::default_af,
    //                          ::SoC::gpio_speed::default_speed,
    //                          ::SoC::gpio_pull::pull_up};
    // ::SoC::exti_line exti{syscfg, exti_pin, {::SoC::exti_trigger_source::falling}};
    // exti.enable_irq(1, 0);

    auto usart1_dma_write{usart1.enable_dma_write(dma2, ::SoC::dma_fifo_threshold::full, ::SoC::dma_memory_burst::inc16)};
    ::SoC::text_ofile file{usart1_dma_write, {}};
    ::adc_test::file = &file;
    // auto cnt{0zu};

    auto&& [coefficient, _]{adc_calibrator->get_result()};
    ::adc_test::coefficient = coefficient;
    adc_calibrator.release();

    ::SoC::adc_regular_group i_sample{adc1,
                                      ::SoC::adc_regular_trigger_source::tim8_trgo,
                                      false,
                                      ::SoC::adc_regular_dma_mode::limited,
                                      {{::SoC::adc_channel::ch10, ::SoC::adc_sampling_time::cycles112}}};
    ::adc_test::i_sample = &i_sample;
    auto i_sample_dma{i_sample.enable_dma(dma2,
                                          ::SoC::dma_mode::normal,
                                          ::SoC::dma_fifo_threshold::full,
                                          ::SoC::dma_memory_burst::inc8,
                                          ::SoC::dma_priority::high)};
    adc1.enable();
    ::adc_test::dma_stream = &i_sample_dma;
    i_sample_dma.read(::adc_test::buffer.begin(), ::adc_test::buffer.end());
    i_sample_dma.enable_irq(2, 0);
    i_sample_dma.set_it_tc(true);
    i_sample.enable(::SoC::adc_trig_edge::rising);

    ::SoC::adc adc2{::SoC::adc::adc2, false};
    ::SoC::adc_regular_group awd_sample{adc2,
                                        ::SoC::adc_regular_trigger_source::software,
                                        true,
                                        ::SoC::adc_regular_dma_mode::none,
                                        {{::SoC::adc_channel::ch11, ::SoC::adc_sampling_time::cycles84}}};
    ::awd_test::awd_sample = &awd_sample;
    adc2.enable();
    awd_sample.enable(::SoC::adc_trig_edge::software);
    ::SoC::analog_watchdog awd{adc2, ::SoC::analog_watchdog::ch11_reg, 0, 3130};
    ::awd_test::awd = &awd;
    awd.enable_irq(0);
    awd.set_it_awd(true);

#pragma GCC unroll 0
    while(true)
    {
        ::SoC::wait_for(0.5_s);
        green_led.toggle();
        ::SoC::println<"电流采样: {}">(file, adc_test::i);
        ::SoC::println<"占空比: {}", true>(file, adc_test::duty);
    }
}
