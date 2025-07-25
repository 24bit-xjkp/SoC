#include "../include/usart.hpp"
#include "../include/init.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::usart::usart(usart_enum usart,
                        ::std::uint32_t baud_rate,
                        ::SoC::usart_mode mode,
                        ::SoC::usart_data_width data_width,
                        ::SoC::usart_stop_bit stop_bit,
                        ::SoC::usart_parity parity,
                        ::SoC::usart_direction direction,
                        ::SoC::usart_hardware_flow_control control,
                        ::SoC::usart_oversampling oversampling) noexcept :
        usart_ptr{::std::bit_cast<::USART_TypeDef*>(usart)}, data_width{data_width}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此串口不应处于使能状态"sv); }
        ::std::uint32_t clk{};
        switch(usart)
        {
            case usart1:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_USART1};
                clk = ::SoC::rcc::apb2_freq;
                irqn = ::USART1_IRQn;
                break;
            case usart2:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_USART2};
                clk = ::SoC::rcc::apb1_freq;
                irqn = ::USART2_IRQn;
                break;
            case usart3:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_USART3};
                clk = ::SoC::rcc::apb1_freq;
                irqn = ::USART3_IRQn;
                break;
            case uart4:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_UART4};
                clk = ::SoC::rcc::apb1_freq;
                irqn = ::UART4_IRQn;
                break;
            case uart5:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_UART5};
                clk = ::SoC::rcc::apb1_freq;
                irqn = ::UART5_IRQn;
                break;
            case usart6:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_USART6};
                clk = ::SoC::rcc::apb2_freq;
                irqn = ::USART6_IRQn;
                break;
        }

        ::LL_USART_ConfigCharacter(usart_ptr,
                                   ::std::to_underlying(data_width),
                                   ::std::to_underlying(parity),
                                   ::std::to_underlying(stop_bit));
        ::LL_USART_SetTransferDirection(usart_ptr, ::std::to_underlying(direction));
        ::LL_USART_SetHWFlowCtrl(usart_ptr, ::std::to_underlying(control));

        usart_ptr->BRR = static_cast<::std::uint16_t>(clk / (baud_rate << ::std::to_underlying(oversampling) >> 4));

        switch(mode)
        {
            case ::SoC::usart_mode::async: [[likely]] ::LL_USART_ConfigAsyncMode(usart_ptr); break;
            case ::SoC::usart_mode::sync: ::LL_USART_ConfigSyncMode(usart_ptr); break;
        }

        enable();
    }

    ::SoC::usart::usart(usart&& other) noexcept
    {
        ::std::memcpy(reinterpret_cast<void*>(this), &other, sizeof(*this));
        other.usart_ptr = nullptr;
    }

    ::SoC::usart::~usart() noexcept
    {
        if(usart_ptr != nullptr)
        {
            disable();
            callback();
            disable_irq();
        }
    }

    void ::SoC::usart::wait_until_write_complete() const noexcept { ::SoC::wait_until(::LL_USART_IsActiveFlag_TC, usart_ptr); }

    void ::SoC::usart::write(::std::uint8_t byte) const noexcept
    {
        if(data_width == ::SoC::usart_data_width::bit8) [[likely]] { ::LL_USART_TransmitData8(usart_ptr, byte); }
        else
        {
            ::LL_USART_TransmitData9(usart_ptr, byte);
        }
        wait_until_write_complete();
    }

    [[gnu::noinline]] void ::SoC::usart::write(const void* buffer, const void* end) const noexcept
    {
        if(data_width == ::SoC::usart_data_width::bit8) [[likely]]
        {
#pragma GCC unroll 0
            for(auto data: ::std::ranges::subrange{reinterpret_cast<const ::std::byte*>(buffer), end})
            {
                wait_until_write_complete();
                ::LL_USART_TransmitData8(usart_ptr, static_cast<::std::uint8_t>(data));
            }
        }
        else
        {
#pragma GCC unroll 0
            for(auto data: ::std::ranges::subrange{reinterpret_cast<const ::std::byte*>(buffer), end})
            {
                wait_until_write_complete();
                ::LL_USART_TransmitData9(usart_ptr, static_cast<::std::uint8_t>(data));
            }
        }
    }

    [[gnu::noinline]] void ::SoC::usart::write(const ::std::uint16_t* buffer, const ::std::uint16_t* end) const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(data_width == ::SoC::usart_data_width::bit9 && ::LL_USART_GetParity(usart_ptr) == ::std::to_underlying(::SoC::usart_parity::none),
                          "只有数据宽度为8位且未启用校验时支持9位输出"sv);
        }
