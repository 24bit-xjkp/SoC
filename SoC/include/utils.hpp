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

        template <::SoC::detail::is_duration lhs_t, ::SoC::detail::is_duration rhs_t>
        using duration_downcast =
            ::SoC::duration<::std::conditional_t<::std::ratio_less_v<typename lhs_t::ratio, typename rhs_t::ratio>,
                                                 typename lhs_t::ratio,
                                                 typename rhs_t::ratio>>;
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
            using downcast_t = ::SoC::detail::duration_downcast<decltype(self), decltype(other)>;
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

        constexpr inline friend ::std::strong_ordering operator<=> (duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            using downcast_t = ::SoC::detail::duration_downcast<decltype(lhs), decltype(rhs)>;
            return lhs.duration_cast<downcast_t>().rep <=> rhs.template duration_cast<downcast_t>().rep;
        }
    };

    using cycles = ::SoC::duration<::std::ratio<1, 168>>;
    using milliseconds = ::SoC::duration<::std::ratio<1>>;
    using systicks = ::SoC::duration<::std::hecto>;
    using microseconds = ::SoC::duration<::std::kilo>;
    using seconds = ::SoC::duration<::std::mega>;
}  // namespace SoC

namespace SoC::literal
{
    consteval inline ::std::size_t operator""_K (unsigned long long i) noexcept { return i * 1'000; }

    consteval inline ::std::size_t operator""_K (long double i) noexcept { return i * 1'000; }

    consteval inline ::std::size_t operator""_M (unsigned long long i) noexcept { return i * 1'000'000; }

    consteval inline ::std::size_t operator""_M (long double i) noexcept { return i * 1'000'000; }

    consteval inline ::SoC::seconds operator""_s (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::microseconds operator""_s (long double i) noexcept { return {static_cast<::std::size_t>(i * 1'000)}; }

    consteval inline ::SoC::microseconds operator""_ms (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::milliseconds operator""_ms (long double i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1'000)};
    }

    consteval inline ::SoC::milliseconds operator""_us (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::cycles operator""_us (long double i) noexcept { return {static_cast<::std::size_t>(i * 168)}; }

    consteval inline ::SoC::cycles operator""_cycle (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::cycles operator""_Kcycle (unsigned long long i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000)};
    }

    consteval inline ::SoC::cycles operator""_Kcycle (long double i) noexcept { return {static_cast<::std::size_t>(i * 1'000)}; }

    consteval inline ::SoC::cycles operator""_Mcycle (unsigned long long i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000'000)};
    }

    consteval inline ::SoC::cycles operator""_Mcycle (long double i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000'000)};
    }
}  // namespace SoC::literal

namespace SoC::detail
{
    /**
     * @brief 等待指定tick
     *
     * @param ticks 要等待的tick
     */
    void wait_for(::SoC::systicks ticks) noexcept;
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
     * @tparam args_t 参数类型列表
     * @param func 可调用对象
     * @param args 参数列表
     */
    template <typename func_t, typename... args_t>
        requires (::std::invocable<func_t, args_t...> && ::std::convertible_to<::std::invoke_result_t<func_t, args_t...>, bool>)
    constexpr inline void wait_until(func_t&& func, args_t&&... args) noexcept
    {
#ifdef __clang__
    #pragma clang loop unroll_count(1)
#else
    #pragma GCC unroll 1
#endif
        while(!::std::invoke_r<bool>(::std::forward<func_t>(func), ::std::forward<args_t>(args)...))
        {
#ifdef __clang__
            ::__yield();
#else
            asm volatile("yield");
#endif
        }
    }

    /**
     * @brief 等待指定时间
     *
     * @param duration 要等待的时间
     */
    inline void wait_for(::SoC::detail::is_duration auto duration) noexcept
    {
        // 提供一个快速通道，降低延迟偏差
        if constexpr(::std::same_as<decltype(duration), ::SoC::cycles>)
        {
            using namespace ::SoC::literal;
            auto cycles{duration.rep};
            constexpr auto mini_wait_cycles{10zu};

            if(cycles < mini_wait_cycles) { return; }
            else if(cycles < 2_K) { return ::SoC::detail::wait_for(duration); }
        }

        auto ticks{duration.template duration_cast<::SoC::systicks>()};
        auto tmp{duration - ticks};
        auto cycles{tmp.template duration_cast<::SoC::cycles>()};

        ::SoC::detail::wait_for(ticks);
        ::SoC::detail::wait_for(cycles);
    }

    inline constinit ::std::size_t systick{};
}  // namespace SoC

namespace SoC
{
    /**
     * @brief 掩码，第shift位为1，其余为0
     *
     * @tparam shift 左移量
     */
    template <::std::size_t shift>
        requires (shift < 32)
    constexpr inline ::std::size_t mask_single_one{1 << shift};

    /**
     * @brief 掩码，低ones位为1，其余为0
     *
     * @tparam ones 掩码中1的数量
     */
    template <::std::size_t ones>
        requires (ones <= 32)
    constexpr inline ::std::size_t mask_all_one{::SoC::mask_single_one<ones> - 1};
    template <>
    constexpr inline ::std::size_t mask_all_one<32>{-1zu};

    /**
     * @brief 掩码，第shift位为0，其余为1
     *
     * @tparam shift 左移量
     */
    template <::std::size_t shift>
        requires (shift < 32)
    constexpr inline ::std::size_t mask_single_zero{~::SoC::mask_single_one<shift>};

    /**
     * @brief 掩码，低zeros位为1，其余为0
     *
     * @tparam zeros 掩码中1的数量
     */
    template <::std::size_t zeros>
        requires (zeros <= 32)
    constexpr inline ::std::size_t mask_all_zero{~::SoC::mask_all_one<zeros>};
}  // namespace SoC

namespace SoC
{
    namespace detail
    {
        using namespace ::std::string_view_literals;
        constexpr auto default_assert_message{"断言失败"sv};
    }  // namespace detail

    /**
     * @brief 基于C++的断言函数
     *
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    void assert(bool expression,
                const ::std::string_view message = ::SoC::detail::default_assert_message,
                ::std::source_location location = ::std::source_location::current()) noexcept;

    /**
     * @brief 日志设备类
     *
     */
    struct log_device_t
    {
    private:
        using write_callback_t = void (*)(void*, const void*, const void*) noexcept;
        write_callback_t write_callback{};
        void* device{};

    public:
        constexpr inline log_device_t() noexcept = default;

        /**
         * @brief 设置日志设备
         *
         * @param write_callback 写入回调函数
         * @param device 日志设备指针
         */
        constexpr inline void set(write_callback_t write_callback, void* device) noexcept
        {
            this->write_callback = write_callback;
            this->device = device;
        }

        /**
         * @brief 获取日志设备
         *
         * @return std::pair<write_callback_t, void*> 回调函数和设备指针
         */
        constexpr inline auto get() const noexcept { return ::std::pair{write_callback, device}; }

        /**
         * @brief 尝试写入数据
         *
         * @param buffer 缓冲区首指针
         * @param end 缓冲区尾哨位
         * @return 是否进行了写入，有已注册的设备则为true，反之为false
         */
        constexpr inline bool write(const void* buffer, const void* end) const noexcept
        {
            if(write_callback != nullptr && device != nullptr) [[likely]]
            {
                write_callback(device, buffer, end);
                return true;
            }
            else { return false; }
        }
    } inline constinit log_device{};
}  // namespace SoC

namespace SoC
{
    using embed_t = ::std::int8_t[];
    using const_embed_t = const ::std::int8_t[];

    template <typename target_type, ::std::size_t bytes>
        requires (::std::is_trivially_copyable_v<target_type> && bytes % sizeof(target_type) == 0)
    constexpr inline auto array_cast(const ::std::int8_t (&array)[bytes]) noexcept
    {
        return ::std::bit_cast<::std::array<target_type, bytes / sizeof(target_type)>>(array);
    }
}  // namespace SoC

namespace SoC
{
    /**
     * @brief 快速终止程序
     *
     */
    [[noreturn, gnu::always_inline, gnu::artificial]] inline void fast_fail() noexcept { __builtin_trap(); }
}  // namespace SoC

namespace SoC
{
    /**
     * @brief 将浮点数正则化，转化为科学记数法形式
     *
     * @param in 输入浮点数，要求是一个正数
     * @return std::pair<int, float>{指数, 小数}
     */
    ::std::pair<int, float> normalize(float in) noexcept;

    /**
     * @brief 将浮点数转化为字符串
     *
     * @param buffer 输出缓冲区
     * @param in 输入浮点数
     * @param precision 小数位数，最多8位
     * @return char* 缓冲区尾指针
     */
    char* ftoa(char* buffer, float in, ::std::size_t precision) noexcept;
}  // namespace SoC
