#pragma once
#include "dma.hpp"

namespace SoC
{
    namespace detail
    {
        /**
         * @brief adc外设枚举
         *
         */
        enum class adc : ::std::size_t
        {
            adc1 = ADC1_BASE,
            adc2 = ADC2_BASE,
            adc3 = ADC3_BASE
        };

        /**
         * @brief adc内部通道
         *
         */
        enum class adc_internal_channel : ::std::size_t
        {
            /// 不使能内部通道
            none = LL_ADC_PATH_INTERNAL_NONE,
            /// 内部带隙基准电压源通道
            vrefint = LL_ADC_PATH_INTERNAL_VREFINT,
            /// 内部温度传感器通道
            temp_sensor = LL_ADC_PATH_INTERNAL_TEMPSENSOR,
            /// 内部电池电压通道
            vbat = LL_ADC_PATH_INTERNAL_VBAT
        };

        /**
         * @brief 组合adc内部通道枚举
         *
         * @param lhs adc内部通道枚举
         * @param rhs adc内部通道枚举
         * @return 组合后的adc内部通道枚举
         */
        constexpr inline adc_internal_channel operator| (adc_internal_channel lhs, adc_internal_channel rhs) noexcept
        {
            return static_cast<adc_internal_channel>(::std::to_underlying(lhs) | ::std::to_underlying(rhs));
        }
    }  // namespace detail

    /**
     * @brief adc分辨率枚举
     *
     */
    enum class adc_resolution : ::std::size_t
    {
        bit12 = LL_ADC_RESOLUTION_12B,
        bit10 = LL_ADC_RESOLUTION_10B,
        bit8 = LL_ADC_RESOLUTION_8B,
        bit6 = LL_ADC_RESOLUTION_6B
    };

    /**
     * @brief adc数据对齐方式枚举
     *
     */
    enum class adc_data_alignment : ::std::size_t
    {
        right = LL_ADC_DATA_ALIGN_RIGHT,
        left = LL_ADC_DATA_ALIGN_LEFT
    };

    /**
     * @brief adc外设
     *
     */
    struct adc
    {
        using adc_enum = ::SoC::detail::adc;

    private:
        ::ADC_TypeDef* adc_ptr;
        ::SoC::adc_resolution resolution;
        ::SoC::adc_data_alignment alignment;
        bool scan_mode;

    public:
        using enum adc_enum;

        /**
         * @brief 初始化adc外设，会开启时钟但不会使能adc
         *
         * @param adc adc外设枚举
         * @param scan_mode 是否为扫描模式
         * @param resolution adc分辨率
         * @param alignment adc数据对齐方式
         */
        adc(adc_enum adc,
            bool scan_mode,
            ::SoC::adc_resolution resolution = ::SoC::adc_resolution::bit12,
            ::SoC::adc_data_alignment alignment = ::SoC::adc_data_alignment::right) noexcept;

        /**
         * @brief 失能adc外设，然后关闭时钟
         *
         */
        ~adc() noexcept;

        inline adc(const adc&) noexcept = delete;
        inline adc& operator= (const adc&) noexcept = delete;
        adc(adc&&) noexcept;
        inline adc& operator= (adc&&) noexcept = delete;

        /**
         * @brief 获取adc外设指针
         *
         * @return adc外设指针
         */
        inline ::ADC_TypeDef* get_adc() const noexcept { return adc_ptr; }

        /**
         * @brief 获取adc外设枚举
         *
         * @return adc外设枚举
         */
        inline adc_enum get_adc_enum() const noexcept { return ::std::bit_cast<adc_enum>(adc_ptr); }

        /**
         * @brief 获取adc分辨率
         *
         * @return adc分辨率
         */
        ::SoC::adc_resolution get_resolution() const noexcept { return resolution; }

        /**
         * @brief 获取adc数据对齐方式
         *
         * @return adc数据对齐方式
         */
        ::SoC::adc_data_alignment get_alignment() const noexcept { return alignment; }

        /**
         * @brief 获取adc扫描模式
         *
         * @return adc扫描模式
         */
        bool get_scan_mode() const noexcept { return scan_mode; }

        /**
         * @brief 设置adc分辨率
         *
         * @param resolution adc分辨率
         */
        void set_resolution(::SoC::adc_resolution resolution) noexcept;

