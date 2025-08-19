#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
import std;
import SoC.freestanding;

namespace SoC
{
    void assert_failed(::std::string_view message, ::std::source_location location)
    {
        DOCTEST_ADD_FAIL_AT(location.file_name(),
                            location.line(),
                            ::std::format("\n函数 `{}` 中断言失败: {}", location.function_name(), message));
        ::std::unreachable();
    }
}  // namespace SoC
