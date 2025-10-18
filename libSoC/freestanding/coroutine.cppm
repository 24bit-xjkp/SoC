/**
 * @file coroutine.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 协程框架实现
 *
 * 本模块提供基于C++20标准的协程支持，包括协程承诺类型、任务类型、调度器等
 */
export module SoC.freestanding:coroutine;
import :ring_buffer;
import :priority_queue;
import :heap;

export namespace SoC
{
    /**
     * @brief 等待体基类
     *
     * @note 可等待体的销毁不涉及类型擦除，无需虚析构函数
     */
    struct awaiter_base  // NOLINT(cppcoreguidelines-virtual-class-destructor)
    {
        /**
         * @brief 等待体回调，通过该函数实现外部向可等待体传递信息
         *
         * @param domain 事件完成方id，如中断函数地址
         * @param detail 外部需要补充的信息，如具体事件类型
         * @return 是否将协程柄从外部维护的等待队列中移除
         */
        virtual bool operator() (::std::uintptr_t domain, ::std::uintptr_t detail) noexcept = 0;
    };

    /**
     * @brief 获取协程柄
     *
     * @note 通过对此类型的对象使用co_await运算符来获取协程柄
     */
    struct get_handle
    {
    };

    /**
     * @brief 调度器基类
     *
     * @note 调度器的销毁不涉及类型擦除，无需虚析构函数
     */
    struct scheduler_base  // NOLINT(cppcoreguidelines-virtual-class-destructor)
    {
        /**
         * @brief 将handle插入到完成队列中
         *
         * @param handle 要插入的协程柄
         */
        virtual void ready_queue_push_back(::std::coroutine_handle<> handle) noexcept = 0;

        /**
         * @brief 将handle插入等待队列中，在等待至少ticks个系统时刻后唤醒
         *
         * @param handle 要插入的协程柄
         * @param ticks 等待的系统时刻数
         */
        virtual void wait_queue_push_back(::std::coroutine_handle<> handle, ::std::size_t ticks) noexcept = 0;
    };

    /**
     * @brief 从协程柄获取承诺引用
     *
     * @tparam promise 要获取的承诺类型
     * @param handle 协程柄
     * @return 承诺引用
     */
    template <typename promise>
    inline promise& get_promise(::std::coroutine_handle<> handle) noexcept
    {
        return ::std::coroutine_handle<promise>::from_address(handle.address()).promise();
    }

    namespace detail
    {
        /**
         * @brief 协程承诺类型基类，不包含分配器
         *
         */
        struct promise_base_no_allocator
        {
        protected:
            using awaiter_t = ::SoC::awaiter_base;

            /**
             * @brief 指向可等待体的指针
             *
             * 当协程处于挂起状态时，通过set_awaitable使此指针指向负责唤醒协程的可等待体对象。
             * 在协程恢复执行前，通过get_awaitable获取可等待体引用并将信息存入可等待体对象中。
             * 协程恢复执行后，通过clear_awaitable将此指针设置为空。
             */
            awaiter_t* awaiter{};

        public:
            /// 协程返回的错误码
            ::std::errc error_code{};  // NOLINT(bugprone-invalid-enum-default-initialization)
            /// 协程调度优先级，越小优先级越高
            ::std::uint32_t priority{};
            /// 调度器
            ::SoC::scheduler_base& scheduler;
            /// 当前协程退出后下一个要执行的协程柄
            ::std::coroutine_handle<> handle_to_resume{::std::noop_coroutine()};

            /**
             * @brief 初始化承诺
             *
             * @param scheduler 调度器引用
             * @param args 协程的其他参数
             * @note 协程的首个参数必须是调度器的无cv限定的引用
             */
            inline promise_base_no_allocator(::std::convertible_to<decltype(scheduler)> auto&& scheduler,
                                             auto&&... args [[maybe_unused]]) noexcept : scheduler{scheduler}
            {
            }

            inline promise_base_no_allocator(auto&&...) noexcept =
                delete("框架使用知调度器的承诺模型. 协程的首个参数必须可以转化为SoC::scheduler_base&");

            /**
             * @brief 设置可等待体
             *
             * @param awaitable 可等待体引用
             */
            inline void set_awaitable(awaiter_t& awaitable) noexcept { this->awaiter = &awaitable; }

            /**
             * @brief 获取可等待体引用
             *
             * @tparam awaitable 可等待体类型
             * @return 可等待体引用
             */
            template <::std::derived_from<awaiter_t> awaitable = awaiter_t>
            inline auto&& get_awaitable() noexcept
            {
                return static_cast<awaitable&>(*this->awaiter);
            }

