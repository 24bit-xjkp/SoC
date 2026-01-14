/**
 * @file ring_buffer.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的环形缓冲区实现
 */

export module SoC.freestanding:ring_buffer;
import :utils;

#ifdef SOC_IN_UNIT_TEST
export
#endif
    namespace SoC::detail
{
    /**
     * @brief 检查迭代器索引是否超出缓冲区范围
     *
     * @param index_to_check 要检查的索引
     * @param head 缓冲区头索引
     * @param tail 缓冲区尾索引
     * @param location 调用位置
     */
    constexpr inline void check_ring_buffer_iterator_index(
        ::std::size_t index_to_check,
        ::std::size_t head,
        ::std::size_t tail,
        ::std::source_location location = ::std::source_location::current()) noexcept(::SoC::optional_noexcept)
    {
        if constexpr(::SoC::use_full_assert)
        {
            using namespace ::std::string_view_literals;
            constexpr ::std::string_view message{"迭代器超出环形缓冲区范围"};
            if(head <= tail) [[likely]] { ::SoC::assert(index_to_check >= head && index_to_check < tail, message, location); }
            else
            {
                ::SoC::assert(index_to_check >= head || index_to_check < tail, message, location);
            }
        }
    }

    /**
     * @brief 检查迭代器是否指向相同的缓冲区
     *
     * @param self 要检查的迭代器持有的缓冲区指针
     * @param other 要检查的迭代器持有的缓冲区指针
     * @param location 调用位置
     */
    constexpr inline void check_ring_buffer_iterator_same_buffer(
        const void* self,
        const void* other,
        ::std::source_location location = ::std::source_location::current()) noexcept(::SoC::optional_noexcept)
    {
        if constexpr(::SoC::use_full_assert)
        {
            using namespace ::std::string_view_literals;
            ::SoC::assert(self == other, "迭代器指向不同的环形缓冲区"sv, location);
        }
    }

    /**
     * @brief 环形缓冲区析构函数
     *
     * @tparam type 元素类型
     * @param head 头索引
     * @param tail 尾索引
     * @param buffer 缓冲区指针
     */
    template <typename type>
    constexpr inline void
        ring_buffer_destructor(::std::size_t head, ::std::size_t tail, ::std::span<::SoC::union_wrapper<type>> buffer) noexcept
    {
        const auto buffer_mask{buffer.size() - 1};
        for(::std::size_t i = head; i != tail; ++i) { buffer[i & buffer_mask].value.~type(); }
    }

    /**
     * @brief 环形缓冲区复制构造函数
     *
     * @tparam type 元素类型
     * @param head 头索引
     * @param tail 尾索引
     * @param src_buffer 源缓冲区
     * @param dst_buffer 目标缓冲区
     */
    template <typename type>
    constexpr inline void ring_buffer_copy_constructor(::std::size_t head,
                                                       ::std::size_t tail,
                                                       ::std::span<const ::SoC::union_wrapper<type>> src_buffer,
                                                       ::std::span<::SoC::union_wrapper<type>> dst_buffer) noexcept
    {
        const auto buffer_mask{src_buffer.size() - 1};
        const auto buffer_shift{::std::countr_zero(src_buffer.size())};
        auto actual_head{head & buffer_mask};
        auto actual_tail{tail & buffer_mask};
        auto src_begin{src_buffer.begin()};
        auto dst_begin{dst_buffer.begin()};
        auto dst_end{dst_buffer.end()};

        if((head >> buffer_shift) == ((tail - 1) >> buffer_shift))
        {
            // 没有发生回绕
            ::std::ranges::uninitialized_copy_n(src_begin, src_buffer.size(), dst_begin, dst_end);
        }
        else
        {
            // 发生回绕，先复制head到缓冲区末尾部分，再复制缓冲区开头到tail部分
            auto second_part_begin{
                ::std::ranges::uninitialized_copy(src_begin + actual_head, src_buffer.end(), dst_begin, dst_end).out};
            ::std::ranges::uninitialized_copy_n(src_begin, actual_tail, second_part_begin, dst_end);
        }
    }

