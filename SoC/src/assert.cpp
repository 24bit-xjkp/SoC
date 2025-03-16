#include <stm32_assert.h>

[[noreturn]] void assert_failed(const char* file [[maybe_unused]], int line [[maybe_unused]], const char* func [[maybe_unused]])
{
    while(true);
}
