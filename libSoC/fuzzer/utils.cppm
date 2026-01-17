export module SoC.fuzzer;

export import std;
export import SoC.freestanding;

export namespace SoC
{
    /**
     * @brief 表示断言失败的异常类型
     *
     */
    struct fuzzer_assert_failed_t
    {
        fuzzer_assert_failed_t(::std::size_t value) noexcept : value{value} {}

        /**
         * @brief 获取断言失败的枚举值
         *
         * @tparam type 枚举类型
         * @return type 枚举值
         */
        template <typename type>
            requires (::std::is_scoped_enum_v<type> && sizeof(type) == sizeof(::std::size_t))
        inline type get() const
        {
            return ::std::bit_cast<type>(value);
        }

    private:
        ::std::size_t value;
    };
}  // namespace SoC
