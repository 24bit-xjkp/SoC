#pragma once
#include "utils.hpp"

namespace SoC
{
    /**
     * @brief 环形缓冲区
     *
     * @tparam type 元素类型
     * @tparam buffer_size 缓冲区容量
     */
    template <typename type, ::std::size_t buffer_size>
        requires (::std::has_single_bit(buffer_size))
    struct ring_buffer
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        using size_t = ::std::size_t;

    private:
        /// 缓冲区元素数组
        ::SoC::union_wrapper<type> buffer[buffer_size];
        ::std::size_t head{};
        ::std::size_t tail{};
        /// 缓冲区容量掩码
        constexpr inline static ::std::size_t buffer_mask = buffer_size - 1;

        /**
         * @brief 断言缓冲区未满
         *
         * @param location 源代码位置信息
         */
        constexpr inline void assert_not_full(::std::source_location location = ::std::source_location::current()) const noexcept
        {
            if constexpr(::SoC::use_full_assert)
            {
                using namespace ::std::string_view_literals;
                ::SoC::assert(!full(), "环形缓冲区已满"sv, location);
            }
            else
            {
                if(full()) [[unlikely]] { ::SoC::fast_fail(); }
            }
        }

        /**
         * @brief 断言缓冲区非空
         *
         * @param location 源代码位置信息
         */
        constexpr inline void assert_not_empty(::std::source_location location = ::std::source_location::current()) const noexcept
        {
            if constexpr(::SoC::use_full_assert)
            {
                using namespace ::std::string_view_literals;
                ::SoC::assert(!empty(), "环形缓冲区已空"sv, location);
            }
            else
            {
                if(empty()) [[unlikely]] { ::SoC::fast_fail(); }
            }
        }

    public:
        /**
         * @brief 构造一个环形缓冲区
         *
         */
        constexpr inline ring_buffer() noexcept {}

        /**
         * @brief 析构一个环形缓冲区
         *
         */
        constexpr inline ~ring_buffer() noexcept
        {
            if constexpr(!::std::is_trivially_destructible_v<type>)
            {
                while(!empty())
                {
                    auto&& ref{buffer[head++ & buffer_mask].value};
                    ref.~type();
                }
            }
        }

        /**
         * @brief 复制构造函数，因非原子性而删除
         *
         */
        constexpr inline ring_buffer(const ring_buffer&) = delete;

        /**
         * @brief 移动构造函数，因非原子性而删除
         *
         */
        constexpr inline ring_buffer(ring_buffer&&) = delete;

        /**
         * @brief 复制赋值运算符，因非原子性而删除
         *
         */
        constexpr inline ring_buffer& operator= (const ring_buffer&) = delete;

        /**
         * @brief 移动赋值运算符，因非原子性而删除
         *
         */
        constexpr inline ring_buffer& operator= (ring_buffer&&) = delete;

        /**
         * @brief 检查缓冲区是否为空
         *
         * @return 缓冲区是否为空
         */
        constexpr inline bool empty() const noexcept { return head == tail; }

        /**
         * @brief 检查缓冲区是否已满
         *
         * @return 缓冲区是否已满
         */
        constexpr inline bool full() const noexcept { return tail - head == buffer_size; }

        /**
         * @brief 获取缓冲区已用大小
         *
         * @return 已用大小
         */
        constexpr inline ::std::size_t size() const noexcept { return tail - head; }

        /**
         * @brief 获取缓冲区容量
         *
         * @return 缓冲区容量
         */
        constexpr inline ::std::size_t capacity() const noexcept { return buffer_size; }

        /**
         * @brief 向缓冲区添加元素
         *
         * @param args 构造参数列表
         */
        constexpr inline void emplace_back(::std::constructible_from<type> auto&&... args) noexcept
        {
            assert_not_full();
            new(&buffer[tail++ & buffer_mask].value) type{::std::forward<decltype((args))>(args)...};
        }

        /**
         * @brief 向缓冲区添加元素（原子操作）
         *
         * @param value 要添加的元素
         */
        constexpr inline void atomic_emplace_back(::std::constructible_from<type> auto&&... args) noexcept
        {
            ::std::atomic_ref tail_ref{tail};
            auto ptr{&buffer[tail_ref.fetch_add(1, ::std::memory_order_relaxed) & buffer_mask].value};
            new(ptr) type{::std::forward<decltype((args))>(args)...};
        }

        /**
         * @brief 从缓冲区移除元素
         *
         * @return 移除的元素
         */
        constexpr inline value_type pop_front() noexcept
        {
            assert_not_empty();
            auto&& ref{buffer[head++ & buffer_mask].value};
            ::SoC::destructure_guard _{ref};
            return ::std::move(ref);
        }

        /**
         * @brief 从缓冲区移除元素（原子操作）
         *
         * @return 移除的元素
         */
        constexpr inline value_type atomic_pop_front() noexcept
        {
            ::std::atomic_ref head_ref{head};
            auto&& ref{buffer[head_ref.fetch_add(1, ::std::memory_order_relaxed) & buffer_mask].value};
            ::SoC::destructure_guard _{ref};
            return ::std::move(ref);
        }
    };
}  // namespace SoC
