/**
 * @file adc.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 adc外设
 */

module;
#include "pch.hpp"
export module SoC:adc;
import :dma;

namespace SoC::detail
{
    /**
     * @brief adc外设枚举
     *
     */
    enum class adc : ::std::uintptr_t
    {
        /// adc1外设
        adc1 = ADC1_BASE,
        /// adc2外设
        adc2 = ADC2_BASE,
        /// adc3外设
        adc3 = ADC3_BASE,
    };

    /**
     * @brief adc内部通道
     *
     */
    enum class adc_internal_channel : ::std::size_t
    {
        /// 不使能内部通道
        none = LL_ADC_PATH_INTERNAL_NONE,
        /// 内部参考电压通道
        vrefint = LL_ADC_PATH_INTERNAL_VREFINT,
        /// 内部温度传感器通道
        temp_sensor = LL_ADC_PATH_INTERNAL_TEMPSENSOR,
        /// 内部电池电压通道
        vbat = LL_ADC_PATH_INTERNAL_VBAT,
    };

    /**
     * @brief 组合adc内部通道枚举
     *
     * @param lhs adc内部通道枚举
     * @param rhs adc内部通道枚举
     * @return 组合后的adc内部通道枚举
     */
    export constexpr inline ::SoC::detail::adc_internal_channel operator| (::SoC::detail::adc_internal_channel lhs,
                                                                           ::SoC::detail::adc_internal_channel rhs) noexcept
    { return static_cast<::SoC::detail::adc_internal_channel>(::SoC::to_underlying(lhs) | ::SoC::to_underlying(rhs)); }

