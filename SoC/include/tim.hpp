#pragma once
#include "utils.hpp"

namespace SoC
{
    namespace detail
    {
        /**
         * @brief tim外设基地址
         *
         */
        enum class tim : ::std::uintptr_t
        {
            tim1 = TIM1_BASE,
            tim2 = TIM2_BASE,
            tim3 = TIM3_BASE,
            tim4 = TIM4_BASE,
            tim5 = TIM5_BASE,
            tim6 = TIM6_BASE,
            tim7 = TIM7_BASE,
            tim8 = TIM8_BASE,
            tim9 = TIM9_BASE,
            tim10 = TIM10_BASE,
            tim11 = TIM11_BASE,
            tim12 = TIM12_BASE,
            tim13 = TIM13_BASE,
            tim14 = TIM14_BASE,
        };

        /**
         * @brief tim外设主通道
         *
         */
        enum class tim_channel : ::std::size_t
        {
            /// 主通道1
            ch1 = LL_TIM_CHANNEL_CH1,
            /// 主通道2
            ch2 = LL_TIM_CHANNEL_CH2,
            /// 主通道3
            ch3 = LL_TIM_CHANNEL_CH3,
            /// 主通道4
            ch4 = LL_TIM_CHANNEL_CH4
        };
    }  // namespace detail

    /**
     * @brief tim外设计数模式
     *
     */
    enum class tim_mode : ::std::size_t
    {
        /// 向上计数
        up = LL_TIM_COUNTERMODE_UP,
        /// 向下计数
        down = LL_TIM_COUNTERMODE_DOWN,
        /// 中间对齐计数，仅向下计数时触发输出比较
        center_down = LL_TIM_COUNTERMODE_CENTER_DOWN,
        /// 中间对齐计数，仅向上计数时触发输出比较
        center_up = LL_TIM_COUNTERMODE_CENTER_UP,
        /// 中间对齐计数，上下计数均触发输出比较
        center_up_down = LL_TIM_COUNTERMODE_CENTER_UP
    };

    /**
     * @brief tim外设时钟分割模式
     *
     */
    enum class tim_clock_div : ::std::size_t
    {
        /// 不分割
        div1 = LL_TIM_CLOCKDIVISION_DIV1,
        /// 时钟分割为2分频
        div2 = LL_TIM_CLOCKDIVISION_DIV2,
        /// 时钟分割为4分频
        div4 = LL_TIM_CLOCKDIVISION_DIV4
    };

    /**
     * @brief tim外设输出比较模式
     *
     */
    enum class tim_oc_mode : ::std::size_t
    {
        /// 冻结模式
        frozen = LL_TIM_OCMODE_FROZEN,
        /// 匹配时设置通道为有效电平
        active = LL_TIM_OCMODE_ACTIVE,
        /// 匹配时设置通道为无效电平
        inactive = LL_TIM_OCMODE_INACTIVE,
        /// 匹配时通道电平翻转
        toggle = LL_TIM_OCMODE_TOGGLE,
        /// 强制输出有效电平
        force_active = LL_TIM_OCMODE_FORCED_ACTIVE,
        /// 强制输出无效电平
        force_inactive = LL_TIM_OCMODE_FORCED_INACTIVE,
        /// PWM模式1，CNT < CCRx输出有效电平，反之输出无效电平
        pwm1 = LL_TIM_OCMODE_PWM1,
        /// PWM模式2，CNT < CCRx输出无效电平，反之输出有效电平
        pwm2 = LL_TIM_OCMODE_PWM2
    };

    /**
     * @brief tim外设输出极性
     *
     */
    enum class tim_oc_polarity : ::std::size_t
    {
        /// 高电平有效
        high = LL_TIM_OCPOLARITY_HIGH,
        /// 低电平有效
        low = LL_TIM_OCPOLARITY_LOW
    };

    /**
     * @brief tim外设
     *
     */
    struct tim
    {
    private:
        friend struct tim_channel;
        using tim_enum = ::SoC::detail::tim;
        ::TIM_TypeDef* tim_ptr;
        ::SoC::detail::dtor_close_clock_callback_t callback;

    public:
        using enum tim_enum;

        /**
         * @brief 初始化tim外设，不启动计数
         *
         * @param tim tim外设枚举
         * @param prescaler 预分频数-1
         * @param auto_reload 自动重装值
         * @param mode tim计数模式
         * @param clock_div 时钟分割，默认不分割
         * @param rep_cnt 重复次数-1，默认为重复1次就产生事件
         */
        explicit tim(tim_enum tim,
                     ::std::uint16_t prescaler,
                     ::std::size_t auto_reload,
                     ::SoC::tim_mode mode = ::SoC::tim_mode::up,
                     ::SoC::tim_clock_div clock_div = ::SoC::tim_clock_div::div1,
                     ::std::uint16_t rep_cnt = 0) noexcept;

