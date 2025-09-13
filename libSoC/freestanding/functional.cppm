/**
 * @file functional.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的多态函数包装器
 */

export module SoC.freestanding:functional;
import :heap;
import :utils;

namespace SoC::detail
{
    /**
     * @brief 判断类型callable_t是否具有static operator() (args_t...)
     *
     * @tparam callable_t 可调用类型
     * @tparam args_t 参数类型列表
     */
    export template <typename callable_t, typename... args_t>
    concept static_call_operator = ::std::invocable<decltype(&callable_t::operator()), args_t...>;

    template <typename callable_t, typename return_t, typename... args_t>
    consteval inline bool function_wrapper_noexcept() noexcept
    {
        if constexpr(::std::is_pointer_v<callable_t>) { return ::std::is_nothrow_invocable_r_v<return_t, callable_t, args_t...>; }
        else if constexpr(::SoC::detail::static_call_operator<callable_t>)
        {
            return ::std::is_nothrow_invocable_r_v<return_t, decltype(&callable_t::operator()), args_t...>;
        }
        else
        {
            return ::std::is_nothrow_invocable_r_v<return_t, callable_t&, args_t...>;
        }
    }

    /**
     * @brief 函数包装体
     *
     * @tparam callable_t 可调用类型
     * @tparam return_t 返回类型
     * @tparam args_t 参数类型列表
     * @param ptr 类型擦除的可调用对象
     * @param args 参数列表
     * @return 返回值
     */
    template <typename callable_t, typename return_t, typename... args_t>
        requires (!::std::is_reference_v<callable_t>)
    inline return_t function_wrapper(void* ptr,
                                     args_t... args) noexcept(function_wrapper_noexcept<callable_t, return_t, args_t...>() ||
                                                              ::SoC::optional_noexcept)
    {
        if constexpr(::std::is_pointer_v<callable_t>)
        {
            return ::std::invoke_r<return_t>(static_cast<callable_t>(ptr), ::std::forward<args_t>(args)...);
        }
        else if constexpr(::SoC::detail::static_call_operator<callable_t>)
        {
            return ::std::invoke_r<return_t>(static_cast<decltype(&callable_t::operator())>(ptr),
                                             ::std::forward<args_t>(args)...);
        }
        else
        {
            return ::std::invoke_r<return_t>(*static_cast<callable_t*>(ptr), ::std::forward<args_t>(args)...);
        }
    }

    /**
     * @brief 用于释放动态分配的可调用对象的回调函数
     *
     * @tparam callable_t 可调用对象类型
     */
    template <typename callable_t>
        requires (!::std::is_reference_v<callable_t>)
    inline ::std::size_t function_destroy_callback(void* ptr) noexcept
    {
        static_cast<callable_t*>(ptr)->~callable_t();
        constexpr auto allocated_size{::std::max(sizeof(callable_t), alignof(callable_t))};
        return allocated_size;
    }

}  // namespace SoC::detail

export namespace SoC
{
    namespace test
    {
        /// @see ::SoC::basic_smart_function
        extern "C++" template <::SoC::is_allocator allocator_t, typename return_t, typename... args_t>
        struct basic_smart_function;
    }  // namespace test

    /**
     * @brief 类型擦除的函数对象，具有智能语义
     *
     * @note 在绑定到指针时使用值语义，无分配开销 @n
     *       在绑定到具有static operator()的类型时，使用值语义，无分配开销 @n
     *       在绑定到其他类型右值时，使用拥有语义，会发生动态分配和释放 @n
     *       在绑定到其他类型左值时，使用引用语义，需要保证左值的生命周期大于等于函数对象的生命周期
     * @tparam allocator_t 分配器类型
     * @tparam return_t 返回类型
     * @tparam args_t 参数类型列表
     */
    template <::SoC::is_allocator allocator_t, typename return_t, typename... args_t>
    struct basic_smart_function
    {
    private:
        template <::SoC::is_allocator, typename, typename...>
        friend struct ::SoC::test::basic_smart_function;

        [[no_unique_address]] allocator_t allocator;
        void* ptr{};
        using func_t = return_t (*)(void*, args_t...) noexcept(::SoC::optional_noexcept);
        func_t func{};
        using destroy_t = ::std::size_t (*)(void*) noexcept;
        destroy_t destroy_callback{};

        /**
         * @brief 析构函数对象并释放内存
         *
         */
        void destroy() noexcept(::SoC::optional_noexcept)
        {
            if(destroy_callback) { allocator.deallocate(ptr, destroy_callback(ptr)); }
        }

    public:
        /**
         * @brief 默认构造函数对象
         *
         * @param allocator 分配器
         */
        inline basic_smart_function(allocator_t allocator = allocator_t{}) noexcept : allocator{allocator} {}

