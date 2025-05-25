#include "../include/tim.hpp"

namespace SoC
{
    using namespace ::std::string_view_literals;

    /**
     * @brief 检查val是否在16位无符号数范围内，用于在16位定时器上实现输入范围检查
     *
     * @param val 要检查的数据
     */
    inline void check_tim_u16(::std::uint32_t val) noexcept
    {
        ::SoC::assert(val <= ::std::numeric_limits<::std::uint16_t>::max(), "此计数器为16位计数器."sv);
    }

    /**
     * @brief 获取tim外设的最大通道枚举
     *
     * @param tim tim外设指针
     * @return 通道枚举
     */
    inline ::SoC::detail::tim_channel get_max_channel(::TIM_TypeDef* tim) noexcept
    {
        switch(::std::bit_cast<::SoC::detail::tim>(tim))
        {
            case ::SoC::tim::tim1: [[fallthrough]];
            case ::SoC::tim::tim2: [[fallthrough]];
            case ::SoC::tim::tim3: [[fallthrough]];
            case ::SoC::tim::tim4: [[fallthrough]];
            case ::SoC::tim::tim5: [[fallthrough]];
            case ::SoC::tim::tim8: return ::SoC::tim_channel::ch4;
            case ::SoC::tim::tim9: [[fallthrough]];
            case ::SoC::tim::tim12: return ::SoC::tim_channel::ch2;
            default: return ::SoC::tim_channel::ch1;
        }
    }

    constexpr inline ::std::strong_ordering operator<=> (::SoC::detail::tim_channel lhs, ::SoC::detail::tim_channel rhs) noexcept
    {
        return ::std::to_underlying(lhs) <=> ::std::to_underlying(rhs);
    }