    /**
     * @brief 环形缓冲区移动构造函数
     *
     * @tparam type 元素类型
     * @param head 头索引
     * @param tail 尾索引
     * @param src_buffer 源缓冲区
     * @param dst_buffer 目标缓冲区
     */
    template <typename type>
    constexpr inline void ring_buffer_move_constructor(::std::size_t head,
                                                       ::std::size_t tail,
                                                       ::std::span<::SoC::union_wrapper<type>> src_buffer,
                                                       ::std::span<::SoC::union_wrapper<type>> dst_buffer) noexcept
    {
        const auto buffer_mask{src_buffer.size() - 1};
        const auto buffer_shift{::std::countr_zero(src_buffer.size())};
        auto actual_head{head & buffer_mask};
        auto actual_tail{tail & buffer_mask};
        auto src_begin{src_buffer.begin()};
        auto dst_begin{dst_buffer.begin()};
        auto dst_end{dst_buffer.end()};

        if((head >> buffer_shift) == ((tail - 1) >> buffer_shift))
        {
            // 没有发生回绕
            ::std::ranges::uninitialized_move_n(src_begin, src_buffer.size(), dst_begin, dst_end);
        }
        else
        {
            // 发生回绕，先移动head到缓冲区末尾部分，再移动缓冲区开头到tail部分
            auto second_part_begin{
                ::std::ranges::uninitialized_move(src_begin + actual_head, src_buffer.end(), dst_begin, dst_end).out};
            ::std::ranges::uninitialized_move_n(src_begin, actual_tail, second_part_begin, dst_end);
        }
    }

    /**
     * @brief 环形缓冲区交换剩余元素
     *
     * @tparam type 元素类型
     * @param src_head 源缓冲区头索引
     * @param src_tail 源缓冲区尾索引
     * @param dst_head 目标缓冲区头索引
     * @param dst_tail 目标缓冲区尾索引
     * @param src_buffer 源缓冲区
     * @param dst_buffer 目标缓冲区
     */
    template <typename type>
    constexpr inline void ring_buffer_swap_rest(::std::size_t src_head,
                                                ::std::size_t& src_tail,
                                                ::std::size_t dst_head,
                                                ::std::size_t& dst_tail,
                                                ::std::span<::SoC::union_wrapper<type>> src_buffer,
                                                ::std::span<::SoC::union_wrapper<type>> dst_buffer) noexcept
    {
        const auto src_buffer_size{src_tail - src_head};
        const auto dst_buffer_size{dst_tail - dst_head};
        const auto src_buffer_mask{src_buffer.size() - 1};
        const auto dst_buffer_mask{dst_buffer.size() - 1};
        for(auto i{dst_buffer_size}; i != src_buffer_size; ++i)
        {
            auto src_index = (src_head + i) & src_buffer_mask;
            auto dst_index = (dst_head + i) & dst_buffer_mask;

            // 移动构造到dst缓冲区
            ::new(&dst_buffer[dst_index].value) type{::std::move(src_buffer[src_index].value)};
            // 析构src缓冲区中的原对象
            src_buffer[src_index].value.~type();
        }
        auto moved_size{src_buffer_size - dst_buffer_size};
        dst_tail += moved_size;
        src_tail -= moved_size;
    }
}  // namespace SoC::detail

export namespace SoC
{
    namespace test
    {
        /// @see SoC::ring_buffer
        extern "C++" template <typename type, ::std::size_t buffer_size>
        struct ring_buffer;

        /// @see SoC::ring_buffer::iter
        extern "C++" template <typename type, ::std::size_t buffer_size, bool is_const>
        struct ring_buffer_iterator_t;
    }  // namespace test

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
        using size_type = ::std::size_t;
        using difference_type = ::std::ptrdiff_t;

