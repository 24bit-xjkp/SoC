#pragma once
#include "heap.hpp"

namespace SoC
{
    /**
     * @brief 环形缓冲区
     *
     * @tparam type 元素类型
     * @tparam allocator_t 分配器类型
     */
    template <typename type, ::SoC::is_allocator allocator_t>
        requires (::std::is_nothrow_move_constructible_v<type> && ::std::is_nothrow_destructible_v<type>)
    struct basic_ring_buffer
    {
        using allocator = allocator_t;
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        using size_t = ::std::size_t;

    private:
        /// 分配器对象
        [[no_unique_address]] allocator_t alloc;
        /// 缓冲区首指针
        pointer buffer;
        /// 已用大小
        ::std::size_t used_size;
        /// 位掩码，为缓冲区容量-1
        ::std::size_t mask;
        /// 缓冲区首索引
        ::std::size_t head;
        /// 缓冲区尾索引
        ::std::size_t tail;

        /**
         * @brief 尾哨位类型
         *
         */
        struct sentinel_t
        {
        private:
            /// 缓冲区尾索引
            ::std::size_t tail;
            template <typename ptr_t>
            friend struct iterator_t;

        public:
            inline sentinel_t(::std::size_t tail) noexcept : tail{tail} {}

            inline ::std::size_t get_tail() const noexcept { return tail; }
        };

        /**
         * @brief 首迭代器类型
         *
         * @tparam ptr_t 指针类型
         */
        template <typename ptr_t>
        struct iterator_t
        {
        private:
            /// 缓冲区首指针
            ptr_t buffer;
            /// 缓冲区首索引
            ::std::size_t head;
            /// 位掩码，为缓冲区容量-1
            ::std::size_t mask;

        public:
            using value_type = ::std::remove_const_t<::std::remove_pointer_t<ptr_t>>;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using reference = value_type&;
            using const_reference = const value_type&;

            inline iterator_t(ptr_t buffer, ::std::size_t head, ::std::size_t mask) noexcept :
                buffer{buffer}, head{head}, mask{mask}
            {
            }

            /**
             * @brief 将迭代器前进1步
             *
             * @return 前进后的迭代器引用
             */
            inline iterator_t& operator++ () noexcept
            {
                ++head;
                return *this;
            }

            /**
             * @brief 访问迭代器处元素
             *
             * @return 元素引用
             */
            inline auto&& operator* () noexcept { return buffer[head & mask]; }

            /**
             * @brief 判断迭代器是否到达容器尾部
             *
             * @param sentinel 尾哨位
             * @return 是否到达容器尾部
             */
            inline bool operator== (sentinel_t sentinel) noexcept { return head == sentinel.get_tail(); }

            /**
             * @brief 访问迭代器处元素
             *
             * @return 指向元素的指针
             */
            inline auto operator->() noexcept
            {
                return buffer + (head & mask);
            }
        };

        /**
         * @brief 将不能平凡重定位的对象重定位到新缓冲区上
         *
         * @param new_buffer 新缓冲区首指针
         */
        inline void do_no_trivially_replace(pointer new_buffer) noexcept
        {
#pragma GCC unroll(4)
            for(auto i{0zu}; i != used_size; ++i)
            {
                auto&& ref{buffer[(head + i) & mask]};
                new(new_buffer + i) value_type{::std::move(ref)};
                if constexpr(!::std::is_trivially_destructible_v<value_type>) { ref.~value_type(); }
            }
        }

        /**
         * @brief 将容器扩容
         *
         */
        [[using gnu: cold, noinline]] inline void expand() noexcept
        {
            auto capacity{used_size};
            auto new_capacity{capacity * 2};
            auto new_buffer{alloc.template allocate<value_type>(new_capacity).ptr};
            if constexpr(::SoC::is_trivially_replaceable<value_type>)
            {
                ::std::memcpy(new_buffer, buffer + head, (capacity - head) * sizeof(value_type));
                ::std::memcpy(new_buffer + capacity - head, buffer, head * sizeof(value_type));
            }
            else
            {
                do_no_trivially_replace(new_buffer);
            }
            alloc.deallocate(buffer, capacity);
            head = 0;
            tail = capacity;
            mask = new_capacity - 1;
            buffer = new_buffer;
        }

        /**
         * @brief 检查容器是否为空
         *
         */
        void check_empty(::std::source_location location = ::std::source_location::current()) const noexcept
        {
            using namespace ::std::string_view_literals;
            if constexpr(::SoC::use_full_assert) { ::SoC::assert(!empty(), "不能访问空容器中的元素"sv, location); }
            else
            {
                if(empty()) [[unlikely]] { ::SoC::fast_fail(); }
            }
        }

    public:
        /**
         * @brief 初始化环形缓冲区
         *
         * @param initial_capacity 初始容量
         * @param alloc 分配器
         */
        inline basic_ring_buffer(::std::size_t initial_capacity = 8, allocator alloc = ::SoC::ram_allocator) noexcept :
            alloc{alloc}, used_size{0}, mask{initial_capacity - 1}, head{0}, tail{0}
        {
            using namespace ::std::string_view_literals;
            if constexpr(::SoC::use_full_assert)
            {
                ::SoC::assert(::std::has_single_bit(initial_capacity), "初始容量必须是2的整数次幂"sv);
            }
            else
            {
                if(!::std::has_single_bit(initial_capacity)) [[unlikely]] { ::SoC::fast_fail(); }
            }
            buffer = alloc.template allocate<value_type>(initial_capacity).ptr;
        }

