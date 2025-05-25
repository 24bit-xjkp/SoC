#include "../include/usart.hpp"
#include "../include/init.hpp"

namespace SoC
{
    ::SoC::usart::usart(usart_enum usart,
                        ::std::uint32_t baud_rate,
                        ::SoC::usart_mode mode,
                        ::SoC::usart_data_width data_width,
                        ::SoC::usart_stop_bit stop_bit,
                        ::SoC::usart_parity parity,
                        ::SoC::usart_direction direction,
                        ::SoC::usart_hardware_flow_control control,
                        ::SoC::usart_oversampling oversampling) noexcept :
        usart_ptr{reinterpret_cast<::USART_TypeDef*>(usart)}, mode{mode}, data_width{data_width}, parity{parity}
    {
        ::SoC::assert(!::LL_USART_IsEnabled(usart_ptr));
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

        ::LL_USART_Enable(usart_ptr);
    }

    ::SoC::usart::usart(usart&& other) noexcept : usart_ptr{::std::exchange(other.usart_ptr, nullptr)}, mode{other.mode} {}

    ::SoC::usart& ::SoC::usart::operator= (usart&& other) noexcept
    {
        ::std::swap(*this, other);
        return *this;
    }

    ::SoC::usart::~usart() noexcept
    {
        if(usart_ptr != nullptr)
        {
            ::LL_USART_Disable(usart_ptr);
            callback();
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

    void ::SoC::usart::write(const void* buffer, const void* end) const noexcept
    {
        if(data_width == ::SoC::usart_data_width::bit8) [[likely]]
        {
            for(auto data: ::std::ranges::subrange{reinterpret_cast<const ::std::byte*>(buffer), end})
            {
                wait_until_write_complete();
                ::LL_USART_TransmitData8(usart_ptr, static_cast<::std::uint8_t>(data));
            }
        }
        else
        {
            for(auto data: ::std::ranges::subrange{reinterpret_cast<const ::std::byte*>(buffer), end})
            {
                wait_until_write_complete();
                ::LL_USART_TransmitData9(usart_ptr, static_cast<::std::uint8_t>(data));
            }
        }
    }

    void ::SoC::usart::write(const ::std::uint16_t* buffer, const ::std::uint16_t* end) const noexcept
    {
        ::SoC::assert(data_width == ::SoC::usart_data_width::bit9 && parity == ::SoC::usart_parity::none);
        for(auto data: ::std::ranges::subrange{buffer, end})
        {
            wait_until_write_complete();
            ::LL_USART_TransmitData9(usart_ptr, data);
        }
    }

    ::std::uint8_t(::SoC::usart::read)() const noexcept
    {
        ::SoC::assert(data_width == ::SoC::usart_data_width::bit8);
        ::SoC::wait_until(::LL_USART_IsActiveFlag_RXNE, usart_ptr);
        return ::LL_USART_ReceiveData8(usart_ptr);
    }

    ::std::uint16_t(::SoC::usart::read9)() const noexcept
    {
        ::SoC::assert(data_width == ::SoC::usart_data_width::bit9);
        ::SoC::wait_until(::LL_USART_IsActiveFlag_RXNE, usart_ptr);
        return ::LL_USART_ReceiveData9(usart_ptr);
    }

    void ::SoC::usart::enable_irq(::std::size_t priority) const noexcept
    {
        ::NVIC_EnableIRQ(irqn);
        ::NVIC_SetPriority(irqn, priority);
    }

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

    void ::SoC::usart::clear_flag_idle() const noexcept { ::LL_USART_ClearFlag_IDLE(usart_ptr); }
}  // namespace SoC
