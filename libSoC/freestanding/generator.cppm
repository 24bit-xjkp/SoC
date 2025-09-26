export module SoC.freestanding:generator;
import :heap;

namespace SoC
{
    namespace detail
    {
        /**
         * @brief 未捕获异常处理实现
         *
         * @tparam no_exception 是否禁用异常处理
         */
        template <bool no_exception>
        struct unhandled_exception_impl;

        template <>
        struct unhandled_exception_impl<false>
        {
            ::std::exception_ptr exception_ptr{};

            /**
             * @brief 处理未捕获异常
             *
             */
            constexpr inline void unhandled_exception() noexcept { exception_ptr = ::std::current_exception(); }
        };

        template <>
        struct unhandled_exception_impl<true>
        {
            /**
             * @brief 处理未捕获异常
             *
             */
            constexpr inline void unhandled_exception() noexcept { ::SoC::fast_fail(); }
        };
    }  // namespace detail

    export namespace test
    {
        /// @see SoC::generator
        extern "C++" template <typename value_type, ::SoC::is_allocator allocator_type = ::SoC::ram_heap_allocator_t>
        struct generator;
    }  // namespace test

    /**
     * @brief 协程生成器
     *
     * @note 为了可以优化掉堆分配，不支持生成器嵌套
     * @tparam value_type 生成器值类型
     * @tparam allocator_type 分配器类型，必须是无状态分配器
     */
    export template <typename value_type, ::SoC::is_allocator allocator_type = ::SoC::ram_heap_allocator_t>
        requires (::std::is_empty_v<allocator_type>)
    struct generator : ::std::ranges::view_interface<generator<value_type, allocator_type>>
    {
        struct promise_type;

    private:
        using handle_t = ::std::coroutine_handle<promise_type>;
        friend struct ::SoC::test::generator<value_type, allocator_type>;

        struct iterator
        {
            handle_t handle{};

            constexpr inline friend iterator& operator++ (iterator& it) noexcept(::SoC::optional_noexcept)
            {
                it.handle.resume();
                if constexpr(!::SoC::optional_noexcept)
                {
                    if(it.handle.done() && it.handle.promise().exception_ptr)
                    {
                        ::std::rethrow_exception(it.handle.promise().exception_ptr);
                    }
                }
                return it;
            }

            constexpr inline friend iterator operator++ (iterator& it, int) noexcept(::SoC::optional_noexcept)
            {
                iterator tmp = it;
                ++it;
                return tmp;
            }

            constexpr inline friend value_type& operator* (iterator& it) noexcept(::SoC::optional_noexcept)
            {
                return *it.handle.promise().ptr;
            }
        };

        struct sentinel
        {
            constexpr inline friend bool operator== (iterator& it, sentinel _ [[maybe_unused]]) noexcept
            {
                return it.handle.done();
            }
        };

    public:
        struct promise_type : ::SoC::detail::unhandled_exception_impl<::SoC::optional_noexcept>
        {
            value_type* ptr{};

            /**
             * @brief 获取返回对象
             *
             * @return 生成器对象
             */
            constexpr inline generator get_return_object() noexcept { return generator{handle_t::from_promise(*this)}; }

            /**
             * @brief 初始挂起
             *
             * @return std::suspend_always，支持惰性初始化
             */
            constexpr inline ::std::suspend_always initial_suspend() noexcept { return {}; }

            /**
             * @brief 最终挂起
             *
             * @return std::suspend_always，支持使用done()判断是否完成
             */
            constexpr inline ::std::suspend_always final_suspend() noexcept { return {}; }

            /**
             * @brief 生产值
             *
             * @param value 要生产的值
             */
            constexpr inline ::std::suspend_always yield_value(::std::convertible_to<value_type&> auto&& value) noexcept
            {
                ptr = ::std::addressof(value);
                return {};
            }

            /**
             * @brief 退出生成器
             *
             */
            constexpr inline void return_void() noexcept {}

            constexpr inline static void* operator new (::std::size_t size) noexcept { return allocator_type{}.allocate(size); }

            constexpr inline static void operator delete (void* ptr, ::std::size_t size) noexcept
            {
                allocator_type{}.deallocate(ptr, size);
            }
        };

        handle_t handle{};

        /**
         * @brief 构造函数
         *
         * @param handle 协程句柄
         */
        constexpr inline explicit generator(handle_t handle) noexcept : handle{handle} {}

        /**
         * @brief 析构函数
         *
         */
        constexpr inline ~generator() noexcept
        {
            if(handle) { handle.destroy(); }
        }

        constexpr inline generator(const generator&) = delete("生成器不可复制");
        constexpr inline generator& operator= (const generator&) = delete("生成器不可复制");

        /**
         * @brief 移动构造函数
         *
         * @param other 要移动的生成器
         */
        constexpr inline generator(generator&& other) noexcept : handle{::std::exchange(other.handle, nullptr)} {}

        /**
         * @brief 移动赋值运算符
         *
         * @param other 要移动的生成器
         * @return generator& 移动后的生成器
         */
        constexpr inline generator& operator= (generator&& other) noexcept
        {
            if(handle) { handle.destroy(); }
            handle = ::std::exchange(other.handle, nullptr);
            return *this;
        }

        /**
         * @brief 获取迭代器
         *
         * @return 迭代器对象
         */
        constexpr inline iterator begin() noexcept(::SoC::optional_noexcept)
        {
            // 启动生成器
            handle.resume();
            if constexpr(!::SoC::optional_noexcept)
            {
                if(handle.done() && this->handle.promise().exception_ptr)
                {
                    ::std::rethrow_exception(this->handle.promise().exception_ptr);
                }
            }
            return {handle};
        }

        /**
         * @brief 获取哨位
         *
         * @return 哨位对象
         */
        constexpr inline sentinel end() noexcept { return {}; }
    };
}  // namespace SoC