            /**
             * @brief 将可等待体指针设置为空
             *
             */
            inline void clear_awaitable() noexcept { awaiter = nullptr; }

            // NOLINTBEGIN(readability-convert-member-functions-to-static, modernize-use-nodiscard)

            /**
             * @brief 在协程初始化后立即挂起
             *
             * @return std::suspend_always
             */
            inline ::std::suspend_always initial_suspend() noexcept { return {}; }

            /**
             * @brief 在协程执行完成后调用回调，然后销毁协程
             *
             */
            inline auto final_suspend() noexcept
            {
                struct awaiter
                {
                    constexpr inline bool await_ready() const noexcept { return false; }

                    constexpr inline ::std::coroutine_handle<> await_suspend(::std::coroutine_handle<> handle) const noexcept
                    {
                        auto&& promise{::SoC::get_promise<promise_base_no_allocator>(handle)};
                        return promise.handle_to_resume;
                    }

                    /**
                     * @brief 该函数不应被执行，因为协程已销毁而不应恢复执行
                     *
                     */
                    [[noreturn]] constexpr inline void await_resume() const noexcept { ::std::unreachable(); }
                };

                return awaiter{};
            }

            // NOLINTEND(readability-convert-member-functions-to-static, modernize-use-nodiscard)

            /**
             * @brief 将返回的错误码放入协程帧
             *
             * @param error_code 要返回的错误码
             */
            inline void return_value(::std::errc error_code) noexcept { this->error_code = error_code; }

#ifdef __cpp_exceptions
            /**
             * @brief 异常指针，用于存储协程内未捕获的异常
             *
             * 当协程内抛出未捕获异常时，异常会被存储到此指针中，
             * 在父程序调用await_resume()或调用task::get_errc()时重新抛出。
             */
            ::std::exception_ptr exception_ptr;

            /**
             * @brief 处理协程内未捕获的异常
             *
             * @note 由于不支持异常处理，因此会终止程序
             */
            inline void unhandled_exception() noexcept { exception_ptr = ::std::current_exception(); }
#else
            /**
             * @brief 处理协程内未捕获的异常
             *
             * @note 由于不支持异常处理，因此会终止程序
             */
            [[noreturn]] inline void unhandled_exception() noexcept  // NOLINT(readability-convert-member-functions-to-static)
            {
                if constexpr(::SoC::use_full_assert)
                {
                    using namespace ::std::string_view_literals;
                    ::SoC::assert(false, "在独立环境下不支持异常处理"sv);
                    ::std::unreachable();
                }
                else
                {
                    ::SoC::fast_fail();
                }
            }
#endif
            /**
             * @brief 直接转发其他co_await表达式
             *
             * @param other 要转发的表达式
             */
            [[using gnu: always_inline, hot, artificial]] inline auto&& await_transform(auto&& other) noexcept
            {
                return ::std::forward<decltype(other)>(other);
            }
        };
    }  // namespace detail

    namespace test
    {
        /// @see SoC::promise_base
        extern "C++" template <::SoC::is_allocator allocator_t>
        struct promise_base;
    }  // namespace test

    /**
     * @brief 协程承诺类型，支持自定义分配器
     *
     * @tparam allocator_t 分配器类型
     */
    template <::SoC::is_allocator allocator_t>
        requires (::std::is_empty_v<allocator_t>)
    struct promise_base : ::SoC::detail::promise_base_no_allocator
    {
    private:
        using base_t = ::SoC::detail::promise_base_no_allocator;
        using base_t::awaiter;
        friend struct ::SoC::test::promise_base<allocator_t>;

    public:
        using allocator = allocator_t;
        using base_t::base_t;

        /**
         * @brief 通过自定义分配器分配协程帧
         *
         * @param size 协程帧大小
         * @return void* 协程帧首指针
         */
        inline static void* operator new (::std::size_t size) noexcept { return allocator::allocate(size); }

#if defined(__cpp_sized_deallocation) && __cpp_sized_deallocation >= 201309L
        /**
         * @brief 通过自定义分配器释放协程帧
         *
         * @param ptr 协程帧首指针
         * @param size 协程帧大小
         */
        inline static void operator delete (void* ptr, ::std::size_t size) noexcept { return allocator::deallocate(ptr, size); }
#else
        /**
         * @brief 重载delete以使用分配器释放promise
         *
         * @param ptr 要释放的内存指针
         */
        constexpr inline static void operator delete (void* ptr) noexcept(::SoC::is_noexcept_allocator<allocator_type>)
        {
            // 由于不支持 sized deallocation，所以这里传递 0 作为大小
            allocator::deallocate(ptr, 0);
        }
#endif
    };

