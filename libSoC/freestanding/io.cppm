/**
 * @file io.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的输入输出接口实现
 */

export module SoC.freestanding:io;
import :allocator;
import :fmt;
import :utils;

namespace SoC
{

    /**
     * @brief 禁止某种类型的输出设备
     *
     * @tparam device_t 设备类型
     */
    export template <typename device_t>
    constexpr inline bool forbidden_output_device{false};

    /**
     * @brief 禁止某种类型的输入设备
     *
     * @tparam device_t 设备类型
     */
    export template <typename device_t>
    constexpr inline bool forbidden_input_device{false};

    namespace detail
    {
        /**
         * @brief 检查某种类型的输出设备是否被显式禁用，并提供更好的报错
         *
         * @tparam device_t 设备类型
         */
        template <typename device_t>
        struct check_forbidden_output_device
        {
            constexpr inline static bool not_forbidden{!::SoC::forbidden_output_device<device_t>};
            static_assert(not_forbidden, "该输出设备类型被显式禁用");
        };

        /**
         * @brief 检查某种类型的输入设备是否被显式禁用，并提供更好的报错
         *
         * @tparam device_t 设备类型
         */
        template <typename device_t>
        struct check_forbidden_input_device
        {
            constexpr inline static bool not_forbidden{!::SoC::forbidden_input_device<device_t>};
            static_assert(not_forbidden, "该输入设备类型被显式禁用");
        };
    }  // namespace detail
}  // namespace SoC

/**
 * @brief 基本概念和函数定义
 *
 */