    ::SoC::tim::tim(::SoC::tim::tim_enum tim,
                    ::std::uint16_t prescaler,
                    ::std::size_t auto_reload,
                    ::SoC::tim_mode mode,
                    ::SoC::tim_clock_div clock_div,
                    ::std::uint16_t rep_cnt) noexcept : tim_ptr{::std::bit_cast<::TIM_TypeDef*>(tim)}
    {
        const auto check_rep_u8{
            [rep_cnt] noexcept -> void
            { ::SoC::assert(rep_cnt <= ::std::numeric_limits<::std::uint8_t>::max(), "此计数器重复次数上限为255."sv); }};
        const auto check_upcnt_only{[mode] noexcept -> void
                                    { ::SoC::assert(mode == ::SoC::tim_mode::up, "此计数器仅支持向上计数"sv); }};

        switch(tim)
        {
            case ::SoC::tim::tim1:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM1};
                check_tim_u16(auto_reload);
                break;
            case ::SoC::tim::tim2:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM2};
                check_rep_u8();
                break;
            case ::SoC::tim::tim3:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM3};
                check_tim_u16(auto_reload);
                check_rep_u8();
                break;
            case ::SoC::tim::tim4:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM4};
                check_tim_u16(auto_reload);
                check_rep_u8();
                break;
            case ::SoC::tim::tim5:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM5};
                check_rep_u8();
                break;
            case ::SoC::tim::tim6:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM6};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
            case ::SoC::tim::tim7:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM7};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
            case ::SoC::tim::tim8:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM8};
                check_tim_u16(auto_reload);
                break;
            case ::SoC::tim::tim9:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM9);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM9};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
            case ::SoC::tim::tim10:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM10);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM10};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
            case ::SoC::tim::tim11:
                ::LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM11);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB2_GRP1_DisableClock, LL_APB2_GRP1_PERIPH_TIM11};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
            case ::SoC::tim::tim12:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM12);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM12};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
            case ::SoC::tim::tim13:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM13);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM13};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
            case ::SoC::tim::tim14:
                ::LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);
                callback = ::SoC::detail::dtor_close_clock_callback_t{::LL_APB1_GRP1_DisableClock, LL_APB1_GRP1_PERIPH_TIM14};
                check_tim_u16(auto_reload);
                check_rep_u8();
                check_upcnt_only();
                break;
        }
        ::LL_TIM_SetCounterMode(tim_ptr, ::std::to_underlying(mode));
        ::LL_TIM_SetClockDivision(tim_ptr, ::std::to_underlying(clock_div));
        ::LL_TIM_SetAutoReload(tim_ptr, auto_reload);
        ::LL_TIM_SetPrescaler(tim_ptr, prescaler);
        ::LL_TIM_SetRepetitionCounter(tim_ptr, rep_cnt);
        // 产生更新时间，立即重置计数器
        ::LL_TIM_GenerateEvent_UPDATE(tim_ptr);
    }

    ::SoC::tim::tim(::SoC::tim&& other) noexcept
    {
        ::std::memcpy(reinterpret_cast<void*>(this), &other, sizeof(*this));
        other.tim_ptr = nullptr;
    }

    ::SoC::tim& ::SoC::tim::operator= (::SoC::tim&& other) noexcept
    {
        ::std::swap(*this, other);
        return *this;
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

    ::SoC::tim_channel::tim_channel(::SoC::tim::tim_view tim,
                                    tim_channel_enum channel,
                                    ::SoC::tim_oc_mode mode,
                                    ::std::uint32_t compare_value,
                                    bool init_state,
                                    ::SoC::tim_oc_polarity polarity) noexcept : tim_ptr{tim.tim_ptr}, channel{channel}
    {
        ::LL_TIM_OC_SetMode(tim_ptr, ::std::to_underlying(channel), ::std::to_underlying(mode));

        ::SoC::assert(channel <= ::SoC::get_max_channel(tim_ptr), "此定时器不具有输入通道"sv);
        set_compare_value(compare_value);

        ::LL_TIM_OC_SetPolarity(tim_ptr, ::std::to_underlying(channel), ::std::to_underlying(polarity));
        if(init_state) [[likely]] { enable(); }
    }

    ::SoC::tim_channel::tim_channel(::SoC::tim_channel&& other) noexcept
    {
        ::std::memcpy(reinterpret_cast<void*>(this), &other, sizeof(*this));
        other.tim_ptr = nullptr;
    }

    ::SoC::tim_channel& ::SoC::tim_channel::operator= (::SoC::tim_channel&& other) noexcept
    {
        ::std::swap(*this, other);
        return *this;
    }

    ::SoC::tim_channel::~tim_channel() noexcept
    {
        if(tim_ptr) { disable(); }
    }

    void ::SoC::tim_channel::enable() const noexcept
    {
        ::LL_TIM_CC_EnableChannel(tim_ptr, ::std::to_underlying(channel));
        if(compl_channel != ::SoC::detail::tim_channel{})
        {
            ::LL_TIM_CC_EnableChannel(tim_ptr, ::std::to_underlying(compl_channel));
        }
    }

    void ::SoC::tim_channel::disable() const noexcept
    {
        ::LL_TIM_CC_DisableChannel(tim_ptr, ::std::to_underlying(channel));
        if(compl_channel != ::SoC::detail::tim_channel{})
        {
            ::LL_TIM_CC_DisableChannel(tim_ptr, ::std::to_underlying(compl_channel));
        }
    }

    void ::SoC::tim_channel::configure_comp_channel(::SoC::tim_oc_polarity polarity) noexcept
    {
        ::SoC::assert(channel != ::SoC::tim_channel::ch4, "定时器的通道4不具有互补通道"sv);
        compl_channel = static_cast<::SoC::detail::tim_channel>(::std::to_underlying(channel) << 2);
        ::LL_TIM_OC_SetPolarity(tim_ptr, ::std::to_underlying(compl_channel), ::std::to_underlying(polarity));
    }

    void ::SoC::tim_channel::set_compare_value(::std::uint32_t compare_value) const noexcept
    {
        if(auto tim_enum{::std::bit_cast<::SoC::detail::tim>(tim_ptr)};
           tim_enum != ::SoC::tim::tim2 && tim_enum != ::SoC::tim::tim5)
        {
            ::SoC::check_tim_u16(compare_value);
        }
        switch(channel)
        {
            case ::SoC::tim_channel::ch1: ::LL_TIM_OC_SetCompareCH1(tim_ptr, compare_value); break;
            case ::SoC::tim_channel::ch2: ::LL_TIM_OC_SetCompareCH2(tim_ptr, compare_value); break;
            case ::SoC::tim_channel::ch3: ::LL_TIM_OC_SetCompareCH3(tim_ptr, compare_value); break;
            case ::SoC::tim_channel::ch4: ::LL_TIM_OC_SetCompareCH4(tim_ptr, compare_value); break;
        }
    }
}  // namespace SoC