        /**
         * @brief 析构环形缓冲区
         *
         */
        inline ~basic_ring_buffer() noexcept
        {
            if(buffer)
            {
                if constexpr(!::std::is_trivially_destructible_v<value_type>)
                {
                    for(auto&& i: *this) { i.~value_type(); }
                }
                alloc.deallocate(buffer, capacity());
            }
        }

        /**
         * @brief 复制构造环形缓冲区
         *
         * @param other 要复制的对象
         */
        inline basic_ring_buffer(const basic_ring_buffer& other) noexcept
            requires (::std::is_nothrow_copy_constructible_v<value_type>)
            : alloc{other.alloc}, used_size{other.used_size}, head{0}, tail{used_size}
        {
            auto capacity{::std::max(::std::bit_ceil(used_size), 8zu)};
            buffer = alloc.template allocate<value_type>(capacity);
            if constexpr(::SoC::is_trivially_replaceable<value_type>)
            {
                if(!empty()) [[likely]]
                {
                    if(head < tail) { ::std::memcpy(buffer, buffer + head, used_size * sizeof(value_type)); }
                    else
                    {
                        auto back_cnt{other.capacity() - head};
                        ::std::memcpy(buffer, buffer + head, back_cnt * sizeof(value_type));
                        ::std::memcpy(buffer + back_cnt, buffer, tail * sizeof(value_type));
                    }
                }
            }
            else
            {
                do_no_trivially_replace(buffer);
            }
            mask = capacity - 1;
        }

        /**
         * @brief 移动构造环形缓冲区对象
         *
         * @param other 要移动的对象
         */
        inline basic_ring_buffer(basic_ring_buffer&& other) noexcept :
            alloc{other.alloc}, buffer{::std::exchange(other.buffer, nullptr)}, used_size{::std::exchange(other.used_size, 0)},
            mask{other.mask}, head{::std::exchange(other.head, 0)}, tail{::std::exchange(other.tail, 0)}
        {
        }

        /**
         * @brief 复制赋值环形缓冲区
         *
         * @param other 要复制的对象
         */
        inline basic_ring_buffer& operator= (const basic_ring_buffer& other) noexcept
        {
            auto temp{other};
            ::std::swap(temp, *this);
            return *this;
        }

        /**
         * @brief 移动赋值环形缓冲区
         *
         * @param other 要移动的对象
         */
        inline basic_ring_buffer& operator= (basic_ring_buffer&& other) noexcept
        {
            auto temp{::std::move(other)};
            ::std::swap(temp, *this);
            return *this;
        }

        /**
         * @brief 获取元素个数
         *
         * @return 元素个数
         */
        inline ::std::size_t size() const noexcept { return used_size; }

        /**
         * @brief 获取容器是否为空
         *
         * @return 容器是否为空
         */
        inline bool empty() const noexcept { return size() == 0; }

        /**
         * @brief 获取容器容量
         *
         * @return 容器容量
         */
        inline ::std::size_t capacity() const noexcept { return mask + 1; }

        /**
         * @brief 在缓冲区末尾插入元素
         *
         * @param ref 要插入的元素
         */
        inline void push_back(const_reference ref) noexcept
        {
            if(size() == capacity()) [[unlikely]] { expand(); }
            new(buffer + tail) value_type{ref};
            tail = (tail + 1) & mask;
            ++used_size;
        }

        /**
         * @brief 在缓冲区末尾构造元素
         *
         * @tparam args_t 参数类型列表
         * @param args 参数列表
         */
        template <typename... args_t>
            requires (::std::is_constructible_v<value_type, args_t...>)
        inline void emplace_back(args_t&&... args) noexcept
        {
            if(size() == capacity()) [[unlikely]] { expand(); }
            new(buffer + tail) value_type{::std::forward<args_t>(args)...};
            tail = (tail + 1) & mask;
            ++used_size;
        }

        /**
         * @brief 从缓冲区头部弹出元素
         *
         * @return 头部元素
         */
        inline value_type pop_front() noexcept
        {
            check_empty();
            value_type result{::std::move(buffer[head])};
            buffer[head].~value_type();
            head = (head + 1) & mask;
            --used_size;
            return result;
        }

        /**
         * @brief 访问头部元素
         *
         * @return 头部元素
         */
        template <typename self_t>
        inline auto&& front(this self_t&& self) noexcept
        {
            self.check_empty();
            return ::std::forward_like<self_t>(self.buffer[self.head]);
        }

        /**
         * @brief 访问尾部元素
         *
         * @return 尾部元素
         */
        template <typename self_t>
        inline auto&& back(this self_t&& self) noexcept
        {
            self.check_empty();
            return ::std::forward_like<self_t>(self.buffer[(self.tail - 1) & self.mask]);
        }

        /**
         * @brief 将容器容量缩减到合适
         *
         */
        inline void shrink_to_fit() noexcept
        {
            auto temp{*this};
            *this = ::std::move(temp);
        }

        using iterator = iterator_t<pointer>;
        using const_iterator = iterator_t<const_pointer>;

        /**
         * @brief 获取首迭代器
         *
         * @return 首迭代器
         */
        template <typename self_t>
        inline auto begin(this self_t&& self) noexcept
        {
            using ptr_t = ::std::conditional_t<::std::is_const_v<self_t>, const_pointer, pointer>;
            return iterator_t<ptr_t>{self.buffer, self.head, self.mask};
        }

        /**
         * @brief 获取尾哨位
         *
         * @return 尾哨位
         */
        inline auto end() const noexcept { return sentinel_t{tail > head ? tail : tail + capacity()}; }
    };

    template <typename type>
    using ring_buffer = ::SoC::basic_ring_buffer<type, ::SoC::user_heap_allocator_t>;
}  // namespace SoC
