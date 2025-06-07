#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"
#include "../include/tim.hpp"
#include "../include/io.hpp"
#include "../include/heap.hpp"
#include "../include/adc.hpp"
#include "../include/exti.hpp"
#include "../include/pid.hpp"

using namespace ::SoC::literal;
using namespace ::std::string_view_literals;

constexpr auto prescaler{1};
constexpr auto arr{::SoC::rcc::apb2_tim_freq / 20_K};
constexpr auto actual_arr{arr / 2};
inline ::SoC::optional<::SoC::text_ofile<::SoC::dma_stream>&> file;

namespace pid_controller
{
    inline float coefficient;
    inline ::SoC::optional<::SoC::dma_stream&> dma_stream;
    inline ::std::array<::std::uint16_t, 4> buffer;
    inline ::SoC::optional<::SoC::adc_regular_group&> i_sample;
    inline ::SoC::optional<::SoC::tim_channel&> channel;
    inline ::SoC::optional<::SoC::pid&> pid;
    inline float duty;
    inline float i_sample_value{};
    inline float i_sample_sum{};
    inline ::std::size_t i_sample_cnt{};
    constexpr inline auto float_atomic_exchange_flag{::std::numeric_limits<float>::quiet_NaN()};

    void calculate_i_sample_value(float i) noexcept
    {
        i_sample_sum += i;
        // 中断即使被打断也不涉及对共享变量的操作，不需要原子交换
        if(::std::exchange(i_sample_value, i_sample_sum / ++i_sample_cnt) == float_atomic_exchange_flag)
        {
            i_sample_sum = 0.f;
            i_sample_cnt = 0;
        }
    }

    extern "C" void DMA2_Stream0_IRQHandler() noexcept
    {
        if(dma_stream->is_it_tc())
        {
            ::std::uint32_t p0_measure{};
#pragma GCC unroll(4)
            for(auto ch0: buffer) { p0_measure += ch0; }
            p0_measure /= 4;
            auto v_p0{p0_measure * coefficient};
            auto i{::std::max((v_p0 - 1.66f) / 0.132f, 0.f)};
            duty = pid(i);
            channel->set_compare_value(actual_arr * duty);
            calculate_i_sample_value(i);
            dma_stream->read(buffer.begin(), buffer.end());
            i_sample->reset_dma();
        }
    }

    float get_i_sample_value() noexcept
    {
        // 可能被中断打断，而中断会修改共享变量，需要原子交换
        ::std::atomic_ref ref{i_sample_value};
        return ref.exchange(float_atomic_exchange_flag, ::std::memory_order_relaxed);
    }
}  // namespace pid_controller

namespace shutdown_awd
{
    inline ::SoC::optional<::SoC::adc_regular_group&> awd_sample;
    inline ::SoC::optional<::SoC::analog_watchdog&> awd;
    inline ::SoC::optional<::SoC::gpio_pin&> shutdown;
    constexpr inline auto awd_noise_threshold{20};

    extern "C" void ADC_IRQHandler() noexcept
    {
        if(awd->is_it_awd())
        {
            using namespace ::std::string_view_literals;
            auto sample{awd_sample->get_result()};
            ::SoC::wait_until([] static noexcept { return awd_sample->get_flag_eocs(); });
            sample += awd_sample->get_result();
            sample >>= 1;
            auto [_, high_threshold]{awd->get_threshold()};
            if(sample >= high_threshold + awd_noise_threshold)
            {
                shutdown->reset();
                ::SoC::println(::file.get(), "\N{ESCAPE}[31m检测到过压，断开电源\N{ESCAPE}[39m"sv);
            }
            awd->clear_flag_awd();
            awd->set_it_awd(false);
        }
    }
}  // namespace shutdown_awd

namespace key_check_tim
{
    inline ::SoC::optional<::SoC::tim&> key_check_tim;
    inline ::SoC::optional<::SoC::gpio_pin&> key_pin;
    constexpr ::std::array pin_list{::SoC::gpio_pin::p2, ::SoC::gpio_pin::p3};
    inline ::std::array<bool, 2> key_pressed{};

