/**
 * @file priority_queue.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的优先队列实现
 */

export module SoC.freestanding:priority_queue;
import :utils;

namespace SoC
{
    template <typename type, ::std::size_t buffer_size, typename compare = std::less<type>>
        requires (::std::is_empty_v<compare>)
    struct priority_queue
    {
        using value_type = type;
        using pointer = type*;
        using const_pointer = const type*;
        using reference = type&;
        using const_reference = const type&;
        using size_t = ::std::size_t;

    private:
        ::std::array<::SoC::union_wrapper<type>, buffer_size> buffer{}; 
        ::std::size_t tail{};
        [[no_unique_address]] compare comp{};

        /**
         * @brief 获取优先队列首指针
         *
         * @return 首指针
         */
        constexpr inline auto begin(this auto&& self) noexcept { return self.buffer; }

        /**
         * @brief 获取优先队列尾哨位
         *
         * @return 尾哨位
         */
        constexpr inline auto end(this auto&& self) noexcept { return self.buffer + self.tail; }

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
            for(auto i{0zu}; i < tail; ++i) { buffer[i].value.~type(); }
        }

        /**
         * @brief 复制构造优先队列
         *
         * @param other 要复制的优先队列
         */
        constexpr inline priority_queue(const priority_queue& other) noexcept : tail{other.tail}
        {
            for(auto i{0zu}; i < tail; ++i) { ::new(&buffer[i].value) type(other.buffer[i].value); }
        }

        /**
         * @brief 移动构造优先队列
         *
         * @param other 要移动的优先队列
         */
        constexpr inline priority_queue(priority_queue&& other) noexcept : tail{other.tail}
        {
            for(auto i{0zu}; i < tail; ++i) { ::new(&buffer[i].value) type(::std::move(other.buffer[i].value)); }
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
            ::std::ranges::swap(temp, *this);
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
            ::std::ranges::swap(temp, *this);
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
         * @param args 构造参数列表
         */
        constexpr inline void emplace_back(::std::constructible_from<type> auto&&... args) noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!full(), "优先队列已满"sv);
            ::new(&buffer[tail++].value) type(::std::forward<decltype(args)>(args)...);
            ::std::ranges::push_heap(begin(), end(), comp);
        }

        /**
         * @brief 从优先队列移除首个元素
         *
         */
        constexpr inline void pop_front() noexcept(::SoC::optional_noexcept)
        {
            using namespace ::std::string_view_literals;
            ::SoC::always_check(!full(), "优先队列已空"sv);
            ::std::ranges::pop_heap(begin(), end(), comp);
            buffer[--tail].value.~type();
        }
    };
}  // namespace SoC
