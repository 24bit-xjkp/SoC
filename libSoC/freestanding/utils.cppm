/**
 * @file utils.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的实用工具实现
 */

export module SoC.freestanding:utils;
import SoC.std;

export namespace SoC
{
    /**
     * @brief 当前构建模式
     *
     */
    enum class build_mode : ::std::size_t
    {
        debug,
        release,
        releasedbg,
        minsizerel,

#ifdef SOC_BUILD_MODE_DEBUG
        current = debug
#elifdef SOC_BUILD_MODE_RELEASE
        current = release
#elifdef SOC_BUILD_MODE_RELEASEDBG
        current = releasedbg,
#elifdef SOC_BUILD_MODE_MINSIZEREL
        current = minsizerel
#else
    #error Unknown build mode
#endif
    };

#ifdef USE_FULL_ASSERT
    /// 是否启用全部的断言
    constexpr inline auto use_full_assert{true};
#else
    /// 是否启用全部的断言
    constexpr inline auto use_full_assert{false};
#endif
}  // namespace SoC

namespace SoC
{
    namespace detail
    {
        template <typename type>
        constexpr inline bool is_ratio_impl{false};
        template <::std::intmax_t num, ::std::intmax_t den>
        constexpr inline bool is_ratio_impl<::std::ratio<num, den>>{true};

        export template <typename type>
        concept is_ratio = ::SoC::detail::is_ratio_impl<type>;
    }  // namespace detail
    export template <::SoC::detail::is_ratio ratio_t>
    struct duration;

    namespace detail
    {
        template <typename type>
        constexpr inline bool is_duration_impl{false};
        template <::std::intmax_t num, ::std::intmax_t den>
        constexpr inline bool is_duration_impl<::SoC::duration<::std::ratio<num, den>>>{true};

        export template <typename type>
        concept is_duration = ::SoC::detail::is_duration_impl<type>;

        template <::SoC::detail::is_duration lhs_t, ::SoC::detail::is_duration rhs_t>
        using duration_downcast =
            ::SoC::duration<::std::conditional_t<::std::ratio_less_v<typename lhs_t::ratio, typename rhs_t::ratio>,
                                                 typename lhs_t::ratio,
                                                 typename rhs_t::ratio>>;
    }  // namespace detail

    export template <::SoC::detail::is_ratio ratio_t>
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

    export {
        /// 系统时钟周期，在此处修改时钟频率
        using cycles = ::SoC::duration<::std::ratio<1, 144>>;
        /// 微秒
        using milliseconds = ::SoC::duration<::std::ratio<1>>;
        /// 毫秒
        using microseconds = ::SoC::duration<::std::kilo>;
        /// 系统时刻周期，在此处修改系统时刻
        using systicks = ::SoC::microseconds;
        /// 秒
        using seconds = ::SoC::duration<::std::mega>;
    }
}  // namespace SoC

export namespace SoC::literal
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

export namespace SoC::detail
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

    /**
     * @brief 判断callable类型的对象能否通过类型为args...的参数列表调用并将返回值转化为result类型
     *
     * @tparam callable 可调用类型
     * @tparam result 返回值类型
     * @tparam args 参数列表类型
     */
    template <typename callable, typename result, typename... args>
    concept invocable_r =
        ::std::invocable<callable, args...> && ::std::convertible_to<::std::invoke_result_t<callable, args...>, result>;
}  // namespace SoC::detail

export namespace SoC
{
    /**
     * @brief 忙等待直到func求值为true
     *
     * @tparam func_t 可调用类型
     * @tparam args_t 参数类型列表
     * @param func 可调用对象
     * @param args 参数列表
     */
    template <typename func_t, typename... args_t>
        requires (::SoC::detail::invocable_r<func_t, bool, args_t...>)
    constexpr inline void wait_until(func_t&& func, args_t&&... args) noexcept
    {
#pragma GCC unroll 0
        while(!::std::invoke_r<bool>(::std::forward<func_t>(func), ::std::forward<args_t>(args)...));
    }
}  // namespace SoC

export namespace SoC
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

export namespace SoC
{
    namespace detail
    {
        /// 断言失败输出的默认消息
        constexpr ::std::string_view default_assert_message{"断言失败"};
    }  // namespace detail

    /**
     * @brief 断言失败时调用的函数
     *
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    extern "C++" [[noreturn]] [[using gnu: noinline, cold]] void assert_failed(::std::string_view message,
                                                                               ::std::source_location location) noexcept;

    /**
     * @brief 基于C++的断言函数
     *
     * @note 在未定义USE_FULL_ASSERT宏时为空
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    constexpr inline void assert(bool expression,
                                 ::std::string_view message = ::SoC::detail::default_assert_message,
                                 ::std::source_location location = ::std::source_location::current()) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
        }
    }

    /**
     * @brief 基于C++的断言函数，不受宏的影响
     *
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    constexpr inline void always_assert(bool expression,
                                        ::std::string_view message = ::SoC::detail::default_assert_message,
                                        ::std::source_location location = ::std::source_location::current()) noexcept
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }

    /**
     * @brief 快速终止程序
     *
     */
    [[noreturn, gnu::always_inline, gnu::artificial]] inline void fast_fail() noexcept { __builtin_trap(); }