    /**
     * @brief 模拟看门狗通道
     *
     */
    enum class analog_watchdog : ::std::size_t
    {
        /// 不使能模拟看门狗
        awd_disable = LL_ADC_AWD_DISABLE,
        /// 监视所有通道规则转换
        all_reg = LL_ADC_AWD_ALL_CHANNELS_REG,
        /// 监视所有通道注入转换
        all_inj = LL_ADC_AWD_ALL_CHANNELS_INJ,
        /// 监视所有通道规则和注入转换
        all_reg_inj = LL_ADC_AWD_ALL_CHANNELS_REG_INJ,
        /// 监视外部通道0规则转换
        ch0_reg = LL_ADC_AWD_CHANNEL_0_REG,
        /// 监视外部通道0注入转换
        ch0_inj = LL_ADC_AWD_CHANNEL_0_INJ,
        /// 监视外部通道0规则和注入转换
        ch0_reg_inj = LL_ADC_AWD_CHANNEL_0_REG_INJ,
        /// 监视外部通道1规则转换
        ch1_reg = LL_ADC_AWD_CHANNEL_1_REG,
        /// 监视外部通道1注入转换
        ch1_inj = LL_ADC_AWD_CHANNEL_1_INJ,
        /// 监视外部通道1规则和注入转换
        ch1_reg_inj = LL_ADC_AWD_CHANNEL_1_REG_INJ,
        /// 监视外部通道2规则转换
        ch2_reg = LL_ADC_AWD_CHANNEL_2_REG,
        /// 监视外部通道2注入转换
        ch2_inj = LL_ADC_AWD_CHANNEL_2_INJ,
        /// 监视外部通道2规则和注入转换
        ch2_reg_inj = LL_ADC_AWD_CHANNEL_2_REG_INJ,
        /// 监视外部通道3规则转换
        ch3_reg = LL_ADC_AWD_CHANNEL_3_REG,
        /// 监视外部通道3注入转换
        ch3_inj = LL_ADC_AWD_CHANNEL_3_INJ,
        /// 监视外部通道3规则和注入转换
        ch3_reg_inj = LL_ADC_AWD_CHANNEL_3_REG_INJ,
        /// 监视外部通道4规则转换
        ch4_reg = LL_ADC_AWD_CHANNEL_4_REG,
        /// 监视外部通道4注入转换
        ch4_inj = LL_ADC_AWD_CHANNEL_4_INJ,
        /// 监视外部通道4规则和注入转换
        ch4_reg_inj = LL_ADC_AWD_CHANNEL_4_REG_INJ,
        /// 监视外部通道5规则转换
        ch5_reg = LL_ADC_AWD_CHANNEL_5_REG,
        /// 监视外部通道5注入转换
        ch5_inj = LL_ADC_AWD_CHANNEL_5_INJ,
        /// 监视外部通道5规则和注入转换
        ch5_reg_inj = LL_ADC_AWD_CHANNEL_5_REG_INJ,
        /// 监视外部通道6规则转换
        ch6_reg = LL_ADC_AWD_CHANNEL_6_REG,
        /// 监视外部通道6注入转换
        ch6_inj = LL_ADC_AWD_CHANNEL_6_INJ,
        /// 监视外部通道6规则和注入转换
        ch6_reg_inj = LL_ADC_AWD_CHANNEL_6_REG_INJ,
        /// 监视外部通道7规则转换
        ch7_reg = LL_ADC_AWD_CHANNEL_7_REG,
        /// 监视外部通道7注入转换
        ch7_inj = LL_ADC_AWD_CHANNEL_7_INJ,
        /// 监视外部通道7规则和注入转换
        ch7_reg_inj = LL_ADC_AWD_CHANNEL_7_REG_INJ,
        /// 监视外部通道8规则转换
        ch8_reg = LL_ADC_AWD_CHANNEL_8_REG,
        /// 监视外部通道8注入转换
        ch8_inj = LL_ADC_AWD_CHANNEL_8_INJ,
        /// 监视外部通道8规则和注入转换
        ch8_reg_inj = LL_ADC_AWD_CHANNEL_8_REG_INJ,
        /// 监视外部通道9规则转换
        ch9_reg = LL_ADC_AWD_CHANNEL_9_REG,
        /// 监视外部通道9注入转换
        ch9_inj = LL_ADC_AWD_CHANNEL_9_INJ,
        /// 监视外部通道9规则和注入转换
        ch9_reg_inj = LL_ADC_AWD_CHANNEL_9_REG_INJ,
        /// 监视外部通道10规则转换
        ch10_reg = LL_ADC_AWD_CHANNEL_10_REG,
        /// 监视外部通道10注入转换
        ch10_inj = LL_ADC_AWD_CHANNEL_10_INJ,
        /// 监视外部通道10规则和注入转换
        ch10_reg_inj = LL_ADC_AWD_CHANNEL_10_REG_INJ,
        /// 监视外部通道11规则转换
        ch11_reg = LL_ADC_AWD_CHANNEL_11_REG,
        /// 监视外部通道11注入转换
        ch11_inj = LL_ADC_AWD_CHANNEL_11_INJ,
        /// 监视外部通道11规则和注入转换
        ch11_reg_inj = LL_ADC_AWD_CHANNEL_11_REG_INJ,
        /// 监视外部通道12规则转换
        ch12_reg = LL_ADC_AWD_CHANNEL_12_REG,
        /// 监视外部通道12注入转换
        ch12_inj = LL_ADC_AWD_CHANNEL_12_INJ,
        /// 监视外部通道12规则和注入转换
        ch12_reg_inj = LL_ADC_AWD_CHANNEL_12_REG_INJ,
        /// 监视外部通道13规则转换
        ch13_reg = LL_ADC_AWD_CHANNEL_13_REG,
        /// 监视外部通道13注入转换
        ch13_inj = LL_ADC_AWD_CHANNEL_13_INJ,
        /// 监视外部通道13规则和注入转换
        ch13_reg_inj = LL_ADC_AWD_CHANNEL_13_REG_INJ,
        /// 监视外部通道14规则转换
        ch14_reg = LL_ADC_AWD_CHANNEL_14_REG,
        /// 监视外部通道14注入转换
        ch14_inj = LL_ADC_AWD_CHANNEL_14_INJ,
        /// 监视外部通道14规则和注入转换
        ch14_reg_inj = LL_ADC_AWD_CHANNEL_14_REG_INJ,
        /// 监视外部通道15规则转换
        ch15_reg = LL_ADC_AWD_CHANNEL_15_REG,
        /// 监视外部通道15注入转换
        ch15_inj = LL_ADC_AWD_CHANNEL_15_INJ,
        /// 监视外部通道15规则和注入转换
        ch15_reg_inj = LL_ADC_AWD_CHANNEL_15_REG_INJ,
        /// 监视外部通道16规则转换
        ch16_reg = LL_ADC_AWD_CHANNEL_16_REG,
        /// 监视外部通道16注入转换
        ch16_inj = LL_ADC_AWD_CHANNEL_16_INJ,
        /// 监视外部通道16规则和注入转换
        ch16_reg_inj = LL_ADC_AWD_CHANNEL_16_REG_INJ,
        /// 监视外部通道17规则转换
        ch17_reg = LL_ADC_AWD_CHANNEL_17_REG,
        /// 监视外部通道17注入转换
        ch17_inj = LL_ADC_AWD_CHANNEL_17_INJ,
        /// 监视外部通道17规则和注入转换
        ch17_reg_inj = LL_ADC_AWD_CHANNEL_17_REG_INJ,
        /// 监视外部通道18规则转换
        ch18_reg = LL_ADC_AWD_CHANNEL_18_REG,
        /// 监视外部通道18注入转换
        ch18_inj = LL_ADC_AWD_CHANNEL_18_INJ,
        /// 监视外部通道18规则和注入转换
        ch18_reg_inj = LL_ADC_AWD_CHANNEL_18_REG_INJ,
        /// 监视内部参考电压通道规则转换
        vrefint_reg = LL_ADC_AWD_CH_VREFINT_REG,
        /// 监视内部参考电压通道注入转换
        vrefint_inj = LL_ADC_AWD_CH_VREFINT_INJ,
        /// 监视内部参考电压通道规则和注入转换
        vrefint_reg_inj = LL_ADC_AWD_CH_VREFINT_REG_INJ,
        /// 监视内部温度传感器通道规则转换
        temp_sensor_reg = LL_ADC_AWD_CH_TEMPSENSOR_REG,
        /// 监视内部温度传感器通道注入转换
        temp_sensor_inj = LL_ADC_AWD_CH_TEMPSENSOR_INJ,
        /// 监视内部温度传感器通道规则和注入转换
        temp_sensor_reg_inj = LL_ADC_AWD_CH_TEMPSENSOR_REG_INJ,
        /// 监视内部电池电压通道规则转换
        vbat_reg = LL_ADC_AWD_CH_VBAT_REG,
        /// 监视内部电池电压通道注入转换
        vbat_inj = LL_ADC_AWD_CH_VBAT_INJ,
        /// 监视内部电池电压通道规则和注入转换
        vbat_reg_inj = LL_ADC_AWD_CH_VBAT_REG_INJ,
    };

