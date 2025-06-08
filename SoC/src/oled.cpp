#include "../include/oled.hpp"
#include "../include/nvic.hpp"

namespace SoC
{
    using font16x16_t = ::std::array<::std::array<::std::uint8_t, 2>, 16>;

    namespace detail
    {
        constexpr inline ::std::int8_t font16x16[]{
#embed "../assets/font.data"
        };
    }  // namespace detail

    constexpr inline auto font16x16{::SoC::array_cast<font16x16_t>(::SoC::detail::font16x16)};

    ::SoC::oled::oled(::SoC::i2c& i2c, ::SoC::dma& dma) noexcept :
        i2c{i2c}, dma_stream{i2c.enable_dma_write(dma,
                                                  ::SoC::dma_fifo_threshold::full,
                                                  ::SoC::dma_memory_burst::inc16,
                                                  ::SoC::dma_memory_data_size::byte)}
    {
        dma_stream.enable_irq(3, 1);
        dma_stream.set_it_tc(true);
    }

    void ::SoC::oled::write_command(::std::uint8_t command) const noexcept
    {
        ::std::uint8_t buffer[]{this->command_prefix, command};
        i2c.write(slave_address, buffer, buffer + 2);
    }

    void ::SoC::oled::write_data(::std::uint8_t data) const noexcept
    {
        ::std::uint8_t buffer[]{this->data_prefix, data};
        i2c.write(slave_address, buffer, buffer + 2);
    }

    void ::SoC::oled::set_cursor(::std::uint8_t page, ::std::uint8_t column) const noexcept
    {
        write_command(0xb0 | page);
        write_command(0x00 | (column & ::SoC::mask_all_zero<4>));
        write_command(0x10 | (column >> 4));
    }

    void ::SoC::oled::flush() const noexcept
    {
#pragma GCC unroll(0)
        for(auto&& [page, page_index]: ::std::views::zip(buffer, ::std::views::iota(0)))
        {
            set_cursor(page_index, 0);
            auto _{i2c.get_condition_guard()};
            i2c.write_address(slave_address);
            i2c.write(data_prefix);
#pragma GCC unroll(0)
            for(auto&& [column, _]: ::std::views::zip(page, ::std::views::iota(0)))
            {
                i2c.wait_until_txe();
                i2c.write(column);
            }
        }
    }

    void ::SoC::oled::init() noexcept
    {
        constexpr ::std::uint8_t commands[]{0xae, 0xd5, 0x80, 0xa8, 0x3f, 0xd3, 0x00, 0x40, 0x8d, 0x14, 0x20, 0x02, 0xa1,
                                            0xc8, 0xda, 0x12, 0x81, 0xcf, 0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0xaf};
        for(auto command: commands) { write_command(command); }

        // constexpr ::std::uint8_t commands[]{command_prefix, 0xae, 0xd5, 0x80, 0xa8, 0x3f, 0xd3, 0x00, 0x40,
        //                                     0x8d,           0x14, 0x20, 0x02, 0xa1, 0xc8, 0xda, 0x12, 0x81,
        //                                     0xcf,           0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0xaf};

        // i2c.start();
        // i2c.write_address(slave_address);
        // dma_stream.write(commands, commands + sizeof(commands));
        flush();
    }

    void ::SoC::oled::write(const void* begin, const void* end) const noexcept
    {
#pragma GCC unroll(2)
        for(auto column{0zu}; auto ch: ::std::ranges::subrange(reinterpret_cast<const ::std::uint8_t*>(begin),
                                                               reinterpret_cast<const ::std::uint8_t*>(end)))
        {
            auto font_index{ch - ' '};
            for(auto&& [page0, page1]: ::SoC::font16x16[font_index])
            {
                buffer[0][column] = page0;
                buffer[1][column] = page1;
                ++column;
            }
        }
        flush();
    }
}  // namespace SoC
