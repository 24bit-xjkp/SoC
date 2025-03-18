#pragma once
#include "pch.hpp"

namespace SoC
{
    namespace detail
    {
        template <typename type>
        constexpr inline bool is_ratio_impl{false};
        template <::std::intmax_t num, ::std::intmax_t den>
        constexpr inline bool is_ratio_impl<::std::ratio<num, den>>{true};

        template <typename type>
        concept is_ratio = ::SoC::detail::is_ratio_impl<type>;
    }  // namespace detail
    template <::SoC::detail::is_ratio ratio_t>
    struct duration;

    namespace detail
    {
        template <typename type>
        constexpr inline bool is_duration_impl{false};
        template <::std::intmax_t num, ::std::intmax_t den>
        constexpr inline bool is_duration_impl<::SoC::duration<::std::ratio<num, den>>>{true};

        template <typename type>
        concept is_duration = ::SoC::detail::is_duration_impl<type>;
    }  // namespace detail

    template <::SoC::detail::is_ratio ratio_t>
    struct duration
    {
        using ratio = ratio_t;
        ::std::size_t rep{};

        template <::SoC::detail::is_duration target_duration>
        constexpr inline auto duration_cast(this auto duration) noexcept
        {
            using duration_t = decltype(duration)::ratio;
            using target_t = target_duration::ratio;
            using to_target = ::std::ratio_divide<duration_t, target_t>;
            return target_duration{static_cast<::std::size_t>(duration.rep * to_target::num / to_target::den)};
        }

    private:
        template <typename operator_t>
        constexpr inline auto duration_operation(this auto self, ::SoC::detail::is_duration auto other) noexcept
        {
            using lhs_t = decltype(self)::ratio;
            using rhs_t = decltype(other)::ratio;
            using downcast_t = duration<::std::conditional_t<::std::ratio_less_v<lhs_t, rhs_t>, lhs_t, rhs_t>>;
            constexpr operator_t op{};
            return downcast_t{op(self.template duration_cast<downcast_t>().rep, other.template duration_cast<downcast_t>().rep)};
        }

    public:
        constexpr inline friend auto operator+ (duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            return lhs.duration_operation<::std::plus<>>(rhs);
        }

        constexpr inline friend auto operator- (duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            return lhs.duration_operation<::std::minus<>>(rhs);
        }
    };

    using cycles = ::SoC::duration<::std::ratio<1, 168>>;
    using milliseconds = ::SoC::duration<::std::ratio<1>>;
    using microseconds = ::SoC::duration<::std::kilo>;
    using seconds = ::SoC::duration<::std::mega>;
}  // namespace SoC

namespace SoC::literal
{
    constexpr inline ::std::size_t operator""_K (unsigned long long i) noexcept { return i * 1'000; }

    constexpr inline ::std::size_t operator""_K (long double i) noexcept { return i * 1'000; }

    constexpr inline ::std::size_t operator""_M (unsigned long long i) noexcept { return i * 1'000'000; }

    constexpr inline ::std::size_t operator""_M (long double i) noexcept { return i * 1'000'000; }

    constexpr inline ::SoC::seconds operator""_s (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    constexpr inline ::SoC::microseconds operator""_s (long double i) noexcept { return {static_cast<::std::size_t>(i * 1'000)}; }

    constexpr inline ::SoC::microseconds operator""_ms (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    constexpr inline ::SoC::milliseconds operator""_ms (long double i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1'000)};
    }

    constexpr inline ::SoC::milliseconds operator""_us (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    constexpr inline ::SoC::cycles operator""_us (long double i) noexcept { return {static_cast<::std::size_t>(i * 168)}; }

    constexpr inline ::SoC::cycles operator""_cycle (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    constexpr inline ::SoC::cycles operator""_Kcycle (unsigned long long i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000)};
    }

    constexpr inline ::SoC::cycles operator""_Kcycle (long double i) noexcept { return {static_cast<::std::size_t>(i * 1'000)}; }

    constexpr inline ::SoC::cycles operator""_Mcycle (unsigned long long i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000'000)};
    }

    constexpr inline ::SoC::cycles operator""_Mcycle (long double i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000'000)};
    }
}  // namespace SoC::literal

namespace SoC::detail
{
    /**
     * @brief 等待指定毫秒数
     *
     * @param microseconds 要等待的毫秒数
     */
    void wait_for(::SoC::microseconds microseconds) noexcept;
    /**
     * @brief 等待指定周期数
     *
     * @param cycles 要等待的周期数
     */
    void wait_for(::SoC::cycles cycles) noexcept;
}  // namespace SoC::detail

namespace SoC
{
    /**
     * @brief 阻塞直到func求值为true
     *
     * @tparam func_t 可调用类型
     * @param func 可调用对象
     */
    template <typename func_t>
        requires (::std::invocable<func_t> && ::std::convertible_to<::std::invoke_result_t<func_t>, bool>)
    constexpr inline void wait_until(func_t&& func) noexcept
    {
        while(!::std::invoke_r<bool>(::std::forward<func_t>(func))) { __yield(); }
    }

    /**
     * @brief 等待指定时间
     *
     * @param duration 要等待的时间
     */
    inline void wait_for(::SoC::detail::is_duration auto duration) noexcept
    {
        auto microseconds{duration.template duration_cast<::SoC::microseconds>()};
        auto tmp{duration - microseconds};
        auto cycles{tmp.template duration_cast<::SoC::cycles>()};

        ::SoC::detail::wait_for(microseconds);
        ::SoC::detail::wait_for(cycles);
    }
}  // namespace SoC

namespace SoC
{
    /**
     * @brief 基于C++的断言函数
     *
     * @param expression 断言表达式
     * @param location 源代码位置
     */
    void assert(bool expression, ::std::source_location location = ::std::source_location::current()) noexcept;
}
