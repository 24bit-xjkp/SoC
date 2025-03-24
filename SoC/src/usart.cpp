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
                callback = dtor_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_USART1};
                clk = ::SoC::rcc::apb2_freq;
                break;
            case usart2:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
                callback = dtor_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_USART2};
                clk = ::SoC::rcc::apb1_freq;
                break;
            case usart3:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
                callback = dtor_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_USART3};
                clk = ::SoC::rcc::apb1_freq;
                break;
            case uart4:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
                callback = dtor_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_UART4};
                clk = ::SoC::rcc::apb1_freq;
                break;
            case uart5:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
                callback = dtor_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_UART5};
                clk = ::SoC::rcc::apb1_freq;
                break;
            case usart6:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
                callback = dtor_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_USART6};
                clk = ::SoC::rcc::apb2_freq;
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

    ::std::byte SoC::usart::read8() const noexcept
    {
        ::SoC::assert(data_width == ::SoC::usart_data_width::bit8);
        ::SoC::wait_until(::LL_USART_IsActiveFlag_RXNE, usart_ptr);
        return static_cast<::std::byte>(::LL_USART_ReceiveData8(usart_ptr));
    }

    ::std::uint16_t SoC::usart::read9() const noexcept
    {
        ::SoC::assert(data_width == ::SoC::usart_data_width::bit9);
        ::SoC::wait_until(::LL_USART_IsActiveFlag_RXNE, usart_ptr);
        return ::LL_USART_ReceiveData9(usart_ptr);
    }
}  // namespace SoC
