#pragma once
#include "allocator.hpp"
#include "fmt.hpp"

/**
 * @brief 基本概念和函数定义
 *
 */
namespace SoC
{
    /**
     * @brief 判断device_t是否是type类型的输出设备，要求满足：
     * - auto device_t::write(const type*, const type*) noexcept，或
     * - auto write(device_t&, const type*, const type*) noexcept，考虑adl
     * @tparam device_t 设备类型
     * @tparam type 输出类型
     */
    template <typename device_t, typename type>
    concept is_output_device =
        ::SoC::detail::is_io_target_type<type> && (requires(device_t& dev, const type* begin, const type* end) {
            { dev.write(begin, end) } noexcept;
        } || requires(device_t& dev, const type* begin, const type* end) {
            { write(dev, begin, end) } noexcept;
        });

    /**
     * @brief 判断device_t是否是一种输出设备，要求满足：
     * - SoC::is_output_device<device_t, char>，或
     * - SoC::is_output_device<device_t, std::byte>
     * @tparam device_t 设备类型
     */
    template <typename device_t>
    concept is_general_output_device = ::SoC::is_output_device<device_t, char> || ::SoC::is_output_device<device_t, ::std::byte>;

    /**
     * @brief 判断device_t是否是type类型的输入设备，要求满足：
     * - type* device_t::read(type*, type*) noexcept，或
     * - type* read(device_t&, type*, type*) noexcept，考虑adl
     * @tparam device_t 设备类型
     * @tparam type 输入类型
     */
    template <typename device_t, typename type>
    concept is_input_device =
        ::SoC::detail::is_io_target_type<type> && (requires(device_t& dev, type* begin, type* end) {
            { dev.read(begin, end) } noexcept -> ::std::same_as<type*>;
        } || requires(device_t& dev, type* begin, type* end) {
            { read(dev, begin, end) } noexcept -> ::std::same_as<type*>;
        });

    /**
     * @brief 判断device_t是否是一种输入设备，要求满足：
     * - SoC::is_input_device<device_t, char>，或
     * - SoC::is_input_device<device_t, std::byte>
     * @tparam device_t 设备类型
     */
    template <typename device_t>
    concept is_general_input_device = ::SoC::is_input_device<device_t, char> || ::SoC::is_input_device<device_t, ::std::byte>;

    /**
     * @brief 判断device_t是否是一种设备
     * - SoC::is_general_output_device<device_t>，或
     * - SoC::is_general_input_device<device_t>
     * @tparam device_t 设备类型
     */
    template <typename device_t>
    concept is_general_device = ::SoC::is_general_output_device<device_t> || ::SoC::is_general_input_device<device_t>;

    /**
     * @brief 判断device_t是否具有就绪标记，要求满足：
     * - bool device_t.is_ready() noexcept，或
     * - bool is_ready(device_t&) noexcept，考虑adl
     * @tparam device_t 设备类型
     */
    template <typename device_t>
    concept has_ready_flag_device = ::SoC::is_general_device<device_t> && (requires(device_t& dev) {
        { dev.is_ready() } noexcept -> ::std::same_as<bool>;
    } || requires(device_t& dev) {
        { is_ready(dev) } noexcept -> ::std::same_as<bool>;
    });

    /**
     * @brief 轮询等待直到设备就绪
     *
     * @tparam device_t 设备类型
     * @param device 设备对象
     */
    template <::SoC::is_general_device device_t>
    constexpr inline void wait_until_device_ready(device_t& device) noexcept
    {
        if constexpr(::SoC::has_ready_flag_device<device_t>)
        {
            if constexpr(requires() { device.is_ready(); })
            {
                ::SoC::wait_until([&device] noexcept { return device.is_ready(); });
            }
            else
            {
                ::SoC::wait_until([&device] noexcept { is_ready(device); });
            }
        }
    }

    /**
     * @brief 将[begin, end)内的数据写入到device
     *
     * @tparam type 输出类型
     * @param device 输出设备
     * @param begin 缓冲区首指针
     * @param end 缓冲区尾哨位
     */
    template <::SoC::detail::is_io_target_type type>
    constexpr inline void write_to_device(::SoC::is_output_device<type> auto& device, const type* begin, const type* end) noexcept
    {
        if constexpr(requires { device.write(begin, end); }) { device.write(begin, end); }
        else
        {
            write(device, begin, end);
        }
    }

}  // namespace SoC

/**
 * @brief 缓冲区定义
 *
 */
namespace SoC
{
    namespace detail
    {
        /**
         * @brief 判断type是否是缓冲区分配器，要求满足：
         * - 满足SoC::is_allocator概念，或
         * - 是void类型，用于静态缓冲区
         * @tparam type 要判断的类型
         */
        template <typename type>
        concept is_buffer_allocator = ::SoC::is_allocator<type> || ::std::same_as<type, void>;

        /**
         * @brief 获取缓冲区对齐
         *
         * @param buffer_size 缓冲区大小4
         * @return 缓冲区对齐
         */
        inline consteval ::std::size_t get_buffer_align(::std::size_t buffer_size) noexcept
        {
            if(buffer_size % 16 == 0) { return 16; }
            else if(buffer_size % 8 == 0) { return 8; }
            else
            {
                return 4;
            }
        }

        template <::SoC::detail::is_io_target_type type,
                  ::std::size_t buffer_size,
                  ::SoC::detail::is_buffer_allocator allocator_type>
        struct buffer_impl
        {
            using value_type = type;
            using pointer = value_type*;
            using allocator_t = allocator_type;

