/**
 * @file fmt.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 格式化字符串实现
 */

export module SoC.freestanding:fmt;
import SoC.std;

export namespace SoC
{
    /**
     * @brief 将字符串字面量包装为可用于非类型模板形参的类型
     *
     * @tparam n 字面量长度
     */
    template <::std::size_t n>
    struct fmt_string
    {
        static_assert(n != 1zu, "格式串不能为空");
        ::std::array<char, n - 1> buffer;

        // NOLINTNEXTLINE(*-avoid-c-arrays)
        constexpr inline fmt_string(const char (&string)[n]) noexcept { ::std::ranges::copy_n(string, n - 1, buffer.begin()); }

        constexpr inline auto begin() const noexcept { return buffer.cbegin(); }

        constexpr inline auto end() const noexcept { return buffer.cend(); }

        constexpr inline auto size() const noexcept { return n - 1; }

        constexpr inline char operator[] (::std::size_t i) const noexcept { return buffer[i]; }

        constexpr inline operator ::std::string_view () const noexcept { return {begin(), end()}; }
    };

    namespace test
    {
        /// @see ::SoC::fmt_string
        extern "C++" template <::SoC::fmt_string fmt>
        struct fmt_parser;
    }  // namespace test

    /**
     * @brief 编译时格式串解析器
     *
     * @tparam fmt 格式串
     */
    template <::SoC::fmt_string fmt>
    struct fmt_parser
    {
    private:
        template <::SoC::fmt_string>
        friend struct ::SoC::test::fmt_parser;

        /**
         * @brief 词法分析使用的括号分类
         *
         */
        enum class brace_t : ::std::uint8_t
        {
            /// 占位符
            placehold,
            /// 单个左括号
            single_left_brace,
            /// 单个右括号
            single_right_brace,
        };

        /// 占位符列表，元组为指向占位符中{的指针
        using placehold_list_t = ::std::vector<const char*>;
        /// 可空的占位符列表，语法分析失败时无值
        using optional_placehold_list_t = ::std::optional<placehold_list_t>;
        /// 可空的数组大小，语法分析失败时无值
        using optional_size_t = ::std::optional<::std::size_t>;
        /// 打包的字符串视图和转义后字符数组大小
        using string_and_size_t = ::std::pair<::std::string_view, ::std::size_t>;

        /**
         * @brief 获取占位符列表
         *
         * @return 词法分析成功时为占位符列表
         */
        constexpr inline static optional_placehold_list_t get_placehold_list() noexcept
        {
            ::std::vector<brace_t> brace_stack{brace_t::placehold};
            placehold_list_t placehold_list{};

            for(auto&& ch: fmt)
            {
                auto&& back{brace_stack.back()};
                if(ch == '{')
                {
                    if(back == brace_t::single_left_brace)
                    {
                        brace_stack.pop_back();
                        placehold_list.pop_back();
                    }
                    else
                    {
                        brace_stack.push_back(brace_t::single_left_brace);
                        placehold_list.push_back(&ch);
                    }
                }
                else if(ch == '}')
                {
                    switch(back)
                    {
                        case brace_t::single_left_brace:
                        case brace_t::single_right_brace: brace_stack.pop_back(); break;
                        case brace_t::placehold: brace_stack.push_back(brace_t::single_right_brace); break;
                        default: break;
                    }
                }
                else
                {
                    switch(back)
                    {
                        case brace_t::single_left_brace:
                        case brace_t::single_right_brace: return ::std::nullopt;
                        default: break;
                    }
                }
            }
            return placehold_list;
        }

        /**
         * @brief 获取字符串视图和转义后字符数组大小
         *
         * @param string 字符串视图
         * @return 字符串视图和转义后字符数组大小
         */
        constexpr inline static string_and_size_t get_string_and_size(::std::string_view string) noexcept
        {
            auto size{string.size()};
            for(auto i{0zu}; i < string.size() - 1; ++i)
            {
                if(auto ch{string[i]}; ch == string[i + 1] && (ch == '{' || ch == '}'))
                {
                    --size;
                    ++i;
                }
            }
            return {string, size};
        }

        /**
         * @brief 获取储存词法分析结果所需的数组大小
         *
         * @return 词法分析成功时为数组大小
         */
        constexpr inline static optional_size_t get_split_string_and_size_array_size() noexcept
        {
            return get_placehold_list().and_then(
                [](optional_placehold_list_t optional) static constexpr noexcept -> optional_size_t
                {
                    ::std::span placehold_list{*optional};
                    if(placehold_list.empty()) { return fmt.size() == 0 ? 0 : 1; }

                    const auto *ptr{placehold_list.front()};
                    ::std::size_t cnt{ptr != fmt.begin()};
                    for(const auto* placehold: placehold_list.subspan(1))
                    {
                        if(ptr + 2 != placehold) { ++cnt; }
                        ptr = placehold;
                    }
                    return cnt + (placehold_list.back() + 2 < fmt.end());
                });
        }