    private:
        ::std::array<::SoC::union_wrapper<type>, buffer_size> buffer{};
        ::std::size_t head{};
        ::std::size_t tail{};
        /// 缓冲区容量掩码
        constexpr inline static ::std::size_t buffer_mask = buffer_size - 1;
        /// 缓冲区容量位宽
        constexpr inline static ::std::size_t buffer_shift{::std::countr_zero(buffer_size)};
        friend struct ::SoC::test::ring_buffer<type, buffer_size>;

        /**
         * @brief 环形缓冲区迭代器
         *
         * @tparam is_const 是否为常量迭代器
         */
        template <bool is_const>
        struct iterator_t : ::std::random_access_iterator_tag
        {
        private:
            friend struct ::SoC::test::ring_buffer_iterator_t<type, buffer_size, is_const>;
            ::std::size_t index;
            using ring_buffer_pointer_t = ::std::conditional_t<is_const, const ring_buffer*, ring_buffer*>;
            ring_buffer_pointer_t ring_buffer_ptr;

        public:
            using value_type = type;
            using difference_type = ::std::ptrdiff_t;
            using pointer = ::std::conditional_t<is_const, const type*, type*>;
            using reference = ::std::conditional_t<is_const, const type&, type&>;

            /**
             * @brief 构造一个环形缓冲区迭代器
             *
             * @param index 索引
             * @param buffer 缓冲区引用
             */
            constexpr inline iterator_t(::std::size_t index = 0, ring_buffer_pointer_t buffer = nullptr) noexcept :
                index{index}, ring_buffer_ptr{buffer}
            {
            }

            /**
             * @brief 前缀递增运算符
             *
             * @return 递增后的迭代器
             */
            constexpr inline friend iterator_t& operator++ (iterator_t& self) noexcept
            {
                ++self.index;
                return self;
            }

            /**
             * @brief 后缀递增运算符
             *
             * @param placehold 占位参数，用于区分前缀和后缀递增运算符
             * @return 递增前的迭代器
             */
            constexpr inline friend iterator_t operator++ (iterator_t& self, int placehold [[maybe_unused]]) noexcept
            {
                auto old{self};
                ++self.index;
                return old;
            }

            /**
             * @brief 迭代器加法运算符
             *
             * @param self 迭代器
             * @param offset 偏移量
             * @return 加法后的迭代器
             */
            constexpr inline friend iterator_t operator+ (const iterator_t& self, ::std::ptrdiff_t offset) noexcept
            {
                return {self.index + offset, self.ring_buffer_ptr};
            }

            /**
             * @brief 迭代器加法运算符
             *
             * @param offset 偏移量
             * @param self 迭代器
             * @return 加法后的迭代器
             */
            constexpr inline friend iterator_t operator+ (::std::ptrdiff_t offset, const iterator_t& self) noexcept
            {
                return {self.index + offset, self.ring_buffer_ptr};
            }

            /**
             * @brief 迭代器加法赋值运算符
             *
             * @param self 迭代器
             * @param offset 偏移量
             * @return 加法后的迭代器
             */
            constexpr inline friend iterator_t& operator+= (iterator_t& self, ::std::ptrdiff_t offset) noexcept
            {
                self.index += offset;
                return self;
            }

            /**
             * @brief 前缀递减运算符
             *
             * @param self 迭代器
             * @return 递减后的迭代器
             */
            constexpr inline friend iterator_t& operator-- (iterator_t& self) noexcept
            {
                --self.index;
                return self;
            }

            /**
             * @brief 后缀递减运算符
             *
             * @param self 迭代器
             * @param placehold 占位参数，用于区分前缀和后缀递减运算符
             * @return 递减前的迭代器
             */
            constexpr inline friend iterator_t operator-- (iterator_t& self, int placehold [[maybe_unused]]) noexcept
            {
                auto old{self};
                --self.index;
                return old;
            }

            /**
             * @brief 迭代器减法运算符
             *
             * @param self 迭代器
             * @param offset 偏移量
             * @return 减法后的迭代器
             */
            constexpr inline friend iterator_t operator- (const iterator_t& self, ::std::ptrdiff_t offset) noexcept
            {
                return {self.index - offset, self.ring_buffer_ptr};
            }