        /**
         * @brief 设置adc数据对齐方式
         *
         * @param alignment adc数据对齐方式
         */
        void set_alignment(::SoC::adc_data_alignment alignment) noexcept;

        /**
         * @brief 设置adc扫描模式
         *
         * @param scan_mode 是否为扫描模式
         */
        void set_scan_mode(bool scan_mode) noexcept;

        /**
         * @brief 使能adc
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能adc
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断adc外设是否使能
         *
         * @return adc外设是否使能
         */
        bool is_enabled() const noexcept;
    };

    /**
     * @brief adc通道枚举
     *
     */
    enum class adc_channel : ::std::size_t
    {
        ch1 = LL_ADC_CHANNEL_1,
        ch2 = LL_ADC_CHANNEL_2,
        ch3 = LL_ADC_CHANNEL_3,
        ch4 = LL_ADC_CHANNEL_4,
        ch5 = LL_ADC_CHANNEL_5,
        ch6 = LL_ADC_CHANNEL_6,
        ch7 = LL_ADC_CHANNEL_7,
        ch8 = LL_ADC_CHANNEL_8,
        ch9 = LL_ADC_CHANNEL_9,
        ch10 = LL_ADC_CHANNEL_10,
        ch11 = LL_ADC_CHANNEL_11,
        ch12 = LL_ADC_CHANNEL_12,
        ch13 = LL_ADC_CHANNEL_13,
        ch14 = LL_ADC_CHANNEL_14,
        ch15 = LL_ADC_CHANNEL_15,
        ch16 = LL_ADC_CHANNEL_16,
        ch17 = LL_ADC_CHANNEL_17,
        ch18 = LL_ADC_CHANNEL_18,
        /// 电池供电电压通道
        ch_vbat = LL_ADC_CHANNEL_VBAT,
        /// 内部参考电压通道
        ch_vrefint = LL_ADC_CHANNEL_VREFINT,
        /// 内部温度传感器通道
        ch_temp_sensor = LL_ADC_CHANNEL_TEMPSENSOR
    };

    /**
     * @brief adc采样时间
     *
     */
    enum class adc_sampling_time : ::std::size_t
    {
        cycles3 = LL_ADC_SAMPLINGTIME_3CYCLES,
        cycles15 = LL_ADC_SAMPLINGTIME_15CYCLES,
        cycles28 = LL_ADC_SAMPLINGTIME_28CYCLES,
        cycles56 = LL_ADC_SAMPLINGTIME_56CYCLES,
        cycles84 = LL_ADC_SAMPLINGTIME_84CYCLES,
        cycles112 = LL_ADC_SAMPLINGTIME_112CYCLES,
        cycles144 = LL_ADC_SAMPLINGTIME_144CYCLES,
        cycles480 = LL_ADC_SAMPLINGTIME_480CYCLES,
    };

    /**
     * @brief adc规则组触发源
     *
     */
    enum class adc_regular_trigger_source : ::std::size_t
    {
        software = LL_ADC_REG_TRIG_SOFTWARE,
        tim1_ch1 = LL_ADC_REG_TRIG_EXT_TIM1_CH1,
        tim1_ch2 = LL_ADC_REG_TRIG_EXT_TIM1_CH2,
        tim1_ch3 = LL_ADC_REG_TRIG_EXT_TIM1_CH3,
        tim2_ch2 = LL_ADC_REG_TRIG_EXT_TIM2_CH2,
        tim2_ch3 = LL_ADC_REG_TRIG_EXT_TIM2_CH3,
        tim2_ch4 = LL_ADC_REG_TRIG_EXT_TIM2_CH4,
        tim2_trgo = LL_ADC_REG_TRIG_EXT_TIM2_TRGO,
        tim3_ch1 = LL_ADC_REG_TRIG_EXT_TIM3_CH1,
        tim3_trgo = LL_ADC_REG_TRIG_EXT_TIM3_TRGO,
        tim4_ch4 = LL_ADC_REG_TRIG_EXT_TIM4_CH4,
        tim5_ch1 = LL_ADC_REG_TRIG_EXT_TIM5_CH1,
        tim5_ch2 = LL_ADC_REG_TRIG_EXT_TIM5_CH2,
        tim5_ch3 = LL_ADC_REG_TRIG_EXT_TIM5_CH3,
        tim8_ch1 = LL_ADC_REG_TRIG_EXT_TIM8_CH1,
        tim8_trgo = LL_ADC_REG_TRIG_EXT_TIM8_TRGO,
        exti_line11 = LL_ADC_REG_TRIG_EXT_EXTI_LINE11,
    };

