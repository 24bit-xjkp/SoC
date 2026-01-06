export module SoC.fuzzer;

export import std;
export import SoC.freestanding;

export namespace SoC
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
}  // namespace SoC