    namespace test
    {
        /// @see SoC::task
        extern "C++" template <::SoC::is_allocator allocator_t>
        struct task;
    }  // namespace test

    /**
     * @brief 支持同步完成的任务
     *
     * @tparam allocator_t 分配器类型
     */
    template <::SoC::is_allocator allocator_t>
    struct task_base
    {
        /// 承诺类型
        struct promise_type;
        /// 协程柄类型
        using handle_t = ::std::coroutine_handle<promise_type>;
        using allocator = allocator_t;

    private:
        /// 协程柄
        handle_t handle{};
        friend struct ::SoC::test::task<allocator_t>;

        /**
         * @brief 销毁协程柄
         *
         * @note 仅能在任务完成后调用，否则断言失败
         */
        void destroy() noexcept(::SoC::optional_noexcept)
        {
            if(handle)
            {
                if constexpr(::SoC::use_full_assert)
                {
                    using namespace ::std::string_view_literals;
                    ::SoC::assert(handle.done(), "任务尚未完成，不能绑定新的协程柄"sv);
                }
                handle.destroy();
            }
        }

    public:
        struct promise_type : ::SoC::promise_base<allocator_t>
        {
        private:
            using base_t = ::SoC::promise_base<allocator_t>;

        public:
            using base_t::base_t;

            /**
             * @brief 获取返回类型
             *
             * @return 任务对象
             */
            inline task_base get_return_object() noexcept { return task_base{task_base::handle_t::from_promise(*this)}; }

            /**
             * @brief 分配失败时直接终止程序
             *
             */
            [[noreturn]] inline static task_base get_return_object_on_allocation_failure() noexcept(::SoC::optional_noexcept)
            {
                ::SoC::fast_fail();
            }

            /**
             * @brief 实现通过co_await获取协程柄
             *
             * @param get_handle 占位参数，用于获取协程柄
             * @return 永不挂起的可等待体，co_await结果为协程柄
             * @note 此函数仅在协程内有效
             */
            inline auto await_transform(::SoC::get_handle get_handle [[maybe_unused]]) noexcept
            {
                struct awaiter
                {
                    handle_t handle;

                    constexpr inline bool await_ready() noexcept { return true; }

                    constexpr inline void await_suspend(::std::coroutine_handle<> handle [[maybe_unused]]) noexcept {}

                    [[nodiscard("获取的协程柄不应忽略")]] constexpr inline handle_t await_resume() const noexcept
                    {
                        return handle;
                    }
                };

                return awaiter{handle_t::from_promise(*this)};
            }
        };

        /**
         * @brief 构造空任务
         *
         */
        inline task_base() noexcept = default;

        /**
         * @brief 构造任务并绑定协程柄
         *
         * @param handle 协程柄
         */
        inline task_base(handle_t handle) noexcept : handle{handle} {}

        inline task_base(const task_base&) noexcept = delete;
        inline task_base& operator= (const task_base&) noexcept = delete;

        /**
         * @brief 移动构造任务
         *
         * @param other 要移动的任务
         */
        inline task_base(task_base&& other) noexcept : handle{::std::exchange(other.handle, nullptr)} {}

        /**
         * @brief 移动赋值任务
         *
         * @param other 要移动的任务
         * @return 移动后的任务引用
         */
        inline task_base& operator= (task_base&& other) noexcept(::SoC::optional_noexcept)  // NOLINT(*noexcept-move*)
        {
            destroy();
            handle = ::std::exchange(other.handle, nullptr);
            return *this;
        }

        /**
         * @brief 销毁协程柄
         *
         */
        inline ~task_base() noexcept { destroy(); }

        /**
         * @brief 获取承诺引用
         *
         * @return 承诺引用
         */
        [[using gnu: always_inline, hot, artificial]] inline auto&& get_promise() noexcept
        {
            return ::SoC::get_promise<promise_type>(handle);
        }

        /**
         * @brief 获取协程柄
         *
         * @return 协程柄
         */
        [[nodiscard]] inline handle_t get_handle() const noexcept { return handle; }

        /**
         * @brief 设置任务的优先级
         *
         * @param priority 优先级
         */
        inline void set_priority(::std::size_t priority) noexcept { get_promise().priority = priority; }