            /// 分配器
            [[no_unique_address]] allocator_t allocator;
            /// 缓冲区首指针
            const pointer begin;

            constexpr inline buffer_impl(allocator_t alloc = allocator_t{}) noexcept :
                allocator{alloc}, begin{allocator.template allocate<value_type>(buffer_size).ptr}
            {
            }

            constexpr inline ~buffer_impl() noexcept { allocator.deallocate(begin, buffer_size); }
        };

        template <::SoC::detail::is_io_target_type type, ::std::size_t buffer_size>
        struct buffer_impl<type, buffer_size, void>
        {
            using value_type = type;
            using pointer = value_type*;
            using allocator_t = void;

            /// 缓冲区数组，可退化为首指针
            alignas(::SoC::detail::get_buffer_align(buffer_size)) value_type begin[buffer_size];

            constexpr inline buffer_impl() noexcept {}
        };
    }  // namespace detail

    /**
     * @brief 缓冲区类型
     *
     * @tparam type io目标类型
     * @tparam buffer_size 缓冲区大小，要求为4字节的整数倍
     * @tparam allocator_t 分配器类型，void为静态缓冲区，满足SoC::is_allocator概念的分配器类型为动态缓冲区
     */
    template <::SoC::detail::is_io_target_type type, ::std::size_t buffer_size, ::SoC::detail::is_buffer_allocator allocator_type>
        requires (buffer_size != 0 && buffer_size % 4 == 0)
    struct buffer : ::SoC::detail::buffer_impl<type, buffer_size, allocator_type>
    {
        using ::SoC::detail::buffer_impl<type, buffer_size, allocator_type>::begin;
        using value_type = type;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using allocator_t = allocator_type;

        /// 缓冲区当前游标
        pointer current{begin};
        /// 有效输入缓冲区的尾哨位
        pointer end{begin};

        constexpr inline buffer() noexcept {}

        /**
         * @brief 获取缓冲区尾哨位
         *
         * @return 缓冲区尾哨位
         */
        constexpr inline pointer get_buffer_end() noexcept { return begin + buffer_size; }

        /**
         * @brief 获取输出缓冲区剩余容量
         *
         * @return 输出缓冲区剩余容量
         */
        constexpr inline ::std::size_t get_obuffer_left() noexcept { return get_buffer_end() - current; }

        /**
         * @brief 获取输入缓冲区剩余容量
         *
         * @return 输入缓冲区剩余容量
         */
        constexpr inline ::std::size_t get_ibuffer_left() noexcept { return end - current; }

        /**
         * @brief 将游标向前移动len个元素
         *
         * @param len 要前进的距离
         * @return 游标的旧值
         */
        constexpr inline pointer advance(::std::size_t len) noexcept { return ::std::exchange(current, current + len); }

        /**
         * @brief 将src处的len个元素写入缓冲区
         *
         * @param src 源指针
         * @param len 要写入的元素个数
         */
        constexpr inline void write(const_pointer src, ::std::size_t len) noexcept
        {
            ::std::memcpy(current, src, len);
            current += len;
        }

        /**
         * @brief 将缓冲区中的len个元素读取到dst处
         *
         * @param dst 目标指针
         * @param len 要读取的元素个数
         */
        constexpr inline void read(pointer dst, ::std::size_t len) noexcept
        {
            ::std::memcpy(dst, current, len);
            current += len;
        }

        /**
         * @brief 清空缓冲区
         *
         */
        constexpr inline void clear() noexcept
        {
            current = begin;
            end = begin;
        }
    };

    /**
     * @brief 静态缓冲区
     *
     * @tparam type io元素类型
     * @tparam buffer_size 缓冲区容量
     */
    template <::SoC::detail::is_io_target_type type, ::std::size_t buffer_size = 64>
    using static_buffer = ::SoC::buffer<type, buffer_size, void>;
    /**
     * @brief 动态缓冲区
     *
     * @tparam type io元素类型
     * @tparam allocator 分配器
     * @tparam buffer_size 缓冲区容量
     */
    template <::SoC::detail::is_io_target_type type, ::SoC::is_allocator allocator, ::std::size_t buffer_size = 64>
    using dynamic_buffer = ::SoC::buffer<type, buffer_size, allocator>;
    /**
     * @brief 默认缓冲区类型，即静态缓冲区
     *
     * @tparam type io元素类型
     */
    template <::SoC::detail::is_io_target_type type>
    using default_buffer = ::SoC::static_buffer<type>;

