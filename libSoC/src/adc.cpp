/**
 * @file adc.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 adc外设
 */

module;
#include "pch.hpp"
module SoC:adc_impl;
import :adc;
import :nvic;

namespace SoC
{
    using namespace ::std::string_view_literals;

    /**
     * @brief 将adc枚举转换为apb2 grp1外设时钟使能位
     *
     * @param adc adc枚举
     * @return apb2 grp1外设时钟使能位
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        adc_enum2grp1_periph(::SoC::detail::adc adc) noexcept
    {
        auto shift{(::SoC::to_underlying(adc) - ::SoC::to_underlying(::SoC::detail::adc::adc1)) >> 8zu};
        return 1zu << (shift + 8);
    }

    static_assert(adc_enum2grp1_periph(::SoC::adc::adc1) == LL_APB2_GRP1_PERIPH_ADC1);
    static_assert(adc_enum2grp1_periph(::SoC::adc::adc2) == LL_APB2_GRP1_PERIPH_ADC2);
    static_assert(adc_enum2grp1_periph(::SoC::adc::adc3) == LL_APB2_GRP1_PERIPH_ADC3);

    ::SoC::adc::adc(adc_enum adc, bool scan_mode, ::SoC::adc_resolution resolution, ::SoC::adc_data_alignment alignment) noexcept
        : adc_ptr{::SoC::bit_cast<::ADC_TypeDef*>(adc)}
    {
        ::SoC::assert(!is_enabled(), "初始化前此adc不应处于使能状态"sv);
        ::LL_APB2_GRP1_EnableClock(::SoC::adc_enum2grp1_periph(adc));
        set_scan_mode(scan_mode);
        set_resolution(resolution);
        set_alignment(alignment);
    }

    void ::SoC::adc::release() noexcept
    {
        if(adc_ptr != nullptr)
        {
            disable();
            ::LL_APB2_GRP1_DisableClock(::SoC::adc_enum2grp1_periph(get_adc_enum()));
            adc_ptr = nullptr;
        }
    }

    ::SoC::adc::~adc() noexcept { release(); }

    ::SoC::adc::adc(adc&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return; }
        adc_ptr = ::std::exchange(other.adc_ptr, nullptr);
        resolution = other.resolution;
        alignment = other.alignment;
        scan_mode = other.scan_mode;
    }

    ::SoC::adc& ::SoC::adc::operator= (adc&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return *this; }
        release();
        adc_ptr = ::std::exchange(other.adc_ptr, nullptr);
        resolution = other.resolution;
        alignment = other.alignment;
        scan_mode = other.scan_mode;
        return *this;
    }

    void ::SoC::adc::enable() const noexcept { ::LL_ADC_Enable(adc_ptr); }

    void ::SoC::adc::disable() const noexcept { ::LL_ADC_Disable(adc_ptr); }

    bool ::SoC::adc::is_enabled() const noexcept { return static_cast<bool>(::LL_ADC_IsEnabled(adc_ptr)); }

    void ::SoC::adc::set_resolution(::SoC::adc_resolution resolution) noexcept
    {
        this->resolution = resolution;
        ::LL_ADC_SetResolution(adc_ptr, ::SoC::to_underlying(resolution));
    }

    void ::SoC::adc::set_alignment(::SoC::adc_data_alignment alignment) noexcept
    {
        this->alignment = alignment;
        ::LL_ADC_SetDataAlignment(adc_ptr, ::SoC::to_underlying(alignment));
    }

    void ::SoC::adc::set_scan_mode(bool scan_mode) noexcept
    {
        this->scan_mode = scan_mode;
        ::LL_ADC_SetSequencersScanMode(adc_ptr, scan_mode ? LL_ADC_SEQ_SCAN_ENABLE : LL_ADC_SEQ_SCAN_DISABLE);
    }
}  // namespace SoC

namespace SoC
{
    ::SoC::adc_regular_group::adc_regular_group(::SoC::adc& adc,
                                                ::SoC::adc_regular_trigger_source trigger_source,
                                                bool continuous_mode,
                                                ::SoC::adc_regular_dma_mode dma_mode,
                                                ::std::initializer_list<::SoC::adc_channel_initializer> channel_list,
                                                ::SoC::adc_regular_seq_discont seq_discont) noexcept :
        adc_ptr{adc.get_adc()}, ranks{channel_list.size()}
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(!adc.is_enabled(), "进行adc规则组配置时, adc不能处于使能状态"sv);
            ::SoC::assert(ranks <= 16, "规则组通道数不能超过16"sv);
            if(adc.get_scan_mode()) [[likely]] { ::SoC::assert(ranks >= 2, "adc在扫描模式下，规则组内至少需要2个通道"sv); }
            else
            {
                ::SoC::assert(ranks == 1, "adc在非扫描模式下，规则组内有且只有一个通道"sv);
            }
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        ::LL_ADC_REG_SetSequencerLength(adc_ptr, scan_ranks_table[ranks - 1]);
        set_trigger_source(trigger_source);
        set_continuous_mode(continuous_mode);
        set_dma_mode(dma_mode);
        set_seq_discont(seq_discont);

        for(auto i{0zu}; auto&& [channel, sampling_time]: channel_list)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            ::LL_ADC_REG_SetSequencerRanks(adc_ptr, rank_table[i++], ::SoC::to_underlying(channel));
            ::LL_ADC_SetChannelSamplingTime(adc_ptr, ::SoC::to_underlying(channel), ::SoC::to_underlying(sampling_time));
        }
    }

    void ::SoC::adc_regular_group::release() noexcept
    {
        if(adc_ptr != nullptr) [[likely]]
        {
            clear_flag_eocs();
            clear_flag_ovr();
            disable_dma();
            disable();
            adc_ptr = nullptr;
        }
    }

    ::SoC::adc_regular_group::~adc_regular_group() noexcept { release(); }

    ::SoC::adc_regular_group::adc_regular_group(adc_regular_group&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return; }
        adc_ptr = ::std::exchange(other.adc_ptr, nullptr);
        ranks = other.ranks;
        trigger_source = other.trigger_source;
        dma_mode = other.dma_mode;
    }

    ::SoC::adc_regular_group& ::SoC::adc_regular_group::operator= (adc_regular_group&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return *this; }
        release();
        adc_ptr = ::std::exchange(other.adc_ptr, nullptr);
        ranks = other.ranks;
        trigger_source = other.trigger_source;
        dma_mode = other.dma_mode;
        return *this;
    }

    void ::SoC::adc_regular_group::set_trigger_source(::SoC::adc_regular_trigger_source trigger_source) noexcept
    {
        this->trigger_source = trigger_source;
        ::LL_ADC_REG_SetTriggerSource(adc_ptr, ::SoC::to_underlying(trigger_source));
    }

    void ::SoC::adc_regular_group::set_continuous_mode(bool continuous_mode) const noexcept
    { ::LL_ADC_REG_SetContinuousMode(adc_ptr, continuous_mode ? LL_ADC_REG_CONV_CONTINUOUS : LL_ADC_REG_CONV_SINGLE); }

    void ::SoC::adc_regular_group::set_dma_mode(::SoC::adc_regular_dma_mode dma_mode) noexcept
    {
        this->dma_mode = dma_mode;
        reset_dma();
    }

    /**
     * @brief 计算不连续转换需要的通道数
     *
     * @param seq_discont 不连续转换模式
     * @return 不连续转换需要的通道数
     */
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::size_t
        seq_discont2need_ranks(::SoC::adc_regular_seq_discont seq_discont) noexcept
    {
        if(seq_discont == ::SoC::adc_regular_seq_discont::disable) [[likely]] { return 0; }
        else
        {
            return (::SoC::to_underlying(seq_discont) >> 13zu) + 1zu;
        }
    }

    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::disable) == 0);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank1) == 1);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank2) == 2);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank3) == 3);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank4) == 4);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank5) == 5);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank6) == 6);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank7) == 7);
    static_assert(::SoC::seq_discont2need_ranks(::SoC::adc_regular_seq_discont::rank8) == 8);

    void ::SoC::adc_regular_group::set_seq_discont(::SoC::adc_regular_seq_discont seq_discont) const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            auto need_ranks{::SoC::seq_discont2need_ranks(seq_discont)};
            ::SoC::assert(need_ranks <= ranks, "不连续转化的通道数不能超过已经配置的通道数"sv);
        }
        ::LL_ADC_REG_SetSequencerDiscont(adc_ptr, ::SoC::to_underlying(seq_discont));
    }

    auto ::SoC::adc_regular_group::enable_dma(::SoC::dma& dma,
                                              ::SoC::dma_mode mode,
                                              ::SoC::dma_fifo_threshold fifo_threshold,
                                              ::SoC::dma_memory_burst burst,
                                              ::SoC::dma_priority priority,
                                              ::SoC::dma_stream::dma_stream_enum selected_stream) const noexcept
        -> ::SoC::dma_stream
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(dma_mode != ::SoC::adc_regular_dma_mode::none, "该adc规则组已配置为不使用dma"sv);
            ::SoC::assert(dma.get_dma_enum() == ::SoC::dma::dma2, "该dma外设不能操作该adc"sv);
        }

        using enum ::SoC::dma_stream::dma_stream_enum;
        using enum ::SoC::dma_channel;
        using enum ::SoC::adc::adc_enum;
        ::SoC::dma_channel channel{};
        ::SoC::dma_stream::dma_stream_enum stream{};
        constexpr static auto check_select_stream_message{"该dma不能使用指定的dma数据流"sv};
        switch(get_adc_enum())
        {
            case adc1:
                channel = ch0;
                if(selected_stream == no_selected_stream) [[likely]] { stream = st0; }
                else
                {
                    if constexpr(::SoC::use_full_assert)
                    {
                        ::SoC::assert(selected_stream == st0 || selected_stream == st4, check_select_stream_message);
                    }
                    stream = selected_stream;
                }
                break;
            case adc2:
                channel = ch1;
                if(selected_stream == no_selected_stream) [[likely]] { stream = st2; }
                else
                {
                    if constexpr(::SoC::use_full_assert)
                    {
                        ::SoC::assert(selected_stream == st2 || selected_stream == st3, check_select_stream_message);
                    }
                    stream = selected_stream;
                }
                break;
            case adc3:
                channel = ch2;
                if(selected_stream == no_selected_stream) [[likely]] { stream = st0; }
                else
                {
                    if constexpr(::SoC::use_full_assert)
                    {
                        ::SoC::assert(selected_stream == st0 || selected_stream == st1, check_select_stream_message);
                    }
                    stream = selected_stream;
                }
                break;
            default: ::std::unreachable();
        }

        return ::SoC::dma_stream{dma,
                                 stream,
                                 channel,
                                 ::LL_ADC_DMA_GetRegAddr(adc_ptr, LL_ADC_DMA_REG_REGULAR_DATA),
                                 ::SoC::dma_direction::p2m,
                                 mode,
                                 false,
                                 true,
                                 ::SoC::dma_periph_data_size::half_word,
                                 ::SoC::dma_memory_data_size::half_word,
                                 priority,
                                 fifo_threshold,
                                 burst,
                                 ::SoC::dma_periph_burst::single};
    }

    void ::SoC::adc_regular_group::enable(::SoC::adc_trig_edge trig_edge) const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            constexpr static auto message{"当且仅当adc触发源为软件触发时触发边沿应该设置为软件"sv};
            if(trigger_source == ::SoC::adc_regular_trigger_source::software)
            {
                ::SoC::assert(trig_edge == ::SoC::adc_trig_edge::software, message);
            }
            else
            {
                ::SoC::assert(trig_edge != ::SoC::adc_trig_edge::software, message);
            }
        }
        if(trig_edge != ::SoC::adc_trig_edge::software)
        {
            ::LL_ADC_REG_StartConversionExtTrig(adc_ptr, ::SoC::to_underlying(trig_edge));
        }
        else
        {
            ::LL_ADC_REG_StartConversionSWStart(adc_ptr);
        }
    }

    void ::SoC::adc_regular_group::disable() const noexcept { ::LL_ADC_REG_StopConversionExtTrig(adc_ptr); }

    bool ::SoC::adc_regular_group::get_flag_eocs() const noexcept
    { return static_cast<bool>(::LL_ADC_IsActiveFlag_EOCS(adc_ptr)); }

    void ::SoC::adc_regular_group::clear_flag_eocs() const noexcept { ::LL_ADC_ClearFlag_EOCS(adc_ptr); }

    bool ::SoC::adc_regular_group::get_flag_ovr() const noexcept { return static_cast<bool>(::LL_ADC_IsActiveFlag_OVR(adc_ptr)); }

    void ::SoC::adc_regular_group::clear_flag_ovr() const noexcept { ::LL_ADC_ClearFlag_OVR(adc_ptr); }

    ::std::size_t(::SoC::adc_regular_group::get_result)() const noexcept { return ::LL_ADC_REG_ReadConversionData12(adc_ptr); }

    void ::SoC::adc_regular_group::disable_dma() const noexcept
    { ::LL_ADC_REG_SetDMATransfer(adc_ptr, ::SoC::to_underlying(::SoC::adc_regular_dma_mode::none)); }

    void ::SoC::adc_regular_group::set_dma() const noexcept
    { ::LL_ADC_REG_SetDMATransfer(adc_ptr, ::SoC::to_underlying(dma_mode)); }

    void ::SoC::adc_regular_group::reset_dma() const noexcept
    {
        disable_dma();
        set_dma();
    }
}  // namespace SoC

