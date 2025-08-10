/**
 * @file oled.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 oled外设
 */

module SoC;
import :oled;

namespace SoC
{
    using font8x16_t = ::std::array<::std::array<::std::uint8_t, 2>, 8>;
    using font16x16_t = ::std::array<::std::array<::std::uint8_t, 2>, 16>;

    constexpr inline auto font8x16{::SoC::array_cast<::SoC::font8x16_t>({
#embed "../assets/font_8x16.data"
    })};
    constexpr inline auto font16x16{::SoC::array_cast<::SoC::font16x16_t>({
#embed "../assets/font_16x16.data"
    })};

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

    void ::SoC::oled::set_cursor(::std::uint8_t page, ::std::uint8_t column) noexcept
    {
        this->page = page;
        this->column = column;
    }

    void ::SoC::oled::flush() noexcept
    {
#pragma GCC unroll(0)
        for(auto&& [page, page_index]: ::std::views::zip(buffer, ::std::views::iota(0)))
        {
            write_command(0xb0 | page_index);
            write_command(0x00 | 0);
            write_command(0x10 | 0);

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
#pragma GCC unroll(0)
        for(auto i{0zu}; i != page; ++i) { ::std::memset(&buffer[i], 0, 128); }
        page = 0;
        column = 0;
    }

    void ::SoC::oled::init() noexcept
    {
        constexpr ::std::uint8_t commands[]{0xae, 0xd5, 0x80, 0xa8, 0x3f, 0xd3, 0x00, 0x40, 0x8d, 0x14, 0x20, 0x02, 0xa1,
                                            0xc8, 0xda, 0x12, 0x81, 0xcf, 0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0xaf};
#pragma GCC unroll(0)
        for(auto command: commands) { write_command(command); }

        // constexpr ::std::uint8_t commands[]{command_prefix, 0xae, 0xd5, 0x80, 0xa8, 0x3f, 0xd3, 0x00, 0x40,
        //                                     0x8d,           0x14, 0x20, 0x02, 0xa1, 0xc8, 0xda, 0x12, 0x81,
        //                                     0xcf,           0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0xaf};

        // i2c.start();
        // i2c.write_address(slave_address);
        // dma_stream.write(commands, commands + sizeof(commands));
        flush();
    }

    template <::SoC::oled_font_size font_size>
    void ::SoC::oled::do_write(const char* begin, const char* end) noexcept
    {
#pragma GCC unroll(0)
        for(::std::uint8_t ch: ::std::ranges::subrange(begin, end))
        {
            constexpr bool is_f8x16{font_size == ::SoC::oled_font_size::f8x16};
            constexpr bool is_f16x16{font_size == ::SoC::oled_font_size::f16x16};
            constexpr auto&& table{[] static noexcept -> auto&&
                                   {
                                       if constexpr(is_f8x16) { return ::SoC::font8x16; }
                                       else if constexpr(is_f16x16) { return ::SoC::font16x16; }
                                   }()};

            switch(ch)
            {
                case '\b':
                {
                    constexpr auto column_per_ch{table.front().size()};
                    auto temp_column{column + 128zu - column_per_ch};
                    if(temp_column < 128) [[unlikely]]
                    {
                        if(page == 0) [[unlikely]] { temp_column = 0; }
                        else
                        {
                            page -= 2;
                        }
                    }
                    else
                    {
                        temp_column -= 128;
                    }
                    column = temp_column;
                }
                    continue;
                case '\r': column = 0; continue;
                case '\n':
                    column = 0;
                    page += 2;
                    continue;
                default: [[likely]] break;
            }

            if constexpr(is_f16x16)
            {
                // 行尾仅剩8列，需要换行
                if(column + 16 > 128) [[unlikely]]
                {
                    column = 0;
                    ::std::memset(&buffer[page][128], 0, 8);
                    ::std::memset(&buffer[page + 1][128], 0, 8);
                    page += 2;
                }
            }

            auto font_index{::std::max(ch - ' ', 0)};
#pragma GCC unroll(2)
            for(auto&& [page0, page1]: table[font_index])
            {
                buffer[page][column] = page0;
                buffer[page + 1][column] = page1;
                ++column;
            }
            if(column == 128) [[unlikely]]
            {
                page += 2;
                column = 0;
            }
        }
        flush();
    }

    void ::SoC::oled::write_f8x16(const char* begin, const char* end) noexcept
    {
        do_write<::SoC::oled_font_size::f8x16>(begin, end);
    }

    void ::SoC::oled::write_f16x16(const char* begin, const char* end) noexcept
    {
        do_write<::SoC::oled_font_size::f16x16>(begin, end);
    }
}  // namespace SoC