    /**
     * @brief 判断type是否是一个缓冲区，要求满足：
     * - 类型别名value_type，且
     * - 类型别名pointer，且
     * - 类型别名const_pointer，且
     * - 类型别名allocator_t，
     * - pointer type::current，且
     * - pointer type::end，且
     * - pointer type::get_buffer_end() noexcept，且
     * - std::size_t type::get_obuffer_left() noexcept，且
     * - std::size_t type::get_ibuffer_left() noexcept，且
     * - pointer type::advance(std::size_t) noexcept，且
     * - void type::write(const_pointer, std::size_t) noexcept，且
     * - void type::read(pointer, std::size_t) noexcept，且
     * - void type::clear() noexcept
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_buffer = requires(type& buffer) {
        typename type::value_type;
        typename type::pointer;
        typename type::const_pointer;
        typename type::allocator_t;

        requires ::std::same_as<decltype(buffer.current), typename type::pointer>;
        requires ::std::same_as<decltype(buffer.end), typename type::pointer>;

        { buffer.get_buffer_end() } noexcept -> ::std::same_as<typename type::pointer>;
        { buffer.get_obuffer_left() } noexcept -> ::std::same_as<::std::size_t>;
        { buffer.get_ibuffer_left() } noexcept -> ::std::same_as<::std::size_t>;
        { buffer.advance(1zu) } noexcept -> ::std::same_as<typename type::pointer>;
        { buffer.write(typename type::const_pointer{}, 1zu) } noexcept -> ::std::same_as<void>;
        { buffer.read(typename type::pointer{}, 1zu) } noexcept -> ::std::same_as<void>;
        { buffer.clear() } noexcept -> ::std::same_as<void>;
    };
}  // namespace SoC

/**
 * @brief 打印支持
 *
 */
namespace SoC
{
    /**
     * @brief 将字符串视图输出到device
     *
     * @param device 输出设备
     * @param string 要输出的字符串视图
     */
    constexpr inline void do_print_arg(::SoC::is_output_device<char> auto& device, ::std::string_view string) noexcept
    {
        ::SoC::write_to_device(device, string.begin(), string.end());
    }

    /**
     * @brief 将C风格字符串输出到device
     *
     * @param device 输出设备
     * @param string 要输出的C风格字符串
     */
    constexpr inline void do_print_arg(::SoC::is_output_device<char> auto& device, const char* string) noexcept
    {
        ::SoC::do_print_arg(device, ::std::string_view{string});
    }

    // 统一预分配的文本缓冲区类型
    using unified_text_buffer = ::std::ranges::subrange<char*>;

    // 统一预分配的二进制缓冲区类型
    using unified_bin_buffer = ::std::ranges::subrange<::std::byte*>;

    namespace detail
    {
        /**
         * @brief 获取输出num_t所需的最大缓冲区大小
         *
         * @tparam num_t 整数或浮点类型
         * @return 最大缓冲区大小
         */
        template <::SoC::detail::is_int_fp num_t>
        consteval inline auto get_max_text_buffer_size() noexcept
        {
            using limit_t = ::std::numeric_limits<num_t>;
            constexpr auto digits{::std::floating_point<num_t> ? limit_t::max_digits10 : limit_t::digits10};
            if constexpr(::std::signed_integral<num_t>)
            {
                // 符号占1字符
                return digits + 1;
            }
            else if(::std::floating_point<num_t>)
            {
                // 符号和小数点占2字符
                return digits + 2;
            }
            else
            {
                return digits;
            }
        }

        template <::std::integral num_t, ::SoC::detail::integer_base base>
        consteval inline auto get_max_text_buffer_size() noexcept
        {
            constexpr auto bits{sizeof(num_t) * ::std::numeric_limits<char>::digits};
            if constexpr(base == ::SoC::integer_base8)
            {
                constexpr auto is_signed{::std::signed_integral<num_t>};
                constexpr auto significant_bits{bits - is_signed};
                // 3位有效二进制对应1位八进制，符号占1字符，0o前缀占2字符
                return (significant_bits + 2) / 3 + is_signed + 2;
            }
            else if constexpr(base == ::SoC::integer_base16)
            {
                // 4位二进制对应1位十六进制，0x前缀占2字符
                return bits / 4 + 2;
            }
            else if constexpr(base == ::SoC::integer_base2)
            {
                // 0b前缀占2字符，符号位同样占用1字符，因此为位数+2
                return bits + 2;
            }
        }

        /**
         * @brief 将进制前缀写入缓冲区
         *
         * @tparam base 进制
         * @param buffer 缓冲区
         * @return char* 写入后的缓冲区指针
         */
        template <::SoC::detail::integer_base base>
        inline char* write_integer_base_prefix(char* buffer) noexcept
        {
            if constexpr(base == ::SoC::integer_base8) { ::std::memcpy(buffer, "0o", 2); }
            else if constexpr(base == ::SoC::integer_base2) { ::std::memcpy(buffer, "0b", 2); }
            else if constexpr(base == ::SoC::integer_base16) { ::std::memcpy(buffer, "0x", 2); }
            return buffer + 2;
        }
    }  // namespace detail

    /**
     * @brief 将类型type表示为文本需要的最大io缓冲区大小
     *
     * @tparam type 要处理的类型
     */
    template <typename type>
    constexpr inline ::std::size_t max_text_buffer_size{0zu};

    template <::SoC::detail::is_int_fp type>
    constexpr inline ::std::size_t max_text_buffer_size<type>{::SoC::detail::get_max_text_buffer_size<type>()};

    template <::std::floating_point type>
    constexpr inline ::std::size_t max_text_buffer_size<::SoC::detail::floating_point_format<type>>{
        ::SoC::detail::get_max_text_buffer_size<type>()};

    template <::std::integral type, ::SoC::detail::integer_base base>
    constexpr inline ::std::size_t max_text_buffer_size<::SoC::detail::integer_format<type, base>>{
        ::SoC::detail::get_max_text_buffer_size<type, base>()};