export namespace SoC
{
    /**
     * @brief 判断device_t是否是type类型的输出设备，要求满足：
     * - !forbidden_output_device<device_t>，即输出设备类型未被禁用，且
     * - ::SoC::detail::is_io_target_type<type>，且
     * - auto device_t::write(const type*, const type*) noexcept，或
     * - auto write(device_t&, const type*, const type*) noexcept，考虑adl
     * @tparam device_t 设备类型
     * @tparam type 输出类型
     */
    template <typename device_t, typename type>
    concept is_output_device =
        ::SoC::detail::check_forbidden_output_device<device_t>::not_forbidden && ::SoC::detail::is_io_target_type<type> && (requires(device_t& dev, const type* begin, const type* end) {
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
     * @brief 判断device_t是否是异步输出设备，要求满足：
     * - 写入数据后立即返回，不等待数据发送完成
     * @tparam type 输出类型
     * @warning 异步输出设备允许进行非阻塞的输出操作以提升性能，但是禁止直接在设备上进行输出操作，必须和缓冲区一起组成文件才能使用
     * @note 由于在异步设备上进行基于临时缓冲区的直接输出操作必须同步完成，因此禁止这类操作
     */
    template <::SoC::is_general_output_device type>
    constexpr inline bool async_output_device{false};

    /**
     * @brief 判断device_t是否是同步输出设备，要求满足：
     * - 写入数据后等待数据发送完成
     * @tparam device_t 设备类型
     * @tparam type 输出类型
     */
    template <typename device_t, typename type>
    concept is_sync_output_device = ::SoC::is_output_device<device_t, type> && !async_output_device<device_t>;

    /**
     * @brief 判断device_t是否是异步输出设备，要求满足：
     * - 写入数据后立即返回，不等待数据发送完成
     * @tparam device_t 设备类型
     * @tparam type 输出类型
     */
    template <typename device_t, typename type>
    concept is_async_output_device = ::SoC::is_output_device<device_t, type> && async_output_device<device_t>;

    /**
     * @brief 判断device_t是否是一种同步输出设备，要求满足：
     * - SoC::is_sync_output_device<device_t, char>，或
     * - SoC::is_sync_output_device<device_t, std::byte>
     */
    template <typename device_t>
    concept is_general_sync_output_device =
        ::SoC::is_sync_output_device<device_t, char> || ::SoC::is_sync_output_device<device_t, ::std::byte>;

    /**
     * @brief 判断device_t是否是一种异步输出设备，要求满足：
     * - SoC::is_async_output_device<device_t, char>，或
     * - SoC::is_async_output_device<device_t, std::byte>
     */
    template <typename device_t>
    concept is_general_async_output_device =
        ::SoC::is_async_output_device<device_t, char> || ::SoC::is_async_output_device<device_t, ::std::byte>;

    /**
     * @brief 判断device_t是否是type类型的输入设备，要求满足：
     * - !forbidden_input_device<device_t>，即输入设备类型未被禁用，且
     * - ::SoC::detail::is_io_target_type<type>，且
     * - type* device_t::read(type*, type*) noexcept，或
     * - type* read(device_t&, type*, type*) noexcept，考虑adl
     * @tparam device_t 设备类型
     * @tparam type 输入类型
     */
    template <typename device_t, typename type>
    concept is_input_device =
        ::SoC::detail::check_forbidden_input_device<device_t>::not_forbidden && ::SoC::detail::is_io_target_type<type> && (requires(device_t& dev, type* begin, type* end) {
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
     * @brief 判断device_t是否是异步输入设备，要求满足：
     * - 写入数据后立即返回，不等待数据发送完成
     * @tparam type 输入类型
     * @warning 异步输入设备允许进行非阻塞的输入操作以提升性能，但是禁止直接在设备上进行输入操作，必须和缓冲区一起组成文件才能使用
     * @note 由于在异步设备上进行基于临时缓冲区的直接输入操作必须同步完成，因此禁止这类操作
     */
    template <::SoC::is_general_input_device type>
    constexpr inline bool async_input_device{false};

    /**
     * @brief 判断device_t是否是同步输入设备，要求满足：
     * - 写入数据后等待数据发送完成
     * @tparam device_t 设备类型
     * @tparam type 输入类型
     */
    template <typename device_t, typename type>
    concept is_sync_input_device = ::SoC::is_input_device<device_t, type> && !async_input_device<device_t>;

    /**
     * @brief 判断device_t是否是异步输入设备，要求满足：
     * - 写入数据后立即返回，不等待数据发送完成
     * @tparam device_t 设备类型
     * @tparam type 输入类型
     */
    template <typename device_t, typename type>
    concept is_async_input_device = ::SoC::is_input_device<device_t, type> && async_input_device<device_t>;

    /**
     * @brief 判断device_t是否是一种同步输入设备，要求满足：
     * - SoC::is_sync_input_device<device_t, char>，或
     * - SoC::is_sync_input_device<device_t, std::byte>
     */
    template <typename device_t>
    concept is_general_sync_input_device =
        ::SoC::is_sync_input_device<device_t, char> || ::SoC::is_sync_input_device<device_t, ::std::byte>;

    /**
     * @brief 判断device_t是否是一种异步输入设备，要求满足：
     * - SoC::is_async_input_device<device_t, char>，或
     * - SoC::is_async_input_device<device_t, std::byte>
     */
    template <typename device_t>
    concept is_general_async_input_device =
        ::SoC::is_async_input_device<device_t, char> || ::SoC::is_async_input_device<device_t, ::std::byte>;

    /**
     * @brief 判断device_t是否是一种设备
     * - SoC::is_general_output_device<device_t>，或
     * - SoC::is_general_input_device<device_t>
     * @tparam device_t 设备类型
     */
    template <typename device_t>
    concept is_general_device = ::SoC::is_general_output_device<device_t> || ::SoC::is_general_input_device<device_t>;

    /**
     * @brief 判断device_t是否是一种同步设备，要求满足：
     * - SoC::is_general_sync_output_device<device_t>，或
     * - SoC::is_general_sync_input_device<device_t>
     */
    template <typename device_t>
    concept is_general_sync_device =
        ::SoC::is_general_sync_output_device<device_t> || ::SoC::is_general_sync_input_device<device_t>;

    /**
     * @brief 判断device_t是否是一种异步设备，要求满足：
     * - SoC::is_general_async_output_device<device_t>，或
     * - SoC::is_general_async_input_device<device_t>
     */
    template <typename device_t>
    concept is_general_async_device =
        ::SoC::is_general_async_output_device<device_t> || ::SoC::is_general_async_input_device<device_t>;

    /**
     * @brief 判断device_t是否具有写就绪标记，要求满足：
     * - bool device_t.is_write_ready() noexcept，或
     * - bool is_write_ready(device_t&) noexcept，考虑adl
     * @tparam device_t 设备类型
     * @note 对于阻塞写入的设备无需此标记
     */
    template <typename device_t>
    concept has_write_ready_flag_device = ::SoC::is_general_device<device_t> && (requires(device_t& dev) {
        { dev.is_write_ready() } noexcept -> ::std::same_as<bool>;
    } || requires(device_t& dev) {
        { is_write_ready(dev) } noexcept -> ::std::same_as<bool>;
    });

    /**
     * @brief 判断device_t是否具有读就绪标记，要求满足：
     * - bool device_t.is_read_ready() noexcept，或
     * - bool is_read_ready(device_t&) noexcept，考虑adl
     * @tparam device_t 设备类型
     * @note 对于阻塞读取的设备无需此标记
     */
    template <typename device_t>
    concept has_read_ready_flag_device = ::SoC::is_general_device<device_t> && (requires(device_t& dev) {
        { dev.is_read_ready() } noexcept -> ::std::same_as<bool>;
    } || requires(device_t& dev) {
        { is_read_ready(dev) } noexcept -> ::std::same_as<bool>;
    });

    /**
     * @brief 轮询等待直到设备写端口就绪
     *
     * @tparam device_t 设备类型
     * @param device 设备对象
     */
    template <::SoC::is_general_device device_t>
    constexpr inline void wait_until_write_ready(device_t& device) noexcept
    {
        if constexpr(::SoC::has_write_ready_flag_device<device_t>)
        {
            if constexpr(requires() { device.is_write_ready(); })
            {
                ::SoC::wait_until([&device] constexpr noexcept { return device.is_write_ready(); });
            }
            else
            {
                ::SoC::wait_until([&device] constexpr noexcept { is_write_ready(device); });
            }
        }
    }

    /**
     * @brief 轮询等待直到设备读端口就绪
     *
     * @tparam device_t 设备类型
     * @param device 设备对象
     */
    template <::SoC::is_general_device device_t>
    constexpr inline void wait_until_read_ready(device_t& device) noexcept
    {
        if constexpr(::SoC::has_read_ready_flag_device<device_t>)
        {
            if constexpr(requires() { device.is_read_ready(); })
            {
                ::SoC::wait_until([&device] constexpr noexcept { return device.is_read_ready(); });
            }
            else
            {
                ::SoC::wait_until([&device] constexpr noexcept { is_read_ready(device); });
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

namespace SoC::detail
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

    template <::SoC::detail::is_io_target_type type, ::std::size_t buffer_size, ::SoC::detail::is_buffer_allocator allocator_type>
    struct buffer_impl
    {
        using value_type = type;
        using pointer = value_type*;
        using allocator_t = allocator_type;

        /// 分配器
        [[no_unique_address]] allocator_t allocator;
        /// 缓冲区首指针
        pointer begin;

        constexpr inline buffer_impl(allocator_t alloc = allocator_t{}) noexcept(::SoC::is_noexcept_allocator<allocator_t>) :
            allocator{alloc}, begin{allocator.template allocate<value_type>(buffer_size).ptr}
        {
        }

        constexpr inline ~buffer_impl() noexcept { allocator.deallocate(begin, buffer_size); }

        inline buffer_impl(const buffer_impl&) = delete;
        inline buffer_impl& operator= (const buffer_impl&) = delete;

        constexpr inline buffer_impl(buffer_impl&& other) noexcept :
            allocator{other.allocator}, begin{::std::exchange(other.begin, nullptr)}
        {
        }

        constexpr inline buffer_impl& operator= (buffer_impl&& other) noexcept
        {
            auto _{::std::move(*this)};
            ::std::ranges::swap(other, *this);
            return *this;
        }
    };

    template <::SoC::detail::is_io_target_type type, ::std::size_t buffer_size>
    struct buffer_impl<type, buffer_size, void>
    {
        using value_type = type;
        using pointer = value_type*;
        using allocator_t = void;

        /// 缓冲区数组，可退化为首指针
        alignas(::SoC::detail::get_buffer_align(buffer_size)) value_type begin[buffer_size];

        constexpr inline buffer_impl() noexcept = default;
    };
}  // namespace SoC::detail

/**
 * @brief 缓冲区定义
 *
 */
export namespace SoC
{

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
    private:
        using base_t = ::SoC::detail::buffer_impl<type, buffer_size, allocator_type>;

    public:
        using base_t::begin;
        using value_type = type;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using allocator_t = allocator_type;

        /// 缓冲区当前游标
        pointer current{begin};
        /// 有效输入缓冲区的尾哨位
        pointer end{begin};

        constexpr inline buffer() noexcept(noexcept(base_t{})) = default;

        constexpr inline ~buffer() noexcept { clear(); }

        constexpr inline buffer(const buffer&) = delete;
        constexpr inline buffer& operator= (const buffer&) = delete;

        constexpr inline buffer(buffer&& other) noexcept = default;

        constexpr inline buffer& operator= (buffer&& other) noexcept = default;

        /**
         * @brief 获取缓冲区尾哨位
         *
         * @return 缓冲区尾哨位
         */
        constexpr inline pointer get_buffer_end() noexcept { return begin + buffer_size; }

        /**
         * @brief 清空缓冲区
         *
         */
        constexpr inline void clear() noexcept
        {
            current = begin;
            end = begin;
        }

        /**
         * @brief 获取输出缓冲区是否为空
         *
         * @return 输出缓冲区是否为空
         */
        [[nodiscard]] constexpr inline bool obuffer_empty() const noexcept { return current == begin; }

        /**
         * @brief 获取输入缓冲区是否为空
         *
         * @return 输入缓冲区是否为空
         */
        [[nodiscard]] constexpr inline bool ibuffer_empty() const noexcept { return current == end; }
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
     * - void type::clear() noexcept，且
     * - bool type::obuffer_empty() const noexcept，且
     * - bool type::ibuffer_empty() const noexcept
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_buffer = requires(type& buffer, const type& const_buffer) {
        typename type::value_type;
        typename type::pointer;
        typename type::const_pointer;
        typename type::allocator_t;

        requires ::std::same_as<decltype(buffer.current), typename type::pointer>;
        requires ::std::same_as<decltype(buffer.end), typename type::pointer>;

        { buffer.get_buffer_end() } noexcept -> ::std::same_as<typename type::pointer>;
        { buffer.clear() } noexcept -> ::std::same_as<void>;
        { const_buffer.obuffer_empty() } noexcept -> ::std::same_as<bool>;
        { const_buffer.ibuffer_empty() } noexcept -> ::std::same_as<bool>;
    };
}  // namespace SoC

namespace SoC::detail
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
        // NOLINTBEGIN(bugprone-not-null-terminated-result)
        if constexpr(base == ::SoC::integer_base8) { ::std::memcpy(buffer, "0o", 2); }
        else if constexpr(base == ::SoC::integer_base2) { ::std::memcpy(buffer, "0b", 2); }
        else if constexpr(base == ::SoC::integer_base16) { ::std::memcpy(buffer, "0x", 2); }
        // NOLINTEND(bugprone-not-null-terminated-result)
        return buffer + 2;
    }
}  // namespace SoC::detail

/**
 * @brief 输出到设备的函数实现
 *
 */
export namespace SoC
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

    /// 统一预分配的文本缓冲区类型
    using unified_text_buffer = ::std::ranges::subrange<char*>;

    /// 统一预分配的二进制缓冲区类型
    using unified_bin_buffer = ::std::ranges::subrange<::std::byte*>;

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

    template <>
    constexpr inline ::std::size_t max_text_buffer_size<::std::source_location>{
        ::SoC::max_text_buffer_size<::std::uint_least32_t>};

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
     * - void do_print_arg(output_t&, arg_t), 考虑adl
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
     * - void do_print_arg(output_t&, arg_t, SoC::unified_text_buffer), 考虑adl
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
                               ::std::to_chars(ptr, buffer.end(), format_wrapper.value, ::SoC::to_underlying(base)).ptr);
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
}  // namespace SoC

namespace SoC::detail
{
    /**
     * @brief 检查输出文件类型是否合法，并提供更好的报错
     *
     * @tparam device_t 输出设备类型
     * @tparam type 输出类型
     */
    template <typename device_t, typename type>
    struct check_output_file_type
    {
        constexpr inline static bool sync_or_async_with_ready_flag{
            ::SoC::is_sync_output_device<device_t, typename type::value_type> ||
            (::SoC::is_async_output_device<device_t, typename type::value_type> && ::SoC::has_write_ready_flag_device<device_t>)};

        static_assert(sync_or_async_with_ready_flag,
                      "绑定到文件的异步输出设备必须支持写就绪标志，以便在刷新缓冲区时等待设备就绪");
    };

    /**
     * @brief 检查输入文件类型是否合法，并提供更好的报错
     *
     * @tparam device_t 输入设备类型
     * @tparam type 输入类型
     */
    template <typename device_t, typename type>
    struct check_input_file_type
    {
        constexpr inline static bool sync_or_async_with_ready_flag{
            ::SoC::is_sync_input_device<device_t, typename type::value_type> ||
            (::SoC::is_async_input_device<device_t, typename type::value_type> && ::SoC::has_read_ready_flag_device<device_t>)};

        static_assert(sync_or_async_with_ready_flag,
                      "绑定到文件的异步输入设备必须支持读就绪标志，以便在刷新缓冲区时等待设备就绪");
    };

    template <typename file_t>
    concept is_output_file_block_flushable = requires(file_t& file) {
        { file.template flush<true>() } noexcept -> ::std::same_as<void>;
    };

    template <typename file_t>
    concept is_output_file_block_flush_with_deduct_this =
        requires(file_t& file) { requires ::std::is_pointer_v<decltype(&file_t::template flush<true>)>; };

    template <typename file_t>
    struct check_output_file_block_flush
    {
        constexpr inline static bool is_output_file_block_flushable{::SoC::detail::is_output_file_block_flushable<file_t>};
        static_assert(is_output_file_block_flushable, "绑定到文件的输出设备必须支持阻塞的刷新操作");
    };
}  // namespace SoC::detail

/**
 * @brief 文件类型定义
 *
 */
export namespace SoC
{
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
        device_t* device{};
        // 输出缓冲区
        buffer_t obuffer{};

        constexpr inline ofile() noexcept(noexcept(buffer_t{})) = default;

        constexpr inline ofile(device_t& device) noexcept(noexcept(buffer_t{})) : device{&device} {}

        inline ofile(const ofile&) = delete;
        inline ofile& operator= (const ofile&) = delete;

        inline ofile(ofile&& other) noexcept : device{::std::exchange(other.device, nullptr)}, obuffer{::std::move(other.obuffer)}
        {
        }

        inline ofile& operator= (ofile&& other) noexcept
        {
            auto _{::std::move(*this)};
            ::std::ranges::swap(_, other);
            return *this;
        }

        /**
         * @brief 刷新缓冲区，将缓冲区内数据写入设备后清空缓冲区
         *
         * @tparam block 是否阻塞直到刷新完成
         */
        template <bool block = false>
        constexpr inline void flush(this ofile& self) noexcept
        {
            ::SoC::write_to_device(*self.device, auto(self.obuffer.begin), self.obuffer.current);
            if constexpr(block) { ::SoC::wait_until_write_ready(*self.device); }
            self.obuffer.clear();
        }

        ~ofile() noexcept
        {
            if(device != nullptr && !obuffer.obuffer_empty()) { flush<true>(); }
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
     * - type::device是输出设备的指针，其中输出设备要求为同步输出设备或异步输出设备且具有写就绪标志，且
     * - type::obuffer是输出缓冲区对象，且
     * - void type::template flush<true>() noexcept
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_output_file = ::std::is_pointer_v<decltype(type::device)> &&
                             ::SoC::detail::check_output_file_type<::std::remove_pointer_t<decltype(type::device)>,
                                                                   type>::sync_or_async_with_ready_flag &&
                             ::SoC::is_buffer<decltype(type::obuffer)> &&
                             ::SoC::detail::check_output_file_block_flush<type>::is_output_file_block_flushable;

    /**
     * @brief 判断file_t是否是输出文件，要求满足：
     * - file_t满足SoC::is_output_file<file_t>，且
     * - void file_t::template flush<false>() noexcept
     * @tparam file_t 文件类型
     */
    template <typename file_t>
    concept is_output_file_no_block_flushable = ::SoC::is_output_file<file_t> && requires(file_t& file) {
        { file.template flush<false>() } noexcept -> ::std::same_as<void>;
    };

    /**
     * @brief 判断type是否是输入文件，要求满足：
     * - type::device是输入设备的指针，其中输入设备要求为同步输入设备或异步输入设备且具有读就绪标志，且
     * - type::ibuffer是输入缓冲区对象，且
     * - void type::flush() noexcept
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_input_file = ::std::is_pointer_v<decltype(type::device)> &&
                            ::SoC::detail::check_input_file_type<::std::remove_pointer_t<decltype(type::device)>,
                                                                 type>::sync_or_async_with_ready_flag &&
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
     * @brief 轮询等待直到绑定到文件的设备就绪
     *
     * @tparam file_t 文件类型
     * @param file 文件对象
     */
    template <::SoC::is_general_file file_t>
    constexpr inline void wait_until_write_ready(file_t& file) noexcept
    {
        ::SoC::wait_until_write_ready(*file.device);
    }

    /**
     * @brief 输出文件萃取器
     *
     * @tparam value_type 输出文件元素类型
     */
    template <typename value_type>
    struct ofile_trait_t
    {
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using callback_t = void (*)(void*) noexcept;

        /// 类型擦除的输出文件指针
        void* file;
        /// 类型擦除的刷新回调函数
        callback_t flush_callback;
        /// 等待直到写入完成回调函数
        callback_t wait_until_write_ready_callback;
        /// 缓冲区当前指针的引用
        pointer& current;
        /// 缓冲区结束指针
        pointer end;

        /**
         * @brief 向缓冲区写入数据
         *
         * @param data 要写入的数据指针
         * @param size 要写入的数据大小
         */
        void write(const_pointer data, ::std::size_t size)
        {
            ::std::memcpy(current, data, size);
            current += size;
        }

        /**
         * @brief 获取缓冲区剩余空间大小
         *
         * @return 缓冲区剩余空间大小
         */
        [[nodiscard]] ::std::size_t get_buffer_size_left() const { return end - current; }

        /**
         * @brief 刷新缓冲区
         *
         */
        void flush() { flush_callback(file); }
    };

    /**
     * @brief 等待直到写入完成
     *
     * @param trait 输出文件萃取器
     */
    template <typename value_type>
    void wait_until_write_ready(::SoC::ofile_trait_t<value_type>& trait)
    {
        trait.wait_until_write_ready_callback(trait.file);
    }

    /**
     * @brief 从输出文件中萃取设备和缓冲区
     *
     * @param file 要萃取的文件
     * @return std::pair{设备引用, 缓冲区引用}
     */
    constexpr inline auto ofile_trait(::SoC::is_output_file auto& file) noexcept
    {
        using file_t = ::std::remove_reference_t<decltype(file)>;
        constexpr auto flush{[](void* file) static noexcept -> void { static_cast<file_t*>(file)->template flush<true>(); }};
        constexpr auto wait_until_write_ready{[](void* file) static noexcept -> void
                                              { ::SoC::wait_until_write_ready(*static_cast<file_t*>(file)); }};

        return ::SoC::ofile_trait_t{
            &file,
            flush,
            wait_until_write_ready,
            file.obuffer.current,
            file.obuffer.get_buffer_end(),
        };
    }
}  // namespace SoC

/**
 * @brief 输出到文件的函数实现
 *
 */
export namespace SoC
{
    /**
     * @brief 将字符串视图输出到file
     *
     * @param trait 输出文件萃取器
     * @param string 字符串视图
     */
    constexpr inline void do_print_arg(::SoC::ofile_trait_t<char>& trait, ::std::string_view string) noexcept
    {
        if(auto buffer_size_left{trait.get_buffer_size_left()}; buffer_size_left >= string.size()) [[likely]]
        {
            trait.write(string.data(), string.size());
        }
        else
        {
            trait.write(string.data(), buffer_size_left);
            trait.flush();
            auto output_size_left{string.size() - buffer_size_left};
            trait.write(string.data() + buffer_size_left, output_size_left);
        }
    }

    /**
     * @brief 将C风格字符串输出到file
     *
     * @param trait 输出文件萃取器
     * @param string C风格字符串
     */
    constexpr inline void do_print_arg(::SoC::ofile_trait_t<char>& trait, const char* string) noexcept
    {
        ::SoC::do_print_arg(trait, ::std::string_view{string});
    }

    /**
     * @brief 将整数或浮点数输出到file
     *
     * @param trait 输出文件萃取器
     * @param num 整数或浮点数
     * @param tmp_buffer 输出缓冲区
     */
    constexpr inline void do_print_arg(::SoC::ofile_trait_t<char>& trait,
                                       ::SoC::detail::is_int_fp auto num,
                                       ::SoC::unified_text_buffer tmp_buffer) noexcept
    {
        constexpr auto max_text_buffer_size{::SoC::max_text_buffer_size<decltype(num)>};
        if(auto buffer_size_left{trait.get_buffer_size_left()}; buffer_size_left >= max_text_buffer_size) [[likely]]
        {
            trait.current = ::std::to_chars(trait.current, trait.end, num).ptr;
        }
        else
        {
            auto ptr{::std::to_chars(tmp_buffer.begin(), tmp_buffer.end(), num).ptr};
            auto output_size{static_cast<::std::size_t>(ptr - tmp_buffer.begin())};
            trait.write(tmp_buffer.begin(), ::std::min(output_size, buffer_size_left));
            if(output_size >= buffer_size_left) { trait.flush(); }
            else [[likely]]
            {
                return;
            }
            auto output_size_left{output_size - buffer_size_left};
            trait.write(tmp_buffer.begin() + buffer_size_left, output_size_left);
        }
    }

    /**
     * @brief 将布尔值输出到设备或文件
     *
     * @tparam output_t 输出设备或文件萃取器
     * @param output 输出设备或文件萃取器
     * @param value 要输出的布尔值
     */
    template <typename output_t>
        requires (::SoC::is_output_device<output_t, char> || ::std::same_as<::SoC::ofile_trait_t<char>, output_t>)
    constexpr inline void do_print_arg(output_t& output, bool value) noexcept
    {
        using namespace ::std::string_view_literals;
        ::SoC::do_print_arg(output, value ? "true"sv : "false"sv);
    }

    /**
     * @brief 将带格式的浮点数输出到文件
     *
     * @param trait 输出文件萃取器
     * @param format_wrapper 带格式的浮点数包装对象
     * @param tmp_buffer 输出缓冲区
     */
    template <::std::floating_point type>
    constexpr inline void do_print_arg(::SoC::ofile_trait_t<char>& trait,
                                       ::SoC::detail::floating_point_format<type> format_wrapper,
                                       ::SoC::unified_text_buffer tmp_buffer) noexcept
    {
        auto&& [num, format, precision]{format_wrapper};
        constexpr auto max_text_buffer_size{::SoC::max_text_buffer_size<decltype(num)>};
        if(auto buffer_size_left{trait.get_buffer_size_left()}; buffer_size_left >= max_text_buffer_size) [[likely]]
        {
            trait.current = ::std::to_chars(trait.current, trait.end, num, format, precision).ptr;
        }
        else
        {
            auto ptr{::std::to_chars(tmp_buffer.begin(), tmp_buffer.end(), num, format, precision).ptr};
            auto output_size{static_cast<::std::size_t>(ptr - tmp_buffer.begin())};
            trait.write(tmp_buffer.begin(), ::std::min(output_size, buffer_size_left));
            if(output_size >= buffer_size_left) { trait.flush(); }
            else [[likely]]
            {
                return;
            }
            auto output_size_left{output_size - buffer_size_left};
            trait.write(tmp_buffer.begin() + buffer_size_left, output_size_left);
        }
    }

    /**
     * @brief 将带进制的整数输出到文件
     *
     * @param trait 输出文件萃取器
     * @param format_wrapper 带进制的整数包装对象
     * @param tmp_buffer 输出缓冲区
     */
    template <::std::integral type, ::SoC::detail::integer_base base>
    constexpr inline void do_print_arg(::SoC::ofile_trait_t<char>& trait,
                                       ::SoC::detail::integer_format<type, base> format_wrapper,
                                       ::SoC::unified_text_buffer tmp_buffer) noexcept
    {
        auto num{format_wrapper.value};
        constexpr auto max_text_buffer_size{::SoC::max_text_buffer_size<decltype(format_wrapper)>};
        if(auto buffer_size_left{trait.get_buffer_size_left()}; buffer_size_left >= max_text_buffer_size) [[likely]]
        {
            trait.current = ::SoC::detail::write_integer_base_prefix<base>(trait.current);
            trait.current = ::std::to_chars(trait.current, trait.end, num, ::SoC::to_underlying(base)).ptr;
        }
        else
        {
            auto ptr{::SoC::detail::write_integer_base_prefix<base>(tmp_buffer.begin())};
            ptr = ::std::to_chars(ptr, tmp_buffer.end(), num, ::SoC::to_underlying(base)).ptr;
            auto output_size{static_cast<::std::size_t>(ptr - tmp_buffer.begin())};
            trait.write(tmp_buffer.begin(), ::std::min(output_size, buffer_size_left));
            if(output_size >= buffer_size_left) { trait.flush(); }
            else [[likely]]
            {
                return;
            }
            auto output_size_left{output_size - buffer_size_left};
            trait.write(tmp_buffer.begin() + buffer_size_left, output_size_left);
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
    concept is_printable_to_file =
        ::SoC::is_output_file<file_t> &&
        (::SoC::is_no_max_text_buffer_size_printable<arg_t, ::SoC::ofile_trait_t<typename file_t::value_type>> ||
         ::SoC::is_has_max_text_buffer_size_printable<arg_t, ::SoC::ofile_trait_t<typename file_t::value_type>>);
}  // namespace SoC

namespace SoC::detail
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
    export template <typename output_t, typename arg_t>
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
        ::SoC::wait_until_write_ready(output);
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
}  // namespace SoC::detail

/**
 * @brief print系列函数实现
 *
 */
export namespace SoC
{
    /**
     * @brief 将参数列表输出到设备
     *
     * @tparam device_t 输出设备类型
     * @tparam args_t 参数类型列表
     * @param device 输出设备
     * @param args 参数列表
     */
    template <::SoC::is_sync_output_device<char> device_t, ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void print(device_t& device, args_t&&... args) noexcept
    {
        ::SoC::detail::print_wrapper(device, ::std::forward<args_t>(args)...);
    }

    template <::SoC::is_async_output_device<char> device_t, ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void print(device_t& device, args_t&&... args) noexcept = delete("异步输出设备不支持直接输出，请使用文件");

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
        auto trait{::SoC::ofile_trait(file)};
        ::SoC::detail::print_wrapper(trait, ::std::forward<args_t>(args)...);
        if constexpr(flush)
        {
            if constexpr(::SoC::is_output_file_no_block_flushable<file_t>) { file.template flush<false>(); }
            else
            {
                file.template flush<true>();
            }
        }
    }

    /**
     * @brief 行尾序列
     *
     */
    enum class end_line_sequence : ::std::uint8_t
    {
        /// 回车
        cr,
        /// 换行
        lf,
        /// 回车换行
        crlf,
        /// 默认换行符
        default_endl = crlf
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
    template <::SoC::end_line_sequence endl = ::SoC::end_line_sequence::default_endl,
              ::SoC::is_sync_output_device<char> device_t,
              ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void println(device_t& device, args_t&&... args) noexcept
    {
        ::SoC::print(device, ::std::forward<args_t>(args)..., ::SoC::detail::get_endl<endl>());
    }

    template <::SoC::end_line_sequence endl = ::SoC::end_line_sequence::default_endl,
              ::SoC::is_async_output_device<char> device_t,
              ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void println(device_t& device, args_t&&... args) noexcept = delete("异步输出设备不支持直接输出，请使用文件");

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
              ::SoC::end_line_sequence endl = ::SoC::end_line_sequence::default_endl,
              ::SoC::is_output_file file_t,
              ::SoC::is_printable_to_file<file_t>... args_t>
    constexpr inline void println(file_t& file, args_t&&... args) noexcept
    {
        ::SoC::print<flush>(file, ::std::forward<args_t>(args)..., ::SoC::detail::get_endl<endl>());
    }
}  // namespace SoC

namespace SoC::detail
{
    /**
     * @brief 获取格式串打印所需参数
     *
     * @tparam type 不含占位符的字符数组类型列表
     */
    export template <typename... type>
    struct get_fmt_arg_t
    {
        ::std::tuple<type...> tuple;
        constexpr inline static auto no_placehold_num{sizeof...(type)};

        /**
         * @brief 获取根据格式串交错的参数
         *
         * @tparam index 格式串索引
         * @return std::string_view 字符串参数
         */
        template <::std::size_t index>
            requires (index < no_placehold_num)
        [[using gnu: always_inline, artificial]] constexpr inline ::std::string_view get_fmt_arg(auto&&) const noexcept
        {
            auto&& array{::std::get<index>(tuple)};
            return {array.begin(), array.end()};
        }

        /**
         * @brief 获取根据格式串交错的参数
         *
         * @tparam index 格式串索引
         * @param arg 参数
         * @return auto&& 返回参数本身
         */
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
     * @tparam parser_t 格式串解析器类型
     * @tparam output_t 可进行输出操作的类型
     * @tparam args_t 参数类型列表
     * @param output 可进行输出操作的对象
     * @param args 参数列表
     */
    template <::SoC::detail::is_fmt_parser parser_t, typename output_t, ::std::size_t... indexes, typename... args_t>
    constexpr inline void print_wrapper(output_t& output, ::std::index_sequence<indexes...>, args_t&&... args) noexcept
    {
        constexpr auto placehold_num{parser_t::get_placehold_num()};
        static_assert(placehold_num == sizeof...(args), "占位符个数和参数个数不同");
        constexpr auto no_placehold_num{parser_t::get_no_placehold_num()};
        if constexpr(no_placehold_num == 0)
        {
            // 不含需要交错输出的字符串
            ::SoC::detail::print_wrapper(output, ::std::forward<args_t>(args)...);
        }
        else
        {
            constexpr ::SoC::detail::get_fmt_arg_t split_string_tuple{parser_t::get_split_string_tuple()};
            constexpr auto tuple_index_array{parser_t::get_tuple_index_array()};
            if constexpr(placehold_num != 0)
            {
                ::SoC::detail::print_wrapper(
                    output,
                    split_string_tuple.template get_fmt_arg<tuple_index_array[indexes]>(
                        ::std::forward<args_t...[(tuple_index_array[indexes] - no_placehold_num) % placehold_num]>(
                            args...[(tuple_index_array[indexes] - no_placehold_num) % placehold_num]))...);
            }
            else
            {
                constexpr auto array{::std::get<0>(split_string_tuple.tuple)};
                ::SoC::detail::print_wrapper(output, ::std::string_view{array.begin(), array.end()});
            }
        }
    }

    /**
     * @brief 获取带行尾序列的格式化字符串
     *
     * @note 该函数在编译时完成格式串拼接
     * @tparam endl 行尾序列
     * @param fmt 格式串解析器
     * @return ::SoC::fmt_string 带行尾序列的格式化字符串
     */
    template <::SoC::end_line_sequence endl>
    consteval inline auto get_fmt_string_with_endl(::SoC::detail::is_fmt_parser auto fmt) noexcept
    {
        constexpr auto endl_string{::SoC::detail::get_endl<endl>()};
        constexpr auto fmt_string{fmt.get_fmt_string()};
        // fmt_string要求输入是空结尾的字符串，因此填充1位
        char buffer[fmt_string.size() + endl_string.size() + 1]{};
        ::std::ranges::copy(fmt_string, buffer);
        ::std::ranges::copy(endl_string, buffer + fmt_string.size());
        return ::SoC::fmt_string{buffer};
    }
}  // namespace SoC::detail

export namespace SoC
{
    /**
     * @brief 将源代码位置信息输出到设备或文件
     *
     * @tparam output_t 输出设备或文件类型
     * @param output 输出设备或文件
     * @param location 源代码位置信息
     * @param tmp_buffer 输出缓冲区
     */
    template <typename output_t>
        requires (::SoC::is_output_device<output_t, char> || ::SoC::is_output_file<output_t>)
    constexpr inline void
        do_print_arg(output_t& output, ::std::source_location location, ::SoC::unified_text_buffer tmp_buffer) noexcept
    {
        using namespace ::SoC::literal;
        using parser = decltype("文件: {}({}:{}) `{}`"_fmt);
        constexpr auto placehold_num{parser::get_placehold_num()};
        constexpr auto no_placehold_num{parser::get_no_placehold_num()};
        const auto wrapper{
            [&output, &tmp_buffer]<::std::size_t... indexes>(::std::index_sequence<indexes...>, auto... args) constexpr noexcept
            {
                constexpr ::SoC::detail::get_fmt_arg_t split_string_tuple{parser::get_split_string_tuple()};
                constexpr auto tuple_index_array{parser::get_tuple_index_array()};
                (::SoC::detail::do_print_arg_wrapper(
                     output,
                     split_string_tuple.template get_fmt_arg<tuple_index_array[indexes]>(
                         args...[(tuple_index_array[indexes] - no_placehold_num) % placehold_num]),
                     tmp_buffer),
                 ...);
            }};
        wrapper(::std::make_index_sequence<placehold_num + no_placehold_num>{},
                location.file_name(),
                location.line(),
                location.column(),
                location.function_name());
    }

    /**
     * @brief 将参数列表输出到设备
     *
     * @tparam fmt 格式串
     * @tparam device_t 输出设备类型
     * @tparam args_t 参数类型列表
     * @param device 输出设备
     * @param fmt 格式串，使用SoC::literal::operator""_fmt创建
     * @param args 参数列表
     */
    template <::SoC::is_sync_output_device<char> device_t, ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void print(device_t& device, ::SoC::detail::is_fmt_parser auto fmt, args_t&&... args) noexcept
    {
        ::SoC::detail::print_wrapper<decltype(fmt)>(device,
                                                    ::std::make_index_sequence<fmt.get_total_num()>{},
                                                    ::std::forward<args_t>(args)...);
    }

    template <::SoC::is_async_output_device<char> device_t, ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void print(device_t& device,
                                ::SoC::detail::is_fmt_parser auto fmt,
                                args_t&&... args) noexcept = delete("异步输出设备不支持直接输出，请使用文件");

    /**
     * @brief 将参数列表输出到文件
     *
     * @tparam flush 输出结束后是否刷新缓冲区
     * @tparam file_t 输出文件类型
     * @tparam args_t 参数类型列表
     * @param file 输出文件
     * @param fmt 格式串，使用SoC::literal::operator""_fmt创建
     * @param args 参数列表
     */
    template <bool flush = false, ::SoC::is_output_file file_t, ::SoC::is_printable_to_file<file_t>... args_t>
    constexpr inline void print(file_t& file, ::SoC::detail::is_fmt_parser auto fmt, args_t&&... args) noexcept
    {
        auto trait{::SoC::ofile_trait(file)};
        ::SoC::detail::print_wrapper<decltype(fmt)>(trait,
                                                    ::std::make_index_sequence<fmt.get_total_num()>{},
                                                    ::std::forward<args_t>(args)...);
        if constexpr(flush)
        {
            if constexpr(::SoC::is_output_file_no_block_flushable<file_t>) { file.template flush<false>(); }
            else
            {
                file.template flush<true>();
            }
        }
    }

    /**
     * @brief 将参数列表输出到设备，输出完成后换行
     *
     * @tparam endl 行尾序列
     * @tparam device_t 输出设备类型
     * @tparam args_t 参数类型列表
     * @param device 输出设备
     * @param fmt 格式串，使用SoC::literal::operator""_fmt创建
     * @param args 参数列表
     */
    template <::SoC::end_line_sequence endl = ::SoC::end_line_sequence::default_endl,
              ::SoC::is_sync_output_device<char> device_t,
              ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void println(device_t& device, ::SoC::detail::is_fmt_parser auto fmt, args_t&&... args) noexcept
    {
        ::SoC::print(device,
                     ::SoC::fmt_parser<::SoC::detail::get_fmt_string_with_endl<endl>(fmt)>{},
                     ::std::forward<args_t>(args)...);
    }

    template <::SoC::is_async_output_device<char> device_t, ::SoC::is_printable_to_device<device_t>... args_t>
    constexpr inline void println(device_t& device,
                                  ::SoC::detail::is_fmt_parser auto fmt,
                                  args_t&&... args) noexcept = delete("异步输出设备不支持直接输出，请使用文件");

    /**
     * @brief 将参数列表输出到文件，输出完成后换行
     *
     * @tparam flush 输出结束后是否刷新缓冲区
     * @tparam endl 行尾序列
     * @tparam file_t 输出文件类型
     * @tparam args_t 参数类型列表
     * @param file 输出文件
     * @param fmt 格式串，使用SoC::literal::operator""_fmt创建
     * @param args 参数列表
     */
    template <bool flush = false,
              ::SoC::end_line_sequence endl = ::SoC::end_line_sequence::default_endl,
              ::SoC::is_output_file file_t,
              ::SoC::is_printable_to_file<file_t>... args_t>
    constexpr inline void println(file_t& file, ::SoC::detail::is_fmt_parser auto fmt, args_t&&... args) noexcept
    {
        ::SoC::print<flush>(file,
                            ::SoC::fmt_parser<::SoC::detail::get_fmt_string_with_endl<endl>(fmt)>{},
                            ::std::forward<args_t>(args)...);
    }
}  // namespace SoC
