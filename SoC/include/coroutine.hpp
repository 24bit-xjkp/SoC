#pragma once
#include "ring_buffer.hpp"
#include "priority_queue.hpp"
#include "functional.hpp"

namespace SoC::coroutine
{
    /**
     * @brief 等待体基类
     *
     */
    struct awaiter_base
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
     * @brief 访问协程优先级
     *
     */
    struct get_priority
    {
    };

    /**
     * @brief 调度器基类
     *
     */
    struct scheduler_base
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

    /**
     * @brief 协程回调函数类型
     *
     */
    using coroutine_callback_t = ::SoC::smart_function<::std::coroutine_handle<>, ::std::errc>;

    /**
     * @brief 协程承诺类型
     *
     */
    struct promise_base
    {
    private:
        using awaiter_t = ::SoC::coroutine::awaiter_base;
        /// 指向可等待体的指针
        awaiter_t* awaiter{};

    public:
        using handle_t = ::std::coroutine_handle<promise_base>;

        /// 协程返回的错误码
        ::std::errc error_code{};
        /// 调度器
        ::SoC::coroutine::scheduler_base& scheduler;
        /// 协程执行完后的回调函数
        ::SoC::coroutine::coroutine_callback_t callback;
        /// 协程调度优先级，越小优先级越高
        ::std::size_t priority{};

        /**
         * @brief 初始化承诺
         *
         * @note 协程的首个参数必须是调度器的无cv限定的引用
         */
        inline promise_base(::std::convertible_to<decltype(scheduler)> auto&& scheduler, auto&&...) noexcept :
            scheduler{scheduler}
        {
        }

        inline promise_base(auto&&...) noexcept =
            delete("框架使用知调度器的承诺模型. 协程的首个参数必须可以转化为SoC::coroutine::scheduler_base&");

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
        template <::std::derived_from<awaiter_t> awaitable>
        inline auto&& get_awaitable() noexcept
        {
            return static_cast<awaitable&>(*this->awaiter);
        }

        /**
         * @brief 将可等待体指针设置为空
         *
         */
        inline void clear_awaitable() noexcept { awaiter = nullptr; }

        /**
         * @brief 通过自定义分配器分配协程帧
         *
         * @param size 协程帧大小
         * @return void* 协程帧首指针
         */
        inline void* operator new (::std::size_t size) noexcept { return ::SoC::ram_allocator.allocate(size); }

        /**
         * @brief 通过自定义分配器释放协程帧
         *
         * @param ptr 协程帧首指针
         * @param size 协程帧大小
         */
        inline void operator delete (void* ptr, ::std::size_t size) noexcept
        {
            return ::SoC::ram_allocator.deallocate(ptr, size);
        }

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
                constexpr inline bool await_ready() noexcept { return false; }

                constexpr inline ::std::coroutine_handle<> await_suspend(::std::coroutine_handle<> handle) noexcept
                {
                    ::std::coroutine_handle<> handle_to_resume{};
                    if(auto&& promise{::SoC::coroutine::get_promise<promise_base>(handle)}; promise.callback)
                    {
                        handle_to_resume = promise.callback(promise.error_code);
                    }
                    handle.destroy();
                    return handle_to_resume ? handle_to_resume : ::std::noop_coroutine();
                }

                /**
                 * @brief 该函数不应被执行，因为协程已销毁而不应恢复执行
                 *
                 */
                [[noreturn]] constexpr inline void await_resume() noexcept
                {
                    if(::SoC::use_full_assert)
                    {
                        using namespace ::std::string_view_literals;
                        ::SoC::assert(false, "已执行完的协程不应被恢复执行"sv);
                        ::std::unreachable();
                    }
                    else
                    {
                        ::SoC::fast_fail();
                    }
                }
            };