    /**
     * @brief 判断类型type是否具有最大io缓冲区大小
     *
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept has_max_text_buffer_size = ::SoC::max_text_buffer_size<::std::remove_cvref_t<type>> != 0;

    /**
     * @brief 判断arg_t类型的对象能否输出到output_t类型的对象中，要求满足：
     * - arg_t不满足概念has_max_text_buffer_size，且
     * - void do_print_arg(output_t& output, arg_t arg), 考虑adl
     * @tparam arg_t 要判断的类型
     * @tparam output_t 可进行输出操作的类型
     */
    template <typename arg_t, typename output_t>
    concept is_no_max_text_buffer_size_printable =
        !::SoC::has_max_text_buffer_size<arg_t> && requires(output_t& output, arg_t arg) {
            { do_print_arg(output, arg) } noexcept -> ::std::same_as<void>;
        };

    /**
     * @brief 判断arg_t类型的对象能否在使用预分配缓冲区的情况下输出到output_t类型的对象中
     * - arg_t满足概念has_max_text_buffer_size，且
     * - void do_print_arg(output_t& output, arg_t arg), 考虑adl
     * @tparam arg_t 要判断的类型
     * @tparam output_t 可进行输出操作的类型
     */
    template <typename arg_t, typename output_t>
    concept is_has_max_text_buffer_size_printable =
        ::SoC::has_max_text_buffer_size<arg_t> && requires(output_t& output, arg_t arg, ::SoC::unified_text_buffer buffer) {
            { do_print_arg(output, arg, buffer) } noexcept -> ::std::same_as<void>;
        };

    /**
     * @brief 将num输出到设备
     *
     * @param device 输出设备
     * @param num 要输出的整数或浮点数
     * @param buffer 输出缓冲区
     */
    constexpr inline void do_print_arg(::SoC::is_output_device<char> auto& device,
                                       ::SoC::detail::is_int_fp auto num,
                                       ::SoC::unified_text_buffer buffer) noexcept
    {
        ::SoC::write_to_device(device, buffer.begin(), ::std::to_chars(buffer.begin(), buffer.end(), num).ptr);
    }

    /**
     * @brief 将带格式的浮点数输出到设备
     *
     * @param device 输出设备
     * @param format_wrapper 带格式的浮点数包装对象
     * @param buffer 输出缓冲区
     */
    template <::std::floating_point type>
    constexpr inline void do_print_arg(::SoC::is_output_device<char> auto& device,
                                       ::SoC::detail::floating_point_format<type> format_wrapper,
                                       ::SoC::unified_text_buffer buffer) noexcept
    {
        auto&& [num, format, precision]{format_wrapper};
        ::SoC::write_to_device(device, buffer.begin(), ::std::to_chars(buffer.begin(), buffer.end(), num, format, precision).ptr);
    }

    /**
     * @brief 将带进制的整数输出到设备
     *
     * @param device 输出设备
     * @param format_wrapper 带进制的整数包装对象
     * @param buffer 输出缓冲区
     */
    template <::std::integral type, ::SoC::detail::integer_base base>
    constexpr inline void do_print_arg(::SoC::is_output_device<char> auto& device,
                                       ::SoC::detail::integer_format<type, base> format_wrapper,
                                       ::SoC::unified_text_buffer buffer) noexcept
    {
        auto ptr{::SoC::detail::write_integer_base_prefix<base>(buffer.begin())};
        ::SoC::write_to_device(device,
                               buffer.begin(),
                               ::std::to_chars(ptr, buffer.end(), format_wrapper.value, ::std::to_underlying(base)).ptr);
    }

    /**
     * @brief 判断类型arg_t能否输出到device_t类型的设备，要求满足：
     * - device_t是文本类输出设备，且
     * - arg_t和device_t满足is_no_max_text_buffer_size_printable，或
     * - arg_t和device_t满足is_has_max_text_buffer_size_printable，或
     * @tparam arg_t 要判断的类型
     * @tparam device_t 输出设备类型
     */
    template <typename arg_t, typename device_t>
    concept is_printable_to_device =
        ::SoC::is_output_device<device_t, char> && (::SoC::is_no_max_text_buffer_size_printable<arg_t, device_t> ||
                                                    ::SoC::is_has_max_text_buffer_size_printable<arg_t, device_t>);

    /**
     * @brief 输出文件类型
     *
     * @tparam type io元素类型
     * @tparam device_t 输出设备类型
     * @tparam buffer_t 缓冲区类型
     */
    template <::SoC::detail::is_io_target_type type,
              ::SoC::is_output_device<type> device_t,
              ::SoC::is_buffer buffer_t = ::SoC::default_buffer<type>>
    struct ofile
    {
        using value_type = type;
        // 输出设备
        device_t& device;
        // 输出缓冲区
        buffer_t obuffer;

        /**
         * @brief 刷新缓冲区，将缓冲区内数据写入设备后清空缓冲区
         *
         * @tparam block 是否阻塞直到刷新完成
         */
        template <bool block = false>
        constexpr inline void flush() noexcept
        {
            ::SoC::write_to_device(device, obuffer.begin, obuffer.current);
            if constexpr(block) { ::SoC::wait_until_device_ready(device); }
            obuffer.clear();
        }
    };

    /**
     * @brief 文本类输出文件类型
     *
     * @tparam device_t 文本类输出设备
     * @tparam buffer_t 文本类输出缓冲区
     */
    template <::SoC::is_output_device<char> device_t, ::SoC::is_buffer buffer_t = ::SoC::default_buffer<char>>
    using text_ofile = ::SoC::ofile<char, device_t, buffer_t>;
    /**
     * @brief 二进制输出文件类型
     *
     * @tparam device_t 二进制输出设备
     * @tparam buffer_t 二进制输出缓冲区
     */
    template <::SoC::is_output_device<::std::byte> device_t, ::SoC::is_buffer buffer_t = ::SoC::default_buffer<::std::byte>>
    using bin_ofile = ::SoC::ofile<::std::byte, device_t, buffer_t>;

