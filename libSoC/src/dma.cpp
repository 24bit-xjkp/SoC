/**
 * @file dma.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 dma外设
 */

module;
#include "pch.hpp"
module SoC:dma_impl;
import :dma;
import :nvic;

namespace SoC
{
    using namespace ::std::string_view_literals;

    /**
     * @brief 将dma枚举转换为ahb1 grp1外设时钟使能位
     *
     * @param dma_enum dma枚举
     * @return ahb1 grp1外设时钟使能位
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        dma_enum2grp1_periph(::SoC::dma::dma_enum dma_enum) noexcept
    {
        auto value{::SoC::to_underlying(dma_enum)};
        constexpr static auto base{::SoC::to_underlying(::SoC::dma::dma_enum::dma1) - (1zu << 10zu)};
        return (value - base) << 11zu;
    }

    static_assert(dma_enum2grp1_periph(::SoC::dma::dma_enum::dma1) == LL_AHB1_GRP1_PERIPH_DMA1);
    static_assert(dma_enum2grp1_periph(::SoC::dma::dma_enum::dma2) == LL_AHB1_GRP1_PERIPH_DMA2);

    ::SoC::dma::dma(dma_enum dma) noexcept : dma_ptr{::SoC::bit_cast<::DMA_TypeDef*>(dma)}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此dma不应处于使能状态"sv); }
        enable();
    }

    ::SoC::dma::~dma() noexcept { release(); }

    void ::SoC::dma::release() noexcept
    {
        if(dma_ptr != nullptr)
        {
            disable();
            dma_ptr = nullptr;
        }
    }

    ::SoC::dma::dma(dma&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return; }
        dma_ptr = ::std::exchange(other.dma_ptr, nullptr);
    }

    ::SoC::dma& ::SoC::dma::operator= (dma&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return *this; }
        release();
        dma_ptr = ::std::exchange(other.dma_ptr, nullptr);
        return *this;
    }

    void ::SoC::dma::enable() const noexcept { ::LL_AHB1_GRP1_EnableClock(::SoC::dma_enum2grp1_periph(get_dma_enum())); }

    void ::SoC::dma::disable() const noexcept { ::LL_AHB1_GRP1_DisableClock(::SoC::dma_enum2grp1_periph(get_dma_enum())); }

    bool ::SoC::dma::is_enabled() const noexcept
    { return static_cast<bool>(::LL_AHB1_GRP1_IsEnabledClock(::SoC::dma_enum2grp1_periph(get_dma_enum()))); }
}  // namespace SoC

namespace SoC
{
    bool ::SoC::dma_stream::check_memory_access() const noexcept
    {
        auto fifo_size{get_fifo_size()};
        return fifo_size == 0 || fifo_size >= get_memory_data_size_num() * get_memory_burst_num();
    }

    bool ::SoC::dma_stream::check_periph_access() const noexcept
    {
        auto fifo_size{get_fifo_size()};
        return fifo_size == 0 || fifo_size >= get_periph_data_size_num() * get_periph_burst_num();
    }

    ::SoC::dma_stream::dma_stream(::SoC::dma& dma,
                                  dma_stream_enum stream,
                                  ::SoC::dma_channel channel,
                                  ::std::uintptr_t periph,
                                  ::SoC::dma_direction direction,
                                  ::SoC::dma_mode mode,
                                  bool pf_increase,
                                  bool mem_increase,
                                  ::SoC::dma_periph_data_size pf_data_size,
                                  ::SoC::dma_memory_data_size mem_data_size,
                                  ::SoC::dma_priority priority,
                                  ::SoC::dma_fifo_threshold fifo_threshold,
                                  ::SoC::dma_memory_burst mem_burst,
                                  ::SoC::dma_periph_burst pf_burst) noexcept :
        dma_ptr{dma.get_dma()}, stream{stream}, direction{direction}, mode{mode}, fifo_threshold{fifo_threshold},
        mem_burst{mem_burst}, mem_data_size{mem_data_size}, pf_data_size{pf_data_size}, pf_burst{pf_burst}
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(direction != ::SoC::dma_direction::m2m, "此构造函数不支持内存到内存模式的dma配置"sv);
            ::SoC::assert(!is_enabled(), "初始化前此dma数据流不应处于使能状态"sv);
        }

        auto stream_v{::SoC::to_underlying(stream)};
        ::LL_DMA_ConfigTransfer(dma_ptr,
                                stream_v,
                                ::SoC::to_underlying(direction) | static_cast<::std::size_t>(::SoC::to_underlying(mode)) |
                                    (pf_increase ? LL_DMA_PERIPH_INCREMENT : LL_DMA_PERIPH_NOINCREMENT) |
                                    (mem_increase ? LL_DMA_MEMORY_INCREMENT : LL_DMA_MEMORY_NOINCREMENT) |
                                    ::SoC::to_underlying(pf_data_size) | ::SoC::to_underlying(mem_data_size) |
                                    ::SoC::to_underlying(priority));
        ::LL_DMA_SetChannelSelection(dma_ptr, stream_v, ::SoC::to_underlying(channel));
        set_fifo(fifo_threshold);

        set_memory_data_size(mem_data_size);
        set_memory_burst(mem_burst);

        set_periph_data_size(pf_data_size);
        set_periph_burst(pf_burst);
        ::LL_DMA_SetPeriphAddress(dma_ptr, stream_v, periph);
    }

    ::SoC::dma_stream::~dma_stream() noexcept { release(); }

    void ::SoC::dma_stream::release() noexcept
    {
        if(dma_ptr != nullptr)
        {
            clear_flag_ht();
            clear_flag_tc();
            clear_flag_te();
            clear_flag_fe();
            clear_flag_dme();
            disable();
            disable_irq();
            dma_ptr = nullptr;
        }
    }

    ::SoC::dma_stream::dma_stream(dma_stream&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return; }
        dma_ptr = ::std::exchange(other.dma_ptr, nullptr);
        stream = other.stream;
        direction = other.direction;
        mode = other.mode;
        fifo_threshold = other.fifo_threshold;
        mem_burst = other.mem_burst;
        mem_data_size = other.mem_data_size;
        pf_data_size = other.pf_data_size;
        pf_burst = other.pf_burst;
        irqn = other.irqn;
    }

    ::SoC::dma_stream& ::SoC::dma_stream::operator= (dma_stream&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return *this; }
        release();
        dma_ptr = ::std::exchange(other.dma_ptr, nullptr);
        stream = other.stream;
        direction = other.direction;
        mode = other.mode;
        fifo_threshold = other.fifo_threshold;
        mem_burst = other.mem_burst;
        mem_data_size = other.mem_data_size;
        pf_data_size = other.pf_data_size;
        pf_burst = other.pf_burst;
        irqn = other.irqn;
        return *this;
    }

    namespace
    {
        /// 内存侧访问错误信息
        constexpr auto memory_access_error_msg{"内存侧操作带宽超出fifo深度"sv};
        /// 外设侧访问错误信息
        constexpr auto periph_access_error_msg{"外设侧操作带宽超出fifo深度"sv};
    }  // namespace

    void ::SoC::dma_stream::set_memory_data_size(::SoC::dma_memory_data_size mem_data_size) noexcept
    {
        this->mem_data_size = mem_data_size;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_memory_access(), ::SoC::memory_access_error_msg); }
        ::LL_DMA_SetMemorySize(dma_ptr, ::SoC::to_underlying(stream), ::SoC::to_underlying(mem_data_size));
    }

    void ::SoC::dma_stream::set_memory_burst(::SoC::dma_memory_burst mem_burst) noexcept
    {
        this->mem_burst = mem_burst;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_memory_access(), ::SoC::memory_access_error_msg); }
        ::LL_DMA_SetMemoryBurstxfer(dma_ptr, ::SoC::to_underlying(stream), ::SoC::to_underlying(mem_burst));
    }

    void ::SoC::dma_stream::set_periph_data_size(::SoC::dma_periph_data_size pf_data_size) noexcept
    {
        this->pf_data_size = pf_data_size;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_periph_access(), ::SoC::periph_access_error_msg); }
        ::LL_DMA_SetPeriphSize(dma_ptr, ::SoC::to_underlying(stream), ::SoC::to_underlying(pf_data_size));
    }

    void ::SoC::dma_stream::set_periph_burst(::SoC::dma_periph_burst pf_burst) noexcept
    {
        this->pf_burst = pf_burst;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_periph_access(), ::SoC::periph_access_error_msg); }
        ::LL_DMA_SetPeriphBurstxfer(dma_ptr, ::SoC::to_underlying(stream), ::SoC::to_underlying(pf_burst));
    }

    void ::SoC::dma_stream::set_fifo(::SoC::dma_fifo_threshold fifo_threshold) noexcept
    {
        this->fifo_threshold = fifo_threshold;
        if(fifo_threshold == ::SoC::dma_fifo_threshold::disable)
        {
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert(mem_burst == ::SoC::dma_memory_burst::single && pf_burst == ::SoC::dma_periph_burst::single,
                              "禁用fifo队列时不能使用突发"sv);
            }
            ::LL_DMA_DisableFifoMode(dma_ptr, ::SoC::to_underlying(stream));
        }
        else
        {
            ::LL_DMA_ConfigFifo(dma_ptr,
                                ::SoC::to_underlying(stream),
                                LL_DMA_FIFOMODE_ENABLE,
                                ::SoC::to_underlying(fifo_threshold));
        }
    }

    void ::SoC::dma_stream::set_priority(::SoC::dma_priority priority) const noexcept
    { ::LL_DMA_SetStreamPriorityLevel(dma_ptr, ::SoC::to_underlying(stream), ::SoC::to_underlying(priority)); }

    void ::SoC::dma_stream::set_mode(::SoC::dma_mode mode) const noexcept
    { ::LL_DMA_SetMode(dma_ptr, ::SoC::to_underlying(stream), ::SoC::to_underlying(mode)); }

    bool ::SoC::dma_stream::is_enabled() const noexcept
    { return static_cast<bool>(::LL_DMA_IsEnabledStream(dma_ptr, ::SoC::to_underlying(stream))); }

    void ::SoC::dma_stream::disable() const noexcept { ::LL_DMA_DisableStream(dma_ptr, ::SoC::to_underlying(stream)); }

    void ::SoC::dma_stream::enable() const noexcept { ::LL_DMA_EnableStream(dma_ptr, ::SoC::to_underlying(stream)); }

    bool ::SoC::dma_stream::check_aligned(::std::uintptr_t num) const noexcept
    {
        auto mask{get_memory_data_size_num() - 1};
        return (num & mask) == 0;
    }

    void ::SoC::dma_stream::set_memory_address(const void* begin) const noexcept
    {
        auto num{::SoC::bit_cast<::std::uintptr_t>(begin)};
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_aligned(num), "缓冲区首地址不满足对齐要求"sv); }
        ::LL_DMA_SetMemoryAddress(dma_ptr, ::SoC::to_underlying(stream), num);
    }

    void ::SoC::dma_stream::set_data_item(::std::size_t size, ::std::size_t item_size) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_aligned(size), "缓冲区大小不满足对齐要求"sv); }
        ::LL_DMA_SetDataLength(dma_ptr,
                               ::SoC::to_underlying(stream),
                               size >> static_cast<::std::size_t>(::std::countr_zero(item_size)));
    }

    void ::SoC::dma_stream::write(const void* begin, const void* end) noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "在开始新的操作前需要保证dma流处于失能状态"sv); }
        clear_flag_tc();
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(direction == ::SoC::dma_direction::m2p, "仅内存到外设模式支持写入操作"sv);
        }
        set_memory_address(begin);
        auto size{static_cast<::std::size_t>(static_cast<const char*>(end) - static_cast<const char*>(begin))};
        set_data_item(size, get_periph_data_size_num());
        enable();
    }

    void ::SoC::dma_stream::read(void* begin, void* end) noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "在开始新的操作前需要保证dma流处于失能状态"sv); }
        clear_flag_tc();
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(direction == ::SoC::dma_direction::p2m, "仅外设到内存模式支持读取操作"sv);
        }
        set_memory_address(begin);
        auto size{static_cast<::std::size_t>(static_cast<char*>(end) - static_cast<char*>(begin))};
        set_data_item(size, get_memory_data_size_num());
        enable();
    }

    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        get_tc_mask(::SoC::dma_stream::dma_stream_enum stream) noexcept
    {
        constexpr static ::std::array dma_tc_mask_table{DMA_LISR_TCIF0, DMA_LISR_TCIF1, DMA_LISR_TCIF2, DMA_LISR_TCIF3};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        auto mask{dma_tc_mask_table[::SoC::to_underlying(stream) & ::SoC::mask_all_one<2>]};
        return mask;
    }

    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st0) == DMA_LISR_TCIF0);
    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st1) == DMA_LISR_TCIF1);
    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st2) == DMA_LISR_TCIF2);
    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st3) == DMA_LISR_TCIF3);
    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st4) == DMA_HISR_TCIF4);
    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st5) == DMA_HISR_TCIF5);
    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st6) == DMA_HISR_TCIF6);
    static_assert(::SoC::get_tc_mask(::SoC::dma_stream::st7) == DMA_HISR_TCIF7);

    bool ::SoC::dma_stream::get_flag_tc() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HISR : dma_ptr->LISR};
        auto mask{::SoC::get_tc_mask(stream)};
        return (ref & mask) == mask;
    }

    void ::SoC::dma_stream::clear_flag_tc() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HIFCR : dma_ptr->LIFCR};
        ref = ::SoC::get_tc_mask(stream);
    }

    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        get_ht_mask(::SoC::dma_stream::dma_stream_enum stream) noexcept
    {
        constexpr static ::std::array dma_ht_mask_table{DMA_LISR_HTIF0, DMA_LISR_HTIF1, DMA_LISR_HTIF2, DMA_LISR_HTIF3};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        auto mask{dma_ht_mask_table[::SoC::to_underlying(stream) & ::SoC::mask_all_one<2>]};
        return mask;
    }

    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st0) == DMA_LISR_HTIF0);
    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st1) == DMA_LISR_HTIF1);
    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st2) == DMA_LISR_HTIF2);
    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st3) == DMA_LISR_HTIF3);
    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st4) == DMA_HISR_HTIF4);
    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st5) == DMA_HISR_HTIF5);
    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st6) == DMA_HISR_HTIF6);
    static_assert(::SoC::get_ht_mask(::SoC::dma_stream::st7) == DMA_HISR_HTIF7);

    bool ::SoC::dma_stream::get_flag_ht() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HISR : dma_ptr->LISR};
        auto mask{::SoC::get_ht_mask(stream)};
        return (ref & mask) == mask;
    }

    void ::SoC::dma_stream::clear_flag_ht() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HIFCR : dma_ptr->LIFCR};
        ref = ::SoC::get_ht_mask(stream);
    }

    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        get_te_mask(::SoC::dma_stream::dma_stream_enum stream) noexcept
    {
        constexpr static ::std::array dma_te_mask_table{DMA_LISR_TEIF0, DMA_LISR_TEIF1, DMA_LISR_TEIF2, DMA_LISR_TEIF3};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        auto mask{dma_te_mask_table[::SoC::to_underlying(stream) & ::SoC::mask_all_one<2>]};
        return mask;
    }

    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st0) == DMA_LISR_TEIF0);
    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st1) == DMA_LISR_TEIF1);
    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st2) == DMA_LISR_TEIF2);
    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st3) == DMA_LISR_TEIF3);
    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st4) == DMA_HISR_TEIF4);
    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st5) == DMA_HISR_TEIF5);
    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st6) == DMA_HISR_TEIF6);
    static_assert(::SoC::get_te_mask(::SoC::dma_stream::st7) == DMA_HISR_TEIF7);

    bool ::SoC::dma_stream::get_flag_te() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HISR : dma_ptr->LISR};
        auto mask{::SoC::get_te_mask(stream)};
        return (ref & mask) == mask;
    }

    void ::SoC::dma_stream::clear_flag_te() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HIFCR : dma_ptr->LIFCR};
        ref = ::SoC::get_te_mask(stream);
    }

    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        get_fe_mask(::SoC::dma_stream::dma_stream_enum stream) noexcept
    {
        constexpr static ::std::array dma_fe_mask_table{DMA_LISR_FEIF0, DMA_LISR_FEIF1, DMA_LISR_FEIF2, DMA_LISR_FEIF3};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        auto mask{dma_fe_mask_table[::SoC::to_underlying(stream) & ::SoC::mask_all_one<2>]};
        return mask;
    }

    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st0) == DMA_LISR_FEIF0);
    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st1) == DMA_LISR_FEIF1);
    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st2) == DMA_LISR_FEIF2);
    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st3) == DMA_LISR_FEIF3);
    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st4) == DMA_HISR_FEIF4);
    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st5) == DMA_HISR_FEIF5);
    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st6) == DMA_HISR_FEIF6);
    static_assert(::SoC::get_fe_mask(::SoC::dma_stream::st7) == DMA_HISR_FEIF7);

    bool ::SoC::dma_stream::get_flag_fe() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HISR : dma_ptr->LISR};
        auto mask{::SoC::get_fe_mask(stream)};
        return (ref & mask) == mask;
    }

    void ::SoC::dma_stream::clear_flag_fe() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HIFCR : dma_ptr->LIFCR};
        ref = ::SoC::get_fe_mask(stream);
    }

    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        get_dme_mask(::SoC::dma_stream::dma_stream_enum stream) noexcept
    {
        constexpr static ::std::array dma_dme_mask_table{DMA_LISR_DMEIF0, DMA_LISR_DMEIF1, DMA_LISR_DMEIF2, DMA_LISR_DMEIF3};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        auto mask{dma_dme_mask_table[::SoC::to_underlying(stream) & ::SoC::mask_all_one<2>]};
        return mask;
    }

    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st0) == DMA_LISR_DMEIF0);
    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st1) == DMA_LISR_DMEIF1);
    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st2) == DMA_LISR_DMEIF2);
    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st3) == DMA_LISR_DMEIF3);
    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st4) == DMA_HISR_DMEIF4);
    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st5) == DMA_HISR_DMEIF5);
    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st6) == DMA_HISR_DMEIF6);
    static_assert(::SoC::get_dme_mask(::SoC::dma_stream::st7) == DMA_HISR_DMEIF7);

    bool ::SoC::dma_stream::get_flag_dme() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HISR : dma_ptr->LISR};
        auto mask{::SoC::get_dme_mask(stream)};
        return (ref & mask) == mask;
    }

    void ::SoC::dma_stream::clear_flag_dme() const noexcept
    {
        auto&& ref{stream > st3 ? dma_ptr->HIFCR : dma_ptr->LIFCR};
        ref = ::SoC::get_dme_mask(stream);
    }

    bool ::SoC::dma_stream::is_transfer_complete() const noexcept { return get_flag_tc() || !is_enabled(); }

    /**
     * @brief 将dma和数据流枚举转换为中断号
     *
     * @param dma dma外设枚举
     * @param stream dma数据流枚举
     * @return 中断号
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::IRQn_Type
        dma_stream2irqn(::SoC::dma::dma_enum dma, ::SoC::dma_stream::dma_stream_enum stream) noexcept
    {
        switch(dma)
        {
            case ::SoC::dma::dma1:
                return stream == ::SoC::dma_stream::st7
                           ? ::DMA1_Stream7_IRQn
                           : static_cast<::IRQn_Type>(::DMA1_Stream0_IRQn + ::SoC::to_underlying(stream));
            case ::SoC::dma::dma2:
            {
                constexpr ::std::size_t base_lt{::DMA2_Stream0_IRQn};
                constexpr ::std::size_t base_ge{::DMA2_Stream5_IRQn - 5};
                ::std::size_t base{stream < ::SoC::dma_stream::st5 ? base_lt : base_ge};
                return static_cast<::IRQn_Type>(base + ::SoC::to_underlying(stream));
            }
            default: ::std::unreachable();
        }
    }

    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st0) == ::DMA1_Stream0_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st1) == ::DMA1_Stream1_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st2) == ::DMA1_Stream2_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st3) == ::DMA1_Stream3_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st4) == ::DMA1_Stream4_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st5) == ::DMA1_Stream5_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st6) == ::DMA1_Stream6_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma1, ::SoC::dma_stream::st7) == ::DMA1_Stream7_IRQn);

    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st0) == ::DMA2_Stream0_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st1) == ::DMA2_Stream1_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st2) == ::DMA2_Stream2_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st3) == ::DMA2_Stream3_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st4) == ::DMA2_Stream4_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st5) == ::DMA2_Stream5_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st6) == ::DMA2_Stream6_IRQn);
    static_assert(::SoC::dma_stream2irqn(::SoC::dma::dma2, ::SoC::dma_stream::st7) == ::DMA2_Stream7_IRQn);

    ::IRQn_Type(::SoC::dma_stream::get_irqn)() noexcept
    {
        if(irqn != 0) [[likely]] { return irqn; }
        else
        {
            irqn = ::SoC::dma_stream2irqn(::SoC::bit_cast<::SoC::dma::dma_enum>(dma_ptr), stream);
            return irqn;
        }
    }

    void ::SoC::dma_stream::enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept
    {
        auto irqn{get_irqn()};
        ::SoC::set_priority(irqn, preempt_priority, sub_priority);
        ::SoC::enable_irq(irqn);
    }

    void ::SoC::dma_stream::enable_irq(::std::size_t encoded_priority) noexcept
    {
        auto irqn{get_irqn()};
        ::SoC::set_priority(irqn, encoded_priority);
        ::SoC::enable_irq(irqn);
    }

    void ::SoC::dma_stream::disable_irq() noexcept { ::SoC::disable_irq(get_irqn()); }

    void ::SoC::dma_stream::set_it_tc(bool enable) const noexcept
    {
        if(enable) { ::LL_DMA_EnableIT_TC(dma_ptr, ::SoC::to_underlying(stream)); }
        else
        {
            ::LL_DMA_DisableIT_TC(dma_ptr, ::SoC::to_underlying(stream));
        }
    }

    bool ::SoC::dma_stream::get_it_tc() const noexcept
    { return static_cast<bool>(::LL_DMA_IsEnabledIT_TC(dma_ptr, ::SoC::to_underlying(stream))); }

    bool ::SoC::dma_stream::is_it_tc() const noexcept { return get_flag_tc() && get_it_tc(); }

    void ::SoC::dma_stream::set_it_ht(bool enable) const noexcept
    {
        if(enable) { ::LL_DMA_EnableIT_HT(dma_ptr, ::SoC::to_underlying(stream)); }
        else
        {
            ::LL_DMA_DisableIT_HT(dma_ptr, ::SoC::to_underlying(stream));
        }
    }

    bool ::SoC::dma_stream::get_it_ht() const noexcept
    { return static_cast<bool>(::LL_DMA_IsEnabledIT_HT(dma_ptr, ::SoC::to_underlying(stream))); }

    bool ::SoC::dma_stream::is_it_ht() const noexcept { return get_flag_ht() && get_it_ht(); }

    void ::SoC::dma_stream::set_it_te(bool enable) const noexcept
    {
        if(enable) { ::LL_DMA_EnableIT_TE(dma_ptr, ::SoC::to_underlying(stream)); }
        else
        {
            ::LL_DMA_DisableIT_TE(dma_ptr, ::SoC::to_underlying(stream));
        }
    }

    bool ::SoC::dma_stream::get_it_te() const noexcept
    { return static_cast<bool>(::LL_DMA_IsEnabledIT_TE(dma_ptr, ::SoC::to_underlying(stream))); }

    bool ::SoC::dma_stream::is_it_te() const noexcept { return get_flag_te() && get_it_te(); }

    void ::SoC::dma_stream::set_it_fe(bool enable) const noexcept
    {
        if(enable) { ::LL_DMA_EnableIT_FE(dma_ptr, ::SoC::to_underlying(stream)); }
        else
        {
            ::LL_DMA_DisableIT_FE(dma_ptr, ::SoC::to_underlying(stream));
        }
    }

    bool ::SoC::dma_stream::get_it_fe() const noexcept
    { return static_cast<bool>(::LL_DMA_IsEnabledIT_FE(dma_ptr, ::SoC::to_underlying(stream))); }

    bool ::SoC::dma_stream::is_it_fe() const noexcept { return get_flag_fe() && get_it_fe(); }

    void ::SoC::dma_stream::set_it_dme(bool enable) const noexcept
    {
        if(enable) { ::LL_DMA_EnableIT_DME(dma_ptr, ::SoC::to_underlying(stream)); }
        else
        {
            ::LL_DMA_DisableIT_DME(dma_ptr, ::SoC::to_underlying(stream));
        }
    }

    bool ::SoC::dma_stream::get_it_dme() const noexcept
    { return static_cast<bool>(::LL_DMA_IsEnabledIT_DME(dma_ptr, ::SoC::to_underlying(stream))); }

    bool ::SoC::dma_stream::is_it_dme() const noexcept { return get_flag_dme() && get_it_dme(); }
}  // namespace SoC
