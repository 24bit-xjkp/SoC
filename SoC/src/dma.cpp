#include "../include/dma.hpp"
#include "../include/nvic.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::std::size_t(::SoC::dma::get_periph)() const noexcept
    {
        switch(get_dma_enum())
        {
            case dma1: return LL_AHB1_GRP1_PERIPH_DMA1;
            case dma2: return LL_AHB1_GRP1_PERIPH_DMA2;
            default: ::std::unreachable();
        }
    }

    ::SoC::dma::dma(dma_enum dma) noexcept : dma_ptr{::std::bit_cast<::DMA_TypeDef*>(dma)}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此dma不应处于使能状态"sv); }
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
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(direction != ::SoC::dma_direction::m2m, "此构造函数不支持内存到内存模式的dma配置"sv);
            ::SoC::assert(!is_enabled(), "初始化前此dma数据流不应处于使能状态"sv);
        }

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
        if(dma_ptr) [[likely]]
        {
            clear_flag_tc();
            disable();
            disable_irq();
        }
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
        this->mem_data_size = mem_data_size;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_memory_access(), ::SoC::memory_access_error_msg); }
        ::LL_DMA_SetMemorySize(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(mem_data_size));
    }

    void ::SoC::dma_stream::set_memory_burst(::SoC::dma_memory_burst mem_burst) noexcept
    {
        this->mem_burst = mem_burst;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_memory_access(), ::SoC::memory_access_error_msg); }
        ::LL_DMA_SetMemoryBurstxfer(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(mem_burst));
    }

    void ::SoC::dma_stream::set_periph_data_size(::SoC::dma_periph_data_size pf_data_size) noexcept
    {
        this->pf_data_size = pf_data_size;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_periph_access(), ::SoC::periph_access_error_msg); }
        ::LL_DMA_SetPeriphSize(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(pf_data_size));
    }

    void ::SoC::dma_stream::set_periph_burst(::SoC::dma_periph_burst pf_burst) noexcept
    {
        this->pf_burst = pf_burst;
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_periph_access(), ::SoC::periph_access_error_msg); }
        ::LL_DMA_SetPeriphBurstxfer(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(pf_burst));
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
        ::LL_DMA_SetStreamPriorityLevel(dma_ptr, ::std::to_underlying(stream), ::std::to_underlying(priority));
    }

    void ::SoC::dma_stream::set_mode(::SoC::dma_mode mode) const noexcept
    {
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
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert(mode != ::SoC::dma_mode::circle,
                              "循环模式下dma数据流不会自动失能, 在开启新的传输前请先失能dma数据流"sv);
            }
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
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_aligned(num), "缓冲区首地址不满足对齐要求"sv); }
        ::LL_DMA_SetMemoryAddress(dma_ptr, ::std::to_underlying(stream), num);
    }

    void ::SoC::dma_stream::set_data_item(::std::size_t size, ::std::size_t item_size) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(check_aligned(size), "缓冲区大小不满足对齐要求"sv); }
        ::LL_DMA_SetDataLength(dma_ptr, ::std::to_underlying(stream), size >> ::std::countr_zero(item_size));
    }

    void ::SoC::dma_stream::write(const void* begin, const void* end) noexcept
    {
        wait_until_disabled();
        clear_flag_tc();
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(direction == ::SoC::dma_direction::m2p, "仅内存到外设模式支持写入操作"sv);
        }
        set_memory_address(begin);
        auto size{static_cast<::std::size_t>(reinterpret_cast<const char*>(end) - reinterpret_cast<const char*>(begin))};
        set_data_item(size, get_periph_data_size_num());
        enable();
    }

    void ::SoC::dma_stream::read(void* begin, void* end) noexcept
    {
        wait_until_disabled();
        clear_flag_tc();
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(direction == ::SoC::dma_direction::p2m, "仅外设到内存模式支持读取操作"sv);
        }
        set_memory_address(begin);
        auto size{static_cast<::std::size_t>(reinterpret_cast<char*>(end) - reinterpret_cast<char*>(begin))};
        set_data_item(size, get_memory_data_size_num());
        enable();
    }

    auto ::SoC::dma_stream::get_tc_mask() const noexcept
    {
        constexpr ::std::size_t dma_tc_mask_table[]{DMA_LISR_TCIF0, DMA_LISR_TCIF1, DMA_LISR_TCIF2, DMA_LISR_TCIF3};
        auto mask{dma_tc_mask_table[::std::to_underlying(stream) & ::SoC::mask_all_one<2>]};
        return mask;
    }

    bool ::SoC::dma_stream::get_flag_tc() const noexcept
    {
        auto&& ref{get_stream() > st3 ? dma_ptr->HISR : dma_ptr->LISR};
        auto mask{get_tc_mask()};
        return (ref & mask) == mask;
    }

    void ::SoC::dma_stream::clear_flag_tc() const noexcept
    {
        auto&& ref{get_stream() > st3 ? dma_ptr->HIFCR : dma_ptr->LIFCR};
        ref = get_tc_mask();
    }

    auto ::SoC::dma_stream::get_ht_mask() const noexcept
    {
        constexpr ::std::size_t dma_ht_mask_table[]{DMA_LISR_HTIF0, DMA_LISR_HTIF1, DMA_LISR_HTIF2, DMA_LISR_HTIF3};
        auto mask{dma_ht_mask_table[::std::to_underlying(stream) & ::SoC::mask_all_one<2>]};
        return mask;
    }

    bool ::SoC::dma_stream::get_flag_ht() const noexcept
    {
        auto&& ref{get_stream() > st3 ? dma_ptr->HISR : dma_ptr->LISR};
        auto mask{get_ht_mask()};
        return (ref & mask) == mask;
    }

    void ::SoC::dma_stream::clear_flag_ht() const noexcept
    {
        auto&& ref{get_stream() > st3 ? dma_ptr->HIFCR : dma_ptr->LIFCR};
        ref = get_ht_mask();
    }

    bool ::SoC::dma_stream::is_ready() const noexcept
    {
        if(mode == ::SoC::dma_mode::circle) { return get_flag_tc(); }
        else
        {
            return !is_enabled();
        }
    }

    ::IRQn_Type(::SoC::dma_stream::get_irqn)() noexcept
    {
        if(irqn != 0) [[likely]] { return irqn; }
        else
        {
            // 由于irqn计算较为复杂，因此采用惰性计算
            using enum ::SoC::dma::dma_enum;
            switch(::std::bit_cast<::SoC::dma::dma_enum>(dma_ptr))
            {
                case dma1:
                    irqn = stream == st7 ? ::DMA1_Stream7_IRQn
                                         : static_cast<::IRQn_Type>(::DMA1_Stream0_IRQn + ::std::to_underlying(stream));
                    break;
                case dma2:
                    switch(stream)
                    {
                        case st0:
                        case st1:
                        case st2:
                        case st3:
                        case st4: irqn = static_cast<::IRQn_Type>(::DMA2_Stream0_IRQn + ::std::to_underlying(stream)); break;
                        default:
                            irqn = static_cast<::IRQn_Type>(::DMA2_Stream5_IRQn - 5 + ::std::to_underlying(stream));
                            break;
                    }
                    break;
            }
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
        if(enable) { ::LL_DMA_EnableIT_TC(dma_ptr, ::std::to_underlying(stream)); }
        else
        {
            ::LL_DMA_DisableIT_TC(dma_ptr, ::std::to_underlying(stream));
        }
    }

    bool ::SoC::dma_stream::get_it_tc() const noexcept { return ::LL_DMA_IsEnabledIT_TC(dma_ptr, ::std::to_underlying(stream)); }

    bool ::SoC::dma_stream::is_it_tc() const noexcept { return get_flag_tc() && get_it_tc(); }

    void ::SoC::dma_stream::set_it_ht(bool enable) const noexcept
    {
        if(enable) { ::LL_DMA_EnableIT_HT(dma_ptr, ::std::to_underlying(stream)); }
        else
        {
            ::LL_DMA_DisableIT_HT(dma_ptr, ::std::to_underlying(stream));
        }
    }

    bool ::SoC::dma_stream::get_it_ht() const noexcept { return ::LL_DMA_IsEnabledIT_HT(dma_ptr, ::std::to_underlying(stream)); }

    bool ::SoC::dma_stream::is_it_ht() const noexcept { return get_flag_ht() && get_it_ht(); }
}  // namespace SoC
