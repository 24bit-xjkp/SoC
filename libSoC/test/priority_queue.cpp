/**
 * @file priority_queue.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 测试优先队列
 */

import "test_framework.hpp";
import SoC.unit_test;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("priority_queue/" NAME)

namespace SoC::test
{
    extern "C++" template <typename type, ::std::size_t buffer_size, template <typename> typename compare = std::less>
    struct priority_queue : ::SoC::priority_queue<type, buffer_size, compare>
    {
        using base_t = ::SoC::priority_queue<type, buffer_size, compare>;
        using base_t::base_t;
        using base_t::buffer;
        using base_t::comp;
        using base_t::tail;
    };
}  // namespace SoC::test

namespace
{
    struct test_struct
    {
        ::std::size_t value{};
        inline static ::std::size_t ctor_cnt{};
        inline static ::std::size_t copy_ctor_cnt{};
        inline static ::std::size_t move_ctor_cnt{};
        inline static ::std::size_t dtor_cnt{};

        static void reset() noexcept
        {
            ctor_cnt = 0;
            copy_ctor_cnt = 0;
            move_ctor_cnt = 0;
            dtor_cnt = 0;
        }

        test_struct(::std::size_t value) noexcept : value{value} { ++ctor_cnt; }

        test_struct(const test_struct& other) noexcept : value{other.value} { ++copy_ctor_cnt; }

        test_struct(test_struct&& other) noexcept : value{::std::exchange(other.value, 0)} { ++move_ctor_cnt; }

        test_struct& operator= (const test_struct&) noexcept = default;

        test_struct& operator= (test_struct&& other) noexcept = default;

        ~test_struct() noexcept { ++dtor_cnt; }

        friend auto operator<=> (const test_struct&, const test_struct&) = default;
        friend bool operator== (const test_struct&, const test_struct&) = default;

        friend auto operator== (const test_struct& lhs, ::std::size_t rhs) noexcept { return lhs.value == rhs; }
    };
}  // namespace

using priority_queue_t = ::SoC::test::priority_queue<::test_struct, 4>;

/// @test 测试优先队列
TEST_SUITE("priority_queue" * ::doctest::description{"测试优先队列"})
{
    REGISTER_TEST_CASE("general operator")
    {
        ::priority_queue_t priority_queue{};

        ::test_struct::reset();
        REQUIRE_EQ(::test_struct::ctor_cnt, 0);
        REQUIRE_EQ(::test_struct::dtor_cnt, 0);

        CHECK_EQ(priority_queue.size(), 0);
        CHECK(priority_queue.empty());
        CHECK_EQ(priority_queue.capacity(), 4zu);

        static_assert(::std::move_constructible<::SoC::union_wrapper<::test_struct>>);
        priority_queue.emplace_back(1zu);
        CHECK_EQ(priority_queue.top(), priority_queue.buffer.front().value);
        CHECK_EQ(priority_queue.top(), 1zu);
        CHECK_EQ(priority_queue.size(), 1);
        CHECK(!priority_queue.empty());
        CHECK_EQ(::test_struct::ctor_cnt, 1);

        priority_queue.emplace_back(3zu);
        priority_queue.emplace_back(2zu);
        priority_queue.emplace_back(4zu);
        CHECK_EQ(priority_queue.top(), 4zu);
        CHECK_EQ(priority_queue.size(), 4);
        CHECK(priority_queue.full());
        CHECK_EQ(::test_struct::ctor_cnt, 4);
        CHECK_THROWS_WITH_AS_MESSAGE(priority_queue.emplace_back(5zu),
                                     ::doctest::Contains{"优先队列已满"},
                                     ::SoC::assert_failed_exception,
                                     "优先队列已满时添加元素应断言失败");

        for(auto ground_truth{4zu}; ground_truth != 0zu; --ground_truth)
        {
            CAPTURE(ground_truth);
            CHECK_EQ(priority_queue.top(), ground_truth);
            priority_queue.pop_front();
        }
        CHECK_EQ(priority_queue.size(), 0);
        CHECK(priority_queue.empty());
        CHECK_EQ(::test_struct::ctor_cnt, 4);
        CHECK_EQ(::test_struct::dtor_cnt, 4);
        CHECK_THROWS_WITH_AS_MESSAGE(priority_queue.pop_front(),
                                     ::doctest::Contains{"优先队列已空"},
                                     ::SoC::assert_failed_exception,
                                     "优先队列已空时移除元素应断言失败");
    }

    REGISTER_TEST_CASE("constructor, destructor, and assignment" * ::doctest::description{"测试构造函数、析构函数和赋值运算符"})
    {
        ::priority_queue_t priority_queue{};
        for(auto i: ::std::views::iota(0zu, 4zu)) { priority_queue.emplace_back(i); }
        ::test_struct::reset();

        // 禁用move相关警告
        // NOLINTBEGIN(clang-analyzer-cplusplus.Move,bugprone-use-after-move,hicpp-invalid-access-moved)

        SUBCASE("copy constructor")
        {
            ::priority_queue_t copied{priority_queue};
            CHECK_EQ(copied.size(), priority_queue.size());
            for(auto&& [ground_truth, result]: ::std::views::zip(priority_queue.buffer, copied.buffer))
            {
                CHECK_EQ(ground_truth.value, result.value);
            }

            CHECK_EQ(::test_struct::ctor_cnt, 0);
            CHECK_EQ(::test_struct::copy_ctor_cnt, 4);
            CHECK_EQ(::test_struct::move_ctor_cnt, 0);
            CHECK_EQ(::test_struct::dtor_cnt, 0);
        }

        SUBCASE("move constructor")
        {
            ::priority_queue_t copied{priority_queue};
            ::priority_queue_t moved{::std::move(priority_queue)};
            CHECK_EQ(moved.size(), copied.size());
            for(auto&& [ground_truth, result]: ::std::views::zip(copied.buffer, moved.buffer))
            {
                CHECK_EQ(ground_truth.value, result.value);
            }
            for(auto&& result: priority_queue.buffer) { CHECK_EQ(result.value, 0zu); }
            CHECK_EQ(::test_struct::ctor_cnt, 0);
            CHECK_EQ(::test_struct::copy_ctor_cnt, 4);
            CHECK_EQ(::test_struct::move_ctor_cnt, 4);
            CHECK_EQ(::test_struct::dtor_cnt, 0);
        }

        SUBCASE("copy assignment")
        {
            ::priority_queue_t copied{};
            copied = priority_queue;
            CHECK_EQ(copied.size(), priority_queue.size());
            for(auto&& [ground_truth, result]: ::std::views::zip(priority_queue.buffer, copied.buffer))
            {
                CHECK_EQ(ground_truth.value, result.value);
            }
            CHECK_EQ(::test_struct::ctor_cnt, 0);
        }

        SUBCASE("move assignment")
        {
            ::priority_queue_t copied{priority_queue};
            ::priority_queue_t moved{};
            moved = ::std::move(priority_queue);
            CHECK_EQ(moved.size(), copied.size());
            for(auto&& [ground_truth, result]: ::std::views::zip(copied.buffer, moved.buffer))
            {
                CHECK_EQ(ground_truth.value, result.value);
            }
            for(auto&& result: priority_queue.buffer) { CHECK_EQ(result.value, 0zu); }
        }

        // NOLINTEND(clang-analyzer-cplusplus.Move,bugprone-use-after-move,hicpp-invalid-access-moved)
    }
}
