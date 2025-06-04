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
        constexpr inline friend auto preprocess(duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            using downcast_t = ::SoC::detail::duration_downcast<duration, decltype(rhs)>;
            return ::std::pair{lhs.template duration_cast<downcast_t>(), rhs.template duration_cast<downcast_t>()};
        }

    public:
        constexpr inline friend auto operator+ (duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            auto&& [lhs_p, rhs_p]{preprocess(lhs, rhs)};
            return decltype(lhs_p){lhs_p.rep + rhs_p.rep};
        }

        constexpr inline friend auto operator- (duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            auto&& [lhs_p, rhs_p]{preprocess(lhs, rhs)};
            return decltype(lhs_p){lhs_p.rep - rhs_p.rep};
        }

        constexpr inline friend ::std::strong_ordering operator<=> (duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            auto&& [lhs_p, rhs_p]{preprocess(lhs, rhs)};
            return lhs_p.rep <=> rhs_p.rep;
        }

        constexpr inline friend bool operator== (duration lhs, ::SoC::detail::is_duration auto rhs) noexcept
        {
            auto&& [lhs_p, rhs_p]{preprocess(lhs, rhs)};
            return lhs_p.rep == rhs_p.rep;
        }
    };

    /// 系统时钟周期，在此处修改时钟频率
    using cycles = ::SoC::duration<::std::ratio<1, 144>>;
    /// 微秒
    using milliseconds = ::SoC::duration<::std::ratio<1>>;
    /// 毫秒
    using microseconds = ::SoC::duration<::std::kilo>;
    /// 系统时刻周期，在此处修改系统时刻
    using systicks = ::SoC::duration<::std::hecto>;
    /// 秒
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
     * @brief 等待直到发生中断
     *
     */
    [[using gnu: always_inline, artificial]] inline void wait_for_interpret() noexcept
    {
#ifdef __clang__
        ::__wfi();
#else
        __WFI();
#endif
    }

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
#pragma GCC unroll 0
        while(!::std::invoke_r<bool>(::std::forward<func_t>(func), ::std::forward<args_t>(args)...));
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

    /// 系统时刻数
    struct systick_t
    {
    private:
        ::std::atomic_uint32_t index{};
        ::std::uint64_t systick[2]{0, 1};
        friend void SysTick_Handler() noexcept;

        /**
         * @brief 递增系统时刻
         *
         * @return 递增后的系统时刻
         */
        ::std::uint64_t operator++ () noexcept;

    public:
        /**
         * @brief 读取系统时刻
         *
         * @return ::std::uint64_t 系统时刻
         */
        ::std::uint64_t load() const noexcept;

        /**
         * @brief 读取系统时刻
         *
         */
        operator ::std::uint64_t () const noexcept;
    } inline constinit systick{};
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
        constexpr ::std::string_view default_assert_message{"断言失败"};
    }  // namespace detail

    /**
     * @brief 基于C++的断言函数
     *
     * @note 在未定义USE_FULL_ASSERT宏时为空
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    void assert(bool expression,
                ::std::string_view message = ::SoC::detail::default_assert_message,
                ::std::source_location location = ::std::source_location::current()) noexcept;

    /**
     * @brief 基于C++的断言函数，不受宏的影响
     *
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    void always_assert(bool expression,
                       ::std::string_view message = ::SoC::detail::default_assert_message,
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
            else
            {
                return false;
            }
        }
    }
    /// 日志设备，用于输出断言信息等
    inline constinit log_device{};
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

    /**
     * @brief 将浮点数四舍五入，使用数值算法
     *
     * @tparam n 保留的小数位数
     * @param value 浮点数
     * @return 四舍五入结果
     */
    template <::std::size_t n>
    constexpr inline auto round(float value) noexcept
    {
        constexpr float scaler{[] consteval noexcept
                               {
                                   float result{1.f};
                                   float temp{10.f};
                                   auto i{n};
                                   while(i != 0)
                                   {
                                       result *= (i & 1) == 1 ? temp : 1.f;
                                       temp *= temp;
                                       i >>= 1;
                                   }
                                   return result;
                               }()};
        return ::std::round(value * scaler) / scaler;
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
    template <::std::invocable<bool> callback_t>
        requires requires(callback_t&& callback) {
            { callback(true) } noexcept -> ::std::same_as<void>;
        }
    struct irq_guard
    {
    private:
        callback_t callback{};

    public:
        /**
         * @brief 构造irq_guard对象并执行callback(false)
         *
         * @param callback 回调函数，满足void callback(bool enable) noexcept，其中enable表示是否启用中断
         */
        constexpr irq_guard(callback_t&& callback) noexcept : callback{::std::forward<callback_t>(callback)} { callback(false); }

        constexpr ~irq_guard() noexcept { callback(true); }
    };
}  // namespace SoC

namespace SoC::detail
{
    /**
     * @brief 判断type是否在list中
     *
     * @tparam type 要判断的类型
     * @tparam list 类型列表
     */
    template <typename type, typename... list>
    concept either = (::std::same_as<type, list> || ...);

    /**
     * @brief 判断type是否是io的目标类型，要求为字符或std::byte
     *
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_io_target_type = ::SoC::detail::either<type, char, ::std::byte>;

    /**
     * @brief 判断type是否是整数、浮点
     *
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_int_fp = ::std::integral<type> || ::std::floating_point<type>;
}  // namespace SoC::detail

namespace SoC::detail
{
    /**
     * @brief 外设对象析构时关闭时钟使用的回调对象类型
     *
     */
    struct dtor_close_clock_callback_t
    {
        using callback_t = void (*)(::std::uint32_t);
        /// 回调函数
        callback_t close_clock_callback;
        /// 回调函数的参数，是时钟枚举
        ::std::uint32_t clock_enum;

        constexpr inline void operator() () const noexcept { close_clock_callback(clock_enum); }
    };
}  // namespace SoC::detail
