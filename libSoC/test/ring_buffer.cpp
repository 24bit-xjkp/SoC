/**
 * @file ring_buffer.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 测试环形缓冲区
 */

import "test_framework.hpp";
import SoC.unit_test;

using namespace ::std::string_view_literals;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REGISTER_TEST_CASE(NAME) TEST_CASE("ring_buffer/" NAME)

namespace SoC::test
{
    extern "C++" template <typename type, ::std::size_t buffer_size>
    struct ring_buffer : ::SoC::ring_buffer<type, buffer_size>
    {
        using base_t = ::SoC::ring_buffer<type, buffer_size>;
        using base_t::base_t;
        using base_t::buffer;
        using base_t::buffer_mask;
        using base_t::head;
        using base_t::iterator_t;
        using base_t::tail;

        template <bool is_const>
        using iterator_impl_t = ::SoC::test::ring_buffer_iterator_t<type, buffer_size, is_const>;
        using iterator = iterator_impl_t<false>;
        using const_iterator = iterator_impl_t<true>;
        using reverse_iterator = ::std::reverse_iterator<iterator>;
        using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

        [[nodiscard]] constexpr inline auto begin(this auto&& self) noexcept
        {
            return iterator_impl_t<::std::is_const_v<::std::remove_reference_t<decltype(self)>>>{self.head, &self};
        }

        [[nodiscard]] constexpr inline auto end(this auto&& self) noexcept
        {
            return iterator_impl_t<::std::is_const_v<::std::remove_reference_t<decltype(self)>>>{self.tail, &self};
        }

        [[nodiscard]] constexpr inline const_iterator cbegin() const noexcept { return {head, this}; }

        [[nodiscard]] constexpr inline const_iterator cend() const noexcept { return {tail, this}; }

        [[nodiscard]] constexpr inline auto rbegin(this auto&& self) noexcept { return ::std::reverse_iterator{self.end()}; }

        [[nodiscard]] constexpr inline auto rend(this auto&& self) noexcept { return ::std::reverse_iterator{self.begin()}; }

        [[nodiscard]] constexpr inline const_reverse_iterator crbegin() const noexcept { return rbegin(); }

        [[nodiscard]] constexpr inline const_reverse_iterator crend() const noexcept { return rend(); }
    };

    extern "C++" template <typename type, ::std::size_t buffer_size, bool is_const>
    struct ring_buffer_iterator_t : ::SoC::test::ring_buffer<type, buffer_size>::template iterator_t<is_const>
    {
        using base_t = ::SoC::test::ring_buffer<type, buffer_size>::template iterator_t<is_const>;
        using base_t::base_t;
        using base_t::index;
        using base_t::ring_buffer_ptr;

        constexpr inline ring_buffer_iterator_t(base_t iter) noexcept : base_t{iter} {}

        constexpr inline friend ring_buffer_iterator_t& operator++ (ring_buffer_iterator_t& self) noexcept
        {
            return static_cast<ring_buffer_iterator_t&>(++static_cast<base_t&>(self));
        }

        constexpr inline friend ring_buffer_iterator_t operator++ (ring_buffer_iterator_t& self,
                                                                   int placehold [[maybe_unused]]) noexcept
        {
            return static_cast<base_t&>(self)++;
        }

        constexpr inline friend ring_buffer_iterator_t operator+ (const ring_buffer_iterator_t& self,
                                                                  ::std::ptrdiff_t offset) noexcept
        {
            return static_cast<const base_t&>(self) + offset;
        }

        constexpr inline friend ring_buffer_iterator_t operator+ (::std::ptrdiff_t offset,
                                                                  const ring_buffer_iterator_t& self) noexcept
        {
            return offset + static_cast<const base_t&>(self);
        }

        constexpr inline friend ring_buffer_iterator_t& operator+= (ring_buffer_iterator_t& self,
                                                                    ::std::ptrdiff_t offset) noexcept
        {
            return static_cast<ring_buffer_iterator_t&>(static_cast<base_t&>(self) += offset);
        }

        constexpr inline friend ring_buffer_iterator_t& operator-- (ring_buffer_iterator_t& self) noexcept
        {
            return static_cast<ring_buffer_iterator_t&>(--static_cast<base_t&>(self));
        }

        constexpr inline friend ring_buffer_iterator_t operator-- (ring_buffer_iterator_t& self,
                                                                   int placehold [[maybe_unused]]) noexcept
        {
            return static_cast<base_t&>(self)--;
        }

        constexpr inline friend ring_buffer_iterator_t operator- (const ring_buffer_iterator_t& self,
                                                                  ::std::ptrdiff_t offset) noexcept
        {
            return static_cast<const base_t&>(self) - offset;
        }

        constexpr inline friend ring_buffer_iterator_t& operator-= (ring_buffer_iterator_t& self,
                                                                    ::std::ptrdiff_t offset) noexcept
        {
            return static_cast<ring_buffer_iterator_t&>(static_cast<base_t&>(self) -= offset);
        }
    };
}  // namespace SoC::test