    /**
     * @brief 拼接模拟看门狗通道
     *
     * @param lhs 模拟看门狗通道
     * @param rhs 模拟看门狗通道
     * @return 拼接后的模拟看门狗通道
     */
    export constexpr inline ::SoC::detail::analog_watchdog operator| (::SoC::detail::analog_watchdog lhs,
                                                                      ::SoC::detail::analog_watchdog rhs) noexcept
    { return ::SoC::detail::analog_watchdog{::SoC::to_underlying(lhs) | ::SoC::to_underlying(rhs)}; }

    /// adc中断引用计数器，用于判断何时失能NVIC侧adc中断
    constinit ::std::uint8_t adc_irq_reference_counter{};
}  // namespace SoC::detail

export namespace SoC
{

    /**
     * @brief adc分辨率枚举
     *
     */
    enum class adc_resolution : ::std::size_t
    {
        /// 12位分辨率，最小转换时间12个ADC时钟周期
        bit12 = LL_ADC_RESOLUTION_12B,
        /// 10位分辨率，最小转换时间10个ADC时钟周期
        bit10 = LL_ADC_RESOLUTION_10B,
        /// 8位分辨率，最小转换时间8个ADC时钟周期
        bit8 = LL_ADC_RESOLUTION_8B,
        /// 6位分辨率，最小转换时间6个ADC时钟周期
        bit6 = LL_ADC_RESOLUTION_6B,
    };

    /**
     * @brief adc数据对齐方式枚举
     *
     */
    enum class adc_data_alignment : ::std::uint16_t
    {
        /// 右对齐，高位进行填充
        right = LL_ADC_DATA_ALIGN_RIGHT,
        /// 左对齐，低位进行填充
        left = LL_ADC_DATA_ALIGN_LEFT,
    };

    /**
     * @brief adc外设
     *
     */
    struct adc
    {
        // adc外设枚举
        using adc_enum = ::SoC::detail::adc;

    private:
        // adc外设指针
        ::ADC_TypeDef* adc_ptr{};
        // adc分辨率
        ::SoC::adc_resolution resolution{};
        // adc数据对齐方式
        ::SoC::adc_data_alignment alignment{};
        // 是否为扫描模式
        bool scan_mode{};

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
        explicit adc(adc_enum adc,
                     bool scan_mode,
                     ::SoC::adc_resolution resolution = ::SoC::adc_resolution::bit12,
                     ::SoC::adc_data_alignment alignment = ::SoC::adc_data_alignment::right) noexcept;

        /**
         * @brief 失能adc外设，然后关闭时钟
         *
         */
        ~adc() noexcept;

        inline adc(const adc&) noexcept = delete("对象独占外设资源，不能复制构造");
        inline adc& operator= (const adc&) noexcept = delete("对象独占外设资源，不能复制赋值");