            /**
             * @brief 迭代器减法运算符
             *
             * @param self 迭代器
             * @param other 另一个迭代器
             * @return 迭代器间距离
             */
            constexpr inline friend difference_type operator- (const iterator_t& self,
                                                               const iterator_t& other) noexcept(::SoC::optional_noexcept)
            {
                if constexpr(::SoC::use_full_assert)
                {
                    ::SoC::detail::check_ring_buffer_iterator_same_buffer(self.ring_buffer_ptr, other.ring_buffer_ptr);
                }
                return self.index - other.index;
            }

            /**
             * @brief 迭代器减法赋值运算符
             *
             * @param self 迭代器
             * @param offset 偏移量
             * @return 减法后的迭代器
             */
            constexpr inline friend iterator_t& operator-= (iterator_t& self, ::std::ptrdiff_t offset) noexcept
            {
                self.index -= offset;
                return self;
            }

            /**
             * @brief 迭代器相等运算符
             *
             * @param self 迭代器
             * @param other 另一个迭代器
             * @return 是否相等
             */
            constexpr inline friend bool operator== (const iterator_t& self, const iterator_t& other) noexcept
            {
                return self.index == other.index && self.ring_buffer_ptr == other.ring_buffer_ptr;
            }

            /**
             * @brief 迭代器比较运算符
             *
             * @param self 迭代器
             * @param other 另一个迭代器
             * @return 比较结果
             */
            constexpr inline friend auto operator<=> (const iterator_t& self,
                                                      const iterator_t& other) noexcept(::SoC::optional_noexcept)
            {
                if constexpr(::SoC::use_full_assert)
                {
                    ::SoC::detail::check_ring_buffer_iterator_same_buffer(self.ring_buffer_ptr, other.ring_buffer_ptr);
                }
                auto head{self.ring_buffer_ptr->head};
                return (self.index - head) <=> (other.index - head);
            }

            /**
             * @brief 迭代器解引用运算符
             *
             * @param self 迭代器
             * @return 值的引用
             */
            constexpr inline friend reference operator* (const iterator_t& self) noexcept(::SoC::optional_noexcept)
            {
                if constexpr(::SoC::use_full_assert)
                {
                    ::SoC::detail::check_ring_buffer_iterator_index(self.index,
                                                                    self.ring_buffer_ptr->head,
                                                                    self.ring_buffer_ptr->tail);
                }
                return self.ring_buffer_ptr->buffer[self.index & buffer_mask].value;
            }

            /**
             * @brief 迭代器下标运算符
             *
             * @param self 迭代器
             * @param offset 偏移量
             * @return 值的引用
             */
            constexpr inline reference operator[] (::std::ptrdiff_t offset) const noexcept(::SoC::optional_noexcept)
            {
                auto actual_index{index + offset};
                if constexpr(::SoC::use_full_assert)
                {
                    ::SoC::detail::check_ring_buffer_iterator_index(actual_index, ring_buffer_ptr->head, ring_buffer_ptr->tail);
                }
                return ring_buffer_ptr->buffer[actual_index & buffer_mask].value;
            }

            /**
             * @brief 迭代器成员访问运算符
             *
             * @param self 迭代器
             * @return 指向值的指针
             */
            constexpr inline pointer operator->() const noexcept(::SoC::optional_noexcept)
            {
                if constexpr(::SoC::use_full_assert)
                {
                    ::SoC::detail::check_ring_buffer_iterator_index(index, ring_buffer_ptr->head, ring_buffer_ptr->tail);
                }
                return &ring_buffer_ptr->buffer[index & buffer_mask].value;
            }
        };

    public:
        using iterator = iterator_t<false>;
        using const_iterator = iterator_t<true>;
        using reverse_iterator = ::std::reverse_iterator<iterator>;
        using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