            return awaiter{};
        }

        /**
         * @brief 将返回的错误码放入协程帧
         *
         * @param error_code 要返回的错误码
         */
        inline void return_value(::std::errc error_code) noexcept { this->error_code = error_code; }

        /**
         * @brief 处理协程内未捕获的异常
         *
         * @note 由于不支持异常处理，因此会终止程序
         */
        [[noreturn]] inline void unhandled_exception() noexcept
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

        /**
         * @brief 实现通过co_await获取协程柄
         *
         * @return 永不挂起的可等待体，co_await结果为协程柄
         * @note 此函数仅在协程内有效
         */
        inline auto await_transform(::SoC::coroutine::get_handle) noexcept
        {
            struct awaiter
            {
                handle_t handle;

                constexpr inline bool await_ready() noexcept { return true; }

                constexpr inline void await_suspend(::std::coroutine_handle<>) noexcept {}

                [[nodiscard("获取的协程柄不应忽略")]] constexpr inline handle_t await_resume() noexcept { return handle; }
            };

            return awaiter{handle_t::from_promise(*this)};
        }

        /**
         * @brief 实现通过co_await访问协程的优先级
         *
         * @return 优先级字段引用
         * @note 此函数仅在协程内有效
         */
        inline auto await_transform(::SoC::coroutine::get_priority) noexcept
        {
            struct awaiter
            {
                ::std::size_t& priority;

                constexpr inline bool await_ready() noexcept { return true; }

                constexpr inline void await_suspend(::std::coroutine_handle<>) noexcept {}

                [[nodiscard("获取的优先级引用不应忽略")]] constexpr inline auto&& await_resume() noexcept { return priority; }
            };

            return awaiter{priority};
        }

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

    namespace detail
    {
        /**
         * @brief 子任务回调类型
         *
         */
        struct sub_task_callback
        {
            /// 子协程柄
            ::std::coroutine_handle<> sub_handle;
            /// 子协程返回值
            ::std::optional<::std::errc> error_code{};
            /// 父协程柄
            ::std::coroutine_handle<> parent_handle{};

            /**
             * @brief 子任务回调函数
             *
             * @param errc 子任务返回值
             */
            inline ::std::coroutine_handle<> operator() (::std::errc errc) noexcept
            {
                error_code = errc;
                // 准备唤醒父协程，父协程为空则会正常返回调用方
                return parent_handle;
            }

            /**
             * @brief 注册子协程的回调函数
             *
             * @param sub_promise 子协程承诺
             */
            inline void set_callback(::SoC::coroutine::promise_base& sub_promise) noexcept
            {
                if constexpr(::SoC::use_full_assert)
                {
                    using namespace ::std::string_view_literals;
                    ::SoC::assert(!sub_promise.callback, "子任务不应设置回调函数. 通过co_await表达式的结果获取子任务返回值"sv);
                }
                else
                {
                    if(sub_promise.callback) [[unlikely]] { ::SoC::fast_fail(); }
                }
                // 设置子协程回调函数
                sub_promise.callback = *this;
            }

            /**
             * @brief 获取子任务返回值
             *
             * @return 子任务返回值
             */
            inline ::std::errc await_resume() noexcept { return *error_code; }
        };
    }  // namespace detail

    /**
     * @brief 支持同步完成的任务
     *
     */
    struct task
    {
        /**
         * @brief 承诺类型
         *
         */
        struct promise_type;
        /// 协程柄类型
        using handle_t = ::std::coroutine_handle<promise_type>;

        struct promise_type : ::SoC::coroutine::promise_base
        {
            /**
             * @brief 获取返回类型
             *
             * @return 任务对象
             */
            inline task get_return_object() noexcept { return task{task::handle_t::from_promise(*this)}; }

            /**
             * @brief 分配失败时直接终止程序
             *
             */
            [[noreturn]] inline static task get_return_object_on_allocation_failure() noexcept { ::SoC::fast_fail(); }
        };

        /// 协程柄
        handle_t handle{};

        /**
         * @brief 获取承诺引用
         *
         * @return 承诺引用
         */
        [[using gnu: always_inline, hot, artificial]] inline auto&& get_promise() noexcept
        {
            return ::SoC::coroutine::get_promise<promise_type>(handle);
        }

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
         * @brief 绑定任务完成后的回调函数
         *
         * @param callback 回调函数
         */
        inline void bind_callback(::SoC::coroutine::coroutine_callback_t callback) noexcept
        {
            get_promise().callback = ::std::move(callback);
        }

        /**
         * @brief 等待子任务同步完成
         *
         * @param sub_task 子任务
         * @return 子任务返回值
         */
        inline friend auto operator co_await(task sub_task) noexcept
        {
            struct awaiter : ::SoC::coroutine::detail::sub_task_callback
            {
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
                    parent_handle = parent;
                    auto&& sub_promise{::SoC::coroutine::get_promise<::SoC::coroutine::promise_base>(sub_handle)};
                    // 注册子协程回调函数
                    set_callback(sub_promise);
                    // 继续执行子协程，它应当是同步完成的，因此可以唤醒
                    return sub_handle;
                }
            };

            return awaiter{sub_task.handle};
        }
    };

    /**
     * @brief 支持异步完成的任务包装体
     *
     */
    struct async_task : private ::SoC::coroutine::detail::sub_task_callback
    {
    private:
        using base_t = ::SoC::coroutine::detail::sub_task_callback;

    public:
        using base_t::operator();
        using base_t::await_resume;

        /**
         * @brief 构造异步完成的任务并将其放入就绪队列
         *
         * @param task 要异步完成的任务
         */
        inline async_task(::SoC::coroutine::task task) noexcept : base_t{task.handle}
        {
            auto&& sub_promise{task.get_promise()};
            set_callback(sub_promise);
            sub_promise.scheduler.ready_queue_push_back(sub_handle);
        }

        /**
         * @brief 获取子协程是否执行完毕
         *
         * @return 子协程是否执行完毕
         */
        inline bool await_ready() noexcept { return error_code.has_value(); }

        /**
         * @brief 注册父协程柄并挂起
         *
         * @param parent 父协程柄
         */
        inline void await_suspend(::std::coroutine_handle<> parent) noexcept
        {
            // 注册父协程柄，子协程执行完毕后回调函数会将父协程柄放入就绪队列
            parent_handle = parent;
            // 不能唤醒子协程，因为它是异步完成的，可能正在等待异步事件
        }
    };

    /**
     * @brief 判断task类型是否可以进行调度，要求满足：
     * - task.handle可转换为std::coroutine_handle<>
     * - task::promise_type派生自SoC::coroutine::promise_base
     *
     * @tparam task 要判断的任务类型
     * @tparam args 任务类型的参数类型列表
     */
    template <typename task, typename... args>
    concept is_task_schedulable = requires(task t) {
        { t.handle } -> ::std::convertible_to<::std::coroutine_handle<>>;
    } && ::std::derived_from<typename ::std::coroutine_traits<task, args...>::promise_type, ::SoC::coroutine::promise_base>;

    /**
     * @brief 协程调度器
     * 支持3级优先级
     * @tparam buffer_size 队列缓冲区大小
     */
    template <::std::size_t queue_size = 8>
    struct scheduler : ::SoC::coroutine::scheduler_base
    {
    private:
        using buffer_t = ::SoC::ring_buffer<::std::coroutine_handle<>, queue_size>;

        /**
         * @brief 等待队列元素类型
         *
         */
        struct wait_item
        {
            ::std::uint64_t target_tick;
            ::std::coroutine_handle<> handle;
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
        buffer_t ready_queue[3]{};
        /// 等待队列
        ::SoC::priority_queue<wait_item, queue_size, compare_t> wait_queue{};
    };
}  // namespace SoC::coroutine

namespace SoC
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

            constexpr inline bool await_ready() noexcept { return false; }

            constexpr inline void await_suspend(::std::coroutine_handle<> handle) noexcept
            {
                auto&& promise{::SoC::coroutine::get_promise<::SoC::coroutine::promise_base>(handle)};
                promise.scheduler.wait_queue_push_back(handle, target_tick);
            }

            constexpr inline void await_resume() noexcept {}
        };

        return awaiter{ticks};
    }
}  // namespace SoC