        /**
         * @brief 转移adc外设资源的所有权
         *
         * @param other 其他adc外设对象
         */
        adc(adc&& other) noexcept;

        /**
         * @brief 转移adc外设资源的所有权
         *
         * @param other 其他adc外设对象
         * @return adc& 本对象
         */
        adc& operator= (adc&& other) noexcept;

        /**
         * @brief 失能adc外设，然后关闭时钟，释放adc外设资源
         *
         */
        void release() noexcept;

        /**
         * @brief 获取adc外设指针
         *
         * @return adc外设指针
         */
        [[nodiscard]] inline ::ADC_TypeDef* get_adc() const noexcept { return adc_ptr; }

        /**
         * @brief 获取adc外设枚举
         *
         * @return adc外设枚举
         */
        [[nodiscard]] inline adc_enum get_adc_enum() const noexcept { return ::SoC::bit_cast<adc_enum>(adc_ptr); }

        /**
         * @brief 获取adc分辨率
         *
         * @return adc分辨率
         */
        [[nodiscard]] ::SoC::adc_resolution get_resolution() const noexcept { return resolution; }

        /**
         * @brief 获取adc数据对齐方式
         *
         * @return adc数据对齐方式
         */
        [[nodiscard]] ::SoC::adc_data_alignment get_alignment() const noexcept { return alignment; }

        /**
         * @brief 获取adc扫描模式
         *
         * @return adc扫描模式
         */
        [[nodiscard]] bool get_scan_mode() const noexcept { return scan_mode; }

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
        [[nodiscard]] bool is_enabled() const noexcept;
    };

    /**
     * @brief adc通道枚举
     *
     */
    enum class adc_channel : ::std::size_t
    {
        /// 外部通道1
        ch1 = LL_ADC_CHANNEL_1,
        /// 外部通道2
        ch2 = LL_ADC_CHANNEL_2,
        /// 外部通道3
        ch3 = LL_ADC_CHANNEL_3,
        /// 外部通道4
        ch4 = LL_ADC_CHANNEL_4,
        /// 外部通道5
        ch5 = LL_ADC_CHANNEL_5,
        /// 外部通道6
        ch6 = LL_ADC_CHANNEL_6,
        /// 外部通道7
        ch7 = LL_ADC_CHANNEL_7,
        /// 外部通道8
        ch8 = LL_ADC_CHANNEL_8,
        /// 外部通道9
        ch9 = LL_ADC_CHANNEL_9,
        /// 外部通道10
        ch10 = LL_ADC_CHANNEL_10,
        /// 外部通道11
        ch11 = LL_ADC_CHANNEL_11,
        /// 外部通道12
        ch12 = LL_ADC_CHANNEL_12,
        /// 外部通道13
        ch13 = LL_ADC_CHANNEL_13,
        /// 外部通道14
        ch14 = LL_ADC_CHANNEL_14,
        /// 外部通道15
        ch15 = LL_ADC_CHANNEL_15,
        /// 外部通道16
        ch16 = LL_ADC_CHANNEL_16,
        /// 外部通道17
        ch17 = LL_ADC_CHANNEL_17,
        /// 外部通道18
        ch18 = LL_ADC_CHANNEL_18,
        /// 内部电池供电电压通道
        ch_vbat = LL_ADC_CHANNEL_VBAT,
        /// 内部参考电压通道
        ch_vrefint = LL_ADC_CHANNEL_VREFINT,
        /// 内部温度传感器通道
        ch_temp_sensor = LL_ADC_CHANNEL_TEMPSENSOR,
    };

    /**
     * @brief adc采样时间
     *
     */
    enum class adc_sampling_time : ::std::uint8_t
    {
        /// 采样3个ADC时钟周期
        cycles3 = LL_ADC_SAMPLINGTIME_3CYCLES,
        /// 采样15个ADC时钟周期
        cycles15 = LL_ADC_SAMPLINGTIME_15CYCLES,
        /// 采样28个ADC时钟周期
        cycles28 = LL_ADC_SAMPLINGTIME_28CYCLES,
        /// 采样56个ADC时钟周期
        cycles56 = LL_ADC_SAMPLINGTIME_56CYCLES,
        /// 采样84个ADC时钟周期
        cycles84 = LL_ADC_SAMPLINGTIME_84CYCLES,
        /// 采样112个ADC时钟周期
        cycles112 = LL_ADC_SAMPLINGTIME_112CYCLES,
        /// 采样144个ADC时钟周期
        cycles144 = LL_ADC_SAMPLINGTIME_144CYCLES,
        /// 采样480个ADC时钟周期
        cycles480 = LL_ADC_SAMPLINGTIME_480CYCLES,
    };

