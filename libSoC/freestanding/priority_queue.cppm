/**
 * @file priority_queue.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的优先队列实现
 */

export module SoC.freestanding:priority_queue;
import :utils;

export namespace SoC
{
    namespace test
    {
        /// @see SoC::priority_queue
        extern "C++" template <typename type, ::std::size_t buffer_size, template <typename> typename compare = std::less>
        struct priority_queue;
    }  // namespace test

    /**
     * @brief 优先队列
     *
     * @tparam type 队列元素类型
     * @tparam buffer_size 队列缓冲区大小
     * @tparam compare 比较类型，模板模板参数，默认使用std::less
     * @note 使用compare<::SoC::union_wrapper<type>>对象来比较元素
     */
    template <typename type, ::std::size_t buffer_size, template <typename> typename compare = std::less>
        requires (::std::is_empty_v<compare<::SoC::union_wrapper<type>>>)
    struct priority_queue
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        using size_t = ::std::size_t;
        using compare_t = compare<::SoC::union_wrapper<type>>;

    private:
        ::std::array<::SoC::union_wrapper<type>, buffer_size> buffer{};
        ::std::size_t tail{};
        [[no_unique_address]] compare_t comp{};
        friend struct ::SoC::test::priority_queue<type, buffer_size, compare>;

    public:
        /**
         * @brief 默认构造优先队列，初始队列为空
         *
         */
        explicit constexpr inline priority_queue() noexcept = default;

        /**
         * @brief 析构优先队列
         *
         */
        constexpr inline ~priority_queue() noexcept
        {
            for(auto i: ::std::ranges::views::iota(0zu, tail)) { buffer[i].value.~type(); }
        }

        /**
         * @brief 复制构造优先队列
         *
         * @param other 要复制的优先队列
         */
        constexpr inline priority_queue(const priority_queue& other) noexcept : tail{other.tail}, comp{other.comp}
        {
            for(auto i: ::std::ranges::views::iota(0zu, tail)) { ::new(&buffer[i].value) type{other.buffer[i].value}; }
        }

        /**
         * @brief 移动构造优先队列
         *
         * @param other 要移动的优先队列
         */
        constexpr inline priority_queue(priority_queue&& other) noexcept : tail{other.tail}, comp{::std::move(other.comp)}
        {
            for(auto i: ::std::ranges::views::iota(0zu, tail))
            {
                ::new(&buffer[i].value) type{::std::move(other.buffer[i].value)};
            }
        }

        /**
         * @brief 交换优先队列内容
         *
         * @param other 要交换的优先队列
         */
        constexpr inline void swap(priority_queue& other) noexcept
        {
            ::std::ranges::swap(buffer, other.buffer);
            ::std::ranges::swap(tail, other.tail);
            ::std::ranges::swap(comp, other.comp);
        }

        /**
         * @brief 复制赋值优先队列
         *
         * @param other 要复制的优先队列
         * @return 优先队列的引用
         */
        constexpr inline priority_queue& operator= (const priority_queue& other) noexcept
        {
            auto temp{other};
            swap(temp);
            return *this;
        }

        /**
         * @brief 移动赋值优先队列
         *
         * @param other 要移动的优先队列
         * @return 优先队列的引用
         */
        constexpr inline priority_queue& operator= (priority_queue&& other) noexcept
        {
            auto temp{::std::move(other)};
            swap(temp);
            return *this;
        }

        /**
         * @brief 获取优先队列大小
         *
         * @return 队列大小
         */
        [[nodiscard]] constexpr inline size_t size() const noexcept { return tail; }

        /**
         * @brief 获取优先队列的最大容量
         *
         * @return 最大容量
         */
        [[nodiscard]] constexpr inline ::std::size_t capacity() const noexcept { return buffer_size; }

        /**
         * @brief 检查优先队列是否为空
         *
         * @return 队列是否为空
         */
        [[nodiscard]] constexpr inline bool empty() const noexcept { return tail == 0; }

        /**
         * @brief 检查优先队列是否已满
         *
         * @return 队列是否已满
         */
        [[nodiscard]] constexpr inline bool full() const noexcept { return tail == buffer_size; }

        /**
         * @brief 获取优先队列的首个元素
         *
         * @return 首个元素的引用
         */
        [[nodiscard]] constexpr inline auto&& top(this auto&& self) noexcept
        {
            return ::std::forward_like<decltype(self)>(self.buffer[0].value);
        }

        /**
         * @brief 向优先队列添加元素
         *
         * @tparam args_t 构造参数类型列表
         * @param args 构造参数列表
         */
        template <typename... args_t>
            requires ::std::constructible_from<type, args_t...>
        constexpr inline void emplace_back(args_t&&... args) noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!full(), "优先队列已满"sv);
            ::new(&buffer[tail++].value) type{::std::forward<decltype(args)>(args)...};
            ::std::ranges::push_heap(::std::span{buffer}.subspan(0, tail), comp);
        }

        /**
         * @brief 从优先队列移除首个元素
         *
         */
        constexpr inline void pop_front() noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!empty(), "优先队列已空"sv);
            ::std::ranges::pop_heap(::std::span{buffer}.subspan(0, tail), comp);
            buffer[--tail].value.~type();
        }
    };
}  // namespace SoC