        /**
         * @brief 获取任务的优先级
         *
         * @return 任务的优先级
         */
        inline ::std::size_t get_priority() noexcept { return get_promise().priority; }

        /**
         * @brief 等待子任务同步完成
         *
         * @param sub_task 子任务
         * @return 子任务返回值
         */
        inline friend auto operator co_await(task_base sub_task) noexcept
        {
            struct awaiter
            {
                handle_t sub_handle;

                /**
                 * @brief 判断子任务是否完成
                 *
                 * @return 需要等待的任务应当是未完成的
                 */
                inline bool await_ready() noexcept { return false; }

                /**
                 * @brief 挂起父协程并将子协程放入调度队列
                 *
                 * @param parent 父协程柄
                 */
                inline ::std::coroutine_handle<> await_suspend(::std::coroutine_handle<> parent) noexcept
                {
                    auto&& sub_promise{sub_handle.promise()};
                    sub_promise.handle_to_resume = parent;
                    // 继续执行子协程，它应当是同步完成的，因此可以唤醒
                    return sub_handle;
                }

                /**
                 * @brief 获取子任务的返回值
                 *
                 * @return 子任务的返回值
                 * @throws 在支持异常时，重新抛出子任务的异常
                 */
                inline ::std::errc await_resume() noexcept(::SoC::optional_noexcept)
                {
                    auto&& sub_promise{sub_handle.promise()};
#ifdef __cpp_exceptions
                    if(sub_promise.exception_ptr) [[unlikely]] { ::std::rethrow_exception(sub_promise.exception_ptr); }
#endif
                    return sub_promise.error_code;
                }
            };

            return awaiter{sub_task.handle};
        }

        /**
         * @brief 判断任务是否完成
         *
         * @return 任务是否完成
         */
        [[nodiscard]] inline bool done() noexcept { return handle.done(); }

        /**
         * @brief 获取任务的错误码
         *
         * @return 任务的错误码
         * @throws 在支持异常时，重新抛出任务的异常
         */
        inline ::std::errc get_errc() noexcept(::SoC::optional_noexcept)
        {
            if constexpr(::SoC::use_full_assert)
            {
                using namespace ::std::string_view_literals;
                ::SoC::assert(done(), "必须在执行结束后才能调用该函数"sv);
            }
            auto&& promise{get_promise()};
#ifdef __cpp_exceptions
            if(promise.exception_ptr) [[unlikely]] { ::std::rethrow_exception(promise.exception_ptr); }
#endif
            return promise.error_code;
        }

        /**
         * @brief 分离任务，使任务在后台运行
         *
         * @return 任务的协程柄
         */
        [[nodiscard("任务分离后将无法获取其协程柄")]] handle_t detach() noexcept { return ::std::exchange(handle, nullptr); }
    };

    /**
     * @brief 支持异步完成的任务包装体
     *
     */
    struct async_task
    {
        typename ::std::coroutine_handle<> sub_handle;

        /**
         * @brief 构造异步完成的任务并将其放入就绪队列
         *
         * @tparam allocator_t 分配器类型
         * @param sub_task 要异步完成的任务
         */
        template <::SoC::is_allocator allocator_t>
        inline async_task(::SoC::task_base<allocator_t> sub_task) noexcept : sub_handle{sub_task.get_handle()}
        {
            sub_task.get_promise().scheduler.ready_queue_push_back(sub_handle);
        }

        /**
         * @brief 获取子协程是否执行完毕
         *
         * @return 子协程是否执行完毕
         */
        [[nodiscard]] inline bool await_ready() const noexcept { return sub_handle.done(); }

        /**
         * @brief 注册父协程柄并挂起
         *
         * @param parent 父协程柄
         */
        inline void await_suspend(::std::coroutine_handle<> parent) const noexcept
        {
            // 注册父协程柄，子协程执行完毕后回调函数会继续执行父协程
            auto&& sub_promise{::SoC::get_promise<::SoC::detail::promise_base_no_allocator>(sub_handle)};
            sub_promise.handle_to_resume = parent;
            // 不能唤醒子协程，因为它是异步完成的，可能正在等待异步事件
        }

        /**
         * @brief 获取子协程的返回值
         *
         * @return 子协程的返回值
         * @throws 在支持异常时，重新抛出子协程的异常
         */
        inline ::std::errc await_resume() const noexcept(::SoC::optional_noexcept)  // NOLINT(modernize-use-nodiscard)
        {
            auto&& sub_promise{::SoC::get_promise<::SoC::detail::promise_base_no_allocator>(sub_handle)};
#ifdef __cpp_exceptions
            if(sub_promise.exception_ptr) [[unlikely]] { ::std::rethrow_exception(sub_promise.exception_ptr); }
#endif
            return sub_promise.error_code;
        }
    };

