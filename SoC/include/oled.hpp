#pragma once
#include "i2c.hpp"

namespace SoC
{
    struct oled
    {
    private:
        constexpr inline static ::std::uint8_t slave_address{0x3c};
        constexpr inline static ::std::uint8_t command_prefix{0x00};
        constexpr inline static ::std::uint8_t data_prefix{0x40};
        inline static ::std::array<::std::array<::std::uint8_t, 128>, 8> buffer{};
        ::SoC::i2c& i2c;
        ::SoC::dma_stream dma_stream;

    public:
        oled(::SoC::i2c& i2c, ::SoC::dma& dma) noexcept;

        inline ::SoC::dma_stream& get_dma() noexcept { return dma_stream; }

        inline ::SoC::i2c& get_i2c() noexcept { return i2c; }

        void write_command(::std::uint8_t command) const noexcept;

        void write_data(::std::uint8_t data) const noexcept;

        void set_cursor(::std::uint8_t page, ::std::uint8_t column) const noexcept;

        void flush() const noexcept;

        void init() noexcept;

        void write(const void* begin, const void* end) const noexcept;
    };
}  // namespace SoC
