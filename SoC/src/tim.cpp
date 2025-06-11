#include "../include/tim.hpp"
#include "../include/nvic.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    /**
     * @brief 检查val是否在16位无符号数范围内，用于在16位定时器上实现输入范围检查
     *
     * @param tim_ptr tim外设指针
     * @param val 要检查的数据
     */
    inline void check_tim_u16(::TIM_TypeDef* tim_ptr, ::std::uint32_t val) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            if(auto tim_enum{::std::bit_cast<::SoC::detail::tim>(tim_ptr)};
               tim_enum != ::SoC::tim::tim2 && tim_enum != ::SoC::tim::tim5)
            {
                ::SoC::assert(val <= ::std::numeric_limits<::std::uint16_t>::max(), "此计数器为16位计数器."sv);
            }
        }
    }

    /**
     * @brief 获取tim外设的最大通道枚举
     *
     * @param tim_ptr tim外设指针
     * @return 通道枚举
     */
    inline ::SoC::detail::tim_channel get_max_channel(::TIM_TypeDef* tim_ptr) noexcept
    {
        switch(::std::bit_cast<::SoC::detail::tim>(tim_ptr))
        {
            case ::SoC::tim::tim1:
            case ::SoC::tim::tim2:
            case ::SoC::tim::tim3:
            case ::SoC::tim::tim4:
            case ::SoC::tim::tim5:
            case ::SoC::tim::tim8: return ::SoC::tim_channel::ch4;
            case ::SoC::tim::tim9:
            case ::SoC::tim::tim12: return ::SoC::tim_channel::ch2;
            default: return ::SoC::tim_channel::ch1;
        }
    }

    constexpr inline ::std::strong_ordering operator<=> (::SoC::detail::tim_channel lhs, ::SoC::detail::tim_channel rhs) noexcept
    {
        return ::std::to_underlying(lhs) <=> ::std::to_underlying(rhs);
    }

    ::SoC::tim::tim(tim_enum tim,
                    ::std::uint16_t prescaler,
                    ::std::size_t auto_reload,
                    ::SoC::tim_mode mode,
                    ::SoC::tim_clock_div clock_div,
                    ::std::uint16_t rep_cnt) noexcept : tim_ptr{::std::bit_cast<::TIM_TypeDef*>(tim)}
    {
        const auto check_rep_u8{[rep_cnt] noexcept -> void
                                {
                                    if constexpr(::SoC::use_full_assert)
                                    {
                                        ::SoC::assert(rep_cnt <= ::std::numeric_limits<::std::uint8_t>::max(),
                                                      "此计数器重复次数上限为255."sv);
                                    }
                                    else
                                    {
                                        // 消除未使用捕获的警告
                                        auto _{rep_cnt};
                                    }
                                }};
        const auto check_upcnt_only{[mode] noexcept -> void
                                    {
                                        if(::SoC::use_full_assert)
                                        {
                                            ::SoC::assert(mode == ::SoC::tim_mode::up, "此计数器仅支持向上计数"sv);
                                        }
                                        else
                                        {
                                            // 消除未使用捕获的警告
                                            auto _{mode};
                                        }
                                    }};
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此定时器不应处于使能状态"sv); }

        switch(tim)
        {
            case tim1:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM1};
                break;
            case tim2:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM2};
                if constexpr(::SoC::use_full_assert) { check_rep_u8(); }
                break;
            case tim3:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM3};
                if constexpr(::SoC::use_full_assert) { check_rep_u8(); }
                break;
            case tim4:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM4};
                if constexpr(::SoC::use_full_assert) { check_rep_u8(); }
                break;
            case tim5:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM5};
                if constexpr(::SoC::use_full_assert) { check_rep_u8(); }
                break;
            case tim6:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM6};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
            case tim7:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM7};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
            case tim8:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM8};
                break;
            case tim9:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM9);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM9};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
            case tim10:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM10);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM10};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
            case tim11:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM11);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM11};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
            case tim12:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM12);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM12};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
            case tim13:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM13);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM13};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
            case tim14:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM14};
                if constexpr(::SoC::use_full_assert)
                {
                    check_rep_u8();
                    check_upcnt_only();
                }
                break;
        }
        ::LL_TIM_SetCounterMode(tim_ptr, ::std::to_underlying(mode));
        ::LL_TIM_SetClockDivision(tim_ptr, ::std::to_underlying(clock_div));
        set_auto_reload(auto_reload);
        ::LL_TIM_SetPrescaler(tim_ptr, prescaler);
        ::LL_TIM_SetRepetitionCounter(tim_ptr, rep_cnt);
    }

    ::SoC::tim::tim(::SoC::tim&& other) noexcept
    {
        ::std::memcpy(reinterpret_cast<void*>(this), &other, sizeof(*this));
        other.tim_ptr = nullptr;
    }

    ::SoC::tim::~tim() noexcept
    {
        if(tim_ptr != nullptr) [[likely]]
        {
            disable();
            callback();
        }
    }

    void ::SoC::tim::enable() const noexcept
    {
        ::LL_TIM_EnableAllOutputs(tim_ptr);
        ::LL_TIM_EnableCounter(tim_ptr);
    }

    void ::SoC::tim::disable() const noexcept
    {
        ::LL_TIM_DisableAllOutputs(tim_ptr);
        ::LL_TIM_DisableCounter(tim_ptr);
    }

    bool ::SoC::tim::is_enabled() const noexcept { return ::LL_TIM_IsEnabledCounter(tim_ptr); }

    bool ::SoC::tim::is_output_enabled() const noexcept { return ::LL_TIM_IsEnabledAllOutputs(tim_ptr); }

    void ::SoC::tim::enable_arr_preload() const noexcept { ::LL_TIM_EnableARRPreload(tim_ptr); }

    void ::SoC::tim::disable_arr_preload() const noexcept { ::LL_TIM_DisableARRPreload(tim_ptr); }

    void ::SoC::tim::set_auto_reload(::std::size_t auto_reload, bool force_update) const noexcept
    {
        check_tim_u16(tim_ptr, auto_reload);
        ::LL_TIM_SetAutoReload(tim_ptr, auto_reload);
        if(force_update) { ::LL_TIM_GenerateEvent_UPDATE(tim_ptr); }
    }

    void ::SoC::tim::set_trigger_output(::SoC::tim_trigger_output trigger) const noexcept
    {
        const auto check{[trigger](::SoC::tim_trigger_output threshold)
                         {
                             if constexpr(::SoC::use_full_assert)
                             {
                                 ::SoC::assert(trigger <= threshold, "此定时器不支持此触发输出"sv);
                             }
                             else
                             {
                                 // 消除未使用捕获的警告
                                 auto _{trigger};
                             }
                         }};
        if constexpr(::SoC::use_full_assert)
        {
            switch(get_tim_enum())
            {
                case tim1: break;
                case tim8: break;
                case tim2:
                case tim3:
                case tim4:
                case tim5:
                case tim9:
                case tim12: check(::SoC::tim_trigger_output::oc2ref); break;
                case tim10:
                case tim11:
                case tim13:
                case tim14: check(::SoC::tim_trigger_output::oc1ref); break;
                case tim6:
                case tim7: check(::SoC::tim_trigger_output::update); break;
            }
        }
        ::LL_TIM_SetTriggerOutput(tim_ptr, ::std::to_underlying(trigger));
    }

    bool ::SoC::tim::is_advanced_tim() const noexcept
    {
        auto tim{get_tim_enum()};
        return tim == tim1 || tim == tim8;
    }

    void ::SoC::tim::check_advanced_tim() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(is_advanced_tim(), "只有高级定时器支持该功能"sv); }
    }

    void ::SoC::tim::set_it_brk(bool enable) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        if(enable) { ::LL_TIM_EnableIT_BRK(tim_ptr); }
        else
        {
            ::LL_TIM_DisableIT_BRK(tim_ptr);
        }
    }

    bool ::SoC::tim::get_it_brk() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        return ::LL_TIM_IsEnabledIT_BRK(tim_ptr);
    }

    void ::SoC::tim::set_it_trig(bool enable) const noexcept
    {
        if(enable) { ::LL_TIM_EnableIT_TRIG(tim_ptr); }
        else
        {
            ::LL_TIM_DisableIT_TRIG(tim_ptr);
        }
    }

    bool ::SoC::tim::get_it_trig() const noexcept { return ::LL_TIM_IsEnabledIT_TRIG(tim_ptr); }

    void ::SoC::tim::set_it_com(bool enable) const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        if(enable) { ::LL_TIM_EnableIT_COM(tim_ptr); }
        else
        {
            ::LL_TIM_DisableIT_COM(tim_ptr);
        }
    }

    bool ::SoC::tim::get_it_com() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        return ::LL_TIM_IsEnabledIT_COM(tim_ptr);
    }

    void ::SoC::tim::set_it_update(bool enable) const noexcept
    {
        if(enable) { ::LL_TIM_EnableIT_UPDATE(tim_ptr); }
        else
        {
            ::LL_TIM_DisableIT_UPDATE(tim_ptr);
        }
    }

    bool ::SoC::tim::get_it_update() const noexcept { return ::LL_TIM_IsEnabledIT_UPDATE(tim_ptr); }

    ::IRQn_Type(::SoC::tim::get_irqn)(::SoC::tim_irq irq) const noexcept
    {
        if(is_advanced_tim())
        {
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert(irq != ::SoC::tim_irq::normal, "高级定时器使用多个中断入口，必须指明要使能的中断"sv);
            }
            constexpr auto tim1_irqn_base{::IRQn_Type::TIM1_BRK_TIM9_IRQn};
            constexpr auto tim8_irqn_base{::IRQn_Type::TIM8_BRK_TIM12_IRQn};
            auto irqn_base{get_tim_enum() == tim1 ? tim1_irqn_base : tim8_irqn_base};
            return static_cast<::IRQn_Type>(irqn_base + ::std::to_underlying(irq));
        }
        else
        {
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert(irq == ::SoC::tim_irq::normal, "非高级定时器只有一个中断入口，必须设置为normal"sv);
            }
            switch(get_tim_enum())
            {
                case tim2: return ::IRQn_Type::TIM2_IRQn;
                case tim3: return ::IRQn_Type::TIM3_IRQn;
                case tim4: return ::IRQn_Type::TIM4_IRQn;
                case tim5: return ::IRQn_Type::TIM5_IRQn;
                case tim6: return ::IRQn_Type::TIM6_DAC_IRQn;
                case tim7: return ::IRQn_Type::TIM7_IRQn;
                case tim9: return ::IRQn_Type::TIM1_BRK_TIM9_IRQn;
                case tim10: return ::IRQn_Type::TIM1_UP_TIM10_IRQn;
                case tim11: return ::IRQn_Type::TIM1_TRG_COM_TIM11_IRQn;
                case tim12: return ::IRQn_Type::TIM8_BRK_TIM12_IRQn;
                case tim13: return ::IRQn_Type::TIM8_UP_TIM13_IRQn;
                case tim14: return ::IRQn_Type::TIM8_TRG_COM_TIM14_IRQn;
                case tim1:
                case tim8:
                default: ::std::unreachable();
            }
        }
    }

    void ::SoC::tim::enable_irq(::SoC::tim_irq irq, ::std::size_t encoded_priority) const noexcept
    {
        auto irqn{get_irqn(irq)};
        ::SoC::enable_irq(irqn);
        ::SoC::set_priority(irqn, encoded_priority);
    }

    void ::SoC::tim::enable_irq(::SoC::tim_irq irq, ::std::size_t preempt_priority, ::std::size_t sub_priority) const noexcept
    {
        auto irqn{get_irqn(irq)};
        ::SoC::enable_irq(irqn);
        ::SoC::set_priority(irqn, preempt_priority, sub_priority);
    }

    void ::SoC::tim::disable_irq(::SoC::tim_irq irq) const noexcept { ::SoC::disable_irq(get_irqn(irq)); }

    bool ::SoC::tim::get_flag_brk() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        return ::LL_TIM_IsActiveFlag_BRK(tim_ptr);
    }

    void ::SoC::tim::clear_flag_brk() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        ::LL_TIM_ClearFlag_BRK(tim_ptr);
    }

    bool ::SoC::tim::get_flag_trig() const noexcept { return ::LL_TIM_IsActiveFlag_TRIG(tim_ptr); }

    void ::SoC::tim::clear_flag_trig() const noexcept { ::LL_TIM_ClearFlag_TRIG(tim_ptr); }

    bool ::SoC::tim::get_flag_com() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        return ::LL_TIM_IsActiveFlag_COM(tim_ptr);
    }

    void ::SoC::tim::clear_flag_com() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_advanced_tim(); }
        ::LL_TIM_ClearFlag_COM(tim_ptr);
    }

    bool ::SoC::tim::get_flag_update() const noexcept { return ::LL_TIM_IsActiveFlag_UPDATE(tim_ptr); }

    void ::SoC::tim::clear_flag_update() const noexcept { ::LL_TIM_ClearFlag_UPDATE(tim_ptr); }

    bool ::SoC::tim::is_it_brk() const noexcept { return get_flag_brk() && get_it_brk(); }

    bool ::SoC::tim::is_it_trig() const noexcept { return get_flag_trig() && get_it_trig(); }

    bool ::SoC::tim::is_it_com() const noexcept { return get_flag_com() && get_it_com(); }

    bool ::SoC::tim::is_it_update() const noexcept { return get_flag_update() && get_it_update(); }
}  // namespace SoC