#pragma GCC unroll 0
        for(auto data: ::std::ranges::subrange{buffer, end})
        {
            wait_until_write_complete();
            ::LL_USART_TransmitData9(usart_ptr, data);
        }
    }

    ::std::uint8_t(::SoC::usart::read)() const noexcept
    {
        ::SoC::wait_until(::LL_USART_IsActiveFlag_RXNE, usart_ptr);
        return ::LL_USART_ReceiveData8(usart_ptr);
    }

    ::std::uint16_t(::SoC::usart::read9)() const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(data_width == ::SoC::usart_data_width::bit9, "此函数仅限数据宽度为9位时使用"sv);
        }
        ::SoC::wait_until(::LL_USART_IsActiveFlag_RXNE, usart_ptr);
        return ::LL_USART_ReceiveData9(usart_ptr);
    }

    [[gnu::noinline]] void* ::SoC::usart::read(void* begin, void* end) const noexcept
    {
        auto ptr{reinterpret_cast<::std::uint8_t*>(begin)};
#pragma GCC unroll 0
        while(ptr != end && !get_flag_idle()) { *ptr++ = read(); }
        clear_flag_idle();
        return ptr;
    }

    [[gnu::noinline]] ::std::uint16_t* ::SoC::usart::read(::std::uint16_t* begin, ::std::uint16_t* end) const noexcept
    {
        auto ptr{reinterpret_cast<::std::uint16_t*>(begin)};
#pragma GCC unroll 0
        while(ptr != end && !get_flag_idle()) { *ptr++ = read9(); }
        clear_flag_idle();
        return ptr;
    }

    void ::SoC::usart::enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) const noexcept
    {
        ::SoC::set_priority(irqn, preempt_priority, sub_priority);
        ::SoC::enable_irq(irqn);
    }

    void ::SoC::usart::enable_irq(::std::size_t encoded_priority) const noexcept
    {
        ::SoC::set_priority(irqn, encoded_priority);
        ::SoC::enable_irq(irqn);
    }

    void ::SoC::usart::disable_irq() const noexcept { ::SoC::disable_irq(irqn); }

    void ::SoC::usart::set_it_txe(bool enable) const noexcept
    {
        if(enable) { ::LL_USART_EnableIT_TXE(usart_ptr); }
        else
        {
            ::LL_USART_DisableIT_TXE(usart_ptr);
        }
    }

    bool ::SoC::usart::get_it_txe() const noexcept { return ::LL_USART_IsEnabledIT_TXE(usart_ptr); }

    bool ::SoC::usart::get_flag_txe() const noexcept { return ::LL_USART_IsActiveFlag_TXE(usart_ptr); }

    bool ::SoC::usart::is_it_txe() const noexcept { return get_flag_txe() && get_it_txe(); }

    void ::SoC::usart::set_it_rxne(bool enable) const noexcept
    {
        if(enable) { ::LL_USART_EnableIT_RXNE(usart_ptr); }
        else
        {
            ::LL_USART_DisableIT_RXNE(usart_ptr);
        }
    }

    bool ::SoC::usart::get_it_rxne() const noexcept { return ::LL_USART_IsEnabledIT_RXNE(usart_ptr); }

    bool ::SoC::usart::get_flag_rxne() const noexcept { return ::LL_USART_IsActiveFlag_RXNE(usart_ptr); }

    bool ::SoC::usart::is_it_rxne() const noexcept { return get_flag_rxne() && get_it_rxne(); }

    void ::SoC::usart::set_it_idle(bool enable) const noexcept
    {
        if(enable) { ::LL_USART_EnableIT_IDLE(usart_ptr); }
        else
        {
            ::LL_USART_DisableIT_IDLE(usart_ptr);
        }
    }

    bool ::SoC::usart::get_it_idle() const noexcept { return ::LL_USART_IsEnabledIT_IDLE(usart_ptr); }

    bool ::SoC::usart::get_flag_idle() const noexcept { return ::LL_USART_IsActiveFlag_IDLE(usart_ptr); }

    bool ::SoC::usart::is_it_idle() const noexcept { return get_flag_idle() && get_it_idle(); }

    void ::SoC::usart::clear_flag_idle() const noexcept { ::LL_USART_ClearFlag_IDLE(usart_ptr); }

    void ::SoC::usart::enable() const noexcept { ::LL_USART_Enable(usart_ptr); }

    void ::SoC::usart::disable() const noexcept { ::LL_USART_Disable(usart_ptr); }

    bool ::SoC::usart::is_enabled() const noexcept { return ::LL_USART_IsEnabled(usart_ptr); }

    void ::SoC::usart::assert_dma(::SoC::dma& dma, ::SoC::dma::dma_enum dma_enum) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(dma.get_dma_enum() == dma_enum, "该dma外设不能操作该串口"sv); }
    }

    /// 选择的dma数据流无效时报错信息
    constexpr auto selected_stream_error_msg{"该串口不能使用指定的dma数据流"sv};
    /// 配置前dma已经处于使能状态时报错信息
    constexpr auto dma_enabled_error_msg{"在配置前该串口的dma不应处于使能状态"sv};

    ::SoC::dma_stream(::SoC::usart::enable_dma_write)(::SoC::dma& dma,
                                                      ::SoC::dma_fifo_threshold fifo_threshold,
                                                      ::SoC::dma_memory_burst default_burst,
                                                      ::SoC::dma_memory_data_size default_data_size,
                                                      ::SoC::dma_priority priority,
                                                      ::SoC::dma_mode mode,
                                                      ::SoC::dma_stream::dma_stream_enum selected_stream) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_dma_write_enabled(), ::SoC::dma_enabled_error_msg); }
        using enum ::SoC::dma::dma_enum;
        using enum ::SoC::dma_stream::dma_stream_enum;
        using enum ::SoC::dma_channel;
        ::SoC::dma::dma_enum dma_enum;
        ::SoC::dma_stream::dma_stream_enum stream;
        ::SoC::dma_channel channel;
        switch(get_usart_enum())
        {
            case usart1:
                dma_enum = dma2;
                stream = st7;
                channel = ch4;
                break;
            case usart2:
                dma_enum = dma1;
                stream = st6;
                channel = ch4;
                break;
            case usart3:
                dma_enum = dma1;
                stream = st3;
                channel = ch4;
                break;
            case uart4:
                dma_enum = dma1;
                stream = st4;
                channel = ch4;
                break;
            case uart5:
                dma_enum = dma1;
                stream = st7;
                channel = ch4;
                break;
            case usart6:
                dma_enum = dma2;
                if(selected_stream == no_selected_stream) [[likely]] { stream = st6; }
                else
                {
                    if constexpr(::SoC::use_full_assert)
                    {
                        ::SoC::assert(selected_stream == st6 || selected_stream == st7, ::SoC::selected_stream_error_msg);
                    }
                    stream = selected_stream;
                }
                channel = ch5;
                break;
        }
        if constexpr(::SoC::use_full_assert) { assert_dma(dma, dma_enum); }
        ::LL_USART_EnableDMAReq_TX(usart_ptr);
        return ::SoC::dma_stream{dma,
                                 stream,
                                 channel,
                                 ::LL_USART_DMA_GetRegAddr(usart_ptr),
                                 ::SoC::dma_direction::m2p,
                                 mode,
                                 false,
                                 true,
                                 ::SoC::dma_periph_data_size::byte,
                                 default_data_size,
                                 priority,
                                 fifo_threshold,
                                 default_burst,
                                 ::SoC::dma_periph_burst::single};
    }

    void ::SoC::usart::disable_dma_write() const noexcept { ::LL_USART_DisableDMAReq_TX(usart_ptr); }

    bool ::SoC::usart::is_dma_write_enabled() const noexcept { return ::LL_USART_IsEnabledDMAReq_TX(usart_ptr); }
}  // namespace SoC
