#pragma once
#include "utils.hpp"

namespace SoC::detail
{
    constexpr auto dynamic_extent{::std::dynamic_extent};
    template <::std::size_t extent>
    using bitmap = ::std::span<::std::size_t, extent>;

    /**
     * @brief 查找1个空闲位并标记为已分配
     *
     * @tparam extent 位图大小
     * @param bitmap 位图对象
     * @return ::std::size_t 位索引，-1表示未找到
     */
    template <::std::size_t extent>
    constexpr inline ::std::size_t find(::SoC::detail::bitmap<extent> bitmap) noexcept
    {
        for(auto block_index{0zu}; block_index != bitmap.size(); ++block_index)
        {
            if(auto& block{bitmap[block_index]}; block != -1)
            {
                auto index{::std::countr_one(block)};
                block |= 1zu << index;
                return block_index * 32 + index;
            }
        }
        return -1zu;
    }

    /**
     * @brief 填充位图中的连续空间
     *
     * @param block_begin 起始块引用
     * @param block_end 结束块引用
     * @param begin_index 起始块内索引
     * @param end_index 结束块内索引
     */
    constexpr inline void
        set(::std::size_t& block_begin, ::std::size_t& block_end, ::std::size_t begin_index, ::std::size_t end_index) noexcept
    {
        block_begin |= -1zu << begin_index;
        block_end |= -1zu >> (31 - end_index);
        for(auto begin{&block_begin + 1}, end{&block_end - 1}; begin < end; ++begin) { *begin = -1zu; }
    }

    /**
     * @brief 查找连续len个空闲位并标记为已分配
     *
     * @tparam extent 位图大小
     * @param bitmap 位图对象
     * @param len 连续位数量
     * @return ::std::size_t 位索引
     */
    template <::std::size_t extent>
    constexpr inline ::std::size_t find(::SoC::detail::bitmap<extent> bitmap, ::std::size_t len) noexcept
    {
        return 0;
    }
}  // namespace SoC::detail

namespace SoC
{
    /**
     * @brief 分配连续多个元素时分配器返回的结果
     *
     * @tparam type 要分配的类型
     */
    template <typename type>
    using allocation_result = ::std::allocation_result<type>;

    /**
     * @brief 判断type是否为分配器，要求满足：
     * - 是无状态分配器，即大小不超过一个指针，且
     * - 可默认构造和复制构造，且
     * - template<typename t> t* type::allocate() noexcept，且
     * - template<typename t> SoC::allocation_result<t*> type::allocate(std::size_t) noexcept，且
     * - void type::deallocate(auto* ptr, std::size_t = 1) noexcept
     * @tparam type 要判断的类型
     */
    template <typename type>
    concept is_allocator = sizeof(type) <= sizeof(void*) && ::std::is_trivially_copy_constructible_v<type> &&
                           ::std::is_default_constructible_v<type> && requires(type& allocator, int* ptr) {
                               { allocator.template allocate<int>() } noexcept -> ::std::same_as<int*>;
                               { allocator.template allocate<int>(2) } noexcept -> ::std::same_as<::SoC::allocation_result<int*>>;
                               { allocator.deallocate(ptr) } noexcept -> ::std::same_as<void>;
                               { allocator.deallocate(ptr, 2) } noexcept -> ::std::same_as<void>;
                           };
}  // namespace SoC
