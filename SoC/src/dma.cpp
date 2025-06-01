#include "../include/dma.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::std::size_t(::SoC::dma::get_periph)() const noexcept
    {
        switch(get_dma_enum())
        {
            case dma1: return LL_AHB1_GRP1_PERIPH_DMA1;
            case dma2: return LL_AHB1_GRP1_PERIPH_DMA2;
        }
    }

    ::SoC::dma::dma(dma_enum dma) noexcept : dma_ptr{::std::bit_cast<::DMA_TypeDef*>(dma)}
    {
        ::SoC::assert(!is_enabled(), "初始化前此dma不应处于使能状态"sv);
        enable();
    }

    ::SoC::dma::~dma() noexcept
    {
        if(dma_ptr) [[likely]] { disable(); }
    }

    ::SoC::dma::dma(dma&& other) noexcept : dma_ptr{::std::exchange(other.dma_ptr, nullptr)} {}

    void ::SoC::dma::enable() const noexcept { ::LL_AHB1_GRP1_EnableClock(get_periph()); }

    void ::SoC::dma::disable() const noexcept { ::LL_AHB1_GRP1_DisableClock(get_periph()); }

    bool ::SoC::dma::is_enabled() const noexcept { return ::LL_AHB1_GRP1_IsEnabledClock(get_periph()); }
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

    /**
     * @brief 断言dma数据流已经失能
     *
     */
    void ::SoC::dma_stream::assert_disabled() const noexcept { ::SoC::assert(!is_enabled(), "dma数据流使能期间不能进行配置"sv); }

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
        mem_data_size{mem_data_size}, mem_burst{mem_burst}, pf_data_size{pf_data_size}, pf_burst{pf_burst}
    {
        ::SoC::assert(direction != ::SoC::dma_direction::m2m, "此构造函数不支持内存到内存模式的dma配置"sv);
        auto disabled{!is_enabled()};
        ::SoC::assert(disabled, "初始化前此dma数据流不应处于使能状态"sv);
        [[assume(disabled)]];

        auto stream_v{::std::to_underlying(stream)};
        ::LL_DMA_ConfigTransfer(dma_ptr,
                                stream_v,
                                ::std::to_underlying(direction) | ::std::to_underlying(mode) |
                                    (pf_increase ? LL_DMA_PERIPH_INCREMENT : LL_DMA_PERIPH_NOINCREMENT) |
                                    (mem_increase ? LL_DMA_MEMORY_INCREMENT : LL_DMA_MEMORY_NOINCREMENT) |
                                    ::std::to_underlying(pf_data_size) | ::std::to_underlying(mem_data_size) |
                                    ::std::to_underlying(priority));
        ::LL_DMA_SetChannelSelection(dma_ptr, stream_v, ::std::to_underlying(channel));
        set_fifo(fifo_threshold);

        set_memory_data_size(mem_data_size);
        auto memory_access{check_memory_access()};
        [[assume(memory_access)]];
        set_memory_burst(mem_burst);

        set_periph_data_size(pf_data_size);
        auto periph_access{check_periph_access()};
        [[assume(periph_access)]];
        set_periph_burst(pf_burst);
        ::LL_DMA_SetPeriphAddress(dma_ptr, stream_v, periph);
    }

    ::SoC::dma_stream::~dma_stream() noexcept
    {
        if(dma_ptr) [[likely]] { disable(); }
    }

    ::SoC::dma_stream::dma_stream(dma_stream&& other) noexcept
    {
        ::std::memcpy(reinterpret_cast<void*>(this), &other, sizeof(other));
        other.dma_ptr = nullptr;
    }

    /// 内存侧访问错误信息
    constexpr auto memory_access_error_msg{"内存侧操作带宽超出fifo深度"sv};
    /// 内存侧访问错误信息
    constexpr auto periph_access_error_msg{"外设侧操作带宽超出fifo深度"sv};

    void ::SoC::dma_stream::set_memory_data_size(::SoC::dma_memory_data_size mem_data_size) noexcept
    {
        assert_disabled();
        this->mem_data_size = mem_data_size;
        ::SoC::assert(check_memory_access(), ::SoC::memory_access_error_msg);
        ::LL_DMA_SetMemorySize(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(mem_data_size));
    }

    void ::SoC::dma_stream::set_memory_burst(::SoC::dma_memory_burst mem_burst) noexcept
    {
        assert_disabled();
        this->mem_burst = mem_burst;
        ::SoC::assert(check_memory_access(), ::SoC::memory_access_error_msg);
        ::LL_DMA_SetMemoryBurstxfer(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(mem_burst));
    }

    void ::SoC::dma_stream::set_periph_data_size(::SoC::dma_periph_data_size pf_data_size) noexcept
    {
        assert_disabled();
        this->pf_data_size = pf_data_size;
        ::SoC::assert(check_periph_access(), ::SoC::periph_access_error_msg);
        ::LL_DMA_SetPeriphSize(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(pf_data_size));
    }

    void ::SoC::dma_stream::set_periph_burst(::SoC::dma_periph_burst pf_burst) noexcept
    {
        assert_disabled();
        this->pf_burst = pf_burst;
        ::SoC::assert(check_periph_access(), ::SoC::periph_access_error_msg);
        ::LL_DMA_SetPeriphBurstxfer(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(pf_burst));
    }

    void ::SoC::dma_stream::set_fifo(::SoC::dma_fifo_threshold fifo_threshold) noexcept
    {
        assert_disabled();
        this->fifo_threshold = fifo_threshold;
        if(fifo_threshold == ::SoC::dma_fifo_threshold::disable)
        {
            ::SoC::assert(mem_burst == ::SoC::dma_memory_burst::single && pf_burst == ::SoC::dma_periph_burst::single,
                          "禁用fifo队列时不能使用突发"sv);
            ::LL_DMA_DisableFifoMode(dma_ptr, ::std::to_underlying(stream));
        }
        else
        {
            ::LL_DMA_ConfigFifo(dma_ptr,
                                ::std::to_underlying(stream),
                                LL_DMA_FIFOMODE_ENABLE,
                                ::std::to_underlying(fifo_threshold));
        }
    }

    void ::SoC::dma_stream::set_priority(::SoC::dma_priority priority) const noexcept
    {
        assert_disabled();
        ::LL_DMA_SetStreamPriorityLevel(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(priority));
    }

    void ::SoC::dma_stream::set_mode(::SoC::dma_mode mode) const noexcept
    {
        assert_disabled();
        ::LL_DMA_SetMode(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(mode));
    }

    bool ::SoC::dma_stream::is_enabled() const noexcept
    {
        return ::LL_DMA_IsEnabledStream(dma_ptr, ::std::to_underlying(stream));
    }

    void ::SoC::dma_stream::disable() const noexcept { ::LL_DMA_DisableStream(dma_ptr, ::std::to_underlying(stream)); }

    void ::SoC::dma_stream::enable() const noexcept { ::LL_DMA_EnableStream(dma_ptr, ::std::to_underlying(stream)); }

    void ::SoC::dma_stream::wait_until_disabled() const noexcept
    {
        if(is_enabled()) [[unlikely]]
        {
            ::SoC::assert(mode != ::SoC::dma_mode::circle, "循环模式下dma数据流不会自动失能, 在开启新的传输前请先失能dma数据流");
            ::SoC::wait_until([this] { return !is_enabled(); });
        }
    }

    bool ::SoC::dma_stream::check_aligned(::std::uintptr_t num) const noexcept
    {
        auto data_align{::std::countr_zero(get_memory_data_size_num())};
        auto buffer_align{::std::countr_zero(num)};
        return buffer_align >= data_align;
    }

    void ::SoC::dma_stream::set_memory_address(const void* begin) const noexcept
    {
        auto num{::std::bit_cast<::std::uintptr_t>(begin)};
        ::SoC::assert(check_aligned(num), "缓冲区首地址不满足对齐要求"sv);
        ::LL_DMA_SetMemoryAddress(dma_ptr, ::std::to_underlying(stream), num);
    }

    void ::SoC::dma_stream::set_data_item(::std::size_t size, ::std::size_t item_size) const noexcept
    {
        ::SoC::assert(check_aligned(size), "缓冲区大小不满足对齐要求"sv);
        ::LL_DMA_SetDataLength(dma_ptr, ::std::to_underlying(stream), size >> ::std::countr_zero(item_size));
    }

    void ::SoC::dma_stream::write(const void* begin, const void* end) noexcept
    {
        wait_until_disabled();
        clear_tc_flag();
        ::SoC::assert(direction == ::SoC::dma_direction::m2p, "仅内存到外设模式支持写入操作"sv);
        set_memory_address(begin);
        auto size{static_cast<::std::size_t>(reinterpret_cast<const char*>(end) - reinterpret_cast<const char*>(begin))};
        set_data_item(size, get_periph_data_size_num());
        enable();
    }

    void ::SoC::dma_stream::read(void* begin, void* end) noexcept
    {
        wait_until_disabled();
        clear_tc_flag();
        ::SoC::assert(direction == ::SoC::dma_direction::p2m, "仅外设到内存模式支持读取操作"sv);
        set_memory_address(begin);
        auto size{static_cast<::std::size_t>(reinterpret_cast<char*>(end) - reinterpret_cast<char*>(begin))};
        set_data_item(size, get_memory_data_size_num());
        enable();
    }

    bool ::SoC::dma_stream::get_tc_flag() const noexcept
    {
        switch(get_stream())
        {
            case st0: return ::LL_DMA_IsActiveFlag_TC0(dma_ptr);
            case st1: return ::LL_DMA_IsActiveFlag_TC1(dma_ptr);
            case st2: return ::LL_DMA_IsActiveFlag_TC2(dma_ptr);
            case st3: return ::LL_DMA_IsActiveFlag_TC3(dma_ptr);
            case st4: return ::LL_DMA_IsActiveFlag_TC4(dma_ptr);
            case st5: return ::LL_DMA_IsActiveFlag_TC5(dma_ptr);
            case st6: return ::LL_DMA_IsActiveFlag_TC6(dma_ptr);
            case st7: return ::LL_DMA_IsActiveFlag_TC7(dma_ptr);
        }
    }

    void ::SoC::dma_stream::clear_tc_flag() const noexcept
    {
        switch(get_stream())
        {
            case st0: ::LL_DMA_ClearFlag_TC0(dma_ptr); break;
            case st1: ::LL_DMA_ClearFlag_TC1(dma_ptr); break;
            case st2: ::LL_DMA_ClearFlag_TC2(dma_ptr); break;
            case st3: ::LL_DMA_ClearFlag_TC3(dma_ptr); break;
            case st4: ::LL_DMA_ClearFlag_TC4(dma_ptr); break;
            case st5: ::LL_DMA_ClearFlag_TC5(dma_ptr); break;
            case st6: ::LL_DMA_ClearFlag_TC6(dma_ptr); break;
            case st7: ::LL_DMA_ClearFlag_TC7(dma_ptr); break;
        }
    }

    bool ::SoC::dma_stream::is_ready() const noexcept
    {
        if(mode == ::SoC::dma_mode::circle) { return get_tc_flag(); }
        else { return !is_enabled(); }
    }
}  // namespace SoC