        /**
         * @brief 构造一个环形缓冲区
         *
         */
        constexpr inline ring_buffer() noexcept = default;

        /**
         * @brief 析构一个环形缓冲区
         *
         */
        constexpr inline ~ring_buffer() noexcept
        {
            if constexpr(!::std::is_trivially_destructible_v<value_type>)
            {
                ::SoC::detail::ring_buffer_destructor<value_type>(head, tail, buffer);
            }
        }

        /**
         * @brief 复制构造函数
         *
         * @param other 要复制的环形缓冲区
         */
        constexpr inline ring_buffer(const ring_buffer& other) noexcept(::std::is_nothrow_copy_constructible_v<value_type>) :
            tail{other.size()}
        {
            ::SoC::detail::ring_buffer_copy_constructor<value_type>(other.head, other.tail, other.buffer, buffer);
        }

        /**
         * @brief 移动构造函数
         *
         * @param other 要移动的环形缓冲区
         */
        constexpr inline ring_buffer(ring_buffer&& other) noexcept(::std::is_nothrow_move_constructible_v<value_type>) :
            tail{other.size()}
        {
            ::SoC::detail::ring_buffer_move_constructor<value_type>(other.head, other.tail, other.buffer, buffer);
            other.head = 0;
            other.tail = 0;
        }

        /**
         * @brief 交换两个环形缓冲区的内容
         *
         * @param other 要交换内容的环形缓冲区
         */
        constexpr inline void swap(ring_buffer& other) noexcept(::std::is_nothrow_swappable_v<value_type>)
        {
            if(this == &other) { return; }

            // 交换公共部分元素
            for(auto&& [this_val, other_val]: ::std::views::zip(*this, other)) { ::std::ranges::swap(this_val, other_val); }

            // 处理剩余元素（如果一个缓冲区比另一个大）
            if(auto self_size{size()}, other_size{other.size()}; self_size > other_size)
            {
                ::SoC::detail::ring_buffer_swap_rest<value_type>(head, tail, other.head, other.tail, buffer, other.buffer);
            }
            else if(other_size > self_size)
            {
                ::SoC::detail::ring_buffer_swap_rest<value_type>(other.head, other.tail, head, tail, other.buffer, buffer);
            }
        }

        /**
         * @brief 复制赋值运算符
         *
         * @param other 要复制的环形缓冲区
         * @return 对当前对象的引用
         */
        constexpr inline ring_buffer&
            operator= (const ring_buffer& other) noexcept(::std::is_nothrow_move_constructible_v<value_type> &&
                                                          ::std::is_nothrow_swappable_v<value_type>)
        {
            ring_buffer temp{other};
            swap(temp);
            return *this;
        }

        /**
         * @brief 移动赋值运算符
         *
         * @param other 要移动的环形缓冲区
         * @return 对当前对象的引用
         */
        constexpr inline ring_buffer&
            operator= (ring_buffer&& other) noexcept(::std::is_nothrow_move_constructible_v<value_type> &&
                                                     ::std::is_nothrow_swappable_v<value_type>)
        {
            ring_buffer temp{::std::move(other)};
            swap(temp);
            return *this;
        }

        /**
         * @brief 获取指向缓冲区开头的迭代器
         *
         * @return 指向缓冲区开头的迭代器
         */
        [[nodiscard]] constexpr inline auto begin(this auto&& self) noexcept
        {
            return iterator_t<::std::is_const_v<::std::remove_reference_t<decltype(self)>>>{self.head, &self};
        }

        /**
         * @brief 获取指向缓冲区开头的常量迭代器
         *
         * @return 指向缓冲区开头的常量迭代器
         */
        [[nodiscard]] constexpr inline const_iterator cbegin() const noexcept { return begin(); }

        /**
         * @brief 获取指向缓冲区末尾的迭代器
         *
         * @return 指向缓冲区末尾的迭代器
         */
        [[nodiscard]] constexpr inline auto end(this auto&& self) noexcept
        {
            return iterator_t<::std::is_const_v<::std::remove_reference_t<decltype(self)>>>{self.tail, &self};
        }

