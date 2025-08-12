/**
 * @file allocator.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 独立的分配器接口实现
 */

module;
#include <version>
export module SoC.freestanding:allocator;
import SoC.std;

namespace SoC::detail
{
    /**
     * @brief 适用于常量表达式的分配器
     *
     * @tparam type 对象类型
     */
    template <typename type>
    [[gnu::unused]] constinit inline ::std::allocator<type> constexpr_allocator{};
}  // namespace SoC::detail

export namespace SoC
{
#ifdef __cpp_lib_allocate_at_least
    /**
     * @brief 分配连续多个元素时分配器返回的结果
     *
     * @tparam type 要分配的类型
     */
    template <typename type>
        requires (::std::is_pointer_v<type>)
    using allocation_result = ::std::allocation_result<type>;

    constexpr inline bool std_allocation_result_available{true};
#else
    /**
     * @brief 分配连续多个元素时分配器返回的结果
     *
     * @tparam type 要分配的类型
     */
    template <typename type>
        requires (::std::is_pointer_v<type>)
    struct allocation_result
    {
        type ptr;
        ::std::size_t count;
    };

    constexpr inline bool std_allocation_result_available{false};
#endif

    /**
     * @brief 判断type是否为分配器，要求满足：
     * - 是无状态分配器，即大小不超过一个指针，且
     * - 可默认构造和平凡复制和移动构造，且
     * - 可平凡复制和移动赋值，且
     * - 可相等性比较，且
     * - void* type::allocate(std::size_t) noexcept，且
     * - template<typename t> t* type::allocate() noexcept，且
     * - template<typename t> SoC::allocation_result<t*> type::allocate(std::size_t) noexcept，且
     * - void type::deallocate(auto* ptr, std::size_t = 1) noexcept，且
     * - void type::deallocate(void* ptr, std::size_t) noexcept，且
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_allocator =
        sizeof(type) <= sizeof(void*) && ::std::is_trivially_copy_constructible_v<type> &&
        ::std::is_trivially_move_constructible_v<type> && ::std::is_default_constructible_v<type> &&
        ::std::is_trivially_copy_assignable_v<type> && ::std::is_trivially_move_assignable_v<type> &&
        ::std::equality_comparable<type> && requires(type& allocator, int* ptr, ::std::size_t n, void* void_ptr) {
            { allocator.allocate(n) } noexcept -> ::std::same_as<void*>;
            { allocator.template allocate<int>() } noexcept -> ::std::same_as<int*>;
            { allocator.template allocate<int>(n) } noexcept -> ::std::same_as<::SoC::allocation_result<int*>>;
            { allocator.deallocate(ptr) } noexcept -> ::std::same_as<void>;
            { allocator.deallocate(ptr, n) } noexcept -> ::std::same_as<void>;
            { allocator.deallocate(void_ptr, n) } noexcept -> ::std::same_as<void>;
        };

    /**
     * @brief 将SoC分配器包装为标准分配器
     *
     * @tparam allocator_t 分配器类型
     * @tparam type 对象类型
     */
    template <::SoC::is_allocator allocator_t, typename type>
    struct allocator_wrapper
    {
        [[no_unique_address]] allocator_t allocator;

        using value_type = type;
        using size_type = ::std::size_t;
        using difference_type = ::std::ptrdiff_t;
        using propagate_on_container_move_assignment = ::std::true_type;

        /**
         * @brief 分配内存
         *
         * @param n 分配对象个数
         * @return 分配内存区域首指针
         */
        constexpr inline type* allocate(::std::size_t n) noexcept { return allocator.template allocate<type>(n).ptr; }

        constexpr inline auto allocate_at_least(::std::size_t n) noexcept
            requires (::SoC::std_allocation_result_available)
        {
            return allocator.template allocate<type>(n);
        }

        /**
         * @brief 释放内存
         *
         * @param ptr 分配内存区域首指针
         * @param n 分配对象个数
         */
        constexpr inline void deallocate(type* ptr, ::std::size_t n) noexcept { return allocator.deallocate(ptr, n); }

        /**
         * @brief 比较两个分配器对象是否相等
         *
         */
        constexpr inline friend bool operator== (::SoC::allocator_wrapper<allocator_t, type> lhs,
                                                 ::SoC::allocator_wrapper<allocator_t, type> rhs) noexcept
        {
            return lhs.allocator == rhs.allocator;
        }
    };

    /**
     * @brief 适用于常量表达式的分配器
     *
     */
    struct constexpr_allocator_t
    {
        /**
         * @brief 在常量表达式中分配内存
         *
         * @tparam type 对象类型
         * @return 分配内存区域首指针
         */
        template <typename type>
        consteval inline auto allocate() const noexcept
        {
            return ::SoC::detail::constexpr_allocator<type>.allocate(1);
        }

        /**
         * @brief 在常量表达式中分配内存
         *
         * @tparam type 对象类型
         * @param n 分配对象个数
         * @return 分配内存区域首指针
         */
        template <typename type>
        consteval inline auto allocate(::std::size_t n) const noexcept
        {
            if constexpr(::SoC::std_allocation_result_available)
            {
                return ::SoC::detail::constexpr_allocator<type>.allocate_at_least(n);
            }
            else
            {
                return ::SoC::allocation_result<type>{::SoC::detail::constexpr_allocator<type>.allocate(n), n};
            }
        }

        /**
         * @brief 在常量表达式中释放内存
         *
         * @tparam type 对象类型
         * @param ptr 分配内存区域首指针
         * @param n 分配对象个数
         */
        template <typename type>
        consteval inline void deallocate(type* ptr, ::std::size_t n = 1) const noexcept
        {
            ::SoC::detail::constexpr_allocator<type>.deallocate(ptr, n);
        }

        /**
         * @brief 比较两个分配器对象是否相等
         *
         */
        constexpr inline friend bool operator== (::SoC::constexpr_allocator_t, ::SoC::constexpr_allocator_t) noexcept
        {
            return true;
        }
    } inline constexpr constexpr_allocator;
}  // namespace SoC