    /**
     * @brief 判断task_t类型是否可以进行调度，要求满足：
     * - task_t::promise_type派生自SoC::promise_base，且
     * - std::coroutine_handle<type> task_t::detach() noexcept，且
     * - task_t::allocator满足SoC::is_allocator约束
     *
     * @tparam task 要判断的任务类型
     * @tparam args 任务类型的参数类型列表
     */
    template <typename task_t, typename... args>
    concept is_task_schedulable =
        requires(task_t task) {
            // 从具有raii的task中分离出协程柄以便放入调度器管理
            { task.detach() } noexcept -> ::std::convertible_to<::std::coroutine_handle<>>;
            requires ::SoC::is_allocator<typename task_t::allocator>;
        } && ::std::derived_from<typename ::std::coroutine_traits<task_t, args...>::promise_type,
                                 ::SoC::promise_base<typename task_t::allocator>>;

    /// @brief 默认任务类型，使用RAM堆分配器
    using task = ::SoC::task_base<::SoC::ram_heap_allocator_t>;
}  // namespace SoC

export namespace SoC
{
    /**
     * @brief 支持在协程中等待指定时间
     *
     * @param duration 要等待的时间
     * @return 等待体
     */
    inline auto operator co_await(::SoC::detail::is_duration auto duration) noexcept
    {
        auto ticks{duration.template duration_cast<::SoC::systicks>().rep};
        if constexpr(::SoC::use_full_assert)
        {
            using namespace ::std::string_view_literals;
            ::SoC::assert(ticks >= 1, "等待时间不能小于1系统时刻"sv);
        }
        else
        {
            ticks = ::std::max(ticks, 1zu);
        }

        struct awaiter
        {
            ::std::size_t target_tick;

            /**
             * @brief 判断是否可以立即完成等待
             *
             * @return false 由于设计为一定要进行等待，所以不能立即完成
             */
            constexpr inline bool await_ready() noexcept { return false; }

            /**
             * @brief 挂起等待体，将当前协程放入等待队列
             *
             * @param handle 当前协程柄
             */
            constexpr inline void await_suspend(::std::coroutine_handle<> handle) noexcept
            {
                auto&& promise{::SoC::get_promise<::SoC::detail::promise_base_no_allocator>(handle)};
                promise.scheduler.wait_queue_push_back(handle, target_tick);
            }

            /**
             * @brief 等待体完成时的回调函数，无操作
             *
             */
            constexpr inline void await_resume() noexcept {}
        };

        return awaiter{ticks};
    }
}  // namespace SoC

export namespace SoC
{
    /**
     * @brief 调度器定时器溢出域，表示协程因调度器定时器溢出而被恢复执行
     *
     * 当协程因定时器溢出事件被唤醒时，`domain`参数会被设置为此值，
     * 用于区分不同来源的唤醒事件。
     */
    constexpr inline ::std::uintptr_t scheduler_timer_overflow_domain{-1u};
    /**
     * @brief 调度器定时器溢出详情，固定为0
     *
     * 当协程因定时器溢出事件被唤醒时，`detail`参数会被设置为此值。
     * 由于定时器溢出事件类型单一，因此详情固定为0。
     */
    constexpr inline ::std::uintptr_t scheduler_timer_overflow_detail{0};

    /**
     * @brief 协程调度器
     * 支持3级优先级
     * @tparam buffer_size 队列缓冲区大小
     */
    template <::std::size_t queue_size = 8>
    struct scheduler : ::SoC::scheduler_base
    {
    private:
        using buffer_t = ::SoC::ring_buffer<::std::coroutine_handle<>, queue_size>;

        /**
         * @brief 等待队列元素类型
         *
         */
        struct wait_item
        {
            ::std::uint64_t target_tick{};
            ::std::coroutine_handle<> handle{};
        };

        /**
         * @brief 比较函数对象，用于等待队列
         *
         */
        struct compare_t
        {
            constexpr inline static bool operator() (const wait_item& lhs, const wait_item& rhs) noexcept
            {
                return lhs.target_tick > rhs.target_tick;
            }
        };

        /// 就绪队列
        ::std::array<buffer_t, 3> ready_queue{};
        /// 等待队列
        ::SoC::priority_queue<wait_item, queue_size, compare_t> wait_queue{};
    };
}  // namespace SoC