    /**
     * @brief 基于C++的检查函数，启用断言时使用断言，反之使用快速终止
     *
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    constexpr inline void always_check(bool expression,
                                       ::std::string_view message = ::SoC::detail::default_assert_message,
                                       ::std::source_location location = ::std::source_location::current()) noexcept
    {
        if constexpr(::SoC::use_full_assert)
        {
            if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
        }
        else
        {
            if(!expression) [[unlikely]] { ::SoC::fast_fail(); }
        }
    }

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

export namespace SoC
{
    /**
     * @brief 位转换，要求满足：
     * - 目标类型和源类型大小相等，且
     * - 目标类型和源类型均为平凡复制类型
     * @tparam to_type 目标类型
     * @tparam from_type 源类型
     * @param value 源值
     * @return 目标类型值
     */
    template <typename to_type, typename from_type>
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline to_type bit_cast(const from_type& value) noexcept
        requires (::std::is_trivially_copyable_v<to_type> && ::std::is_trivially_copyable_v<from_type> &&
                  sizeof(to_type) == sizeof(from_type))
    {
        return __builtin_bit_cast(to_type, value);
    }

    /**
     * @brief 将字节数组转化为target_type类型对象数组，配合#embed使用，要求满足：
     * - target_type可平凡复制，且
     * - 字节数组大小可以被对象大小整除
     * @tparam target_type 目标类型
     * @tparam bytes 字节数
     * @param array 字节数组
     */
    template <typename target_type, ::std::size_t bytes>
        requires (::std::is_trivially_copyable_v<target_type> && bytes % sizeof(target_type) == 0)
    constexpr inline auto array_cast(const ::std::uint8_t (&array)[bytes]) noexcept
    {
        return ::SoC::bit_cast<::std::array<target_type, bytes / sizeof(target_type)>>(array);
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
        constexpr float scaler{[] static consteval noexcept
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

    /**
     * @brief 将枚举类型转换为其底层类型
     *
     * @tparam enum_type 枚举类型
     * @param value 枚举值
     * @return 枚举值的底层类型对应的值
     */
    template <typename enum_type>
        requires ::std::is_enum_v<enum_type>
    [[using gnu: always_inline, artificial]] [[nodiscard]] constexpr inline ::std::underlying_type_t<enum_type>
        to_underlying(enum_type value) noexcept
    {
        return static_cast<::std::underlying_type_t<enum_type>>(value);
    }
}  // namespace SoC

export namespace SoC::detail
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

    /**
     * @brief 联合体包装器，用于跳过自动构造析构
     *
     * @tparam type 要存储的类型
     */
    template <typename type>
    union union_wrapper
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;

        value_type value;

        constexpr inline union_wrapper() noexcept {}

        constexpr inline ~union_wrapper() noexcept {}
    };

    /**
     * @brief 析构守卫
     *
     * @tparam type 要存储的类型
     */
    template <typename type>
    struct destructure_guard
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        reference ref;

        ~destructure_guard() noexcept { ref.~type(); }
    };
}  // namespace SoC::detail

export namespace SoC
{
    /**
     * @brief 可空引用
     *
     * @tparam type 引用类型
     */
    template <typename type>
        requires (::std::is_lvalue_reference_v<type>)
    struct optional
    {
        using value_type = ::std::remove_reference_t<type>;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;

    private:
        pointer ptr;

    public:
        explicit constexpr inline optional() noexcept : ptr{nullptr} {}

        explicit constexpr inline optional(type& ref) noexcept : ptr{::std::addressof(ref)} {}

        constexpr inline optional& operator= (type& ref) noexcept
        {
            ptr = ::std::addressof(ref);
            return *this;
        }

        constexpr inline operator reference() noexcept { return *ptr; }

        constexpr inline operator const_reference() const noexcept { return *ptr; }

        constexpr inline auto&& operator* (this auto&& self) noexcept { return *self.ptr; }

        constexpr inline operator bool() const noexcept { return ptr != nullptr; }

        constexpr inline auto operator->(this auto&& self) noexcept { return self.ptr; }

        template <typename... args_t>
        constexpr inline ::std::invoke_result_t<value_type, args_t...>
            operator() (this auto&& self, args_t&&... args) noexcept(noexcept((*self.ptr)(::std::forward<args_t>(args)...)))
            requires requires() { (*self.ptr)(::std::forward<args_t>(args)...); }
        {
            return (*self.ptr)(::std::forward<args_t>(args)...);
        }

        /**
         * @brief 获取绑定对象的引用
         *
         * @return 对象引用
         */
        constexpr inline auto&& get(this auto&& self) noexcept { return *self.ptr; }
    };

    /**
     * @brief 联合体包装器，用于跳过自动构造析构
     *
     * @tparam type 要存储的类型
     */
    template <typename type>
    union union_wrapper
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;

        value_type value;

        constexpr inline union_wrapper() noexcept {}

        constexpr inline ~union_wrapper() noexcept {}
    };

    /**
     * @brief 析构守卫
     *
     * @tparam type 要存储的类型
     */
    template <typename type>
    struct destructure_guard
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        reference ref;

        ~destructure_guard() noexcept { ref.~type(); }
    };

    namespace detail
    {
        /**
         * @brief 容器是否已满的概念，要求满足：
         * - bool container::full() noexcept
         * @tparam container_t 容器类型
         */
        template <typename container_t>
        concept container_with_full = requires(container_t&& container) {
            { container.full() } noexcept -> ::std::same_as<bool>;
        };

        /**
         * @brief 容器是否为空的概念，要求满足：
         * - bool container::empty() noexcept
         * @tparam container_t 容器类型
         */
        template <typename container_t>
        concept container_with_empty = requires(container_t&& container) {
            { container.empty() } noexcept -> ::std::same_as<bool>;
        };
    }  // namespace detail
}  // namespace SoC

export namespace SoC
{
    /**
     * @brief 指定类型type是否可以平凡重定位，特化此模板以进行指定
     *
     * @tparam type 要指定的类型
     */
    template <typename type>
    constexpr inline bool is_trivially_replaceable_v{true};

    /**
     * @brief 判断类型type是否可以平凡重定位
     *
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_trivially_replaceable = ::SoC::is_trivially_replaceable_v<type>;
}  // namespace SoC
