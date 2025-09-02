export module SoC.unit_test;

export import std;
export import SoC.freestanding;

namespace SoC
{
    export struct assert_failed_exception : ::std::runtime_error
    {
        using ::std::runtime_error::runtime_error;
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