namespace SoC
{
    ::SoC::adc_internal_channel::adc_internal_channel(internal_channel_enum internal_channel) noexcept :
        internal_channel{internal_channel}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前adc内部通道不应处于使能状态"sv); }
        enable();
    }

    ::SoC::adc_internal_channel::~adc_internal_channel() noexcept { release(); }

    void ::SoC::adc_internal_channel::release() noexcept
    {
        if(internal_channel != none)
        {
            disable();
            internal_channel = none;
        }
    }

    ::SoC::adc_internal_channel::adc_internal_channel(adc_internal_channel&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return; }
        internal_channel = ::std::exchange(other.internal_channel, none);
    }

    ::SoC::adc_internal_channel& ::SoC::adc_internal_channel::operator= (adc_internal_channel&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return *this; }
        release();
        internal_channel = ::std::exchange(other.internal_channel, none);
        return *this;
    }

    void ::SoC::adc_internal_channel::enable() const noexcept
    {
        auto current_channels{::LL_ADC_GetCommonPathInternalCh(ADC)};
        ::LL_ADC_SetCommonPathInternalCh(ADC, current_channels | ::SoC::to_underlying(internal_channel));
    }

    void ::SoC::adc_internal_channel::disable() const noexcept
    {
        auto current_channels{::LL_ADC_GetCommonPathInternalCh(ADC)};
        ::LL_ADC_SetCommonPathInternalCh(ADC, current_channels & ~::SoC::to_underlying(internal_channel));
    }

    bool ::SoC::adc_internal_channel::is_enabled() const noexcept
    { return (::LL_ADC_GetCommonPathInternalCh(ADC) & ::SoC::to_underlying(internal_channel)) != 0; }
}  // namespace SoC