namespace SoC
{
    ::SoC::tim_channel::tim_channel(::SoC::tim& tim,
                                    tim_channel_enum channel,
                                    ::SoC::tim_oc_mode mode,
                                    ::std::uint32_t compare_value,
                                    bool init_state,
                                    ::SoC::tim_oc_polarity polarity) noexcept :
        tim_ptr{tim.tim_ptr}, channel{channel}, channel_mode{tim_channel_mode::oc}
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(!is_enabled(), "初始化前此通道不应处于使能状态"sv);
            ::SoC::assert(channel <= ::SoC::get_max_channel(tim_ptr), "此定时器不具有指定的通道"sv);
        }

        ::LL_TIM_OC_SetMode(tim_ptr, ::std::to_underlying(channel), ::std::to_underlying(mode));
        set_compare_value(compare_value);

        ::LL_TIM_OC_SetPolarity(tim_ptr, ::std::to_underlying(channel), ::std::to_underlying(polarity));
        if(init_state) [[likely]] { enable(); }
    }

    ::SoC::tim_channel::tim_channel(::SoC::tim_channel&& other) noexcept
    {
        ::std::memcpy(reinterpret_cast<void*>(this), &other, sizeof(*this));
        other.tim_ptr = nullptr;
    }

    ::SoC::tim_channel::~tim_channel() noexcept
    {
        if(tim_ptr) { disable(); }
    }

    void ::SoC::tim_channel::enable() const noexcept
    {
        ::LL_TIM_CC_EnableChannel(tim_ptr, ::std::to_underlying(channel));
        if(has_compl_channel()) { ::LL_TIM_CC_EnableChannel(tim_ptr, ::std::to_underlying(compl_channel)); }
    }

    void ::SoC::tim_channel::disable() const noexcept
    {
        ::LL_TIM_CC_DisableChannel(tim_ptr, ::std::to_underlying(channel));
        if(has_compl_channel()) { ::LL_TIM_CC_DisableChannel(tim_ptr, ::std::to_underlying(compl_channel)); }
    }

    bool ::SoC::tim_channel::is_enabled() const noexcept
    {
        return ::LL_TIM_CC_IsEnabledChannel(tim_ptr, ::std::to_underlying(channel));
    }

    bool ::SoC::tim_channel::is_compl_enabled() const noexcept
    {
        return has_compl_channel() && ::LL_TIM_CC_IsEnabledChannel(tim_ptr, ::std::to_underlying(compl_channel));
    }

    void ::SoC::tim_channel::check_mode_oc() const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(channel_mode == ::SoC::tim_channel::tim_channel_mode::oc, "此通道应处于输出比较模式"sv);
        }
    }

    void ::SoC::tim_channel::configure_compl_channel(::SoC::tim_oc_polarity polarity) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            ::SoC::assert(!has_compl_channel(), "初始化互补通道前此对象不应该有关联的互补通道"sv);
            check_mode_oc();
            ::SoC::assert(channel != ::SoC::tim_channel::ch4, "定时器的通道4不具有互补通道"sv);
        }
        compl_channel = static_cast<::SoC::detail::tim_channel>(::std::to_underlying(channel) << 2);
        ::LL_TIM_OC_SetPolarity(tim_ptr, ::std::to_underlying(compl_channel), ::std::to_underlying(polarity));
    }

    void ::SoC::tim_channel::remove_compl_channel() noexcept
    {
        if(has_compl_channel()) [[likely]]
        {
            ::LL_TIM_CC_DisableChannel(tim_ptr, ::std::to_underlying(compl_channel));
            compl_channel = ::SoC::tim_channel::tim_channel_enum{};
        }
    }

    void ::SoC::tim_channel::enable_oc_preload() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_mode_oc(); }
        ::LL_TIM_OC_EnablePreload(tim_ptr, ::std::to_underlying(channel));
    }

    void ::SoC::tim_channel::disable_oc_preload() const noexcept
    {
        if constexpr(::SoC::use_full_assert) { check_mode_oc(); }
        ::LL_TIM_OC_DisablePreload(tim_ptr, ::std::to_underlying(channel));
    }

    void ::SoC::tim_channel::set_compare_value(::std::uint32_t compare_value, bool force_update) const noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            check_mode_oc();
            check_tim_u16(tim_ptr, compare_value);
        }
        switch(channel)
        {
            case ch1: ::LL_TIM_OC_SetCompareCH1(tim_ptr, compare_value); break;
            case ch2: ::LL_TIM_OC_SetCompareCH2(tim_ptr, compare_value); break;
            case ch3: ::LL_TIM_OC_SetCompareCH3(tim_ptr, compare_value); break;
            case ch4: ::LL_TIM_OC_SetCompareCH4(tim_ptr, compare_value); break;
        }
        if(force_update) { ::LL_TIM_GenerateEvent_UPDATE(tim_ptr); }
    }

    ::std::size_t(::SoC::tim_channel::get_it_flag_mask)() const noexcept
    {
        auto shift{::std::countr_zero(::std::to_underlying(channel))};
        [[assume(shift <= 12)]];
        shift /= 3;
        return 1zu << shift;
    }

    void ::SoC::tim_channel::set_it_cc(bool enable) const noexcept
    {
        auto mask{get_it_flag_mask()};
        if(enable) { tim_ptr->DIER |= mask; }
        else
        {
            tim_ptr->DIER &= ~mask;
        }
    }

    bool ::SoC::tim_channel::get_it_cc() const noexcept
    {
        auto mask{get_it_flag_mask()};
        return (tim_ptr->DIER & mask) == mask;
    }

    bool ::SoC::tim_channel::get_flag_cc() const noexcept
    {
        auto mask{get_it_flag_mask()};
        return (tim_ptr->SR & mask) == mask;
    }

    void ::SoC::tim_channel::clear_flag_cc() const noexcept { tim_ptr->SR = ~get_it_flag_mask(); }

    bool ::SoC::tim_channel::is_it_cc() const noexcept { return get_flag_cc() && get_it_cc(); }
}  // namespace SoC
