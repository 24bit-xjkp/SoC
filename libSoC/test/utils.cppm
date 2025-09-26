export module SoC.unit_test;

export import std;
export import SoC.freestanding;

export namespace SoC
{
    /**
     * @brief 断言失败异常
     *
     */
    struct assert_failed_exception : ::std::runtime_error
    {
        using ::std::runtime_error::runtime_error;
    };

    /**
     * @brief 适配SoC分配器的operator new/delete封装
     *
     */
    struct std_allocator
    {
        /**
         * @brief 分配内存
         *
         * @param size 要分配的内存大小
         * @return void* 分配的内存指针
         */
        constexpr inline static void* allocate(::std::size_t size) { return ::operator new (size); }

        /**
         * @brief 分配内存
         *
         * @tparam type 要分配的内存类型
         * @return type* 分配的内存指针
         */
        template <typename type>
        constexpr inline static type* allocate()
        {
            return ::operator new (sizeof(type), ::std::align_val_t{alignof(type)});
        }

        /**
         * @brief 分配内存
         *
         * @tparam type 要分配的内存类型
         * @param n 要分配的对象个数
         * @return ::SoC::allocation_result<type*> 分配的内存指针和对象个数
         */
        template <typename type>
        constexpr inline static ::SoC::allocation_result<type*> allocate(::std::size_t n)
        {
            return ::SoC::allocation_result<type*>{::operator new (sizeof(type) * n, ::std::align_val_t{alignof(type)}), n};
        }

        /**
         * @brief 释放内存
         *
         * @tparam type 要释放的内存类型
         * @param ptr 要释放的内存指针
         * @param n 要释放的对象个数
         */
        template <typename type>
        constexpr inline static void deallocate(type* ptr, ::std::size_t n = 1)
        {
            ::operator delete (ptr, sizeof(type) * n, ::std::align_val_t{alignof(type)});
        }

        /**
         * @brief 释放内存
         *
         * @param ptr 要释放的内存指针
         * @param size 要释放的内存大小
         */
        constexpr inline static void deallocate(void* ptr, ::std::size_t size) { ::operator delete (ptr, size); }

        /**
         * @brief 判断两个分配器是否相等
         *
         * @param lhs 左操作数
         * @param rhs 右操作数
         * @return 两个分配器是否相等
         */
        constexpr inline friend bool operator== (std_allocator lhs [[maybe_unused]], std_allocator rhs [[maybe_unused]]) noexcept
        {
            return true;
        }
    };
}  // namespace SoC

module :private;

namespace SoC
{
    extern "C++" void assert_failed(::std::string_view message, ::std::source_location location)
    {
        throw ::SoC::assert_failed_exception{::std::format("\n{}({}:{}): 函数 `{}` 中断言失败: {}",
                                                           location.file_name(),
                                                           location.line(),
                                                           location.column(),
                                                           location.function_name(),
                                                           message)};
    }
}  // namespace SoC