    /**
     * @brief adc规则组不连续扫描
     *
     */
    enum class adc_regular_seq_discont : ::std::size_t
    {
        /// 禁用不连续扫描
        disable = LL_ADC_REG_SEQ_DISCONT_DISABLE,
        /// 每次扫描1通道
        rank1 = LL_ADC_REG_SEQ_DISCONT_1RANK,
        /// 每次扫描2通道
        rank2 = LL_ADC_REG_SEQ_DISCONT_2RANKS,
        /// 每次扫描3通道
        rank3 = LL_ADC_REG_SEQ_DISCONT_3RANKS,
        /// 每次扫描4通道
        rank4 = LL_ADC_REG_SEQ_DISCONT_4RANKS,
        /// 每次扫描5通道
        rank5 = LL_ADC_REG_SEQ_DISCONT_5RANKS,
        /// 每次扫描6通道
        rank6 = LL_ADC_REG_SEQ_DISCONT_6RANKS,
        /// 每次扫描7通道
        rank7 = LL_ADC_REG_SEQ_DISCONT_7RANKS,
        /// 每次扫描8通道
        rank8 = LL_ADC_REG_SEQ_DISCONT_8RANKS,
    };

    /**
     * @brief adc规则组dma模式
     *
     */
    enum class adc_regular_dma_mode : ::std::size_t
    {
        /// 不使用dma
        none = LL_ADC_REG_DMA_TRANSFER_NONE,
        /// 有限次dma，对应dma单次传输
        limited = LL_ADC_REG_DMA_TRANSFER_LIMITED,
        /// 无限次dma，对应dma循环传输
        unlimited = LL_ADC_REG_DMA_TRANSFER_UNLIMITED
    };

    /**
     * @brief adc通道初始化器
     *
     */
    struct adc_channel_initializer
    {
        /// 要初始化的adc通道
        ::SoC::adc_channel channel;
        /// 该通道的采样时间
        ::SoC::adc_sampling_time sampling_time;

        inline adc_channel_initializer(::SoC::adc_channel channel, ::SoC::adc_sampling_time sampling_time) noexcept :
            channel{channel}, sampling_time{sampling_time}
        {
        }
    };

    /**
     * @brief adc触发边沿
     *
     */
    enum class adc_trig_edge : ::std::size_t
    {
        software = 0,
        rising = LL_ADC_REG_TRIG_EXT_RISING,
        falling = LL_ADC_REG_TRIG_EXT_FALLING,
        rising_falling = LL_ADC_REG_TRIG_EXT_RISINGFALLING
    };

    /**
     * @brief adc规则组
     *
     */
    struct adc_regular_group
    {
    private:
        ::ADC_TypeDef* adc_ptr;
        ::std::size_t ranks;
        ::SoC::adc_regular_trigger_source trigger_source;
        ::SoC::adc_regular_dma_mode dma_mode;

        /// adc通道转换顺序表
        constexpr inline static ::std::size_t rank_table[]{
            LL_ADC_REG_RANK_1,
            LL_ADC_REG_RANK_2,
            LL_ADC_REG_RANK_3,
            LL_ADC_REG_RANK_4,
            LL_ADC_REG_RANK_5,
            LL_ADC_REG_RANK_6,
            LL_ADC_REG_RANK_7,
            LL_ADC_REG_RANK_8,
            LL_ADC_REG_RANK_9,
            LL_ADC_REG_RANK_10,
            LL_ADC_REG_RANK_11,
            LL_ADC_REG_RANK_12,
            LL_ADC_REG_RANK_13,
            LL_ADC_REG_RANK_14,
            LL_ADC_REG_RANK_15,
            LL_ADC_REG_RANK_16,
        };

