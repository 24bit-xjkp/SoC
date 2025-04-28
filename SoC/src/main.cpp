/**
 * @file main.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 专项训练串口实验实现代码
 * @version 0.1
 * @date 2025-04-28
 *
 */

#include "../include/init.hpp"
#include "../include/gpio.hpp"
#include "../include/usart.hpp"

char global_in_buffer[64];

struct usart_t
{
    struct out_buffer_t
    {
        const char* buffer{};
        const char* end{};
    } out_buffer{};

    struct in_file_buffer_t
    {
        char* begin{global_in_buffer};
        char* current{global_in_buffer};
        char* end{global_in_buffer + sizeof(global_in_buffer)};

        void clear()
        {
            begin = global_in_buffer;
            current = global_in_buffer;
            end = global_in_buffer + sizeof(global_in_buffer);
        }
    } in_buffer{};

    ::SoC::usart* usart{};
    bool transmit_end{true};
    bool receive_end{};

    void wait_for_eot() const noexcept
    {
        ::SoC::wait_until([this] noexcept { return transmit_end; });
    }

    void write(::std::uint8_t byte) noexcept
    {
        wait_for_eot();
        transmit_end = false;
        usart->write(byte);
        transmit_end = true;
    }

    void write(const void* buffer, const void* end) noexcept
    {
        wait_for_eot();
        transmit_end = false;
        out_buffer.buffer = reinterpret_cast<const char*>(buffer);
        out_buffer.end = reinterpret_cast<const char*>(end);
        usart->set_it_txe(true);
    }
} usart{};

extern "C" void USART1_IRQHandler() noexcept
{
    auto&& [obuf, ibuf, dev, eot, eor]{usart};
    if(dev->get_flag_txe() && dev->get_it_txe())
    {
        if(auto& cur{obuf.buffer}; cur != obuf.end) { dev->write(*cur++); }
        else
        {
            eot = true;
            dev->set_it_txe(false);
        }
    }
    else if(dev->get_flag_rxne() && dev->get_it_rxne())
    {
        *ibuf.current++ = dev->read();
        eor = true;
    }
}

enum class operate_t
{
    add,
    sub,
    mul,
    div
};

int main()
{
    using namespace ::SoC::literal;
    using namespace ::std::string_view_literals;

    ::SoC::system_clock_init();
    ::SoC::gpio_port gpio_a{::SoC::gpio_port::gpio_a};
    ::SoC::gpio_pin{gpio_a,
                    ::SoC::gpio_pin::pin_9 | ::SoC::gpio_pin::pin_10,
                    ::SoC::gpio_mode::alternate,
                    ::SoC::gpio_alternate_function::af7,
                    ::SoC::gpio_speed::high};
    ::SoC::usart usart1{::SoC::usart::usart1, 115.2_K};
    ::SoC::log_device.set(usart1.write_wrapper, &usart1);
    ::usart.usart = &usart1;
    usart1.set_it_rxne(true);
    usart1.enable_irq(1);

    auto&& [_, ibuf, dev, _, eor]{usart};
    while(true)
    {
        ::SoC::wait_for_interpret();
        ::SoC::irq_guard _{[dev](bool enable) noexcept { dev->set_it_rxne(enable); }};

        if(!::std::exchange(eor, false)) { continue; }

        if(auto cur{*(ibuf.current - 1)}; cur == '\r')
        {
            constexpr auto new_line{"\r\n"sv};
            usart.write(new_line.begin(), new_line.end());
            ::std::int64_t a, b, result;
            ::operate_t operate{};
            const char* iptr{ibuf.begin};
            char obuf[64];
            auto optr{::std::copy(ibuf.begin, ibuf.current - 1, obuf)};
            *optr++ = '=';

            if(auto [ptr, ec]{::std::from_chars(iptr, ibuf.current, a)}; ec == ::std::errc{}) { iptr = ptr; }
            else { goto Invalid_Input; }

            switch(*iptr++)
            {
                case '+': operate = ::operate_t::add; break;
                case '-': operate = ::operate_t::sub; break;
                case '*': operate = ::operate_t::mul; break;
                case '/': operate = ::operate_t::div; break;
                default: goto Invalid_Input;
            }

            if(auto [ptr, ec]{::std::from_chars(iptr, ibuf.current, b)}; ec == ::std::errc{}) { iptr = ptr; }
            else { goto Invalid_Input; }

            if(auto cur{*iptr}; cur != '\r') { goto Invalid_Input; }

            switch(operate)
            {
                case ::operate_t::add: result = a + b; break;
                case ::operate_t::sub: result = a - b; break;
                case ::operate_t::mul: result = a * b; break;
                case ::operate_t::div: result = a / b; break;
            }

            optr = ::std::to_chars(optr, obuf + sizeof(obuf), result).ptr;
            *optr++ = '\r';
            *optr++ = '\n';
            usart.write(obuf, optr);

            ibuf.clear();
            continue;

        Invalid_Input:
            constexpr auto error_message{"Invalid input expression.\r\n"sv};
            usart.write(error_message.begin(), error_message.end());
            ibuf.clear();
            continue;
        }
        else { usart.write(cur); }
    }
}