namespace
{
    struct test_struct
    {
        ::std::size_t value{};
        inline static auto ctor_cnt{0zu};
        inline static auto copy_ctor_cnt{0zu};
        inline static auto move_ctor_cnt{0zu};
        inline static auto dtor_cnt{0zu};

        static void reset() noexcept
        {
            ctor_cnt = 0;
            dtor_cnt = 0;
            copy_ctor_cnt = 0;
            move_ctor_cnt = 0;
        }

        test_struct(::std::size_t value = 0) noexcept : value{value} { ++ctor_cnt; }

        ~test_struct() noexcept { ++dtor_cnt; }

        test_struct(const test_struct& other) noexcept : value{other.value} { ++copy_ctor_cnt; }

        test_struct(test_struct&& other) noexcept : value{other.value} { ++move_ctor_cnt; }

        test_struct& operator= (const test_struct&) noexcept = default;

        test_struct& operator= (test_struct&&) noexcept = default;

        inline friend bool operator== (const test_struct& self, const test_struct& other) noexcept
        {
            return self.value == other.value;
        }
    };

    using ring_buffer_t = ::SoC::test::ring_buffer<::test_struct, 4>;
}  // namespace

namespace doctest
{
    template <>
    struct StringMaker<test_struct>
    {
        static ::doctest::String convert(const test_struct& value)
        {
            ::std::array<char, ::std::numeric_limits<::std::size_t>::digits10 + 1> buffer{};
            ::std::to_chars(buffer.begin(), buffer.end(), value.value);
            return {buffer.data()};
        }
    };

    template <>
    struct StringMaker<ring_buffer_t>
    {
        static ::doctest::String convert(const ring_buffer_t& value)
        {
            ::doctest::String buffer{"["};
            for(const auto&& [i, element]: ::std::views::zip(::std::views::iota(1zu), value))
            {
                buffer += ::doctest::StringMaker<test_struct>::convert(element);
                buffer += i == value.size() ? "" : ", ";
            }
            buffer += "]";
            return buffer;
        }
    };
}  // namespace doctest