    /**
     * @brief adc规则组触发源
     *
     */
    enum class adc_regular_trigger_source : ::std::size_t
    {
        /// 软件触发
        software = LL_ADC_REG_TRIG_SOFTWARE,
        /// 定时器1通道1比较事件触发
        tim1_ch1 = LL_ADC_REG_TRIG_EXT_TIM1_CH1,
        /// 定时器1通道2比较事件触发
        tim1_ch2 = LL_ADC_REG_TRIG_EXT_TIM1_CH2,
        /// 定时器1通道3比较事件触发
        tim1_ch3 = LL_ADC_REG_TRIG_EXT_TIM1_CH3,
        /// 定时器2通道2比较事件触发
        tim2_ch2 = LL_ADC_REG_TRIG_EXT_TIM2_CH2,
        /// 定时器2通道3比较事件触发
        tim2_ch3 = LL_ADC_REG_TRIG_EXT_TIM2_CH3,
        /// 定时器2通道4比较事件触发
        tim2_ch4 = LL_ADC_REG_TRIG_EXT_TIM2_CH4,
        /// 定时器2触发事件触发
        tim2_trgo = LL_ADC_REG_TRIG_EXT_TIM2_TRGO,
        /// 定时器3通道1比较事件触发
        tim3_ch1 = LL_ADC_REG_TRIG_EXT_TIM3_CH1,
        /// 定时器3触发事件触发
        tim3_trgo = LL_ADC_REG_TRIG_EXT_TIM3_TRGO,
        /// 定时器4通道4比较事件触发
        tim4_ch4 = LL_ADC_REG_TRIG_EXT_TIM4_CH4,
        /// 定时器5通道1比较事件触发
        tim5_ch1 = LL_ADC_REG_TRIG_EXT_TIM5_CH1,
        /// 定时器5通道2比较事件触发
        tim5_ch2 = LL_ADC_REG_TRIG_EXT_TIM5_CH2,
        /// 定时器5通道3比较事件触发
        tim5_ch3 = LL_ADC_REG_TRIG_EXT_TIM5_CH3,
        /// 定时器8通道1比较事件触发
        tim8_ch1 = LL_ADC_REG_TRIG_EXT_TIM8_CH1,
        /// 定时器8触发事件触发
        tim8_trgo = LL_ADC_REG_TRIG_EXT_TIM8_TRGO,
        /// 外部中断线11触发事件触发
        exti_line11 = LL_ADC_REG_TRIG_EXT_EXTI_LINE11,
    };

    /**
     * @brief adc规则组不连续扫描
     *
     */
    enum class adc_regular_seq_discont : ::std::uint16_t
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
    enum class adc_regular_dma_mode : ::std::uint16_t
    {
        /// 不使用dma
        none = LL_ADC_REG_DMA_TRANSFER_NONE,
        /// 有限次dma，对应dma单次传输
        limited = LL_ADC_REG_DMA_TRANSFER_LIMITED,
        /// 无限次dma，对应dma循环传输
        unlimited = LL_ADC_REG_DMA_TRANSFER_UNLIMITED,
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
        // 软件触发时不涉及触发边沿
        software = 0,
        // 上升沿触发
        rising = LL_ADC_REG_TRIG_EXT_RISING,
        // 下降沿触发
        falling = LL_ADC_REG_TRIG_EXT_FALLING,
        // 上升沿和下降沿均触发
        rising_falling = LL_ADC_REG_TRIG_EXT_RISINGFALLING,
    };

    /**
     * @brief adc规则组
     *
     */
    struct adc_regular_group
    {
    private:
        // adc外设指针
        ::ADC_TypeDef* adc_ptr{};
        // adc通道数
        ::std::size_t ranks{};
        // 触发源
        ::SoC::adc_regular_trigger_source trigger_source{};
        // dma模式
        ::SoC::adc_regular_dma_mode dma_mode{};

        /// adc通道转换顺序表
        constexpr inline static auto rank_table{
            ::std::to_array<::std::uint16_t>({
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
            }),
        };