        /**
         * @brief 将函数绑定到可调用对象
         *
         * @tparam callable_t 可调用类型
         * @param callable 可调用对象
         * @param allocator 分配器
         */
        template <::SoC::detail::invocable_r<return_t, args_t...> callable_t>
            requires (::std::is_move_constructible_v<::std::remove_reference_t<callable_t>>)
        inline basic_smart_function(callable_t&& callable, allocator_t allocator = allocator_t{}) noexcept :
            func{::SoC::detail::function_wrapper<::std::remove_reference_t<callable_t>, return_t, args_t...>},
            allocator{allocator}
        {
            using no_ref_callable_t = ::std::remove_cvref_t<callable_t>;
            if constexpr(::std::is_lvalue_reference_v<callable_t>)
            {
                // 对于左值采用引用语义
                ptr = const_cast<void*>(static_cast<const void*>(&callable));
            }
            else if constexpr(::std::is_pointer_v<no_ref_callable_t>)
            {
                // 对于指针采用值语义直接储存
                ptr = callable;
            }
            else
            {
                if constexpr(::SoC::detail::static_call_operator<no_ref_callable_t, args_t...>)
                {
                    // 对于具有static operator()的类型进行优化，直接存储指针
                    ptr = &no_ref_callable_t::operator();
                }
                else
                {
                    // 对于其他类型，采用拥有语义，进行动态分配
                    ptr = allocator.template allocate<no_ref_callable_t>();
                    ::new(ptr) no_ref_callable_t{::std::forward<callable_t>(callable)};
                    destroy_callback = ::SoC::detail::function_destroy_callback<no_ref_callable_t>;
                }
            }
        }

        /**
         * @brief 将函数绑定到可调用对象
         *
         * @tparam callable_t 可调用类型
         * @param callable 可调用对象
         */
        template <::SoC::detail::invocable_r<return_t, args_t...> callable_t>
            requires (::std::is_move_constructible_v<::std::remove_reference_t<callable_t>>)
        inline basic_smart_function& operator= (callable_t&& callable) noexcept
        {
            basic_smart_function temp{::std::forward<callable_t>(callable), allocator};
            ::std::swap(*this, temp);
            return *this;
        }

        /**
         * @brief 析构函数，根据情况释放内存
         *
         */
        inline ~basic_smart_function() noexcept { destroy(); }

        /**
         * @brief 移动构造函数
         *
         * @param other 其他对象
         */
        inline basic_smart_function(basic_smart_function&& other) noexcept :
            ptr{::std::exchange(other.ptr, nullptr)}, func{::std::exchange(other.func, nullptr)}, allocator{other.allocator},
            destroy_callback{::std::exchange(other.destroy_callback, nullptr)}
        {
        }

        /**
         * @brief 移动赋值函数
         *
         * @param other 其他对象
         * @return 当前对象引用
         */
        inline basic_smart_function& operator= (basic_smart_function&& other) noexcept
        {
            auto temp{::std::move(other)};
            ::std::swap(*this, temp);
            return *this;
        }

        /**
         * @brief 禁止拷贝构造
         *
         */
        inline basic_smart_function(const basic_smart_function&) noexcept =
            delete("为简单和高效起见，实现为仅支持移动的类型擦除包装器");
        inline basic_smart_function(basic_smart_function&,
                                    allocator_t = allocator_t{}) noexcept = delete("应当使用引用而不是构造新对象");
        /**
         * @brief 禁止拷贝赋值
         *
         */
        inline basic_smart_function&
            operator= (const basic_smart_function&) noexcept = delete("为简单和高效起见，实现为仅支持移动的类型擦除包装器");
        inline basic_smart_function& operator= (basic_smart_function&) noexcept = delete("应当使用引用而不是构造新对象");

        /**
         * @brief 调用对象
         *
         * @param args 参数列表
         * @return 返回值
         */
        inline return_t operator() (args_t... args) noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(func, "未绑定到可调用对象"sv);
            return func(ptr, ::std::forward<args_t>(args)...);
        }

        /**
         * @brief 判断函数是否绑定到可调用对象
         *
         * @return 是否绑定到可调用对象
         */
        inline operator bool() const noexcept { return func != nullptr; }

        /**
         * @brief 设置分配器
         *
         * @param allocator 分配器
         */
        inline void set_allocator(allocator_t allocator) noexcept { this->allocator = allocator; }

        /**
         * @brief 获取分配器
         *
         * @return 分配器
         */
        inline allocator_t get_allocator() const noexcept { return allocator; }

        /**
         * @brief 清空函数引用绑定的可调用对象
         *
         * @return 函数引用
         */
        inline basic_smart_function& operator= (::std::nullptr_t) noexcept
        {
            destroy();
            ptr = nullptr;
            func = nullptr;
            destroy_callback = nullptr;
            return *this;
        }
    };

    /**
     * @brief 类型擦除的函数对象，具有智能语义
     *
     * @tparam return_t 返回类型
     * @tparam args_t 参数类型列表
     */
    template <typename return_t, typename... args_t>
    using smart_function = ::SoC::basic_smart_function<::SoC::ram_heap_allocator_t, return_t, args_t...>;
}  // namespace SoC
