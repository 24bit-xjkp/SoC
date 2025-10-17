/**
 * @file utils.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的实用工具实现
 */

module;
#if defined(__cpp_exceptions) && defined(SOC_IN_UNIT_TEST)
    #define SOC_FAST_FAIL_EXCEPTION
#endif
export module SoC.freestanding:utils;
import SoC.std;

export namespace SoC
{
    /**
     * @brief 当前构建模式
     *
     */
    enum class build_mode : ::std::uint8_t
    {
        /// 调试模式
        debug = 0,
        /// 发布模式
        release = 1,
        /// 发布调试模式
        releasedbg = 2,
        /// 最小尺寸发布模式
        minsizerel = 3,
        /// 覆盖率模式
        coverage = 4,

#ifdef SOC_BUILD_MODE_DEBUG
        /// 当前构建模式为调试模式
        current = debug
#elifdef SOC_BUILD_MODE_RELEASE
        /// 当前构建模式为发布模式
        current = release
#elifdef SOC_BUILD_MODE_RELEASEDBG
        /// 当前构建模式为发布调试模式
        current = releasedbg,
#elifdef SOC_BUILD_MODE_MINSIZEREL
        /// 当前构建模式为最小尺寸发布模式
        current = minsizerel
#elifdef SOC_BUILD_MODE_COVERAGE
        /// 当前构建模式为覆盖率模式
        current = coverage
#else
    #error Unknown build mode
#endif
    };

    /**
     * @brief 判断当前构建模式是否为指定模式
     *
     * @tparam args 指定构建模式参数包
     * @return true 当前构建模式为指定模式之一
     * @return false 当前构建模式不在指定模式之列
     */
    constexpr inline bool is_build_mode(::std::same_as<::SoC::build_mode> auto... args) noexcept
    {
        return ((::SoC::build_mode::current == args) || ...);
    }

#ifdef USE_FULL_ASSERT
    /// 是否启用全部的断言
    constexpr inline auto use_full_assert{true};
#else
    /// 是否启用全部的断言
    constexpr inline auto use_full_assert{false};
#endif

#ifdef __cpp_exceptions
    /// 条件性noexcept，在独立环境下为禁用异常；在宿主环境下支持异常
    constexpr inline auto optional_noexcept{false};
#else
    /// 条件性noexcept，在独立环境下为禁用异常；在宿主环境下支持异常
    constexpr inline auto optional_noexcept{true};
#endif

#ifdef SOC_IN_UNIT_TEST
    /// 是否在单元测试中
    constexpr inline auto in_unit_test{true};
#else
    /// 是否在单元测试中
    constexpr inline auto in_unit_test{false};
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
        using microseconds = ::SoC::duration<::std::ratio<1>>;
        /// 毫秒
        using milliseconds = ::SoC::duration<::std::kilo>;
        /// 系统时刻周期，在此处修改系统时刻
        using systicks = ::SoC::microseconds;
        /// 秒
        using seconds = ::SoC::duration<::std::mega>;
    }
}  // namespace SoC

namespace SoC::detail
{
    /**
     * @brief 对浮点数进行四舍五入取整，针对字面量
     *
     * @note 由于std::round的constexpr化尚未完成，因此使用手动实现的四舍五入取整
     * @param i 待取整的浮点数
     * @return consteval 取整后的整数
     */
    consteval inline ::std::size_t constexpr_literal_round(long double i) noexcept
    {
        auto integral{static_cast<::std::size_t>(i)};
        auto error{i - static_cast<long double>(integral)};
        return error >= 0.5l ? integral + 1 : integral;
    }
}  // namespace SoC::detail