        /// adc通道数对应配置表
        constexpr inline static ::std::array scan_ranks_table{
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

        /// 没有选定的dma数据流，即使用默认配置
        constexpr inline static auto no_selected_stream{static_cast<::SoC::dma_stream::dma_stream_enum>(-1)};

    public:
        /**
         * @brief 获取adc外设指针
         *
         * @return adc外设指针
         */
        [[nodiscard]] inline ::ADC_TypeDef* get_adc() const noexcept { return adc_ptr; }

        /**
         * @brief 获取adc外设枚举
         *
         * @return adc外设枚举
         */
        [[nodiscard]] inline ::SoC::adc::adc_enum get_adc_enum() const noexcept
        { return ::SoC::bit_cast<::SoC::adc::adc_enum>(adc_ptr); }

        /**
         * @brief 获取adc规则组中通道数量
         *
         * @return adc规则组中通道数量
         */
        [[nodiscard]] inline ::std::size_t get_rank_num() const noexcept { return ranks; }

        /**
         * @brief 获取adc规则组的触发源
         *
         * @return adc规则组的触发源
         */
        [[nodiscard]] inline ::SoC::adc_regular_trigger_source get_trigger_source() const noexcept { return trigger_source; }

        /**
         * @brief 获取adc规则组的dma模式
         *
         * @return adc规则组的dma模式
         */
        [[nodiscard]] inline ::SoC::adc_regular_dma_mode get_dma_mode() const noexcept { return dma_mode; }

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
        explicit adc_regular_group(::SoC::adc& adc,
                                   ::SoC::adc_regular_trigger_source trigger_source,
                                   bool continuous_mode,
                                   ::SoC::adc_regular_dma_mode dma_mode,
                                   ::std::initializer_list<::SoC::adc_channel_initializer> channel_list,
                                   ::SoC::adc_regular_seq_discont seq_discont = ::SoC::adc_regular_seq_discont::disable) noexcept;

        /**
         * @brief 失能adc规则组，清除标志并失能dma，但不会停止已经开始的转换
         *
         */
        ~adc_regular_group() noexcept;

        inline adc_regular_group(const adc_regular_group&) noexcept = delete("对象独占外设资源，不能复制构造");
        inline adc_regular_group& operator= (const adc_regular_group&) noexcept = delete("对象独占外设资源，不能复制赋值");

        /**
         * @brief 转移adc外设规则组资源的所有权
         *
         * @param other 其他adc规则组对象
         */
        adc_regular_group(adc_regular_group&& other) noexcept;

        /**
         * @brief 转移adc外设规则组资源的所有权
         *
         * @param other 其他adc规则组对象
         * @return adc_regular_group& 本对象
         */
        adc_regular_group& operator= (adc_regular_group&& other) noexcept;

        /**
         * @brief 失能adc规则组，清除标志并失能dma，然后释放adc规则组资源，但不会停止已经开始的转换
         *
         */
        void release() noexcept;

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
         * @param mode dma传输模式
         * @param fifo_threshold fifo队列阈值
         * @param burst 默认内存侧突发
         * @param priority dma传输优先级
         * @param selected_stream 要使用的dma数据流，默认使用序号最小的数据流
         * @return dma数据流对象
         */
        [[nodiscard("该函数返回具有raii的dma数据流对象，不应该弃用返回值")]] ::SoC::dma_stream
            enable_dma(::SoC::dma& dma,
                       ::SoC::dma_mode mode,
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
         */
        void disable() const noexcept;

        /**
         * @brief 获取转换完成标志
         *
         * @return 转换完成标志
         */
        [[nodiscard]] bool get_flag_eocs() const noexcept;

        /**
         * @brief 清除转换完成标志
         *
         */
        void clear_flag_eocs() const noexcept;

        /**
         * @brief 获取溢出标志
         *
         * @return 溢出标志
         */
        [[nodiscard]] bool get_flag_ovr() const noexcept;

        /**
         * @brief 清除溢出标志
         *
         */
        void clear_flag_ovr() const noexcept;

        /**
         * @brief 获取adc结果
         *
         * @return adc结果
         */
        [[nodiscard]] ::std::size_t get_result() const noexcept;

        /**
         * @brief 停止dma，将adc的dma标志清除
         *
         */
        void disable_dma() const noexcept;

        /**
         * @brief 根据dma配置，设置adc的dma标志
         *
         */
        void set_dma() const noexcept;

        /**
         * @brief 首先清除dma标志，然后根据dma配置重设adc的dma标志
         *
         */
        void reset_dma() const noexcept;
    };

    /**
     * @brief adc内部通道控制
     *
     */
    struct adc_internal_channel
    {
        using internal_channel_enum = ::SoC::detail::adc_internal_channel;
        using enum internal_channel_enum;

    private:
        // adc内部通道枚举
        internal_channel_enum internal_channel;

    public:
        /**
         * @brief 使能adc内部通道
         *
         * @note 如果内部通道已使能，会断言失败
         * @param internal_channel 要使能的内部通道
         */
        adc_internal_channel(internal_channel_enum internal_channel) noexcept;

        /**
         * @brief 失能adc内部通道
         *
         */
        ~adc_internal_channel() noexcept;

        inline adc_internal_channel(const adc_internal_channel&) noexcept = delete("对象独占外设资源，不能复制构造");
        inline adc_internal_channel& operator= (const adc_internal_channel&) noexcept = delete("对象独占外设资源，不能复制赋值");

        /**
         * @brief 移动构造函数
         *
         * @param other 其他adc内部通道对象
         */
        adc_internal_channel(adc_internal_channel&& other) noexcept;

        /**
         * @brief 移动赋值运算符
         *
         * @param other 其他adc内部通道对象
         * @return adc_internal_channel& 本对象
         */
        adc_internal_channel& operator= (adc_internal_channel&& other) noexcept;

        /**
         * @brief 获取当前使能的adc内部通道枚举
         *
         * @return 使能的adc内部通道枚举
         */
        [[nodiscard]] inline internal_channel_enum get_channel() const noexcept { return internal_channel; }

        /**
         * @brief 失能adc内部通道，释放adc内部通道资源
         *
         */
        void release() noexcept;

        /**
         * @brief 使能adc内部通道
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能adc内部通道
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断adc内部通道是否使能
         *
         * @return 是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;
    };

    /**
     * @brief adc校准器
     *
     */
    struct adc_calibrator
    {
    private:
        // adc外设对象
        ::SoC::adc& adc;
        // adc内部通道对象
        ::SoC::adc_internal_channel internal_channel;
        // adc校准缓冲区类型
        using buffer_t = ::std::array<::std::array<::std::uint16_t, 2>, 8>;
        // adc校准缓冲区指针
        ::SoC::unique_ptr<buffer_t> buffer;
        // adc外设旧采样模式
        bool old_scan_mode;
        // adc外设旧分辨率
        ::SoC::adc_resolution old_resolution;
        // adc外设旧对齐方式
        ::SoC::adc_data_alignment old_alignment;

        // adc校准分辨率
        constexpr inline static auto resolution{::SoC::adc_resolution::bit12};
        // adc校准采样模式
        constexpr inline static auto scan_mode{true};
        // adc校准对齐方式
        constexpr inline static auto alignment{::SoC::adc_data_alignment::right};
        // adc校准规则组对象指针
        ::SoC::unique_ptr<::SoC::adc_regular_group> adc_regular_group;
        // adc校准dma数据流对象指针
        ::SoC::unique_ptr<::SoC::dma_stream> dma_stream;

    public:
        /**
         * @brief 创建adc校准器，不会阻塞控制流
         *
         * @note 会开启adc内部通道，使用SoC::ram_allocator分配和释放内存
         * @param adc adc外设对象，在F407上只能使用ADC1
         * @param dma dma外设对象
         */
        explicit adc_calibrator(::SoC::adc& adc, ::SoC::dma& dma) noexcept;

        inline adc_calibrator(const adc_calibrator&) noexcept = delete;
        inline adc_calibrator& operator= (const adc_calibrator&) noexcept = delete;
        inline adc_calibrator(adc_calibrator&&) noexcept = delete;
        inline adc_calibrator& operator= (adc_calibrator&&) noexcept = delete;

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
        [[nodiscard]] bool is_sample_ready() const noexcept;

        /**
         * @brief 获取校准结果，会阻塞直到采样完成
         *
         * @return std::pair{1LSB对应的电压, 温度}
         */
        [[nodiscard]] ::std::pair<float, float> get_result() const noexcept;
    };

    /**
     * @brief 模拟看门狗
     *
     */
    struct analog_watchdog
    {
        using awd_enum = ::SoC::detail::analog_watchdog;

    private:
        // adc外设指针
        ::ADC_TypeDef* adc_ptr{};
        // 模拟看门狗通道枚举
        awd_enum awd_channel{};
        // 低门限
        ::std::uint16_t low_threshold{};
        // 高门限
        ::std::uint16_t high_threshold{};
        // 模拟看门狗中断NVIC侧是否使能
        bool nvic_irq_enabled{};
        // ADC中断号
        constexpr inline static auto irqn{::IRQn_Type::ADC_IRQn};

    public:
        using enum awd_enum;

        /// 模拟看门狗门限最大值，共12位
        constexpr inline static ::std::uint16_t threshold_max{(1u << 12u) - 1};

        /**
         * @brief 配置adc模拟看门狗，会使能模拟看门狗
         *
         * @param adc adc外设
         * @param channel 要监视的通道
         * @param low_threshold 低门限，范围为[0, threshold_max]
         * @param high_threshold 高门限，范围为[0, threshold_max]
         */
        explicit analog_watchdog(::SoC::adc& adc,
                                 awd_enum channel,
                                 ::std::uint16_t low_threshold,
                                 ::std::uint16_t high_threshold) noexcept;

        /**
         * @brief 清除标志，关闭中断，然后失能模拟看门狗
         *
         * @note 由于所有ADC中断共用一个NVIC入口，所以只会在无其他对象使用adc中断时失能NVIC侧中断
         */
        ~analog_watchdog() noexcept;

        inline analog_watchdog(const analog_watchdog&) noexcept = delete("对象独占外设资源，不能复制构造");
        inline analog_watchdog& operator= (const analog_watchdog&) noexcept = delete("对象独占外设资源，不能赋值");

        /**
         * @brief 转移adc模拟看门狗资源的所有权
         *
         * @param other 其他对象
         */
        analog_watchdog(analog_watchdog&& other) noexcept;
        /**
         * @brief 转移adc模拟看门狗资源的所有权
         *
         * @param other 其他对象
         * @return analog_watchdog& 本对象
         */
        analog_watchdog& operator= (analog_watchdog&& other) noexcept;

        /**
         * @brief 清除标志，关闭中断，然后失能模拟看门狗，释放模拟看门狗资源
         *
         * @note 由于所有ADC中断共用一个NVIC入口，所以只会在无其他对象使用adc中断时失能NVIC侧中断
         */
        void release() noexcept;

        /**
         * @brief 获取adc外设指针
         *
         * @return adc外设指针
         */
        [[nodiscard]] inline ::ADC_TypeDef* get_adc() const noexcept { return adc_ptr; }

        /**
         * @brief 获取adc外设枚举
         *
         * @return adc外设枚举
         */
        [[nodiscard]] inline ::SoC::adc::adc_enum get_adc_enum() const noexcept
        { return ::SoC::bit_cast<::SoC::adc::adc_enum>(adc_ptr); }

        /**
         * @brief 获取模拟看门狗监视的通道
         *
         * @return 模拟看门狗通道枚举
         */
        [[nodiscard]] inline awd_enum get_channel() const noexcept { return awd_channel; }

        /**
         * @brief 判断模拟看门狗是否使能
         *
         * @return 模拟看门狗是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;

        /**
         * @brief 使能模拟看门狗
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能模拟看门狗
         *
         */
        void disable() const noexcept;

        /**
         * @brief 设置低门限
         *
         * @param threshold 门限值，范围为[0, threshold_max]
         */
        void set_low_threshold(::std::uint16_t threshold) noexcept;

        /**
         * @brief 设置高门限
         *
         * @param threshold 门限值，范围为[0, threshold_max]
         */
        void set_high_threshold(::std::uint16_t threshold) noexcept;

        /**
         * @brief 获取门限值
         *
         * @return std::pair{低门限, 高门限}
         */
        [[nodiscard]] inline ::std::pair<::std::uint16_t, ::std::uint16_t> get_threshold() const noexcept
        { return ::std::pair{low_threshold, high_threshold}; }

        /**
         * @brief 使能模拟看门狗中断
         *
         * @param preempt_priority 抢占中断优先级
         * @param sub_priority 响应中断优先级
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         */
        void enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept;

        /**
         * @brief 使能模拟看门狗中断
         *
         * @param encoded_priority 编码后的中断优先级
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         */
        void enable_irq(::std::size_t encoded_priority) noexcept;

        /**
         * @brief 失能模拟看门狗中断
         *
         * @note 函数是非并发安全的，NVIC侧中断操作应该在非中断上下文中进行
         */
        void disable_irq() noexcept;

        /**
         * @brief 设置模拟看门狗中断源状态
         *
         * @param enable 模拟看门狗中断源是否使能
         * @note 函数是并发安全的，可以在中断上下文中调用
         */
        void set_it_awd(bool enable) const noexcept;

        /**
         * @brief 获取模拟看门狗中断源状态
         *
         * @return 模拟看门狗中断源状态
         */
        [[nodiscard]] bool get_it_awd() const noexcept;

        /**
         * @brief 判断是否是模拟看门狗中断
         *
         * @return 是否是模拟看门狗中断
         */
        [[nodiscard]] bool is_it_awd() const noexcept;

        /**
         * @brief 获取模拟看门狗中断标志
         *
         * @return 模拟看门狗中断标志
         */
        [[nodiscard]] bool get_flag_awd() const noexcept;

        /**
         * @brief 清除模拟看门狗中断标志
         *
         */
        void clear_flag_awd() const noexcept;
    };
}  // namespace SoC