    /**
     * @brief 判断type是否是输出文件，要求满足：
     * - type::device是输出设备的左值引用，且
     * - type::obuffer是输出缓冲区对象，且
     * - void type::flush() noexcept
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_output_file =
        ::std::is_lvalue_reference_v<decltype(type::device)> &&
        ::SoC::is_output_device<::std::remove_reference_t<decltype(type::device)>, typename type::value_type> &&
        ::SoC::is_buffer<decltype(type::obuffer)> && requires(type& file) {
            { file.flush() } noexcept -> ::std::same_as<void>;
        };

    /**
     * @brief 判断type是否是输入文件，要求满足：
     * - type::device是输入设备的左值引用，且
     * - type::ibuffer是输入缓冲区对象，且
     * - void type::flush() noexcept
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_input_file =
        ::std::is_lvalue_reference_v<decltype(type::device)> &&
        ::SoC::is_input_device<::std::remove_reference_t<decltype(type::device)>, typename type::value_type> &&
        ::SoC::is_buffer<decltype(type::ibuffer)> && requires(type& file) {
            { file.clear() } noexcept -> ::std::same_as<void>;
        };

    /**
     * @brief 判断type是否是文件，要求满足：
     * - SoC::is_output_file<type>，或
     * - SoC::is_input_file<type>
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_general_file = ::SoC::is_output_file<type> || ::SoC::is_input_file<type>;

    /**
     * @brief 从输出文件中萃取设备和缓冲区
     *
     * @param file 要萃取的文件
     * @return std::pair{设备引用, 缓冲区引用}
     */
    constexpr inline auto ofile_trait(::SoC::is_output_file auto& file) noexcept
    {
        return ::std::pair<decltype((file.device)), decltype((file.obuffer))>{file.device, file.obuffer};
    }

    /**
     * @brief 从输入文件中萃取设备和缓冲区
     *
     * @param file 要萃取的文件
     * @return std::pair{设备引用, 缓冲区引用}
     */
    constexpr inline auto ifile_trait(::SoC::is_input_file auto& file) noexcept
    {
        return ::std::pair<decltype((file.device)), decltype((file.ibuffer))>{file.device, file.ibuffer};
    }

    /**
     * @brief 轮询等待直到绑定到文件的设备就绪
     *
     * @tparam file_t 文件类型
     * @param file 文件对象
     */
    template <::SoC::is_general_file file_t>
    constexpr inline void wait_until_device_ready(file_t& file) noexcept
    {
        ::SoC::wait_until_device_ready(file.device);
    }

    /**
     * @brief 将字符串视图输出到file
     *
     * @param file 输出文件
     * @param string 字符串视图
     */
    constexpr inline void do_print_arg(::SoC::is_output_file auto& file, ::std::string_view string) noexcept
    {
        auto&& [device, buffer]{::SoC::ofile_trait(file)};
        if(auto buffer_size_left{buffer.get_obuffer_left()}; buffer_size_left >= string.size()) [[likely]]
        {
            buffer.write(string.data(), string.size());
        }
        else
        {
            buffer.write(string.data(), buffer_size_left);
            file.template flush<true>();
            auto output_size_left{string.size() - buffer_size_left};
            buffer.write(string.data() + buffer_size_left, output_size_left);
        }
    }

    /**
     * @brief 将C风格字符串输出到file
     *
     * @param file 输出文件
     * @param string C风格字符串
     */
    constexpr inline void do_print_arg(::SoC::is_output_file auto& file, const char* string) noexcept
    {
        ::SoC::do_print_arg(file, ::std::string_view{string});
    }

    /**
     * @brief 将整数或浮点数输出到file
     *
     * @param file 输出文件
     * @param num 整数或浮点数
     * @param tmp_buffer 输出缓冲区
     */
    constexpr inline void do_print_arg(::SoC::is_output_file auto& file,
                                       ::SoC::detail::is_int_fp auto num,
                                       ::SoC::unified_text_buffer tmp_buffer) noexcept
    {
        auto&& [device, buffer]{::SoC::ofile_trait(file)};
        constexpr auto max_text_buffer_size{::SoC::detail::get_max_text_buffer_size<decltype(num)>()};
        if(auto buffer_size_left{buffer.get_obuffer_left()}; buffer_size_left >= max_text_buffer_size) [[likely]]
        {
            auto&& current{buffer.current};
            current = ::std::to_chars(current, buffer.get_buffer_end(), num).ptr;
        }
        else
        {
            auto ptr{::std::to_chars(tmp_buffer.begin(), tmp_buffer.end(), num).ptr};
            auto output_size{static_cast<::std::size_t>(ptr - tmp_buffer.begin())};
            buffer.write(tmp_buffer.begin(), ::std::min(output_size, buffer_size_left));
            if(output_size >= buffer_size_left) { file.template flush<true>(); }
            else [[likely]]
            {
                return;
            }
            auto output_size_left{output_size - buffer_size_left};
            buffer.write(tmp_buffer.begin() + buffer_size_left, output_size_left);
        }
    }