namespace SoC
{
    ::SoC::adc_calibrator::adc_calibrator(::SoC::adc& adc, ::SoC::dma& dma) noexcept :
        adc{adc}, internal_channel{::SoC::adc_internal_channel::vrefint | ::SoC::adc_internal_channel::temp_sensor},
        buffer{::SoC::ram_allocator.allocate<buffer_t>()},
        adc_regular_group{::SoC::ram_allocator.allocate<::SoC::adc_regular_group>()},
        dma_stream{::SoC::ram_allocator.allocate<::SoC::dma_stream>()}
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(!adc.is_enabled(), "校准前adc不应处于使能状态"sv);
            ::SoC::assert(adc.get_adc_enum() == ::SoC::adc::adc1, "只有adc1支持采样校准所需的内部通道"sv);
        }

        old_resolution = adc.get_resolution();
        if(old_resolution != resolution) { adc.set_resolution(resolution); }
        old_scan_mode = adc.get_scan_mode();
        if(old_scan_mode != scan_mode) { adc.set_scan_mode(scan_mode); }
        old_alignment = adc.get_alignment();
        if(old_alignment != alignment) { adc.set_alignment(alignment); }

        constexpr static auto sampling_time{::SoC::adc_sampling_time::cycles144};
        auto& regular_group{
            *new(adc_regular_group)::SoC::adc_regular_group{
                                                            adc, ::SoC::adc_regular_trigger_source::software,
                                                            true, ::SoC::adc_regular_dma_mode::limited,
                                                            {{::SoC::adc_channel::ch_vrefint, sampling_time}, {::SoC::adc_channel::ch_temp_sensor, sampling_time}},
                                                            },
        };
        auto& adc_dma_stream{
            *new(dma_stream)::SoC::dma_stream{
                regular_group.enable_dma(dma,
                                         ::SoC::dma_mode::normal,
                                         ::SoC::dma_fifo_threshold::full,
                                         ::SoC::dma_memory_burst::inc8)},  // NOLINT(readability-trailing-comma)
        };
        adc.enable();
        adc_dma_stream.read(buffer->begin(), buffer->end());
        regular_group.enable(::SoC::adc_trig_edge::software);
    }

    ::SoC::adc_calibrator::~adc_calibrator() noexcept
    {
        adc.disable();
        if(old_resolution != resolution) { adc.set_resolution(old_resolution); }
        if(old_scan_mode != scan_mode) { adc.set_scan_mode(old_scan_mode); }
        if(old_alignment != alignment) { adc.set_alignment(old_alignment); }
    }

    bool ::SoC::adc_calibrator::is_sample_ready() const noexcept { return dma_stream->get_flag_tc(); }

    ::std::pair<float, float>(::SoC::adc_calibrator::get_result)() const noexcept
    {
        ::SoC::wait_until([this] noexcept { return is_sample_ready(); });
        constexpr static float temp1{TEMPSENSOR_CAL1_TEMP};
        constexpr static float temp2{TEMPSENSOR_CAL2_TEMP};
        constexpr static float delta_temp{temp2 - temp1};
        auto temp_sensor1{static_cast<float>(*TEMPSENSOR_CAL1_ADDR)};
        auto temp_sensor2{static_cast<float>(*TEMPSENSOR_CAL2_ADDR)};
        auto k{(temp_sensor2 - temp_sensor1) / delta_temp};
        auto b{(temp_sensor1 * temp2 - temp_sensor2 * temp1) / delta_temp};

        /// 带隙基准电源标称值对应的adc值
        auto vrefint_typical{static_cast<float>(*VREFINT_CAL_ADDR)};
        /// 温度系数标称值30ppm/°C
        constexpr static auto vrefint_temp_coeff_typical{30e-6f};
        /// 带隙基准电源温度标称值30°C
        constexpr static auto vrefint_temp_typical{30.f};
        /// 测量内部基准电压和温度时使用的参考电压
        constexpr static auto vref{3.3f};

        /// adc采样Vrefint的平均值
        ::std::size_t raw_vrefint{};
        /// adc采样温度传感器的平均值
        ::std::size_t raw_temp{};
#pragma GCC unroll(2)
        for(auto&& [vrefint, temp]: *buffer)
        {
            raw_vrefint += vrefint;
            raw_temp += temp;
        }
        constexpr static auto size{buffer_t{}.size()};
        raw_vrefint /= size;
        raw_temp /= size;

        /// 实际温度
        auto temp{vrefint_temp_typical};
        /// 实际Vref引脚电压
        auto actual_vref{0.f};
        // 这是一个非线性方程，使用迭代法求解
#pragma GCC unroll(2)
        for(auto i{0zu}; i != 4; ++i)
        {
            /// 根据温度修正内部参考电压
            auto vrefint_cal{(1 + (temp - vrefint_temp_typical) * vrefint_temp_coeff_typical) * vrefint_typical};
            // 计算实际Vref引脚电压
            actual_vref = vrefint_cal * vref / static_cast<float>(raw_vrefint);
            // 将温度传感器的adc采样值转化为Vref=3.3下的值
            auto temp_cal{static_cast<float>(raw_temp) * vref / actual_vref};
            // 计算实际温度
            temp = (temp_cal - b) / k;
        }
        constexpr static auto max_adc_value{(1zu << 12zu) - 1};
        constexpr static float lsb{1.f / max_adc_value};
        return ::std::pair{actual_vref * lsb, temp};
    }
}  // namespace SoC

