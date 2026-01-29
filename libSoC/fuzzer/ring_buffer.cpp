import SoC.fuzzer;

using namespace ::std::string_view_literals;

struct test_struct
{
    ::std::uint8_t value;

    friend bool operator== (test_struct self, test_struct other) noexcept = default;
};

namespace SoC::test
{
    extern "C++" template <typename type, ::SoC::detail::ring_buffer_size_t buffer_size>
    struct ring_buffer : ::SoC::ring_buffer<type, buffer_size>
    {
        using base_t = ::SoC::ring_buffer<type, buffer_size>;
        using base_t::base_t;
        using base_t::head;
        using base_t::tail;
    };

    template <typename type, ::SoC::detail::ring_buffer_size_t buffer_size>
    constexpr inline void swap(::SoC::test::ring_buffer<type, buffer_size>& lhs,
                               ::SoC::test::ring_buffer<type, buffer_size>& rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }
}  // namespace SoC::test

extern "C" int LLVMFuzzerTestOneInput(const ::std::uint8_t* data, ::std::size_t size)
{
    constexpr auto max_buffer_size{::std::numeric_limits<::SoC::detail::ring_buffer_size_t>::max() + 1zu};
    constexpr auto buffer_size{max_buffer_size / 2};
    using value_type = ::test_struct;
    using error_code_t = ::SoC::detail::ring_buffer_fuzzer_error_code;
    using my_ring_buffer_t = ::SoC::test::ring_buffer<value_type, buffer_size>;
    my_ring_buffer_t my_ring_buffer{};
    ::SoC::assert(my_ring_buffer.empty(), "环形缓冲区初始状态非空"sv);
    ::SoC::assert(my_ring_buffer.capacity() == buffer_size, "环形缓冲区初始状态容量不一致"sv);
    ::std::vector<value_type> std_ring_buffer{};

    // 检查环形缓冲区状态是否与标准容器的状态一致
    const auto do_check{
        [&std_ring_buffer](my_ring_buffer_t& my_ring_buffer, ::std::source_location location = ::std::source_location::current())
        {
            ::SoC::assert(my_ring_buffer.size() == std_ring_buffer.size(), "环形缓冲区大小不一致"sv, location);
            if(my_ring_buffer.empty()) { return; }
            ::SoC::assert(my_ring_buffer.front() == std_ring_buffer.front(), "环形缓冲区前端元素不一致"sv, location);
            ::SoC::assert(my_ring_buffer.back() == std_ring_buffer.back(), "环形缓冲区前端元素不一致"sv, location);

            for(auto&& [i, value, ground_truth]: ::std::views::zip(::std::views::iota(0zu), my_ring_buffer, std_ring_buffer))
            {
                ::SoC::assert(value == ground_truth, ::std::format("环形缓冲区索引为{}处的元素不一致"sv, i), location);
            }
        }};
    // 检查异常码是否符合预期
    constexpr static auto check_error{[](const ::SoC::fuzzer_assert_failed_t& error,
                                         error_code_t expect_error_code,
                                         ::std::string_view message,
                                         ::std::source_location location = ::std::source_location::current()) static
                                      { ::SoC::assert(error.get<error_code_t>() == expect_error_code, message, location); }};
    // 检查迭代器是否符合预期
    constexpr auto check_iterator{
        []<typename type>(type begin,
                          type end,
                          auto std_ring_buffer,
                          ::std::source_location location = ::std::source_location::current()) static
        {
            for(auto iter{begin}; auto&& [i, value]: ::std::views::zip(::std::views::iota(0zu), std_ring_buffer))
            {
                ::SoC::assert(value == *iter, "operator* 不一致"sv, location);
                ::SoC::assert(value.value == iter->value, "operator-> 不一致"sv, location);
                ::SoC::assert(value == begin[i], "operator[] 不一致"sv, location);
                ::SoC::assert(value == *(begin + i), "operator+(iter, diff) 不一致"sv, location);
                ::SoC::assert(value == *(i + begin), "operator+(diff, iter) 不一致"sv, location);
                ::SoC::assert(iter == begin + i, "operator== 不一致"sv, location);
                ::SoC::assert(iter - i == begin, "operator-(iter, diff) 不一致"sv, location);
                ::SoC::assert(static_cast<::SoC::detail::ring_buffer_size_t>(iter - begin) == i,
                              "operator-(iter, iter) 不一致"sv,
                              location);
                ::SoC::assert(iter >= auto{iter}, "operator<=> 不一致"sv, location);

                auto iter_copy{iter};
                ::SoC::assert(++iter_copy == iter + 1, "operator++() 不一致"sv, location);
                ::SoC::assert(--iter_copy == iter, "operator--() 不一致"sv, location);
                ::SoC::assert(iter_copy++ == iter, "operator++(int) 不一致"sv, location);
                ::SoC::assert(iter_copy == iter + 1, "operator++(int) 不一致"sv, location);
                ::SoC::assert(iter_copy-- == iter + 1, "operator--(int) 不一致"sv, location);
                ::SoC::assert(iter_copy == iter, "operator--(int) 不一致"sv, location);
                ::SoC::assert((iter_copy += 1) == iter + 1, "operator+= 不一致"sv, location);
                ::SoC::assert((iter_copy -= 1) == iter, "operator+= 不一致"sv, location);

                ++iter;
            }

            my_ring_buffer_t other_ring_buffer{};
            try
            {
                if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::iterator>)
                {
                    auto _{other_ring_buffer.begin() <=> begin};
                }
                else if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::const_iterator>)
                {
                    auto _{other_ring_buffer.cbegin() <=> begin};
                }
                else if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::reverse_iterator>)
                {
                    auto _{other_ring_buffer.rbegin() <=> begin};
                }
                else if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::const_reverse_iterator>)
                {
                    auto _{other_ring_buffer.crbegin() <=> begin};
                }
                ::SoC::assert(false, "operator<=>比较指向不同的迭代器，但没有抛出异常"sv, location);
            }
            catch(const ::SoC::fuzzer_assert_failed_t& error)
            {
                check_error(error,
                            error_code_t::different_buffer,
                            "operator<=>比较指向不同的迭代器，但抛出的异常不是different_buffer"sv,
                            location);
            }

            bool is_iter_equal{};
            if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::iterator>)
            {
                is_iter_equal = begin == other_ring_buffer.begin();
            }
            else if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::const_iterator>)
            {
                is_iter_equal = begin == other_ring_buffer.cbegin();
            }
            else if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::reverse_iterator>)
            {
                is_iter_equal = begin == other_ring_buffer.rbegin();
            }
            else if constexpr(::std::same_as<decltype(begin), my_ring_buffer_t::const_reverse_iterator>)
            {
                is_iter_equal = begin == other_ring_buffer.crbegin();
            }
            ::SoC::assert(!is_iter_equal, "指向不同缓冲器的迭代器应当不相等"sv, location);

            try
            {
                auto _{*end};
                ::SoC::assert(false, "解引用end哨位，但没有抛出异常"sv);
            }
            catch(const ::SoC::fuzzer_assert_failed_t& error)
            {
                check_error(error, error_code_t::out_of_range, "解引用end哨位，但抛出的异常不是out_of_range"sv, location);
            }
        }};

    /// @brief 环形缓冲区操作类型
    enum class operation_t : ::std::uint8_t
    {
        /// 入队操作
        push,
        /// 出队操作
        pop,
        /// 交换操作
        swap,
        /// 移位操作，head和tail都加上一个buffer_size
        shift,
        /// 复制赋值操作
        copy_assign,
        /// 移动赋值操作
        move_assign,
        /// 相等操作
        equal,
        /// 迭代器操作
        iterator,
        /// 常量迭代器操作
        const_iterator,
        /// 反向迭代器操作
        reverse_iterator,
        /// 常量反向迭代器操作
        const_reverse_iterator,
        /// 结束标记
        end_operation
    };

    while(true)
    {
        if(size == 0) { break; }
        auto operation{static_cast<operation_t>((*data++) % ::std::to_underlying(operation_t::end_operation))};
        --size;

        switch(operation)
        {
            case operation_t::push:
            {
                if(size == 0) { break; }
                auto val{*data++};
                --size;
                if(my_ring_buffer.full())
                {
                    try
                    {
                        my_ring_buffer.emplace_back(val);
                        ::SoC::assert(false, "环形缓冲区已满，但没有抛出异常"sv);
                    }
                    catch(const ::SoC::fuzzer_assert_failed_t& error)
                    {
                        check_error(error, error_code_t::full, "环形缓冲区已满，但抛出的异常不是full"sv);
                    }
                }
                else
                {
                    my_ring_buffer.emplace_back(val);
                    std_ring_buffer.emplace_back(val);
                }
                do_check(my_ring_buffer);
                break;
            }
            case operation_t::pop:
            {
                if(my_ring_buffer.empty())
                {
                    try
                    {
                        my_ring_buffer.pop_front();
                        ::SoC::assert(false, "环形缓冲区已空，但没有抛出异常"sv);
                    }
                    catch(const ::SoC::fuzzer_assert_failed_t& error)
                    {
                        check_error(error, error_code_t::empty, "环形缓冲区已空，但抛出的异常不是empty"sv);
                    }
                }
                else
                {
                    my_ring_buffer.pop_front();
                    std_ring_buffer.erase(std_ring_buffer.begin());
                }
                do_check(my_ring_buffer);
                break;
            }
            case operation_t::swap:
            {
                // 交换自身
                ::std::ranges::swap(my_ring_buffer, my_ring_buffer);
                do_check(my_ring_buffer);

                // 交换另一个空队列
                {
                    my_ring_buffer_t another_ring_buffer{};
                    ::std::ranges::swap(my_ring_buffer, another_ring_buffer);
                    do_check(another_ring_buffer);
                    ::std::ranges::swap(my_ring_buffer, another_ring_buffer);
                    do_check(my_ring_buffer);
                }

                // 交换另一个非空队列
                {
                    my_ring_buffer_t another_ring_buffer{my_ring_buffer};
                    ::std::ranges::swap(my_ring_buffer, another_ring_buffer);
                    do_check(another_ring_buffer);
                    ::std::ranges::swap(my_ring_buffer, another_ring_buffer);
                    do_check(my_ring_buffer);
                }
                break;
            }
            case operation_t::shift:
            {
                // 人为加速head和tail回绕的出现
                my_ring_buffer.head += buffer_size;
                my_ring_buffer.tail += buffer_size;
                do_check(my_ring_buffer);
                break;
            }
            case operation_t::copy_assign:
            {
                my_ring_buffer_t another_queue{};
                another_queue = my_ring_buffer;
                do_check(another_queue);
                break;
            }
            case operation_t::move_assign:
            {
                my_ring_buffer_t another_queue{::std::move(my_ring_buffer)};
                do_check(another_queue);
                my_ring_buffer = ::std::move(another_queue);
                do_check(my_ring_buffer);
                break;
            }
            case operation_t::equal:
            {
                ::SoC::assert(my_ring_buffer == auto{my_ring_buffer}, "环形缓冲区相等操作结果不一致"sv);
                break;
            }
            case operation_t::iterator:
            {
                check_iterator(my_ring_buffer.begin(), my_ring_buffer.end(), std_ring_buffer);
                break;
            }
            case operation_t::const_iterator:
            {
                check_iterator(my_ring_buffer.cbegin(), my_ring_buffer.cend(), std_ring_buffer);
                break;
            }
            case operation_t::reverse_iterator:
            {
                check_iterator(my_ring_buffer.rbegin(), my_ring_buffer.rend(), std_ring_buffer | ::std::views::reverse);
                break;
            }
            default:
            {
                check_iterator(my_ring_buffer.crbegin(), my_ring_buffer.crend(), std_ring_buffer | ::std::views::reverse);
                break;
            }
        }
    }
    return 0;
}