    /**
     * @brief 将布尔值输出到设备或文件
     *
     * @param output 输出设备或文件
     * @param value 要输出的布尔值
     */
    template <typename output_t>
        requires (::SoC::is_output_device<output_t, char> || ::SoC::is_output_file<output_t>)
    constexpr inline void do_print_arg(output_t& output, bool value) noexcept
    {
        using namespace ::std::string_view_literals;
        ::SoC::do_print_arg(output, value ? "true"sv : "false"sv);
    }

    /**
     * @brief 将带格式的浮点数输出到文件
     *
     * @param file 输出文件
     * @param format_wrapper 带格式的浮点数包装对象
     * @param tmp_buffer 输出缓冲区
     */
    template <::std::floating_point type>
    constexpr inline void do_print_arg(::SoC::is_output_file auto& file,
                                       ::SoC::detail::floating_point_format<type> format_wrapper,
                                       ::SoC::unified_text_buffer tmp_buffer) noexcept
    {
        auto&& [num, format, precision]{format_wrapper};
        auto&& [device, buffer]{::SoC::ofile_trait(file)};
        constexpr auto max_text_buffer_size{::SoC::detail::get_max_text_buffer_size<decltype(num)>()};
        if(auto buffer_size_left{buffer.get_obuffer_left()}; buffer_size_left >= max_text_buffer_size) [[likely]]
        {
            auto&& current{buffer.current};
            current = ::std::to_chars(current, buffer.get_buffer_end(), num, format, precision).ptr;
        }
        else
        {
            auto ptr{::std::to_chars(tmp_buffer.begin(), tmp_buffer.end(), num, format, precision).ptr};
            auto output_size{static_cast<::std::size_t>(ptr - tmp_buffer.begin())};
            buffer.write(tmp_buffer.begin(), ::std::min(output_size, buffer_size_left));
            if(output_size >= buffer_size_left) { file.template flush<true>(); }
            else [[likely]]
            {
                return;
            }
            auto output_size_left{output_size - buffer_size_left};
            buffer.write(tmp_buffer.begin() + buffer_size_left, output_size_left);
        }
    }

    /**
     * @brief 将带进制的整数输出到文件
     *
     * @param device 输出文件
     * @param format_wrapper 带进制的整数包装对象
     * @param tmp_buffer 输出缓冲区
     */
    template <::std::integral type, ::SoC::detail::integer_base base>
    constexpr inline void do_print_arg(::SoC::is_output_file auto& file,
                                       ::SoC::detail::integer_format<type, base> format_wrapper,
                                       ::SoC::unified_text_buffer tmp_buffer) noexcept
    {
        auto&& [device, buffer]{::SoC::ofile_trait(file)};
        auto num{format_wrapper.value};
        constexpr auto max_text_buffer_size{::SoC::detail::get_max_text_buffer_size<type, base>()};
        if(auto buffer_size_left{buffer.get_obuffer_left()}; buffer_size_left >= max_text_buffer_size) [[likely]]
        {
            auto&& current{buffer.current};
            current = ::SoC::detail::write_integer_base_prefix<base>(current);
            current = ::std::to_chars(current, buffer.get_buffer_end(), num, ::std::to_underlying(base)).ptr;
        }
        else
        {
            auto ptr{::SoC::detail::write_integer_base_prefix<base>(tmp_buffer.begin())};
            ptr = ::std::to_chars(ptr, tmp_buffer.end(), num, ::std::to_underlying(base)).ptr;
            auto output_size{static_cast<::std::size_t>(ptr - tmp_buffer.begin())};
            buffer.write(tmp_buffer.begin(), ::std::min(output_size, buffer_size_left));
            if(output_size >= buffer_size_left) { file.template flush<true>(); }
            else [[likely]]
            {
                return;
            }
            auto output_size_left{output_size - buffer_size_left};
            buffer.write(tmp_buffer.begin() + buffer_size_left, output_size_left);
        }
    }

    /**
     * @brief 判断类型arg_t能否输出到file_t类型的文件，要求满足：
     * - file_t是文本类输出文件，且
     * - arg_t和file_t满足is_no_max_text_buffer_size_printable，或
     * - arg_t和file_t满足is_has_max_text_buffer_size_printable，或
     * @tparam arg_t 要判断的类型
     * @tparam file_t 输出文件类型
     */
    template <typename arg_t, typename file_t>
    concept is_printable_to_file = ::SoC::is_output_file<file_t> && (::SoC::is_no_max_text_buffer_size_printable<arg_t, file_t> ||
                                                                     ::SoC::is_has_max_text_buffer_size_printable<arg_t, file_t>);

    namespace detail
    {
        /**
         * @brief 打印函数包装器
         *
         * @tparam output_t 可进行输出操作的类型
         * @tparam arg_t 要打印的类型
         * @param output 可进行输出操作的对象
         * @param arg 要输出的对象
         * @param buffer 统一预分配的缓冲区
         */
        template <typename output_t, typename arg_t>
        [[using gnu: always_inline, artificial]] constexpr inline void
            do_print_arg_wrapper(output_t& output, arg_t&& arg, ::SoC::unified_text_buffer buffer) noexcept
        {
            if constexpr(::SoC::is_no_max_text_buffer_size_printable<arg_t, output_t>)
            {
                do_print_arg(output, ::std::forward<arg_t>(arg));
            }
            else
            {
                do_print_arg(output, ::std::forward<arg_t>(arg), buffer);
            }
        }

        /// io缓冲区的对齐值
        constexpr ::std::size_t io_buffer_align{4};

