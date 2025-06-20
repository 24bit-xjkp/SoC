#pragma once
#include "heap.hpp"

namespace SoC
{
    namespace detail
    {
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
        inline return_t function_wrapper(void* ptr, args_t... args) noexcept
        {
            return ::std::invoke_r<return_t>(*reinterpret_cast<callable_t*>(ptr), ::std::forward<args_t>(args)...);
        }

        template <typename type>
            requires (!::std::is_reference_v<type>)
        inline ::std::size_t function_destroy_callback(void* ptr) noexcept
        {
            reinterpret_cast<type*>(ptr)->~type();
            return sizeof(type);
        }
    }  // namespace detail

    /**
     * @brief 类型擦除的函数对象
     *
     * @tparam allocator_t 分配器类型
     * @tparam return_t
     * @tparam args_t
     */
    template <::SoC::is_allocator allocator_t, typename return_t, typename... args_t>
    struct basic_function
    {
    private:
        [[no_unique_address]] allocator_t allocator;
        void* ptr{};
        using func_t = return_t (*)(void*, args_t...) noexcept;
        func_t func{};
        using destroy_t = ::std::size_t (*)(void*) noexcept;
        destroy_t destroy_callback{};
        template <typename, typename...>
        friend struct function_ref;

        /**
         * @brief 析构函数对象并释放内存
         *
         */
        void destroy() noexcept
        {
            if(ptr)
            {
                auto size{destroy_callback(ptr)};
                if(ptr != this) { allocator.deallocate(ptr, size); }
            }
        }

    public:
        /**
         * @brief 默认构造函数对象
         *
         * @param allocator 分配器
         */
        inline basic_function(allocator_t allocator = ::SoC::ram_allocator) noexcept : allocator{allocator} {}

        /**
         * @brief 将函数绑定到可调用对象
         *
         * @tparam callable_t 可调用类型
         * @param callable 可调用对象
         * @param allocator 分配器
         */
        template <::SoC::detail::invocable_r<return_t, args_t...> callable_t>
            requires (::std::is_rvalue_reference_v<callable_t &&> &&
                      ::std::is_move_constructible_v<::std::remove_reference_t<callable_t>>)
        inline basic_function(callable_t&& callable, allocator_t allocator = ::SoC::ram_allocator) :
            func{::SoC::detail::function_wrapper<::std::remove_reference_t<callable_t>, return_t, args_t...>},
            allocator{allocator},
            destroy_callback{::SoC::detail::function_destroy_callback<::std::remove_reference_t<callable_t>>}
        {
            if constexpr(!::std::is_empty_v<callable_t>)
            {
                ptr = allocator.allocate(sizeof(callable_t));
                new(ptr)::std::remove_cvref_t<callable_t>{::std::move(callable)};
            }
            else
            {
                ptr = this;
            }
        }

        /**
         * @brief 将函数绑定到可调用对象
         *
         * @tparam callable_t 可调用类型
         * @param callable 可调用对象
         */
        template <::SoC::detail::invocable_r<return_t, args_t...> callable_t>
            requires (::std::is_rvalue_reference_v<callable_t &&> &&
                      ::std::is_move_constructible_v<::std::remove_reference_t<callable_t>>)
        inline basic_function& operator= (callable_t&& callable) noexcept
        {
            basic_function temp{::std::forward<callable_t>(callable), allocator};
            ::std::swap(*this, temp);
            return *this;
        }

        inline ~basic_function() noexcept { destroy(); }

        inline basic_function(basic_function&& other) noexcept :
            ptr{::std::exchange(other.ptr, nullptr)}, func{::std::exchange(other.func, nullptr)}, allocator{other.allocator},
            destroy_callback{other.destroy_callback}
        {
        }

        inline basic_function& operator= (basic_function&& other) noexcept
        {
            auto temp{::std::move(other)};
            ::std::swap(*this, temp);
            return *this;
        }

        inline basic_function(const basic_function&) noexcept = delete;
        inline basic_function& operator= (const basic_function&) noexcept = delete;

        /**
         * @brief 调用对象
         *
         * @param args 参数列表
         * @return 返回值
         */
        inline return_t operator() (args_t... args) noexcept { return func(ptr, ::std::forward<args_t>(args)...); }

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
        inline basic_function& operator= (::std::nullptr_t) noexcept
        {
            destroy();
            ptr = nullptr;
            func = nullptr;
            destroy_callback = nullptr;
            return *this;
        }
    };

    template <typename return_t, typename... args_t>
    using function = ::SoC::basic_function<::SoC::user_heap_allocator_t, return_t, args_t...>;

    /**
     * @brief 类型擦除的函数引用
     *
     * @tparam return_t 返回类型
     * @tparam args_t 参数列表
     */
    template <typename return_t, typename... args_t>
    struct function_ref
    {
    private:
        void* ptr{};
        using func_t = return_t (*)(void*, args_t...) noexcept;
        func_t func{};

    public:
        /**
         * @brief 默认构造，不绑定到可调用对象
         *
         */
        inline function_ref() noexcept = default;

        /**
         * @brief 将函数引用绑定到可调用对象
         *
         * @tparam callable_t 可调用类型
         * @param callable 可调用对象
         */
        template <::SoC::detail::invocable_r<return_t, args_t...> callable_t>
            requires (::std::is_lvalue_reference_v<callable_t>)
        inline function_ref(callable_t&& callable) noexcept :
            ptr{const_cast<void*>(reinterpret_cast<const void*>(::std::addressof(callable)))},
            func{::SoC::detail::function_wrapper<::std::remove_reference_t<callable_t>, return_t, args_t...>}
        {
        }

        /**
         * @brief 将函数引用绑定到函数对象
         *
         * @tparam allocator 函数对象使用的分配器类型
         * @param fun 函数对象引用
         */
        template <typename allocator>
        inline function_ref(::SoC::basic_function<allocator, return_t, args_t...>& fun) noexcept : ptr{fun.ptr}, func{fun.func}
        {
        }

        // /**
        //  * @brief 将函数引用绑定到可调用对象
        //  *
        //  * @tparam callable_t 可调用类型
        //  * @param callable 可调用对象
        //  */
        // template <::SoC::detail::invocable_r<return_t, args_t...> callable_t>
        //     requires (::std::is_lvalue_reference_v<callable_t>)
        // inline function_ref& operator= (callable_t&& callable) noexcept
        // {
        //     ptr = const_cast<void*>(reinterpret_cast<const void*>(::std::addressof(callable)));
        //     func = ::SoC::detail::function_wrapper<::std::remove_reference_t<callable_t>, return_t, args_t...>;
        //     return *this;
        // }

        /**
         * @brief 清空函数引用绑定的可调用对象
         *
         * @return 函数引用
         */
        inline function_ref& operator= (::std::nullptr_t) noexcept
        {
            ptr = nullptr;
            func = nullptr;
            return *this;
        }

        /**
         * @brief 调用对象
         *
         * @param args 参数列表
         * @return 返回值
         */
        inline return_t operator() (args_t... args) noexcept { return func(ptr, ::std::forward<args_t>(args)...); }

        /**
         * @brief 判断函数引用是否绑定到可调用对象
         *
         * @return 是否绑定到可调用对象
         */
        inline operator bool() const noexcept { return func != nullptr; }
    };
}  // namespace SoC