        /**
         * @brief 获取占位符的数量
         *
         * @return 词法分析成功时为占位符数量
         */
        constexpr inline static optional_size_t get_placehold_num_impl() noexcept
        {
            return get_placehold_list().and_then(
                [](optional_placehold_list_t optional) static constexpr noexcept -> optional_size_t { return optional->size(); });
        }

        /**
         * @brief 获取储存词法分析结果所需的数组大小
         *
         * @return 词法分析成功时为数组大小
         */
        constexpr inline static auto get_is_placehold_array() noexcept
        {
            return get_placehold_list().and_then(
                [](optional_placehold_list_t optional) static constexpr noexcept
                {
                    constexpr auto size{*get_split_string_and_size_array_size() + *get_placehold_num_impl()};
                    ::std::array<bool, size> array{};
                    ::std::span placehold_list{*optional};
                    if(placehold_list.empty())
                    {
                        array.front() = false;
                        return ::std::optional{array};
                    }
                    auto i{0zu};
                    const auto *ptr{placehold_list.front()};
                    if(ptr == fmt.begin()) { array[i++] = true; }
                    else
                    {
                        array[i++] = false;
                        array[i++] = true;
                    }
                    for(const auto* placehold: placehold_list.subspan(1))
                    {
                        if(ptr + 2 != placehold)
                        {
                            array[i++] = false;
                            array[i++] = true;
                        }
                        else
                        {
                            array[i++] = true;
                        }
                        ptr = placehold;
                    }
                    array.back() = placehold_list.back() + 2 >= fmt.end();
                    return ::std::optional{array};
                });
        }

        /**
         * @brief 获取字符串视图和转义后字符数组大小组成的数组
         *
         * @return std::array{字符串视图和转义后字符数组大小, 不含占位符的字符串个数}
         */
        constexpr inline static auto get_split_string_and_size_array() noexcept
        {
            ::std::array<string_and_size_t, *get_split_string_and_size_array_size()> buffer{};
            auto placehold_list{*get_placehold_list()};
            ::std::span placehold_view{placehold_list};
            if(placehold_view.empty())
            {
                buffer[0] = get_string_and_size({fmt.begin(), fmt.end()});
                return buffer;
            }
            auto ptr{fmt.begin()};
            if(ptr == placehold_view.front()) { placehold_view = placehold_view.subspan(1); }
            auto i{0zu};
            for(const auto* placehold: placehold_view)
            {
                if(ptr == fmt.begin())
                {
                    buffer[i++] = get_string_and_size({ptr == placehold_list.front() ? ptr + 2 : ptr, placehold});
                }
                else if(ptr + 2 != placehold) { buffer[i++] = get_string_and_size({ptr + 2, placehold}); }
                ptr = placehold;
            }
            if(i != buffer.size()) { buffer[i] = get_string_and_size({ptr + 2, fmt.end()}); }
            return buffer;
        }

        /**
         * @brief 获取转义后的字符数组
         *
         * @tparam size 转义后字符数组的大小
         * @param string 字符串视图
         * @return 字符数组
         */
        template <::std::size_t size>
        constexpr inline static ::std::array<char, size> get_split_string(::std::string_view string) noexcept
        {
            ::std::array<char, size> buffer{};
            auto buffer_index{0zu};
            for(auto i{0zu}; i < string.size() - 1; ++i)
            {
                auto ch{string[i]};
                buffer[buffer_index++] = ch;
                if(ch == string[i + 1] && (ch == '{' || ch == '}')) { ++i; }
            }
            if(buffer_index != size) { buffer[buffer_index] = string.back(); }
            return buffer;
        }

        /**
         * @brief 获取不含占位符的字符串数组构成的元组
         *
         * @tparam indexes 索引包
         * @return 不含占位符的字符串数组构成的元组
         */
        template <::std::size_t... indexes>
        constexpr inline static auto get_split_string_tuple_impl(::std::index_sequence<indexes...>) noexcept
        {
            constexpr auto string_and_size_array{get_split_string_and_size_array()};
            return ::std::tuple{get_split_string<::std::get<indexes>(string_and_size_array).second>(
                ::std::get<indexes>(string_and_size_array).first)...};
        }

    public:
        /**
         * @brief 获取占位符的数量
         *
         * @return 占位符数量
         */
        constexpr inline static ::std::size_t get_placehold_num() noexcept
        {
            constexpr auto placehold_num{get_placehold_num_impl()};
            static_assert(placehold_num, "格式串词法错误");
            return *placehold_num;
        }

        /**
         * @brief 获取不含占位符的字符串的个数
         *
         * @return 字符串个数
         */
        constexpr inline static ::std::size_t get_no_placehold_num() noexcept
        {
            constexpr auto size{get_split_string_and_size_array_size()};
            static_assert(size, "格式串词法错误");
            return *size;
        }