namespace SoC
{
    ::SoC::analog_watchdog::analog_watchdog(::SoC::adc& adc,
                                            awd_enum channel,
                                            ::std::uint16_t low_threshold,
                                            ::std::uint16_t high_threshold) noexcept :
        adc_ptr{adc.get_adc()}, awd_channel{channel}, low_threshold{low_threshold}, high_threshold{high_threshold}
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此模拟看门狗不应处于使能状态"sv); }
        set_low_threshold(low_threshold);
        set_high_threshold(high_threshold);
        enable();
    }

    ::SoC::analog_watchdog::~analog_watchdog() noexcept { release(); }

    void ::SoC::analog_watchdog::release() noexcept
    {
        if(adc_ptr != nullptr)
        {
            clear_flag_awd();
            set_it_awd(false);
            disable();
            disable_irq();
            adc_ptr = nullptr;
        }
    }

    ::SoC::analog_watchdog::analog_watchdog(analog_watchdog&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return; }
        adc_ptr = ::std::exchange(other.adc_ptr, nullptr);
        awd_channel = other.awd_channel;
        low_threshold = other.low_threshold;
        high_threshold = other.high_threshold;
        nvic_irq_enabled = other.nvic_irq_enabled;
    }

    ::SoC::analog_watchdog& ::SoC::analog_watchdog::operator= (analog_watchdog&& other) noexcept
    {
        if(this == &other) [[unlikely]] { return *this; }
        release();
        adc_ptr = ::std::exchange(other.adc_ptr, nullptr);
        awd_channel = other.awd_channel;
        low_threshold = other.low_threshold;
        high_threshold = other.high_threshold;
        nvic_irq_enabled = other.nvic_irq_enabled;
        return *this;
    }

    namespace
    {
        constexpr auto awd_disable{::SoC::to_underlying(::SoC::analog_watchdog::awd_disable)};
    }

    bool ::SoC::analog_watchdog::is_enabled() const noexcept
    { return ::LL_ADC_GetAnalogWDMonitChannels(adc_ptr) != ::SoC::awd_disable; }

    void ::SoC::analog_watchdog::enable() const noexcept
    { ::LL_ADC_SetAnalogWDMonitChannels(adc_ptr, ::SoC::to_underlying(awd_channel)); }

    void ::SoC::analog_watchdog::disable() const noexcept { ::LL_ADC_SetAnalogWDMonitChannels(adc_ptr, ::SoC::awd_disable); }

    void ::SoC::analog_watchdog::set_low_threshold(::std::uint16_t threshold) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(threshold <= threshold_max, "模拟看门狗低门限值超出范围"sv);
            ::SoC::assert(low_threshold < high_threshold, "低阈值应小于高阈值"sv);
        }
        low_threshold = threshold;
        ::LL_ADC_SetAnalogWDThresholds(adc_ptr, LL_ADC_AWD_THRESHOLD_LOW, low_threshold);
    }

    void ::SoC::analog_watchdog::set_high_threshold(::std::uint16_t threshold) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(threshold <= threshold_max, "模拟看门狗高门限值超出范围"sv);
            ::SoC::assert(low_threshold < high_threshold, "低阈值应小于高阈值"sv);
        }
        high_threshold = threshold;
        ::LL_ADC_SetAnalogWDThresholds(adc_ptr, LL_ADC_AWD_THRESHOLD_HIGH, high_threshold);
    }

    void ::SoC::analog_watchdog::enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept
    {
        if(!nvic_irq_enabled)
        {
            ::SoC::enable_irq(irqn);
            ::SoC::set_priority(irqn, preempt_priority, sub_priority);
            ++::SoC::detail::adc_irq_reference_counter;
            nvic_irq_enabled = true;
        }
    }

    void ::SoC::analog_watchdog::enable_irq(::std::size_t encoded_priority) noexcept
    {
        if(!nvic_irq_enabled)
        {
            ::SoC::enable_irq(irqn);
            ::SoC::set_priority(irqn, encoded_priority);
            ++::SoC::detail::adc_irq_reference_counter;
            nvic_irq_enabled = true;
        }
    }

    void ::SoC::analog_watchdog::disable_irq() noexcept
    {
        if(nvic_irq_enabled && --::SoC::detail::adc_irq_reference_counter == 0)
        {
            ::SoC::disable_irq(irqn);
            nvic_irq_enabled = false;
        }
    }

    void ::SoC::analog_watchdog::set_it_awd(bool enable) const noexcept
    {
        if(enable) { ::LL_ADC_EnableIT_AWD1(adc_ptr); }
        else
        {
            ::LL_ADC_DisableIT_AWD1(adc_ptr);
        }
    }

    bool ::SoC::analog_watchdog::get_it_awd() const noexcept { return static_cast<bool>(::LL_ADC_IsEnabledIT_AWD1(adc_ptr)); }

    bool ::SoC::analog_watchdog::is_it_awd() const noexcept { return get_flag_awd() && get_it_awd(); }

    bool ::SoC::analog_watchdog::get_flag_awd() const noexcept { return static_cast<bool>(::LL_ADC_IsActiveFlag_AWD1(adc_ptr)); }

    void ::SoC::analog_watchdog::clear_flag_awd() const noexcept { ::LL_ADC_ClearFlag_AWD1(adc_ptr); }
}  // namespace SoC