        /**
         * @brief 获取指向缓冲区末尾的常量迭代器
         *
         * @return 指向缓冲区末尾的常量迭代器
         */
        [[nodiscard]] constexpr inline const_iterator cend() const noexcept { return end(); }

        /**
         * @brief 获取指向缓冲区开头的反向迭代器
         *
         * @return 指向缓冲区开头的反向迭代器
         */
        [[nodiscard]] constexpr inline auto rbegin(this auto&& self) noexcept { return ::std::reverse_iterator{self.end()}; }

        /**
         * @brief 获取指向缓冲区开头的常量反向迭代器
         *
         * @return 指向缓冲区开头的常量反向迭代器
         */
        [[nodiscard]] constexpr inline const_reverse_iterator crbegin() const noexcept { return rend(); }

        /**
         * @brief 获取指向缓冲区末尾的反向迭代器
         *
         * @return 指向缓冲区末尾的反向迭代器
         */
        [[nodiscard]] constexpr inline auto rend(this auto&& self) noexcept { return ::std::reverse_iterator{self.begin()}; }

        /**
         * @brief 获取指向缓冲区末尾的常量反向迭代器
         *
         * @return 指向缓冲区末尾的常量反向迭代器
         */
        [[nodiscard]] constexpr inline const_reverse_iterator crend() const noexcept { return rbegin(); }

        /**
         * @brief 检查缓冲区是否为空
         *
         * @return 缓冲区是否为空
         */
        [[nodiscard]] constexpr inline bool empty() const noexcept { return head == tail; }

        /**
         * @brief 检查缓冲区是否已满
         *
         * @return 缓冲区是否已满
         */
        [[nodiscard]] constexpr inline bool full() const noexcept { return tail - head == buffer_size; }

        /**
         * @brief 获取缓冲区已用大小
         *
         * @return 已用大小
         */
        [[nodiscard]] constexpr inline ::std::size_t size() const noexcept { return tail - head; }

        /**
         * @brief 获取缓冲区容量
         *
         * @return 缓冲区容量
         */
        [[nodiscard]] constexpr inline ::std::size_t capacity() const noexcept { return buffer_size; }

        /**
         * @brief 访问缓冲区第一个元素
         *
         * @return 第一个元素的引用
         */
        constexpr inline auto&& front(this auto&& self) noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!self.empty(), "环形缓冲区已空"sv);
            return self.buffer[self.head & buffer_mask].value;
        }

        /**
         * @brief 访问缓冲区最后一个元素
         *
         * @return 最后一个元素的引用
         */
        constexpr inline auto&& back(this auto&& self) noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!self.empty(), "环形缓冲区已空"sv);
            return self.buffer[(self.tail - 1) & buffer_mask].value;
        }

        /**
         * @brief 向缓冲区添加元素
         *
         * @tparam args_t 构造参数类型
         * @param args 构造参数列表
         */
        template <typename... args_t>
            requires ::std::constructible_from<value_type, args_t...>
        constexpr inline void emplace_back(args_t&&... args) noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!full(), "环形缓冲区已满"sv);
            ::new(&buffer[tail++ & buffer_mask].value) value_type{::std::forward<args_t>(args)...};
        }

        /**
         * @brief 从缓冲区移除元素
         *
         */
        constexpr inline void pop_front() noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!empty(), "环形缓冲区已空"sv);
            auto&& ref{buffer[head++ & buffer_mask].value};
            ref.~value_type();
        }

        /**
         * @brief 检查两个环形缓冲区是否相等
         *
         * @param lhs 第一个环形缓冲区
         * @param rhs 第二个环形缓冲区
         * @return 两个环形缓冲区是否相等
         */
        constexpr inline friend bool operator== (const ring_buffer& lhs, const ring_buffer& rhs) noexcept
        {
            return ::std::ranges::equal(lhs, rhs);
        }
    };
}  // namespace SoC
