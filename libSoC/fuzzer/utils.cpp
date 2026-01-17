module SoC.fuzzer;

namespace SoC
{
    extern "C++" void assert_failed(::std::string_view message, ::std::source_location location)
    {
        ::std::println(::std::cerr,
                       "[ERROR] {}({}:{}): 函数 `{}` 中断言失败: {}",
                       location.file_name(),
                       location.line(),
                       location.column(),
                       location.function_name(),
                       message);
        __builtin_trap();
    }

    extern "C++" void fuzzer_assert_failed(::std::size_t value)
    {
        // NOLINTNEXTLINE(hicpp-exception-baseclass)
        throw ::SoC::fuzzer_assert_failed_t{value};
    }
}  // namespace SoC
