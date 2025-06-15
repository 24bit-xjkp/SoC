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
        /// 中间对齐计数，仅向下计数时触发输出比较中断
        center_down = LL_TIM_COUNTERMODE_CENTER_DOWN,
        /// 中间对齐计数，仅向上计数时触发输出比较中断
        center_up = LL_TIM_COUNTERMODE_CENTER_UP,
        /// 中间对齐计数，上下计数均触发输出比较中断
        center_up_down = LL_TIM_COUNTERMODE_CENTER_UP_DOWN
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
     * @brief 定时器触发输出
     *
     */
    enum class tim_trigger_output : ::std::size_t
    {
        /// 强制触发更新事件
        reset = LL_TIM_TRGO_RESET,
        /// 定时器使能事件
        enable = LL_TIM_TRGO_ENABLE,
        /// 定时器更新事件
        update = LL_TIM_TRGO_UPDATE,
        /// 通道1捕获/比较
        cc1 = LL_TIM_TRGO_CC1IF,
        /// 通道1输出比较事件，不受输出配置影响
        oc1ref = LL_TIM_TRGO_OC1REF,
        /// 通道2输出比较事件，不受输出配置影响
        oc2ref = LL_TIM_TRGO_OC2REF,
        /// 通道3输出比较事件，不受输出配置影响
        oc3ref = LL_TIM_TRGO_OC3REF,
        /// 通道4输出比较事件，不受输出配置影响
        oc4ref = LL_TIM_TRGO_OC4REF
    };

    /**
     * @brief 定时器中断类型
     *
     */
    enum class tim_irq : ::std::size_t
    {
        /// 高级定时器刹车中断
        brk,
        /// 高级定时器更新中断
        update,
        /// 高级定时器换向和触发中断
        com_trig,
        /// 高级定时器通道比较/捕获中断
        cc,
        /// 非高级定时器使用，统一中断入口
        normal
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

        /**
         * @brief 判断当前定时器是不是高级定时器
         *
         */
        bool is_advanced_tim() const noexcept;

        /**
         * @brief 检查当前定时器是不是高级定时器，不是则断言失败
         *
         * @param location 源代码位置
         */
        void check_advanced_tim(::std::source_location location = ::std::source_location::current()) const noexcept;

        /**
         * @brief 根据中断类型获取中断号枚举
         *
         * @param irq 中断类型
         * @return 中断号枚举
         */
        ::IRQn_Type get_irqn(::SoC::tim_irq irq) const noexcept;

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
        tim& operator= (tim&& other) noexcept = delete;
        ~tim() noexcept;

        /**
         * @brief 获取tim外设指针
         *
         * @return tim外设指针
         */
        constexpr inline ::TIM_TypeDef* get_tim() const noexcept { return tim_ptr; }

        /**
         * @brief 获取tim外设枚举
         *
         * @return tim外设枚举
         */
        constexpr inline tim_enum get_tim_enum() const noexcept { return ::std::bit_cast<tim_enum>(tim_ptr); }

        /**
         * @brief 使能tim外设，包括计数和高级定时器的输出
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能tim外设，包括计数和高级定时器的输出，不会关闭时钟
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断tim外设计数是否使能
         *
         * @return tim外设计数是否使能
         */
        bool is_enabled() const noexcept;

        /**
         * @brief 判断tim外设的输出是否使能
         *
         * @note 高级定时器的输出使能才有意义，但此函数不会检查定时器类型
         * @return tim外设的输出是否使能
         */
        bool is_output_enabled() const noexcept;

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

        /**
         * @brief 设置定时器触发输出
         *
         * @param trigger 定时器触发输出
         */
        void set_trigger_output(::SoC::tim_trigger_output trigger) const noexcept;

        /**
         * @brief 设置定时器刹车中断源是否使能
         *
         * @note 只有高级定时器支持
         * @param enable 是否使能
         */
        void set_it_brk(bool enable) const noexcept;

        /**
         * @brief 获取定时器刹车中断源是否使能
         *
         * @note 只有高级定时器支持
         * @return 定时器刹车中断源是否使能
         */
        bool get_it_brk() const noexcept;

        /**
         * @brief 设置定时器触发中断源是否使能
         *
         * @param enable 是否使能
         */
        void set_it_trig(bool enable) const noexcept;

        /**
         * @brief 获取定时器触发中断源是否使能
         *
         * @return 定时器触发中断源是否使能
         */
        bool get_it_trig() const noexcept;

        /**
         * @brief 设置定时器换向中断源是否使能
         *
         * @note 只有高级定时器支持
         * @param enable 是否使能
         */
        void set_it_com(bool enable) const noexcept;

        /**
         * @brief 获取定时器换向中断源是否使能
         *
         * @note 只有高级定时器支持
         * @return 定时器换向中断源是否使能
         */
        bool get_it_com() const noexcept;

        /**
         * @brief 设置定时器更新中断源是否使能
         *
         * @param enable 是否使能
         */
        void set_it_update(bool enable) const noexcept;

        /**
         * @brief 获取定时器更新中断源是否使能
         *
         * @return 定时器更新中断源是否使能
         */
        bool get_it_update() const noexcept;

        /**
         * @brief 使能定时器中断
         *
         * @param irq 要使能的中断
         * @param encoded_priority 编码后的优先级
         */
        void enable_irq(::SoC::tim_irq irq, ::std::size_t encoded_priority) const noexcept;

        /**
         * @brief 使能定时器中断
         *
         * @param irq 要使能的中断
         * @param preempt_priority 抢占优先级
         * @param sub_priority 响应优先级
         */
        void enable_irq(::SoC::tim_irq irq, ::std::size_t preempt_priority, ::std::size_t sub_priority) const noexcept;

        /**
         * @brief 失能定时器中断
         *
         * @param irq 要失能的中断
         */
        void disable_irq(::SoC::tim_irq irq) const noexcept;

        /**
         * @brief 获取定时器刹车标志
         *
         * @note 只有高级定时器支持
         * @return 定时器刹车标志
         */
        bool get_flag_brk() const noexcept;

        /**
         * @brief 清除定时器刹车标志
         *
         * @note 只有高级定时器支持
         */
        void clear_flag_brk() const noexcept;

        /**
         * @brief 获取定时器触发标志
         *
         * @return 定时器触发标志
         */
        bool get_flag_trig() const noexcept;

        /**
         * @brief 清除定时器触发标志
         *
         */
        void clear_flag_trig() const noexcept;

        /**
         * @brief 获取定时器换向标志
         *
         * @note 只有高级定时器支持
         * @return 定时器换向标志
         */
        bool get_flag_com() const noexcept;

        /**
         * @brief 清除定时器换向标志
         *
         * @note 只有高级定时器支持
         */
        void clear_flag_com() const noexcept;

        /**
         * @brief 获取定时器更新标志
         *
         * @return 定时器更新标志
         */
        bool get_flag_update() const noexcept;

        /**
         * @brief 清除定时器更新标志
         *
         */
        void clear_flag_update() const noexcept;

        /**
         * @brief 判断当前中断是否是刹车中断
         *
         * @note 只有高级定时器支持
         * @return 是否是刹车中断
         */
        bool is_it_brk() const noexcept;

        /**
         * @brief 判断当前中断是否是触发中断
         *
         * @return 是否是触发中断
         */
        bool is_it_trig() const noexcept;

        /**
         * @brief 判断当前中断是否是换向中断
         *
         * @note 只有高级定时器支持
         * @return 是否是换向中断
         */
        bool is_it_com() const noexcept;

        /**
         * @brief 判断当前中断是否是更新中断
         *
         * @return 是否是更新中断
         */
        bool is_it_update() const noexcept;
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
         * @param location 源代码位置
         */
        void check_mode_oc(::std::source_location location = ::std::source_location::current()) const noexcept;

        /**
         * @brief 获取通道对应中断和标志的掩码
         *
         * @return 通道对应中断和标志的掩码
         */
        ::std::size_t get_it_flag_mask() const noexcept;

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
         * @brief 判断tim外设主通道是否使能
         *
         * @return tim外设通道是否使能
         */
        bool is_enabled() const noexcept;

        /**
         * @brief 判断tim外设互补通道是否使能
         *
         * @return true 关联的互补通道已使能
         * @return false 关联的互补通道未使能或未关联互补通道
         */
        bool is_compl_enabled() const noexcept;

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

        /**
         * @brief 设置定时器比较/捕获中断是否使能
         *
         * @param enable 是否使能
         */
        void set_it_cc(bool enable) const noexcept;

        /**
         * @brief 判断定时器比较/捕获中断是否使能
         *
         * @return 定时器比较/捕获中断是否使能
         */
        bool get_it_cc() const noexcept;

        /**
         * @brief 获取定时器比较/捕获标志
         *
         * @return 定时器比较/捕获标志
         */
        bool get_flag_cc() const noexcept;

        /**
         * @brief 清除定时器比较/捕获标志
         *
         */
        void clear_flag_cc() const noexcept;

        /**
         * @brief 判断当前中断是否为定时器比较/捕获中断
         *
         * @return 当前中断是否为定时器比较/捕获中断
         */
        bool is_it_cc() const noexcept;
    };
}  // namespace SoC