        /// adc通道数对应配置表
        constexpr inline static ::std::size_t scan_ranks_table[]{
            LL_ADC_REG_SEQ_SCAN_DISABLE,
            LL_ADC_REG_SEQ_SCAN_ENABLE_2RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_3RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_4RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_5RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_6RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_7RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_8RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_9RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_10RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_11RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_12RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_13RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_14RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_15RANKS,
            LL_ADC_REG_SEQ_SCAN_ENABLE_16RANKS,
        };

        /// 没有选定的dma数据流，即使用模式配置
        constexpr inline static auto no_selected_stream{static_cast<::SoC::dma_stream::dma_stream_enum>(-1zu)};

    public:
        /**
         * @brief 获取adc外设指针
         *
         * @return adc外设指针
         */
        inline ::ADC_TypeDef* get_adc() const noexcept { return adc_ptr; }

        /**
         * @brief 获取adc外设枚举
         *
         * @return adc外设枚举
         */
        inline ::SoC::adc::adc_enum get_adc_enum() const noexcept { return ::std::bit_cast<::SoC::adc::adc_enum>(adc_ptr); }

        /**
         * @brief 获取adc规则组中通道数量
         *
         * @return adc规则组中通道数量
         */
        inline ::std::size_t get_rank_num() const noexcept { return ranks; }

        /**
         * @brief 获取adc规则组的触发源
         *
         * @return adc规则组的触发源
         */
        inline ::SoC::adc_regular_trigger_source get_trigger_source() const noexcept { return trigger_source; }

        /**
         * @brief 获取adc规则组的dma模式
         *
         * @return adc规则组的dma模式
         */
        inline ::SoC::adc_regular_dma_mode get_dma_mode() const noexcept { return dma_mode; }

        /**
         * @brief 创建adc规则组，不会开始转换
         *
         * @param adc adc外设
         * @param trigger_source adc规则组触发源
         * @param continuous_mode 是否连续转化
         * @param dma_mode adc的dma模式
         * @param channel_list adc规则组使用的通道列表
         * @param seq_discont adc规则组不连续扫描
         */
        adc_regular_group(::SoC::adc& adc,
                          ::SoC::adc_regular_trigger_source trigger_source,
                          bool continuous_mode,
                          ::SoC::adc_regular_dma_mode dma_mode,
                          ::std::initializer_list<::SoC::adc_channel_initializer> channel_list,
                          ::SoC::adc_regular_seq_discont seq_discont = ::SoC::adc_regular_seq_discont::disable) noexcept;

        /**
         * @brief 失能adc规则组，但不会停止已经开始的转换
         *
         */
        ~adc_regular_group() noexcept;

        inline adc_regular_group(const adc_regular_group&) noexcept = delete;
        inline adc_regular_group& operator= (const adc_regular_group&) noexcept = delete;
        inline adc_regular_group(adc_regular_group&& other) noexcept;
        inline adc_regular_group& operator= (adc_regular_group&&) noexcept = delete;

        /**
         * @brief 设置adc规则组触发源
         *
         * @param trigger_source adc规则组触发源
         */
        void set_trigger_source(::SoC::adc_regular_trigger_source trigger_source) noexcept;

        /**
         * @brief 设置adc规则组连续转化模式
         *
         * @param continuous_mode 是否连续转化
         */
        void set_continuous_mode(bool continuous_mode) const noexcept;

        /**
         * @brief 设置adc规则组dma模式
         *
         * @param dma_mode adc的dma模式
         */
        void set_dma_mode(::SoC::adc_regular_dma_mode dma_mode) noexcept;

        /**
         * @brief 设置adc规则组不连续扫描模式
         *
         * @param seq_discont adc规则组不连续扫描
         */
        void set_seq_discont(::SoC::adc_regular_seq_discont seq_discont) const noexcept;

        /**
         * @brief 使能adc规则组dma写入
         *
         * @param dma dma外设
         * @param fifo_threshold fifo队列阈值
         * @param burst 默认内存侧突发
         * @param priority dma传输优先级
         * @param selected_stream 要使用的dma数据流，默认使用序号最小的数据流
         * @return dma数据流对象
         */
        [[nodiscard("该函数返回具有raii的dma数据流对象，不应该弃用返回值")]] ::SoC::dma_stream
            enable_dma(::SoC::dma& dma,
                       ::SoC::dma_fifo_threshold fifo_threshold = ::SoC::dma_fifo_threshold::disable,
                       ::SoC::dma_memory_burst burst = ::SoC::dma_memory_burst::single,
                       ::SoC::dma_priority priority = ::SoC::dma_priority::low,
                       ::SoC::dma_stream::dma_stream_enum selected_stream = no_selected_stream) const noexcept;

