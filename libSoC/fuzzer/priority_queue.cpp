import SoC.fuzzer;

using namespace ::std::string_view_literals;

namespace SoC::test
{
    extern "C++" template <typename type, ::std::size_t buffer_size, template <typename> typename compare = std::less>
    struct priority_queue : ::SoC::priority_queue<type, buffer_size, compare>
    {
        using base_t = ::SoC::priority_queue<type, buffer_size, compare>;
        using base_t::base_t;
        using typename base_t::error_code;
    };
}  // namespace SoC::test

extern "C" int LLVMFuzzerTestOneInput(const ::std::uint8_t* data, ::std::size_t size)
{
    constexpr auto buffer_size{64zu};
    using my_priority_queue_t = ::SoC::test::priority_queue<::std::uint8_t, buffer_size, ::std::greater>;
    using error_code_t = my_priority_queue_t::error_code;
    my_priority_queue_t my_priority_queue{};
    ::SoC::assert(my_priority_queue.capacity() == buffer_size, "优先队列容量与缓冲区大小不相等"sv);
    ::std::flat_multiset<::std::uint8_t> std_priority_queue{};

    const auto do_check{
        [&std_priority_queue](my_priority_queue_t my_priority_queue,
                              ::std::source_location location = ::std::source_location::current())
        {
            ::SoC::assert(my_priority_queue.size() == std_priority_queue.size(), "优先队列大小与标准库容器大小不相等"sv);
            for(auto&& ground_truth: std_priority_queue)
            {
                ::SoC::assert(ground_truth == my_priority_queue.top(), "优先队列中元素与标准库容器中元素不相等"sv, location);
                my_priority_queue.pop_front();
            }
        }};

    /// @brief 操作类型
    enum class operation_t : ::std::uint8_t
    {
        /// 入队操作
        push,
        /// 出队操作
        pop,
        /// 复制赋值操作
        copy_assign,
        /// 移动构造操作，同时测试移动赋值操作
        move_construct,
        /// 交换操作
        swap,
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
                auto value{static_cast<::std::uint8_t>(*data++)};
                --size;
                if(!my_priority_queue.full())
                {
                    my_priority_queue.emplace_back(value);
                    std_priority_queue.emplace(value);
                }
                else
                {
                    try
                    {
                        my_priority_queue.emplace_back(value);
                        ::SoC::assert(false, "优先队列已满，但没有抛出异常"sv);
                    }
                    catch(const ::SoC::fuzzer_assert_failed_t& error)
                    {
                        ::SoC::assert(error.get<error_code_t>() == error_code_t::priority_queue_full,
                                      "优先队列已满，但抛出的异常不是priority_queue_full"sv);
                    }
                }
                do_check(my_priority_queue);
                break;
            }
            case operation_t::pop:
            {
                if(!my_priority_queue.empty())
                {
                    my_priority_queue.pop_front();
                    std_priority_queue.erase(std_priority_queue.begin());
                }
                else
                {
                    try
                    {
                        my_priority_queue.pop_front();
                        ::SoC::assert(false, "优先队列已空，但没有抛出异常"sv);
                    }
                    catch(const ::SoC::fuzzer_assert_failed_t& error)
                    {
                        ::SoC::assert(error.get<error_code_t>() == error_code_t::priority_queue_empty,
                                      "优先队列已空，但抛出的异常不是priority_queue_empty"sv);
                    }
                }
                do_check(my_priority_queue);
                break;
            }
            case operation_t::copy_assign:
            {
                my_priority_queue_t another_queue{};
                another_queue = my_priority_queue;
                do_check(another_queue);
                break;
            }
            case operation_t::move_construct:
            {
                my_priority_queue_t another_queue{::std::move(my_priority_queue)};
                do_check(another_queue);
                my_priority_queue = ::std::move(another_queue);
                do_check(my_priority_queue);
                break;
            }
            default:
            {
                my_priority_queue_t another_queue{};
                my_priority_queue.swap(another_queue);
                do_check(another_queue);
                my_priority_queue.swap(another_queue);
                do_check(my_priority_queue);
                break;
            }
        }
    }

    return 0;
}