    extern "C" void TIM7_IRQHandler() noexcept
    {
        if(key_check_tim->is_it_update())
        {
            key_check_tim->clear_flag_update();
            for(auto&& [pin, key_pressed]: ::std::views::zip(pin_list, key_pressed))
            {
                auto press{!key_pin->read(pin)};
                if(!key_pressed && press)
                {
                    if constexpr(::SoC::use_full_assert)
                    {
                        ::SoC::println<"按键{}被按下">(::file.get(), ::std::countr_zero(::std::to_underlying(pin)) - 1);
                    }
                    auto delta{pin == ::SoC::gpio_pin::p2 ? 0.1f : -0.1f};
                    ::pid_controller::pid->step(delta);
                }
                key_pressed = press;
            }
        }
    }
}  // namespace key_check_tim

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
    ::shutdown_awd::shutdown = shutdown;
    green_led.set();
    shutdown.set();

    ::SoC::gpio_port gpio_c{::SoC::gpio_port::pc};
    ::SoC::gpio_pin tim8_ch1_pin{gpio_c, ::SoC::gpio_pin::p6, ::SoC::gpio_mode::alternate, ::SoC::gpio_af::af3};
    ::SoC::gpio_pin adc_pin{gpio_c,
                            ::SoC::gpio_pin::p0 | ::SoC::gpio_pin::p1 | ::SoC::gpio_pin::p2 | ::SoC::gpio_pin::p3,
                            ::SoC::gpio_mode::analog};

    ::SoC::tim tim8{::SoC::tim::tim8, prescaler - 1, actual_arr, ::SoC::tim_mode::center_up_down};
    tim8.set_trigger_output(::SoC::tim_trigger_output::update);
    ::SoC::tim_channel tim8_ch1{tim8,
                                ::SoC::tim_channel::ch1,
                                ::SoC::tim_oc_mode::pwm1,
                                static_cast<::std::uint32_t>(actual_arr * 0.8)};
    tim8_ch1.enable_oc_preload();
    ::pid_controller::channel = tim8_ch1;
    ::SoC::pid pid{1.05f, 0.2f, 0.02f, 0.02f, 1.05f, 2.05f};
    ::pid_controller::pid = pid;
    tim8.enable();

    ::SoC::gpio_pin key_pin{gpio_c,
                            ::SoC::gpio_pin::p2 | ::SoC::gpio_pin::p3,
                            ::SoC::gpio_mode::input,
                            ::SoC::gpio_af::default_af,
                            ::SoC::gpio_speed::default_speed,
                            ::SoC::gpio_pull::pull_up};
    ::key_check_tim::key_pin = key_pin;
    constexpr auto key_check_tim_prescaler{::SoC::rcc::apb1_tim_freq / 10_K};
    constexpr auto key_check_tim_arr{10_K / 50};
    ::SoC::tim key_check_tim{::SoC::tim::tim7, key_check_tim_prescaler - 1, key_check_tim_arr};
    key_check_tim.set_it_update(true);
    key_check_tim.enable_irq(::SoC::tim_irq::normal, 3, 2);
    ::key_check_tim::key_check_tim = key_check_tim;
    key_check_tim.enable();

    auto usart1_dma_write{usart1.enable_dma_write(dma2, ::SoC::dma_fifo_threshold::full, ::SoC::dma_memory_burst::inc16)};
    ::SoC::text_ofile file{usart1_dma_write, {}};
    ::file = file;
    // auto cnt{0zu};

    auto&& [coefficient, _]{adc_calibrator->get_result()};
    ::pid_controller::coefficient = coefficient;
    adc_calibrator.release();

    ::SoC::adc_regular_group i_sample{adc1,
                                      ::SoC::adc_regular_trigger_source::tim8_trgo,
                                      false,
                                      ::SoC::adc_regular_dma_mode::limited,
                                      {{::SoC::adc_channel::ch10, ::SoC::adc_sampling_time::cycles112}}};
    ::pid_controller::i_sample = i_sample;
    auto i_sample_dma{i_sample.enable_dma(dma2,
                                          ::SoC::dma_mode::normal,
                                          ::SoC::dma_fifo_threshold::full,
                                          ::SoC::dma_memory_burst::inc8,
                                          ::SoC::dma_priority::high)};
    adc1.enable();
    ::pid_controller::dma_stream = i_sample_dma;
    i_sample_dma.read(::pid_controller::buffer.begin(), ::pid_controller::buffer.end());
    i_sample_dma.enable_irq(1, 0);
    i_sample_dma.set_it_tc(true);
    i_sample.enable(::SoC::adc_trig_edge::rising);

    ::SoC::adc adc2{::SoC::adc::adc2, false};
    ::SoC::adc_regular_group awd_sample{adc2,
                                        ::SoC::adc_regular_trigger_source::software,
                                        true,
                                        ::SoC::adc_regular_dma_mode::none,
                                        {{::SoC::adc_channel::ch11, ::SoC::adc_sampling_time::cycles84}}};
    ::shutdown_awd::awd_sample = awd_sample;
    adc2.enable();
    awd_sample.enable(::SoC::adc_trig_edge::software);
    ::SoC::analog_watchdog awd{adc2, ::SoC::analog_watchdog::ch11_reg, 0, 3130};
    ::shutdown_awd::awd = awd;
    awd.enable_irq(0);
    awd.set_it_awd(true);

#pragma GCC unroll 0
    while(true)
    {
        ::SoC::wait_for(0.5_s);
        green_led.toggle();
        ::SoC::println<"电流采样: {}A">(file, ::pid_controller::i_sample_value);
        ::SoC::println<"占空比: {}%">(file, ::SoC::round<2>(::pid_controller::duty * 100.f));
        ::SoC::println<"pid目标值: {}A">(file, ::pid_controller::pid->get_target());
        ::SoC::println<true>(file, "--------------------"sv);
    }
}