export namespace SoC::literal
{
    consteval inline ::std::size_t operator""_K (unsigned long long i) noexcept { return i * 1'000; }

    consteval inline ::std::size_t operator""_K (long double i) noexcept
    {
        return ::SoC::detail::constexpr_literal_round(i * 1'000);
    }

    consteval inline ::std::size_t operator""_M (unsigned long long i) noexcept { return i * 1'000'000; }

    consteval inline ::std::size_t operator""_M (long double i) noexcept
    {
        return ::SoC::detail::constexpr_literal_round(i * 1'000'000);
    }

    consteval inline ::SoC::seconds operator""_s (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::milliseconds operator""_s (long double i) noexcept
    {
        return {::SoC::detail::constexpr_literal_round(i * 1'000)};
    }

    consteval inline ::SoC::milliseconds operator""_ms (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::microseconds operator""_ms (long double i) noexcept
    {
        return {::SoC::detail::constexpr_literal_round(i * 1'000)};
    }

    consteval inline ::SoC::microseconds operator""_us (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::cycles operator""_us (long double i) noexcept
    {
        constexpr auto cycles_per_us{::SoC::microseconds{1}.duration_cast<::SoC::cycles>().rep};
        return {::SoC::detail::constexpr_literal_round(i * cycles_per_us)};
    }

    consteval inline ::SoC::cycles operator""_cycle (unsigned long long i) noexcept { return {static_cast<::std::size_t>(i)}; }

    consteval inline ::SoC::cycles operator""_cycle (long double i) noexcept
    {
        return {::SoC::detail::constexpr_literal_round(i)};
    }

    consteval inline ::SoC::cycles operator""_Kcycle (unsigned long long i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000)};
    }

    consteval inline ::SoC::cycles operator""_Kcycle (long double i) noexcept
    {
        return {::SoC::detail::constexpr_literal_round(i * 1'000)};
    }

    consteval inline ::SoC::cycles operator""_Mcycle (unsigned long long i) noexcept
    {
        return {static_cast<::std::size_t>(i * 1000'000)};
    }

    consteval inline ::SoC::cycles operator""_Mcycle (long double i) noexcept
    {
        return {::SoC::detail::constexpr_literal_round(i * 1000'000)};
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
    constexpr inline void wait_until(func_t&& func, args_t&&... args) noexcept(::SoC::optional_noexcept)
    {
#pragma GCC unroll 0
        while(!::std::invoke_r<bool>(::std::forward<func_t>(func), ::std::forward<args_t>(args)...)) { ; }
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
    constexpr inline ::std::size_t mask_single_one{1zu << shift};

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

        /// 快速终止输出的默认消息
        constexpr ::std::string_view fast_fail_message{"程序被快速终止"};
    }  // namespace detail

    /**
     * @brief 断言失败时调用的函数
     *
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    extern "C++" [[noreturn]] [[using gnu: noinline, cold]] void
        assert_failed(::std::string_view message, ::std::source_location location) noexcept(::SoC::optional_noexcept);

    /**
     * @brief 基于C++的断言函数
     *
     * @note 在未定义USE_FULL_ASSERT宏时为空
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    constexpr inline void
        assert(bool expression,
               ::std::string_view message = ::SoC::detail::default_assert_message,
               ::std::source_location location = ::std::source_location::current()) noexcept(::SoC::optional_noexcept)
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
    constexpr inline void
        always_assert(bool expression,
                      ::std::string_view message = ::SoC::detail::default_assert_message,
                      ::std::source_location location = ::std::source_location::current()) noexcept(::SoC::optional_noexcept)
    {
        if(!expression) [[unlikely]] { ::SoC::assert_failed(message, location); }
    }

#ifdef SOC_FAST_FAIL_EXCEPTION
    /**
     * @brief 快速终止异常
     *
     * @note 仅开启异常支持的在单元测试中抛出该异常
     */
    struct fast_fail_exception : ::std::runtime_error
    {
        using ::std::runtime_error::runtime_error;
    };
#endif

    /**
     * @brief 快速终止程序
     *
     * @note 在未开启异常支持或未在单元测试中调用时，该函数会触发快速识别，导致程序立即终止
     * @throw SoC::fast_fail_exception 仅开启异常支持的在单元测试中通过异常实现以便测试框架捕获
     */
    [[noreturn, gnu::always_inline, gnu::artificial]] inline void fast_fail() noexcept(::SoC::optional_noexcept ||
                                                                                       !::SoC::in_unit_test)
    {
#ifdef SOC_FAST_FAIL_EXCEPTION
        throw ::SoC::fast_fail_exception{::SoC::detail::fast_fail_message.data()};
#else
        __builtin_trap();
#endif
    }

    /**
     * @brief 基于C++的检查函数，启用断言时使用断言，反之使用快速终止
     *
     * @param expression 断言表达式
     * @param message 要输出的消息
     * @param location 源代码位置
     */
    constexpr inline void
        always_check(bool expression,
                     ::std::string_view message = ::SoC::detail::default_assert_message,
                     ::std::source_location location = ::std::source_location::current()) noexcept(::SoC::optional_noexcept ||
                                                                                                   !::SoC::use_full_assert)
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

    namespace test
    {
        /// @see ::SoC::log_device_t
        extern "C++" struct log_device_t;
    }  // namespace test

    /**
     * @brief 日志设备类
     *
     */
    struct log_device_t
    {
    private:
        friend struct ::SoC::test::log_device_t;
        /// 写入回调函数类型
        using write_callback_t = void (*)(void*, const void*, const void*) noexcept;
        /// 写入回调函数
        write_callback_t write_callback{};
        /// 日志设备指针
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
        [[nodiscard]] constexpr inline auto get() const noexcept { return ::std::pair{write_callback, device}; }

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
    constexpr inline auto array_cast(const ::std::uint8_t (&array)[bytes]) noexcept  // NOLINT(*-avoid-c-arrays)
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
                                       result *= (i & 1zu) == 1zu ? temp : 1.f;
                                       temp *= temp;
                                       i >>= 1zu;
                                   }
                                   return result;
                               }()};
        // 在一定范围内补偿float的精度问题，避免round(1.0005f * 1e3) / 1e3 != 1.001f
        constexpr auto esp{1e-5f};
        constexpr auto r_scaler{1.f / scaler};
        return ::std::round(value * scaler + ::std::copysign(esp, value)) * r_scaler;
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
}  // namespace SoC::detail

export namespace SoC
{
    namespace test
    {
        /// @see SoC::optional
        extern "C++" template <typename type>
        struct optional;
    }  // namespace test

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
        friend struct ::SoC::test::optional<type>;

    public:
        explicit constexpr inline optional() noexcept : ptr{nullptr} {}

        explicit constexpr inline optional(type& ref) noexcept : ptr{::std::addressof(ref)} {}

        constexpr inline optional& operator= (type& ref) noexcept
        {
            ptr = ::std::addressof(ref);
            return *this;
        }

        /**
         * @brief 清除绑定对象
         *
         * @param ptr 空指针
         * @return optional& 引用自身
         */
        constexpr inline optional& operator= (::std::nullptr_t ptr [[maybe_unused]]) noexcept
        {
            this->ptr = nullptr;
            return *this;
        }

        /**
         * @brief 获取绑定对象的引用
         *
         * @return 绑定对象引用
         */
        constexpr inline operator reference() noexcept { return *ptr; }

        /**
         * @brief 获取绑定对象的常量引用
         *
         * @return 绑定对象常量引用
         */
        constexpr inline operator const_reference() const noexcept { return *ptr; }

        /**
         * @brief 获取绑定对象的引用，不进行检查
         *
         * @return 绑定对象引用
         */
        constexpr inline auto&& operator* (this auto&& self) noexcept { return *self.ptr; }

        /**
         * @brief 判断是否绑定了对象
         *
         * @return true 已绑定对象
         * @return false 未绑定对象
         */
        constexpr inline explicit operator bool() const noexcept { return ptr != nullptr; }

        /**
         * @brief 获取绑定对象的指针
         *
         * @return 绑定对象指针
         */
        constexpr inline auto* operator->(this auto&& self) noexcept { return self.ptr; }

        /**
         * @brief 对绑定对象的引用进行索引操作，不进行检查
         *
         * @return 索引操作结果
         */
        constexpr inline decltype(auto) operator[] (this auto&& self, ::std::size_t index) noexcept(noexcept((*self.ptr)[index]))
        {
            return (*self.ptr)[index];
        }

        /**
         * @brief 调用绑定对象的函数
         *
         * @tparam args_t 参数类型列表
         * @param args 参数列表
         * @throws 若调用的函数潜在抛出则向上传播，若为noexcept则也为noexcept
         * @return 函数返回值
         */
        template <typename... args_t>
        constexpr inline decltype(auto)
            operator() (this auto&& self, args_t&&... args) noexcept(noexcept((*self.ptr)(::std::forward<args_t>(args)...)))
            requires requires() { (*self.ptr)(::std::forward<args_t>(args)...); }
        {
            return (*self.ptr)(::std::forward<args_t>(args)...);
        }

        /**
         * @brief 判断是否绑定了对象
         *
         * @return true 已绑定对象
         * @return false 未绑定对象
         */
        constexpr inline bool has_value() const noexcept { return ptr != nullptr; }

        /**
         * @brief 获取绑定对象的引用
         *
         * @return 对象引用
         */
        constexpr inline auto&& value(this auto&& self) noexcept(::SoC::optional_noexcept)
        {
            if constexpr(::SoC::use_full_assert)
            {
                using namespace ::std::string_view_literals;
                ::SoC::assert(self.ptr != nullptr, "尝试获取空optional的值"sv);
            }
            return *self.ptr;
        }

        /**
         * @brief 获取绑定对象的引用，若为空则返回默认值
         *
         * @param default_value 默认值
         * @return 对象引用或默认值
         */
        constexpr inline auto&& value_or(this auto&& self, ::std::convertible_to<value_type> auto&& default_value) noexcept
        {
            return self.ptr != nullptr ? *self.ptr : ::std::forward<type>(default_value);
        }

        /**
         * @brief 清除绑定对象
         *
         */
        constexpr inline void reset() noexcept { ptr = nullptr; }
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

        constexpr inline union_wrapper() noexcept
        {
            // 在覆盖率测试模式下，防止构造函数被优化
            if constexpr(::SoC::is_build_mode(::SoC::build_mode::coverage))
            {
                ::std::atomic_signal_fence(::std::memory_order_relaxed);
            }
        }

        constexpr inline ~union_wrapper() noexcept
        {
            // 在覆盖率测试模式下，防止析构函数被优化
            if constexpr(::SoC::is_build_mode(::SoC::build_mode::coverage))
            {
                ::std::atomic_signal_fence(::std::memory_order_relaxed);
            }
        }

        constexpr inline union_wrapper(const union_wrapper&) noexcept = default;
        constexpr inline union_wrapper& operator= (const union_wrapper&) noexcept = default;
        constexpr inline union_wrapper(union_wrapper&&) noexcept = default;
        constexpr inline union_wrapper& operator= (union_wrapper&&) noexcept = default;
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

        constexpr inline explicit destructure_guard(reference ref) noexcept : ref{ref} {}

        constexpr inline ~destructure_guard() noexcept { ref.~type(); }

        constexpr inline destructure_guard(const destructure_guard&) noexcept = default;
        constexpr inline destructure_guard& operator= (const destructure_guard&) noexcept = default;
        constexpr inline destructure_guard(destructure_guard&&) noexcept = default;
        constexpr inline destructure_guard& operator= (destructure_guard&&) noexcept = default;
    };

    /**
     * @brief 可移动值类型，用于在移动语义下自动清空值
     *
     * @tparam type 值类型
     */
    template <typename type>
    struct moveable_value
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        /// 存储的值
        value_type value{};

        constexpr inline moveable_value() noexcept = default;

        /**
         * @brief 构造函数，将储存的值初始化为value
         *
         * @param value 要存储的值
         */
        constexpr inline explicit moveable_value(value_type value) noexcept : value{value} {}

        /**
         * @brief 赋值运算符，将储存的值赋值为value
         *
         * @param value 要赋值的值
         * @return 赋值后对象的引用
         */
        constexpr inline moveable_value& operator= (value_type value) noexcept
        {
            this->value = value;
            return *this;
        }

        /**
         * @brief 移动构造函数，自动将other.value清空
         *
         * @param other 要移动的值
         */
        constexpr inline moveable_value(moveable_value&& other) noexcept : value{::std::exchange(other.value, value_type{})} {}

        /**
         * @brief 移动赋值运算符，自动将other.value清空
         *
         * @param other 要移动的值
         * @return moveable_value& 移动赋值后的对象
         */
        constexpr inline moveable_value& operator= (moveable_value&& other) noexcept
        {
            value = ::std::exchange(other.value, value_type{});
            return *this;
        }

        /**
         * @brief 复制构造函数，不进行额外操作
         *
         * @param other 要复制的值
         */
        constexpr inline moveable_value(const moveable_value&) = default;
        /**
         * @brief 复制赋值运算符，不进行额外操作
         *
         * @param other 要复制的值
         * @return moveable_value& 复制赋值后的对象
         */
        constexpr inline moveable_value& operator= (const moveable_value&) = default;

        /**
         * @brief 析构函数，不进行额外操作
         */
        constexpr inline ~moveable_value() noexcept = default;

        /**
         * @brief 类型转换运算符，返回存储的值
         *
         * @return value_type 存储的值
         */
        constexpr inline operator value_type() const noexcept { return value; }

        /**
         * @brief 解引用运算符，要求满足：
         * - *value 成立
         *
         * @return 对value解引用的结果
         */
        constexpr inline auto&& operator* (this auto&& self) noexcept
            requires requires { *self.value; }
        {
            return ::std::forward_like<decltype(self)>(*self.value);
        }

        /**
         * @brief 成员访问运算符，要求满足：
         * - value->xxx 成立
         *
         * @return 对value的成员访问结果
         */
        constexpr inline auto operator->(this auto&& self) noexcept
            requires ::std::is_pointer_v<value_type> || requires { self.value.operator->(); }
        {
            return self.value;
        }

        /**
         * @brief 下标运算符，要求满足：
         * - value[index] 成立
         *
         * @return 对value[index]的引用
         */
        constexpr inline auto&& operator[] (this auto&& self, ::std::size_t index) noexcept
            requires requires { self.value[index]; }
        {
            return ::std::forward_like<decltype(self)>(self.value[index]);
        }
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