/// @test 测试环形缓冲区
TEST_SUITE("ring_buffer" * ::doctest::description{"测试环形缓冲区"})
{
    /// @test 测试环形缓冲区的基本属性
    REGISTER_TEST_CASE("basic_properties" * ::doctest::description{"测试环形缓冲区的基本属性"})
    {
        ::ring_buffer_t buffer{};

        CHECK(buffer.empty());
        CHECK_FALSE(buffer.full());
        CHECK_EQ(buffer.size(), 0zu);
        CHECK_EQ(buffer.capacity(), 4zu);
        CHECK_EQ(buffer.buffer_mask, 3zu);

        buffer.tail = buffer.capacity();
        CHECK_FALSE(buffer.empty());
        CHECK(buffer.full());
        CHECK_EQ(buffer.size(), buffer.capacity());

        buffer.head = -1zu - 1;
        buffer.tail = buffer.head + 2;
        CHECK_FALSE(buffer.empty());
        CHECK_FALSE(buffer.full());
        CHECK_EQ(buffer.size(), 2zu);
        buffer.tail = buffer.head + buffer.capacity();
        CHECK_FALSE(buffer.empty());
        CHECK(buffer.full());
        CHECK_EQ(buffer.size(), buffer.capacity());
    }

    /// @test 测试环形缓冲区的emplace_back和pop_front操作
    REGISTER_TEST_CASE("emplace_back_and_pop_front" * ::doctest::description{"测试环形缓冲区的emplace_back和pop_front操作"})
    {
        ::ring_buffer_t buffer{};

        // 测试添加元素
        buffer.emplace_back(1zu);
        CHECK_FALSE(buffer.empty());
        CHECK_FALSE(buffer.full());
        CHECK_EQ(buffer.size(), 1zu);

        buffer.emplace_back(2zu);
        buffer.emplace_back(3zu);
        buffer.emplace_back(4zu);
        CHECK_FALSE(buffer.empty());
        CHECK(buffer.full());
        CHECK_EQ(buffer.size(), 4zu);

        // 测试移除元素
        CHECK_EQ(buffer.front(), 1zu);
        CHECK_EQ(buffer.back(), 4zu);
        buffer.pop_front();
        CHECK_FALSE(buffer.empty());
        CHECK_FALSE(buffer.full());
        CHECK_EQ(buffer.size(), 3zu);

        CHECK_EQ(buffer.front(), 2zu);
        CHECK_EQ(buffer.back(), 4zu);
        buffer.pop_front();
        CHECK_EQ(buffer.front(), 3zu);
        CHECK_EQ(buffer.back(), 4zu);
        buffer.pop_front();
        CHECK_EQ(buffer.front(), 4zu);
        CHECK_EQ(buffer.back(), 4zu);
        buffer.pop_front();
        CHECK(buffer.empty());
        CHECK_FALSE(buffer.full());
        CHECK_EQ(buffer.size(), 0zu);
    }

    /// @test 测试环形缓冲区的边界条件
    REGISTER_TEST_CASE("boundary_conditions" * ::doctest::description{"测试环形缓冲区的边界条件"})
    {
        ::ring_buffer_t buffer{};

        // 测试从空缓冲区弹出元素
        CHECK_THROWS_WITH_AS_MESSAGE(buffer.pop_front(),
                                     ::doctest::Contains{"环形缓冲区已空"},
                                     ::SoC::assert_failed_exception,
                                     "从空缓冲区弹出元素应断言失败"sv);
        // 测试访问空缓冲区中的元素
        const ::doctest::Contains exception_string{"环形缓冲区已空"};
        constexpr auto message{"访问空缓冲区中元素应断言失败"sv};
        CHECK_THROWS_WITH_AS_MESSAGE(buffer.front(), exception_string, ::SoC::assert_failed_exception, message);
        CHECK_THROWS_WITH_AS_MESSAGE(buffer.back(), exception_string, ::SoC::assert_failed_exception, message);

        // 测试向满缓冲区添加元素
        buffer.emplace_back(1zu);
        buffer.emplace_back(2zu);
        buffer.emplace_back(3zu);
        buffer.emplace_back(4zu);
        CHECK_THROWS_WITH_AS_MESSAGE(buffer.emplace_back(5zu),
                                     ::doctest::Contains{"环形缓冲区已满"},
                                     ::SoC::assert_failed_exception,
                                     "向满缓冲区添加元素应断言失败"sv);

        // 测试环绕写入和读取
        buffer.pop_front();        // 移除1
        buffer.emplace_back(5zu);  // 添加5
        buffer.pop_front();        // 移除2
        buffer.emplace_back(6zu);  // 添加6

        CHECK_EQ(buffer.size(), 4zu);
        CHECK_EQ(buffer.front(), 3zu);
        buffer.pop_front();
        CHECK_EQ(buffer.front(), 4zu);
        buffer.pop_front();
        CHECK_EQ(buffer.front(), 5zu);
        buffer.pop_front();
        CHECK_EQ(buffer.front(), 6zu);
        buffer.pop_front();
        CHECK(buffer.empty());
    }

    /// @test 测试环形缓冲区的迭代器
    REGISTER_TEST_CASE("iterator" * ::doctest::description{"测试环形缓冲区的迭代器"})
    {
        ::ring_buffer_t buffer{};
        constexpr ::std::array table{1zu, 2zu, 3zu, 4zu};

        SUBCASE("is random access iterator")
        {
            constexpr auto message{"环形缓冲区迭代器应满足随机访问迭代器要求"sv};
            CHECK_MESSAGE(::std::random_access_iterator<::ring_buffer_t::iterator>, message);
            CHECK_MESSAGE(::std::random_access_iterator<::ring_buffer_t::const_iterator>, message);
            CHECK_MESSAGE(::std::random_access_iterator<::ring_buffer_t::reverse_iterator>, message);
            CHECK_MESSAGE(::std::random_access_iterator<::ring_buffer_t::const_reverse_iterator>, message);
        }

        SUBCASE("iterator arithmetic")
        {
            constexpr auto ground_truth{1zu};
            buffer.emplace_back(ground_truth);
            buffer.pop_front();
            buffer.emplace_back(ground_truth);
            auto begin{buffer.begin()};
            auto end{buffer.end()};
            constexpr auto check_index_message{"迭代器超出环形缓冲区范围，应当断言失败"sv};
            constexpr auto check_same_buffer_message{"两个迭代器指向不同的环形缓冲区，应当断言失败"sv};
            constexpr auto indirect_addressing_message{"访问未越界，不应断言失败"sv};
            const ::doctest::Contains check_index_exception_string{"迭代器超出环形缓冲区范围"};
            const ::doctest::Contains check_same_buffer_exception_string{"迭代器指向不同的环形缓冲区"};

            SUBCASE("basic")
            {
                REQUIRE_EQ(begin.index, 1zu);
                REQUIRE_EQ(end.index, 2zu);
                REQUIRE_EQ(begin.ring_buffer_ptr, &buffer);
                REQUIRE_EQ(end.ring_buffer_ptr, &buffer);
                REQUIRE_EQ(buffer.head, 1);
                REQUIRE_EQ(buffer.tail, 2);
            }

            SUBCASE("check_index")
            {
                SUBCASE("no wrap")
                {
                    constexpr auto head{1zu};
                    constexpr auto tail{2zu};
                    CHECK_NOTHROW_MESSAGE(::SoC::detail::check_ring_buffer_iterator_index(1, head, tail),
                                          "迭代器内索引范围为[1, 2)，不应断言失败"sv);
                    CHECK_THROWS_WITH_AS_MESSAGE(::SoC::detail::check_ring_buffer_iterator_index(0, head, tail),
                                                 check_index_exception_string,
                                                 ::SoC::assert_failed_exception,
                                                 check_index_message);
                    CHECK_THROWS_WITH_AS_MESSAGE(::SoC::detail::check_ring_buffer_iterator_index(2, head, tail),
                                                 check_index_exception_string,
                                                 ::SoC::assert_failed_exception,
                                                 check_index_message);
                }

                SUBCASE("wrap-around")
                {
                    constexpr auto head{-1zu};
                    constexpr auto tail{1zu};
                    constexpr auto message{"迭代器内索引范围为[-1zu, 1)，不应断言失败"sv};
                    CHECK_NOTHROW_MESSAGE(::SoC::detail::check_ring_buffer_iterator_index(-1zu, head, tail), message);
                    CHECK_NOTHROW_MESSAGE(::SoC::detail::check_ring_buffer_iterator_index(0, head, tail), message);
                    CHECK_THROWS_WITH_AS_MESSAGE(::SoC::detail::check_ring_buffer_iterator_index(1, head, tail),
                                                 check_index_exception_string,
                                                 ::SoC::assert_failed_exception,
                                                 check_index_message);
                }
            }

            SUBCASE("check_same_buffer")
            {
                CHECK_NOTHROW_MESSAGE(
                    ::SoC::detail::check_ring_buffer_iterator_same_buffer(begin.ring_buffer_ptr, end.ring_buffer_ptr),
                    "两个迭代器指向同一环形缓冲区，不应断言失败"sv);
                ::ring_buffer_t other_buffer{};
                CHECK_THROWS_WITH_AS_MESSAGE(
                    ::SoC::detail::check_ring_buffer_iterator_same_buffer(begin.ring_buffer_ptr,
                                                                          other_buffer.begin().ring_buffer_ptr),
                    check_same_buffer_exception_string,
                    ::SoC::assert_failed_exception,
                    check_same_buffer_message);
            }

            SUBCASE("operator++")
            {
                decltype(begin) next{};
                CHECK_NOTHROW(next = ++begin);
                CHECK_EQ(begin.index, 2zu);
                CHECK_EQ(next.index, 2zu);
                CHECK_EQ(next.ring_buffer_ptr, &buffer);
            }

            SUBCASE("operator++(int)")
            {
                decltype(begin) next{};
                CHECK_NOTHROW(next = begin++);
                CHECK_EQ(begin.index, 2zu);
                CHECK_EQ(next.index, 1zu);
                CHECK_EQ(next.ring_buffer_ptr, &buffer);
            }

            SUBCASE("operator+")
            {
                SUBCASE("positive offset")
                {
                    decltype(begin) next{};
                    CHECK_NOTHROW(next = begin + 1);
                    CHECK_EQ(begin.index, 1zu);
                    CHECK_EQ(next.index, 2zu);
                    CHECK_EQ(next.ring_buffer_ptr, &buffer);

                    CHECK_NOTHROW(next = 1 + begin);
                    CHECK_EQ(begin.index, 1zu);
                    CHECK_EQ(next.index, 2zu);
                    CHECK_EQ(next.ring_buffer_ptr, &buffer);
                }

                SUBCASE("negative offset")
                {
                    decltype(end) prev{};
                    CHECK_NOTHROW(prev = end - 1);
                    CHECK_EQ(end.index, 2zu);
                    CHECK_EQ(prev.index, 1zu);
                    CHECK_EQ(prev.ring_buffer_ptr, &buffer);

                    CHECK_NOTHROW(prev = -1 + end);
                    CHECK_EQ(end.index, 2zu);
                    CHECK_EQ(prev.index, 1zu);
                    CHECK_EQ(prev.ring_buffer_ptr, &buffer);
                }
            }

            SUBCASE("operator+=")
            {
                SUBCASE("positive")
                {
                    decltype(begin) next{};
                    CHECK_NOTHROW(next = begin += 1);
                    CHECK_EQ(begin.index, 2zu);
                    CHECK_EQ(next.index, 2zu);
                    CHECK_EQ(next.ring_buffer_ptr, &buffer);
                }

                SUBCASE("negative")
                {
                    decltype(end) prev{};
                    CHECK_NOTHROW(prev = end += -1);
                    CHECK_EQ(end.index, 1zu);
                    CHECK_EQ(prev.index, 1zu);
                    CHECK_EQ(prev.ring_buffer_ptr, &buffer);
                }
            }

            SUBCASE("operator--")
            {
                decltype(end) prev{};
                CHECK_NOTHROW(prev = --end);
                CHECK_EQ(end.index, 1zu);
                CHECK_EQ(prev.index, 1zu);
                CHECK_EQ(prev.ring_buffer_ptr, &buffer);
            }

            SUBCASE("operator--(int)")
            {
                decltype(end) prev{};
                CHECK_NOTHROW(prev = end--);
                CHECK_EQ(end.index, 1zu);
                CHECK_EQ(prev.index, 2zu);
                CHECK_EQ(prev.ring_buffer_ptr, &buffer);
            }

            SUBCASE("operator-")
            {
                SUBCASE("positive offset")
                {
                    decltype(end) prev{};
                    CHECK_NOTHROW(prev = end - 1);
                    CHECK_EQ(end.index, 2zu);
                    CHECK_EQ(prev.index, 1zu);
                    CHECK_EQ(prev.ring_buffer_ptr, &buffer);
                }

                SUBCASE("negative offset")
                {
                    decltype(begin) next{};
                    CHECK_NOTHROW(next = begin - -1);
                    CHECK_EQ(begin.index, 1zu);
                    CHECK_EQ(next.index, 2zu);
                    CHECK_EQ(next.ring_buffer_ptr, &buffer);
                }

                SUBCASE("difference")
                {
                    ::std::ptrdiff_t diff{};
                    CHECK_NOTHROW(diff = end - begin);
                    CHECK_EQ(diff, 1);
                    ::ring_buffer_t other_buffer{};
                    CHECK_THROWS_WITH_AS_MESSAGE(diff = begin - other_buffer.begin(),
                                                 check_same_buffer_exception_string,
                                                 ::SoC::assert_failed_exception,
                                                 check_same_buffer_message);
                }
            }

            SUBCASE("operator-=")
            {
                SUBCASE("positive")
                {
                    decltype(end) prev{};
                    CHECK_NOTHROW(prev = end -= 1);
                    CHECK_EQ(end.index, 1zu);
                    CHECK_EQ(prev.index, 1zu);
                    CHECK_EQ(prev.ring_buffer_ptr, &buffer);
                }

                SUBCASE("negative")
                {
                    decltype(begin) next{};
                    CHECK_NOTHROW(next = begin -= -1);
                    CHECK_EQ(begin.index, 2zu);
                    CHECK_EQ(next.index, 2zu);
                    CHECK_EQ(next.ring_buffer_ptr, &buffer);
                }
            }

            SUBCASE("operator==")
            {
                SUBCASE("equal")
                {
                    CHECK_EQ(begin, begin);
                    CHECK_EQ(begin + 1, end);
                    CHECK_EQ(end, end);
                    CHECK_EQ(begin, end - 1);
                }

                SUBCASE("not equal")
                {
                    CHECK_NE(begin, end);
                    CHECK_NE(begin, begin + 1);
                    CHECK_NE(end, end - 1);
                    ::ring_buffer_t other_buffer{};
                    // 保证head相同
                    other_buffer.emplace_back();
                    other_buffer.pop_front();
                    other_buffer.emplace_back();
                    CHECK_NE(begin, other_buffer.begin());
                    CHECK_NE(buffer.cbegin(), other_buffer.cbegin());
                }
            }

            SUBCASE("operator<=>")
            {
                CHECK_LT(begin, end);
                CHECK_GT(end, begin);
                CHECK_LE(begin, begin);
                CHECK_GE(end, end);
                ::ring_buffer_t other_buffer{};
                CHECK_THROWS_WITH_AS_MESSAGE(begin <=> other_buffer.begin(),
                                             check_same_buffer_exception_string,
                                             ::SoC::assert_failed_exception,
                                             check_same_buffer_message);
                CHECK_THROWS_WITH_AS_MESSAGE(buffer.cbegin() <=> other_buffer.cbegin(),
                                             check_same_buffer_exception_string,
                                             ::SoC::assert_failed_exception,
                                             check_same_buffer_message);

                // 测试回绕情况
                buffer.head = -1zu;
                buffer.tail = 1;
                begin = buffer.begin();
                end = buffer.end();
                CHECK_LT(begin, end);
                CHECK_LT(begin + 1, end);
                CHECK_LE(begin + 2, end);
                CHECK_GE(begin + 2, end);
                CHECK_GT(begin + 3, end);
            }

            SUBCASE("operator*")
            {
                ::test_struct value{};
                CHECK_NOTHROW_MESSAGE(value = *begin, indirect_addressing_message);
                CHECK_EQ(value, ground_truth);
                CHECK_THROWS_WITH_AS_MESSAGE(value = *end,
                                             check_index_exception_string,
                                             ::SoC::assert_failed_exception,
                                             check_index_message);
            }

            SUBCASE("operator[]")
            {
                ::test_struct value{};
                CHECK_NOTHROW_MESSAGE(value = begin[0], indirect_addressing_message);
                CHECK_EQ(value, ground_truth);
                CHECK_NOTHROW_MESSAGE(value = end[-1], indirect_addressing_message);
                CHECK_EQ(value, ground_truth);
                CHECK_THROWS_WITH_AS_MESSAGE(value = begin[1],
                                             check_index_exception_string,
                                             ::SoC::assert_failed_exception,
                                             check_index_message);
                CHECK_THROWS_WITH_AS_MESSAGE(value = end[0],
                                             check_index_exception_string,
                                             ::SoC::assert_failed_exception,
                                             check_index_message);
            }

            SUBCASE("operator->")
            {
                ::std::size_t value{};
                CHECK_NOTHROW_MESSAGE(value = begin->value, indirect_addressing_message);
                CHECK_EQ(value, ground_truth);
                CHECK_THROWS_WITH_AS_MESSAGE(value = end->value,
                                             check_index_exception_string,
                                             ::SoC::assert_failed_exception,
                                             check_index_message);
            }
        }

        const auto check_buffer_data{
            [&buffer, &table]
            {
                constexpr auto iota{::std::views::iota(0zu)};
                const auto reverse_table{table | ::std::views::reverse};

                auto begin{buffer.begin()};
                for(auto&& [i, value, ground_truth]: ::std::views::zip(iota, buffer, table))
                {
                    CAPTURE(i);
                    CHECK_EQ(value, ground_truth);
                    CHECK_EQ(begin[i], ground_truth);
                    CHECK_EQ((begin + i)->value, value);
                }

                auto&& const_buffer{::std::as_const(buffer)};
                auto cbegin{const_buffer.cbegin()};
                for(auto&& [i, value, ground_truth]: ::std::views::zip(iota, const_buffer, table))
                {
                    CAPTURE(i);
                    CHECK_EQ(value, ground_truth);
                    CHECK_EQ(cbegin[i], ground_truth);
                    CHECK_EQ((cbegin + i)->value, value);
                }

                auto rbegin{buffer.rbegin()};
                for(auto&& [i, value, ground_truth]: ::std::views::zip(iota, buffer | ::std::views::reverse, reverse_table))
                {
                    CAPTURE(i);
                    CHECK_EQ(value, ground_truth);
                    CHECK_EQ(rbegin[i], ground_truth);
                    CHECK_EQ((rbegin + i)->value, value);
                }

                auto crbegin{buffer.crbegin()};
                for(auto&& [i, value, ground_truth]: ::std::views::zip(iota, const_buffer | ::std::views::reverse, reverse_table))
                {
                    CAPTURE(i);
                    CHECK_EQ(value, ground_truth);
                    CHECK_EQ(crbegin[i], ground_truth);
                    CHECK_EQ((crbegin + i)->value, value);
                }
            }};

        SUBCASE("continuous data")
        {
            for(auto value: table) { buffer.emplace_back(value); }
            REQUIRE_EQ(buffer.size(), table.size());
            REQUIRE_EQ(buffer.head, 0);
            REQUIRE_EQ(buffer.tail, table.size());
            check_buffer_data();
        }

        SUBCASE("discontinuous data")
        {
            buffer.emplace_back();
            buffer.emplace_back();
            buffer.pop_front();
            buffer.pop_front();
            for(auto value: table) { buffer.emplace_back(value); }
            REQUIRE_EQ(buffer.size(), table.size());
            constexpr auto head_start{2zu};
            REQUIRE_EQ(buffer.head, head_start);
            REQUIRE_EQ(buffer.tail, head_start + table.size());
            check_buffer_data();
        }

        SUBCASE("wrap-around data")
        {
            constexpr auto head_start{-1zu - 2};
            buffer.head = head_start;
            buffer.tail = head_start;
            for(auto value: table) { buffer.emplace_back(value); }
            REQUIRE_EQ(buffer.size(), table.size());
            REQUIRE_EQ(buffer.head, head_start);
            REQUIRE_EQ(buffer.tail, head_start + table.size());
            check_buffer_data();
        }
    }

    /// @test 测试环形缓冲区的相等运算符
    REGISTER_TEST_CASE("operator==" * ::doctest::description{"测试环形缓冲区的相等运算符"})
    {
        ::ring_buffer_t buffer1{};
        ::ring_buffer_t buffer2{};
        CHECK_EQ(buffer1, buffer2);
        for(auto i: ::std::views::iota(0zu, buffer1.capacity()))
        {
            buffer1.emplace_back(i);
            buffer2.emplace_back(i);
            CHECK_EQ(buffer1, buffer2);
        }
        buffer2.front() = 100;
        CHECK_NE(buffer1, buffer2);
        buffer2.pop_front();
        CHECK_NE(buffer1, buffer2);
        buffer1.pop_front();
        CHECK_EQ(buffer1, buffer2);
    }

    /// @test 测试环形缓冲区的交换函数
    REGISTER_TEST_CASE("swap" * ::doctest::description{"测试环形缓冲区的交换函数"})
    {
        ::ring_buffer_t buffer1{};
        ::ring_buffer_t buffer1_gt{};
        ::ring_buffer_t buffer2{};
        ::ring_buffer_t buffer2_gt{};
        const auto check_self_swap{[&]
                                   {
                                       buffer1.swap(buffer1);
                                       buffer2.swap(buffer2);
                                       CHECK_EQ(buffer1, buffer1_gt);
                                       CHECK_EQ(buffer2, buffer2_gt);
                                   }};
        const auto check_swap{[&]
                              {
                                  buffer1.swap(buffer2);
                                  CHECK_EQ(buffer1, buffer2_gt);
                                  CHECK_EQ(buffer2, buffer1_gt);
                                  // 恢复原状
                                  buffer1.swap(buffer2);
                                  CHECK_EQ(buffer1, buffer1_gt);
                                  CHECK_EQ(buffer2, buffer2_gt);
                              }};

        SUBCASE("buffer in same size")
        {
            const auto fill_buffer{[&]
                                   {
                                       for(auto i: ::std::views::iota(0zu, buffer1.capacity()))
                                       {
                                           buffer1.emplace_back(i);
                                           // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
                                           buffer1_gt.emplace_back(i);
                                           buffer2.emplace_back(i + buffer1.capacity());
                                           buffer2_gt.emplace_back(i + buffer1.capacity());
                                       }
                                       // NOLINTNEXTLINE(clang-analyzer-core.NonNullParamChecker)
                                       REQUIRE_EQ(buffer1, buffer1_gt);
                                       REQUIRE_EQ(buffer2, buffer2_gt);
                                   }};

            SUBCASE("continuous data")
            {
                fill_buffer();

                SUBCASE("self swap") { check_self_swap(); }
                SUBCASE("swap") { check_swap(); }
            }

            SUBCASE("discontinuous data")
            {
                for(auto* buffer: {&buffer1, &buffer1_gt, &buffer2, &buffer2_gt})
                {
                    buffer->emplace_back();
                    buffer->pop_front();
                }
                fill_buffer();

                SUBCASE("self swap") { check_self_swap(); }
                SUBCASE("swap") { check_swap(); }
            }

            SUBCASE("wrap-around data")
            {
                constexpr auto head_start{-1zu - 2};
                for(auto* buffer: {&buffer1, &buffer1_gt, &buffer2, &buffer2_gt})
                {
                    buffer->head = head_start;
                    buffer->tail = head_start;
                }
                fill_buffer();
                for(auto* buffer: {&buffer1, &buffer1_gt, &buffer2, &buffer2_gt})
                {
                    REQUIRE_EQ(buffer->head, head_start);
                    REQUIRE_EQ(buffer->tail, head_start + buffer->capacity());
                }

                SUBCASE("self swap") { check_self_swap(); }
                SUBCASE("swap") { check_swap(); }
            }
        }

        SUBCASE("buffer in different size")
        {
            const auto fill_buffer{[&]
                                   {
                                       for(auto i: ::std::views::iota(0zu, buffer1.capacity()))
                                       {
                                           buffer1.emplace_back(i);
                                           // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
                                           buffer1_gt.emplace_back(i);
                                           buffer2.emplace_back(i + buffer1.capacity());
                                           buffer2_gt.emplace_back(i + buffer1.capacity());
                                       }
                                       // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
                                       buffer2.pop_front();
                                       buffer2_gt.pop_front();
                                       // NOLINTNEXTLINE(clang-analyzer-core.NonNullParamChecker)
                                       REQUIRE_EQ(buffer1, buffer1_gt);
                                       REQUIRE_EQ(buffer2, buffer2_gt);
                                   }};

            SUBCASE("continuous data")
            {
                fill_buffer();

                SUBCASE("self swap") { check_self_swap(); }
                SUBCASE("swap") { check_swap(); }
            }

            SUBCASE("discontinuous data")
            {
                for(auto* buffer: {&buffer1, &buffer1_gt, &buffer2, &buffer2_gt})
                {
                    buffer->emplace_back();
                    buffer->pop_front();
                }
                fill_buffer();

                SUBCASE("self swap") { check_self_swap(); }
                SUBCASE("swap") { check_swap(); }
            }

            SUBCASE("wrap-around data")
            {
                constexpr auto head_start{-1zu - 2};
                for(auto* buffer: {&buffer1, &buffer1_gt, &buffer2, &buffer2_gt})
                {
                    buffer->head = head_start;
                    buffer->tail = head_start;
                }
                fill_buffer();
                for(auto* buffer: {&buffer1, &buffer1_gt})
                {
                    REQUIRE_EQ(buffer->head, head_start);
                    REQUIRE_EQ(buffer->tail, head_start + buffer->capacity());
                }
                for(auto* buffer: {&buffer2, &buffer2_gt})
                {
                    // pop_front() 会使 head 增加 1
                    REQUIRE_EQ(buffer->head, head_start + 1);
                    REQUIRE_EQ(buffer->tail, head_start + buffer->capacity());
                }

                SUBCASE("self swap") { check_self_swap(); }
                SUBCASE("swap") { check_swap(); }
            }
        }
    }

    /// @test 测试环形缓冲区的构造函数、析构函数和赋值运算符
    REGISTER_TEST_CASE("constructor, destructor and assignment" *
                       ::doctest::description{"测试环形缓冲区的构造函数、析构函数和赋值运算符"})
    {
        SUBCASE("destructor")
        {
            ::test_struct::reset();
            REQUIRE_EQ(::test_struct::ctor_cnt, 0);
            REQUIRE_EQ(::test_struct::dtor_cnt, 0);

            {
                ::ring_buffer_t buffer{};
                buffer.emplace_back(1zu);
                buffer.emplace_back(2zu);
                buffer.emplace_back(3zu);

                CHECK_EQ(::test_struct::ctor_cnt, 3);
                CHECK_EQ(::test_struct::dtor_cnt, 0);
                CHECK_EQ(::test_struct::copy_ctor_cnt, 0);
                CHECK_EQ(::test_struct::move_ctor_cnt, 0);

                // 测试显式弹出元素时的析构
                buffer.pop_front();
                CHECK_EQ(::test_struct::ctor_cnt, 3);
                CHECK_EQ(::test_struct::dtor_cnt, 1);
                CHECK_EQ(::test_struct::copy_ctor_cnt, 0);
                CHECK_EQ(::test_struct::move_ctor_cnt, 0);
            }

            // 测试缓冲区析构时的元素析构
            CHECK_EQ(::test_struct::ctor_cnt, 3);
            CHECK_EQ(::test_struct::dtor_cnt, 3);
            CHECK_EQ(::test_struct::copy_ctor_cnt, 0);
            CHECK_EQ(::test_struct::move_ctor_cnt, 0);
        }

        auto buffer_ptr{::std::make_unique<::ring_buffer_t>()};
        ::ring_buffer_t& buffer{*buffer_ptr};
        ::ring_buffer_t buffer_gt{};
        const auto fill_data(
            [&]
            {
                for(auto i: ::std::views::iota(0zu, buffer.capacity()))
                {
                    buffer.emplace_back(i);
                    buffer_gt.emplace_back(i);
                }
                REQUIRE_EQ(buffer, buffer_gt);
            });
        const auto check_copy_ctor{[&]
                                   {
                                       ::test_struct::reset();
                                       ::ring_buffer_t copied_buffer{buffer};
                                       // NOLINTNEXTLINE(clang-analyzer-core.NonNullParamChecker)
                                       CHECK_EQ(copied_buffer, buffer_gt);
                                       CHECK_EQ(::test_struct::ctor_cnt, 0);
                                       CHECK_EQ(::test_struct::dtor_cnt, 0);
                                       CHECK_EQ(::test_struct::copy_ctor_cnt, 4);
                                       CHECK_EQ(::test_struct::move_ctor_cnt, 0);
                                       ::test_struct::reset();
                                       buffer_ptr.reset();
                                       CHECK_EQ(::test_struct::ctor_cnt, 0);
                                       CHECK_EQ(::test_struct::dtor_cnt, 4);
                                       CHECK_EQ(::test_struct::copy_ctor_cnt, 0);
                                       CHECK_EQ(::test_struct::move_ctor_cnt, 0);
                                   }};
        const auto check_move_ctor{[&]
                                   {
                                       ::test_struct::reset();
                                       ::ring_buffer_t moved_buffer{::std::move(buffer)};
                                       // NOLINTNEXTLINE(clang-analyzer-core.NonNullParamChecker)
                                       CHECK_EQ(moved_buffer, buffer_gt);
                                       CHECK_EQ(::test_struct::ctor_cnt, 0);
                                       CHECK_EQ(::test_struct::dtor_cnt, 0);
                                       CHECK_EQ(::test_struct::copy_ctor_cnt, 0);
                                       CHECK_EQ(::test_struct::move_ctor_cnt, 4);
                                       ::test_struct::reset();
                                       buffer_ptr.reset();
                                       CHECK_EQ(::test_struct::ctor_cnt, 0);
                                       CHECK_EQ(::test_struct::dtor_cnt, 0);
                                       CHECK_EQ(::test_struct::copy_ctor_cnt, 0);
                                       CHECK_EQ(::test_struct::move_ctor_cnt, 0);
                                   }};
        const auto check_copy_assign{[&]
                                     {
                                         ::test_struct::reset();
                                         buffer = buffer_gt;
                                         CHECK_EQ(buffer, buffer_gt);
                                     }};
        const auto check_move_assign{[&]
                                     {
                                         ::test_struct::reset();
                                         buffer = auto{buffer_gt};
                                         CHECK_EQ(buffer, buffer_gt);
                                     }};

        SUBCASE("continuous data")
        {
            fill_data();
            SUBCASE("copy constructor") { check_copy_ctor(); }
            SUBCASE("move constructor") { check_move_ctor(); }
            SUBCASE("copy assignment") { check_copy_assign(); }
            SUBCASE("move assignment") { check_move_assign(); }
        }

        SUBCASE("discontinuous data")
        {
            buffer.emplace_back();
            buffer.pop_front();
            buffer_gt.emplace_back();
            buffer_gt.pop_front();
            fill_data();
            SUBCASE("copy constructor") { check_copy_ctor(); }
            SUBCASE("move constructor") { check_move_ctor(); }
            SUBCASE("copy assignment") { check_copy_assign(); }
            SUBCASE("move assignment") { check_move_assign(); }
        }

        SUBCASE("wrap-around data")
        {
            constexpr auto head_start{-1zu - 2};
            buffer.head = head_start;
            buffer.tail = head_start;
            buffer_gt.head = head_start;
            buffer_gt.tail = head_start;
            fill_data();
            SUBCASE("copy constructor") { check_copy_ctor(); }
            SUBCASE("move constructor") { check_move_ctor(); }
            SUBCASE("copy assignment") { check_copy_assign(); }
            SUBCASE("move assignment") { check_move_assign(); }
        }
    }
}