        /**
         * @brief 获取统一预分配文本缓冲区的大小
         *
         * @tparam args_t 输出参数类型
         * @return 统一预分配文本缓冲区的大小
         */
        template <typename... args_t>
        constexpr inline ::std::size_t get_unified_text_buffer_size() noexcept
        {
            constexpr auto buffer_size_unaligned{::std::max({::SoC::max_text_buffer_size<::std::remove_reference_t<args_t>>...})};
            return (buffer_size_unaligned + io_buffer_align - 1) / io_buffer_align * io_buffer_align;
        }

        /**
         * @brief 打印函数包装体，将参数列表打印
         *
         * @tparam output_t 可进行输出操作的类型
         * @tparam args_t 参数类型列表
         * @param output 可进行输出操作的对象
         * @param args 参数列表
         */
        template <typename output_t, typename... args_t>
        constexpr inline void print_wrapper(output_t& output, args_t&&... args) noexcept
        {
            ::SoC::wait_until_device_ready(output);
            if constexpr(constexpr auto buffer_size{::SoC::detail::get_unified_text_buffer_size<args_t...>()}; buffer_size != 0)
            {
                ::std::array<char, buffer_size> buffer;
                (::SoC::detail::do_print_arg_wrapper(output, ::std::forward<args_t>(args), buffer), ...);
            }
            else
            {
                (do_print_arg(output, ::std::forward<args_t>(args)), ...);
            }
        }
    }  // namespace detail

    /**
     * @brief 将参数列表输出到设备
     *
     * @tparam device_t 输出设备类型
     * @tparam args_t 参数类型列表
     * @param device 输出设备
     * @param args 参数列表
     */
    template <::SoC::is_output_device<char> device_t, ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void print(device_t& device, args_t&&... args) noexcept
    {
        ::SoC::detail::print_wrapper(device, ::std::forward<args_t>(args)...);
    }

    /**
     * @brief 将参数列表输出到文件
     *
     * @tparam flush 输出结束后是否刷新缓冲区
     * @tparam file_t 输出文件类型
     * @tparam args_t 参数类型列表
     * @param file 输出文件
     * @param args 参数列表
     */
    template <bool flush = false, ::SoC::is_output_file file_t, ::SoC::is_printable_to_file<file_t>... args_t>
    constexpr inline void print(file_t& file, args_t&&... args) noexcept
    {
        ::SoC::detail::print_wrapper(file, ::std::forward<args_t>(args)...);
        if constexpr(flush) { file.flush(); }
    }

    /**
     * @brief 行尾序列
     *
     */
    enum class end_line_sequence
    {
        /// 回车
        cr,
        /// 换行
        lf,
        /// 回车换行
        crlf
    };

    namespace detail
    {
        /**
         * @brief 获取行尾序列endl对应的字符串视图
         *
         * @tparam endl 行尾序列
         * @return 字符串视图
         */
        template <::SoC::end_line_sequence endl>
        consteval inline ::std::string_view get_endl() noexcept
        {
            using namespace ::std::string_view_literals;
            switch(endl)
            {
                case ::SoC::end_line_sequence::cr: return "\r"sv;
                case ::SoC::end_line_sequence::lf: return "\n"sv;
                case ::SoC::end_line_sequence::crlf: return "\r\n"sv;
            }
        }
    }  // namespace detail

    /**
     * @brief 将参数列表输出到设备，输出完成后换行
     *
     * @tparam endl 行尾序列
     * @tparam device_t 输出设备类型
     * @tparam args_t 参数类型列表
     * @param device 输出设备
     * @param args 参数列表
     */
    template <::SoC::end_line_sequence endl = ::SoC::end_line_sequence::crlf,
              ::SoC::is_output_device<char> device_t,
              ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void println(device_t& device, args_t&&... args) noexcept
    {
        ::SoC::print(device, ::std::forward<args_t>(args)..., ::SoC::detail::get_endl<endl>());
    }

    /**
     * @brief 将参数列表输出到文件，输出完成后换行
     *
     * @tparam flush 输出结束后是否刷新缓冲区
     * @tparam endl 行尾序列
     * @tparam file_t 输出文件类型
     * @tparam args_t 参数类型列表
     * @param file 输出文件
     * @param args 参数列表
     */
    template <bool flush = false,
              ::SoC::end_line_sequence endl = ::SoC::end_line_sequence::crlf,
              ::SoC::is_output_file file_t,
              ::SoC::is_printable_to_file<file_t>... args_t>
    constexpr inline void println(file_t& file, args_t&&... args) noexcept
    {
        ::SoC::print<flush>(file, ::std::forward<args_t>(args)..., ::SoC::detail::get_endl<endl>());
    }
}  // namespace SoC

/**
 * @brief 格式串支持
 *
 */
namespace SoC
{
    namespace detail
    {
        /**
         * @brief 获取格式串打印所需参数
         *
         * @tparam type 不含占位符的字符数组类型列表
         */
        template <typename... type>
        struct get_fmt_arg_t
        {
            ::std::tuple<type...> tuple;
            constexpr inline static auto no_placehold_num{sizeof...(type)};

            template <::std::size_t index, typename arg_t>
                requires (index < no_placehold_num)
            [[using gnu: always_inline, artificial]] constexpr inline ::std::string_view get_fmt_arg(arg_t&&) const noexcept
            {
                auto&& array{::std::get<index>(tuple)};
                return {array.begin(), array.end()};
            }