        /**
         * @brief 使能adc规则组触发
         *
         * @note 对于软件触发，会立即开始转化；对于外部触发，会使能触发源
         * @param trig_edge adc触发边沿
         */
        void enable(::SoC::adc_trig_edge trig_edge) const noexcept;

        /**
         * @brief 失能adc规则组触发
         *
         * @note 对于软件触发，不会进行操作；对于外部触发，会失能触发源，但不会停止已经开始的转化
         */
        void disable() const noexcept;

        /**
         * @brief 获取转换完成标志
         *
         * @return 转换完成标志
         */
        bool get_eocs_flag() const noexcept;

        /**
         * @brief 设置转换完成标志
         *
         */
        void clear_eocs_flag() const noexcept;

        /**
         * @brief 获取adc结果
         *
         * @return adc结果
         */
        ::std::size_t get_result() const noexcept;
    };

    /**
     * @brief adc内部通道控制
     *
     */
    struct adc_internal_channel
    {
        using internal_channel_enum = ::SoC::detail::adc_internal_channel;
        using enum internal_channel_enum;

        /**
         * @brief 使能adc内部通道
         *
         * @param internal_channel 要使能的内部通道
         */
        inline adc_internal_channel(internal_channel_enum internal_channel) noexcept
        {
            ::LL_ADC_SetCommonPathInternalCh(ADC, ::std::to_underlying(internal_channel));
        }

        /**
         * @brief 失能所有adc内部通道
         *
         */
        ~adc_internal_channel() noexcept { ::LL_ADC_SetCommonPathInternalCh(ADC, ::std::to_underlying(none)); }

        inline adc_internal_channel(const adc_internal_channel&) noexcept = delete;
        inline adc_internal_channel& operator= (const adc_internal_channel&) noexcept = delete;
        inline adc_internal_channel(adc_internal_channel&&) noexcept = delete;
        inline adc_internal_channel& operator= (adc_internal_channel&&) noexcept = delete;
    };

    /**
     * @brief adc校准器
     *
     */
    struct adc_calibrator
    {
    private:
        ::SoC::adc& adc;
        ::std::array<::std::array<::std::uint16_t, 2>, 8> buffer;
        [[no_unique_address]] ::SoC::adc_internal_channel internal_channel;
        bool old_scan_mode;
        ::SoC::adc_resolution old_resolution;
        ::SoC::adc_data_alignment old_alignment;

        constexpr inline static auto resolution{::SoC::adc_resolution::bit12};
        constexpr inline static auto scan_mode{true};
        constexpr inline static auto alignment{::SoC::adc_data_alignment::right};

        union dma_stream_union
        {
            ::SoC::dma_stream obj;

            inline dma_stream_union() {}

            inline ~dma_stream_union()
            {
                obj.clear_tc_flag();
                ::std::destroy_at(&obj);
            }
        } dma_stream;

        union adc_regular_group_union
        {
            ::SoC::adc_regular_group obj;

            inline adc_regular_group_union() {}

            inline ~adc_regular_group_union()
            {
                obj.clear_eocs_flag();
                ::std::destroy_at(&obj);
            }
        } adc_regular_group;

    public:
        /**
         * @brief 创建adc校准器，不会阻塞控制流
         *
         * @note 会开启adc内部通道
         * @param adc adc外设对象
         * @param dma dma外设对象
         */
        adc_calibrator(::SoC::adc& adc, ::SoC::dma& dma) noexcept;

        /**
         * @brief 关闭adc校准器
         *
         * @note 会关闭adc内部通道、adc和dma数据流
         */
        ~adc_calibrator() noexcept;

        /**
         * @brief 判断采样是否完成
         *
         * @return 采样是否完成
         */
        bool is_sample_ready() const noexcept;

        /**
         * @brief 获取校准结果，会阻塞直到采样完成
         *
         * @return std::pair{1LSB对应的电压, 温度}
         */
        ::std::pair<float, float> get_result() const noexcept;
    };
}  // namespace SoC