        tim(const tim&) noexcept = delete;
        tim& operator= (const tim&) noexcept = delete;
        tim(tim&& other) noexcept;
        tim& operator= (tim&& other) noexcept;
        ~tim() noexcept;

        /**
         * @brief 获取tim外设指针
         *
         * @return tim外设指针
         */
        constexpr inline ::TIM_TypeDef* get_tim() const noexcept { return tim_ptr; }

        /**
         * @brief 使能tim外设
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能tim外设
         *
         */
        void disable() const noexcept;

        /**
         * @brief 使能arr预装载功能
         *
         */
        void enable_arr_preload() const noexcept;

        /**
         * @brief 失能arr预装载功能
         *
         */
        void disable_arr_preload() const noexcept;

        /**
         * @brief 设置自动重装值
         *
         * @param auto_reload 自动重装值
         * @param force_update 是否强制立即更新，为true通过产生update事件实现立即更新
         */
        void set_auto_reload(::std::size_t auto_reload, bool force_update = false) const noexcept;
    };

    /**
     * @brief tim外设通道
     *
     */
    struct tim_channel
    {
    private:
        /**
         * @brief tim外设通道工作模式
         *
         */
        enum class tim_channel_mode : ::std::size_t
        {
            /// 输出比较模式
            oc,
            /// 输入捕获模式
            ic,
            /// 编码模式
            encode
        };

        using tim_channel_enum = ::SoC::detail::tim_channel;
        ::TIM_TypeDef* tim_ptr;
        tim_channel_enum channel;
        tim_channel_enum compl_channel{};
        tim_channel_mode channel_mode;

        /**
         * @brief 检查当前通道的模式是否是输出比较模式
         *
         */
        void check_mode_oc() const noexcept;

    public:
        using enum tim_channel_enum;

        /**
         * @brief 初始化输出比较模式的通道
         *
         * @param tim tim外设
         * @param channel 定时器输出枚举
         * @param mode 输出比较模式
         * @param compare_value 比较值
         * @param init_state 初始状态，true表示使能，false表示失能
         * @param polarity 输出极性
         */
        explicit tim_channel(::SoC::tim& tim,
                             ::SoC::tim_channel::tim_channel_enum channel,
                             ::SoC::tim_oc_mode mode,
                             ::std::uint32_t compare_value,
                             bool init_state = true,
                             ::SoC::tim_oc_polarity polarity = ::SoC::tim_oc_polarity::high) noexcept;

        tim_channel(const tim_channel&) noexcept = delete;
        tim_channel& operator= (const tim_channel&) noexcept = delete;
        tim_channel(tim_channel&& other) noexcept;
        tim_channel& operator= (tim_channel&& other) noexcept = delete;
        ~tim_channel() noexcept;

        /**
         * @brief 获取tim外设指针
         *
         * @return tim外设指针
         */
        constexpr inline ::TIM_TypeDef* get_tim() const noexcept { return tim_ptr; }

        /**
         * @brief 获取tim主通道枚举
         *
         * @return tim主通道枚举
         */
        constexpr inline tim_channel_enum get_channel() const noexcept { return channel; }

        /**
         * @brief 判断当前tim通道对象是否有绑定互补通道
         *
         * @return 是否绑定互补通道
         */
        constexpr inline bool has_compl_channel() const noexcept { return compl_channel == tim_channel_enum{}; }

        /**
         * @brief 使能tim外设通道，同时处理关联的互补通道
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能tim外设通道，同时处理关联的互补通道
         *
         */
        void disable() const noexcept;

        /**
         * @brief 配置互补通道，将互补通道关联到此对象
         *
         * @param polarity 互补通道输出极性
         */
        void configure_compl_channel(::SoC::tim_oc_polarity polarity = ::SoC::tim_oc_polarity::low) noexcept;

        /**
         * @brief 关闭并移除绑定的互补通道
         *
         * @note 若不存在互补通道则不进行操作
         */
        void remove_compl_channel() noexcept;

        /**
         * @brief 使能输出比较模式下ccr预装载
         *
         */
        void enable_oc_preload() const noexcept;

        /**
         * @brief 失能输出比较模式下ccr预装载
         *
         */
        void disable_oc_preload() const noexcept;

        /**
         * @brief 设置比较值
         *
         * @param compare_value 比较值
         * @param force_update 是否强制立即更新，为true通过产生update事件实现立即更新
         */
        void set_compare_value(::std::uint32_t compare_value, bool force_update = false) const noexcept;
    };
}  // namespace SoC