            template <::std::size_t index, typename arg_t>
                requires (index >= no_placehold_num)
            [[using gnu: always_inline, artificial]] constexpr inline auto&& get_fmt_arg(arg_t&& arg) const noexcept
            {
                return ::std::forward<arg_t>(arg);
            }
        };

        /**
         * @brief 打印函数包装体，将参数列表打印
         *
         * @tparam output_t 可进行输出操作的类型
         * @tparam args_t 参数类型列表
         * @param output 可进行输出操作的对象
         * @param args 参数列表
         */
        template <::SoC::fmt_string fmt, typename output_t, ::std::size_t... indexes, typename... args_t>
        constexpr inline void print_wrapper(output_t& output, ::std::index_sequence<indexes...>, args_t&&... args) noexcept
        {
            using parser = ::SoC::fmt_parser<fmt>;
            constexpr auto placehold_num{parser::get_placehold_num()};
            static_assert(placehold_num == sizeof...(args), "占位符个数和参数个数不同");
            constexpr auto no_placehold_num{parser::get_no_placehold_num()};
            if constexpr(no_placehold_num == 0)
            {
                // 不含需要交错输出的字符串
                ::SoC::detail::print_wrapper(output, ::std::forward<args_t>(args)...);
            }
            else
            {
                constexpr ::SoC::detail::get_fmt_arg_t split_string_tuple{parser::get_split_string_tuple()};
                constexpr auto tuple_index_array{parser::get_tuple_index_array()};
                ::SoC::detail::print_wrapper(
                    output,
                    split_string_tuple.template get_fmt_arg<tuple_index_array[indexes]>(
                        ::std::forward<args_t...[(tuple_index_array[indexes] - no_placehold_num) % placehold_num]>(
                            args...[(tuple_index_array[indexes] - no_placehold_num) % placehold_num]))...);
            }
        }

        template <::SoC::fmt_string fmt, ::SoC::end_line_sequence endl>
        constexpr inline auto get_fmt_string_with_endl() noexcept
        {
            constexpr auto endl_string{::SoC::detail::get_endl<endl>()};
            // fmt_string要求输入是空结尾的字符串，因此填充1位
            char buffer[fmt.size() + endl_string.size() + 1]{};
            ::std::ranges::copy(fmt, buffer);
            ::std::ranges::copy(endl_string, buffer + fmt.size());
            return ::SoC::fmt_string{buffer};
        }
    }  // namespace detail

    /**
     * @brief 将参数列表输出到设备
     *
     * @tparam fmt 格式串
     * @tparam device_t 输出设备类型
     * @tparam args_t 参数类型列表
     * @param device 输出设备
     * @param args 参数列表
     */
    template <::SoC::fmt_string fmt, ::SoC::is_output_device<char> device_t, ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void print(device_t& device, args_t&&... args) noexcept
    {
        using parser = ::SoC::fmt_parser<fmt>;
        ::SoC::detail::print_wrapper<fmt>(device,
                                          ::std::make_index_sequence<parser::get_total_num()>{},
                                          ::std::forward<args_t>(args)...);
    }

    /**
     * @brief 将参数列表输出到文件
     *
     * @tparam fmt 格式串
     * @tparam flush 输出结束后是否刷新缓冲区
     * @tparam file_t 输出文件类型
     * @tparam args_t 参数类型列表
     * @param file 输出文件
     * @param args 参数列表
     */
    template <::SoC::fmt_string fmt,
              bool flush = false,
              ::SoC::is_output_file file_t,
              ::SoC::is_printable_to_file<file_t>... args_t>
    constexpr inline void print(file_t& file, args_t&&... args) noexcept
    {
        using parser = ::SoC::fmt_parser<fmt>;
        ::SoC::detail::print_wrapper<fmt>(file,
                                          ::std::make_index_sequence<parser::get_total_num()>{},
                                          ::std::forward<args_t>(args)...);
        if constexpr(flush) { file.flush(); }
    }

    /**
     * @brief 将参数列表输出到设备，输出完成后换行
     *
     * @tparam fmt 格式串
     * @tparam endl 行尾序列
     * @tparam device_t 输出设备类型
     * @tparam args_t 参数类型列表
     * @param device 输出设备
     * @param args 参数列表
     */
    template <::SoC::fmt_string fmt,
              ::SoC::end_line_sequence endl = ::SoC::end_line_sequence::crlf,
              ::SoC::is_output_device<char> device_t,
              ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void println(device_t& device, args_t&&... args) noexcept
    {
        ::SoC::print<::SoC::detail::get_fmt_string_with_endl<fmt, endl>()>(device, ::std::forward<args_t>(args)...);
    }

    /**
     * @brief 将参数列表输出到文件，输出完成后换行
     *
     * @tparam flush 输出结束后是否刷新缓冲区
     * @tparam endl 行尾序列
     * @tparam file_t 输出文件类型
     * @tparam args_t 参数类型列表
     * @param file 输出文件
     * @param args 参数列表
     */
    template <::SoC::fmt_string fmt,
              bool flush = false,
              ::SoC::end_line_sequence endl = ::SoC::end_line_sequence::crlf,
              ::SoC::is_output_file file_t,
              ::SoC::is_printable_to_file<file_t>... args_t>
    constexpr inline void println(file_t& file, args_t&&... args) noexcept
    {
        ::SoC::print<::SoC::detail::get_fmt_string_with_endl<fmt, endl>(), flush>(file, ::std::forward<args_t>(args)...);
    }
}  // namespace SoC
