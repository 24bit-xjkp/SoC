/**
 * @file i2c.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 i2c外设
 */

module;
#include "pch.hpp"
module SoC:i2c_impl;
import :i2c;
import :init;

namespace SoC
{
    using namespace ::std::string_view_literals;
    using namespace ::SoC::literal;

    /**
     * @brief 将i2c枚举转换为apb1 grp1外设时钟使能位
     *
     * @param i2c i2c枚举
     * @return apb1 grp1外设时钟使能位
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline auto
        i2c_enum2grp1_periph(::SoC::i2c::i2c_enum i2c) noexcept
    {
        auto shift{(::SoC::to_underlying(i2c) - ::SoC::to_underlying(::SoC::i2c::i2c1)) >> 10zu};
        return 1zu << (shift + 21zu);
    }

    static_assert(i2c_enum2grp1_periph(::SoC::i2c::i2c1) == LL_APB1_GRP1_PERIPH_I2C1);
    static_assert(i2c_enum2grp1_periph(::SoC::i2c::i2c2) == LL_APB1_GRP1_PERIPH_I2C2);
    static_assert(i2c_enum2grp1_periph(::SoC::i2c::i2c3) == LL_APB1_GRP1_PERIPH_I2C3);

    ::SoC::i2c::i2c(i2c_enum i2c,
                    ::std::size_t clock_speed,
                    ::std::size_t address,
                    ::SoC::i2c_mode mode,
                    ::SoC::i2c_duty_cycle duty,
                    ::SoC::i2c_type_ack ack,
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    ::SoC::i2c_owm_address_size address_size) noexcept : i2c_ptr{reinterpret_cast<::I2C_TypeDef*>(i2c)}
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(!is_enabled(), "初始化前此i2c外设不应处于使能状态"sv);
            ::SoC::assert(clock_speed <= 400_K, "i2c外设时钟速度不能超过400KHz"sv);
        }
        periph = ::SoC::i2c_enum2grp1_periph(i2c);
        ::LL_APB1_GRP1_EnableClock(periph);
        ::LL_I2C_ConfigSpeed(i2c_ptr, ::SoC::rcc::apb1_freq, clock_speed, ::SoC::to_underlying(duty));
        ::LL_I2C_SetOwnAddress1(i2c_ptr, address, ::SoC::to_underlying(address_size));
        ::LL_I2C_SetMode(i2c_ptr, ::SoC::to_underlying(mode));
        enable();
        set_ack(ack);
    }

    ::SoC::i2c::~i2c() noexcept
    {
        if(i2c_ptr != nullptr)
        {
            disable();
            ::LL_APB1_GRP1_DisableClock(periph);
        }
    }

    void ::SoC::i2c::enable() const noexcept { ::LL_I2C_Enable(i2c_ptr); }

    void ::SoC::i2c::disable() const noexcept { ::LL_I2C_Disable(i2c_ptr); }

    bool ::SoC::i2c::is_enabled() const noexcept { return static_cast<bool>(::LL_I2C_IsEnabled(i2c_ptr)); }

    void ::SoC::i2c::set_ack(::SoC::i2c_type_ack ack) const noexcept
    {
        ::LL_I2C_AcknowledgeNextData(i2c_ptr, ::SoC::to_underlying(ack));
    }

    bool ::SoC::i2c::get_flag_busy() const noexcept { return static_cast<bool>(::LL_I2C_IsActiveFlag_BUSY(i2c_ptr)); }

    bool ::SoC::i2c::get_flag_sb() const noexcept { return static_cast<bool>(::LL_I2C_IsActiveFlag_SB(i2c_ptr)); }

    bool ::SoC::i2c::get_flag_addr() const noexcept { return static_cast<bool>(::LL_I2C_IsActiveFlag_ADDR(i2c_ptr)); }

    void ::SoC::i2c::clear_flag_addr() const noexcept { ::LL_I2C_ClearFlag_ADDR(i2c_ptr); }

    bool ::SoC::i2c::get_flag_btf() const noexcept { return static_cast<bool>(::LL_I2C_IsActiveFlag_BTF(i2c_ptr)); }

    bool ::SoC::i2c::get_flag_stop() const noexcept { return static_cast<bool>(::LL_I2C_IsActiveFlag_STOP(i2c_ptr)); }

    void ::SoC::i2c::clear_flag_stop() const noexcept { ::LL_I2C_ClearFlag_STOP(i2c_ptr); }

    bool ::SoC::i2c::get_flag_txe() const noexcept { return static_cast<bool>(::LL_I2C_IsActiveFlag_TXE(i2c_ptr)); }

    void ::SoC::i2c::start() const noexcept
    {
        ::LL_I2C_GenerateStartCondition(i2c_ptr);
        ::SoC::wait_until([this] noexcept { return this->get_flag_sb(); });
    }

    void ::SoC::i2c::stop() const noexcept
    {

        ::SoC::wait_until([this] noexcept { return this->get_flag_btf(); });
        ::LL_I2C_GenerateStopCondition(i2c_ptr);
    }

    void ::SoC::i2c::write(::std::uint8_t value) const noexcept { ::LL_I2C_TransmitData8(i2c_ptr, value); }

    void ::SoC::i2c::write_address(::std::size_t address) const noexcept
    {
        write(address << 1zu);
        ::SoC::wait_until([this] noexcept { return this->get_flag_addr(); });
        clear_flag_addr();
    }

    void ::SoC::i2c::wait_until_idle() const noexcept
    {
        ::SoC::wait_until([this] noexcept { return !this->get_flag_busy(); });
    }

    void ::SoC::i2c::wait_until_txe() const noexcept
    {
        ::SoC::wait_until([this] noexcept { return this->get_flag_txe(); });
    }

    void ::SoC::i2c::write(::std::size_t address, const void* begin, const void* end) const noexcept
    {
        wait_until_idle();
        auto _{get_condition_guard()};
        write_address(address);

#pragma GCC unroll(0)
        for(auto byte:
            ::std::ranges::subrange(static_cast<const ::std::uint8_t*>(begin), static_cast<const ::std::uint8_t*>(end)))
        {
            wait_until_txe();
            write(byte);
        }
    }

    void ::SoC::i2c::assert_dma(::SoC::dma& dma) noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(dma.get_dma_enum() == dma.dma1, "该dma外设不能操作该i2c外设"sv); }
    }

    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline auto
        i2c_enum2dma_stream_channel(::SoC::i2c::i2c_enum i2c) noexcept
    {
        auto index{(::SoC::to_underlying(i2c) - ::SoC::to_underlying(::SoC::i2c::i2c1)) >> 10zu};
        using enum ::SoC::dma_channel;
        using enum ::SoC::dma_stream::dma_stream_enum;
        constexpr ::std::array table{
            ::std::pair{st6, ch1},
            ::std::pair{st7, ch7},
            ::std::pair{st4, ch3}
        };
        return table[index];
    }

    static_assert(i2c_enum2dma_stream_channel(::SoC::i2c::i2c1) ==
                  ::std::pair{::SoC::dma_stream::dma_stream_enum::st6, ::SoC::dma_channel::ch1});
    static_assert(i2c_enum2dma_stream_channel(::SoC::i2c::i2c2) ==
                  ::std::pair{::SoC::dma_stream::dma_stream_enum::st7, ::SoC::dma_channel::ch7});
    static_assert(i2c_enum2dma_stream_channel(::SoC::i2c::i2c3) ==
                  ::std::pair{::SoC::dma_stream::dma_stream_enum::st4, ::SoC::dma_channel::ch3});

    ::SoC::dma_stream(::SoC::i2c::enable_dma_write)(::SoC::dma& dma,
                                                    ::SoC::dma_fifo_threshold fifo_threshold,
                                                    ::SoC::dma_memory_burst default_burst,
                                                    ::SoC::dma_memory_data_size default_data_size,
                                                    ::SoC::dma_priority priority,
                                                    ::SoC::dma_mode mode,
                                                    ::SoC::dma_stream::dma_stream_enum selected_stream) const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(!is_dma_write_enabled(), "在配置前该i2c外设的dma不应处于使能状态"sv);
        }
        auto i2c_enum{get_i2c_enum()};
        auto [stream, channel]{i2c_enum2dma_stream_channel(i2c_enum)};

        if(i2c_enum == i2c1 && selected_stream != no_selected_stream) [[unlikely]]
        {
            using enum ::SoC::dma_stream::dma_stream_enum;
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert(selected_stream == st6 || selected_stream == st7, "该i2c外设不能使用指定的dma数据流"sv);
            }
            stream = selected_stream;
        }

        if constexpr(::SoC::use_full_assert) { assert_dma(dma); }
        ::LL_I2C_EnableDMAReq_TX(i2c_ptr);
        return ::SoC::dma_stream{dma,
                                 stream,
                                 channel,
                                 ::LL_I2C_DMA_GetRegAddr(i2c_ptr),
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

    void ::SoC::i2c::disable_dma_write() const noexcept { ::LL_I2C_DisableDMAReq_TX(i2c_ptr); }

    bool ::SoC::i2c::is_dma_write_enabled() const noexcept { return static_cast<bool>(::LL_I2C_IsEnabledDMAReq_TX(i2c_ptr)); }
}  // namespace SoC