        /**
         * @brief 获取占位符和不含占位符的字符串的总数
         *
         * @return 元素总数
         */
        constexpr inline static ::std::size_t get_total_num() noexcept { return get_no_placehold_num() + get_placehold_num(); }

        /**
         * @brief 获取不含占位符的字符串构成的元组
         *
         * @return 获取不含占位符的字符串构成的元组，纯占位符格式串时为void
         */
        constexpr inline static auto get_split_string_tuple() noexcept
        {
            if constexpr(constexpr auto size{get_no_placehold_num()}; size != 0)
            {
                return get_split_string_tuple_impl(::std::make_index_sequence<size>{});
            }
            else
            {
                return;
            }
        }

        /**
         * @brief 获取元组中的索引数组
         *
         * @note 相当于获取由std::tuple_cat(split_string_tuple, args)拼成的新元组中的索引，其中args为占位符的实际参数构成的元组
         * @return 索引数组
         */
        constexpr inline static auto get_tuple_index_array() noexcept
        {
            constexpr auto is_placehold_array{*get_is_placehold_array()};
            auto split_tuple_index{0zu};
            auto placehold_tuple_index{get_no_placehold_num()};
            ::std::array<::std::size_t, is_placehold_array.size()> result{};
            for(auto&& [is_placehold, res]: ::std::views::zip(is_placehold_array, result))
            {
                res = is_placehold ? placehold_tuple_index++ : split_tuple_index++;
            }
            return result;
        }

        /**
         * @brief 获取原始格式串
         *
         * @return 格式串
         */
        constexpr inline static auto get_fmt_string() noexcept { return fmt; }
    };

    namespace literal
    {
        /**
         * @brief 将字符串字面量转化为格式串解析器
         *
         * @tparam fmt_string 包装成格式串的字符串字面量
         * @return 格式串解析器
         */
        template <::SoC::fmt_string fmt_string>
        inline consteval auto operator""_fmt () noexcept
        {
            return ::SoC::fmt_parser<fmt_string>{};
        }
    }  // namespace literal
}  // namespace SoC

namespace SoC::detail
{
    template <typename type>
    constexpr inline bool is_fmt_parser_impl{false};

    template <::SoC::fmt_string fmt_string>
    constexpr inline bool is_fmt_parser_impl<::SoC::fmt_parser<fmt_string>>{true};

    export template <typename type>
    concept is_fmt_parser = ::SoC::detail::is_fmt_parser_impl<type>;

    /**
     * @brief 带格式的浮点数包装体
     *
     * @tparam type 浮点类型
     */
    template <::std::floating_point type>
    struct floating_point_format
    {
        type value;
        ::std::chars_format format;
        ::std::size_t precision;
    };

    /**
     * @brief 整数进制
     *
     */
    enum class integer_base : ::std::uint8_t
    {
        bin = 2,
        oct = 8,
        hex = 16
    };

    template <::std::integral type, ::SoC::detail::integer_base base_v>
    struct integer_format
    {
        type value;
        constexpr inline static auto base{base_v};
    };
}  // namespace SoC::detail

export namespace SoC
{
    /// 二进制
    constexpr auto integer_base2{::SoC::detail::integer_base::bin};
    /// 八进制
    constexpr auto integer_base8{::SoC::detail::integer_base::oct};
    /// 十六进制
    constexpr auto integer_base16{::SoC::detail::integer_base::hex};

    /**
     * @brief 带格式的浮点数
     *
     * @param value 浮点数
     * @param format 浮点格式
     * @param precision 精度
     * @return 包装类型
     */
    constexpr inline auto format(::std::floating_point auto value, ::std::chars_format format, ::std::size_t precision) noexcept
    {
        return ::SoC::detail::floating_point_format{value, format, precision};
    }

    /**
     * @brief 带格式的浮点数
     *
     * @param value 浮点数
     * @param format 浮点格式
     * @return 包装类型
     */
    constexpr inline auto format(::std::floating_point auto value, ::std::chars_format format) noexcept
    {
        return ::SoC::detail::floating_point_format{value, format, 6};
    }

    /**
     * @brief 带格式的浮点数
     *
     * @param value 浮点数
     * @param precision 精度
     * @return 包装类型
     */
    constexpr inline auto format(::std::floating_point auto value, ::std::size_t precision) noexcept
    {
        return ::SoC::detail::floating_point_format{value, ::std::chars_format::general, precision};
    }

    /**
     * @brief 带格式的整数
     *
     * @tparam base 进制
     * @param value 整数
     * @return 包装类型
     */
    template <::SoC::detail::integer_base base>
    constexpr inline auto format(::std::integral auto value) noexcept
    {
        return ::SoC::detail::integer_format<decltype(value), base>{value};
    }
}  // namespace SoC
