module;
#include <pch.hpp>
module SoC;

namespace SoC
{
    ::std::uint64_t(::SoC::systick_t::load)() const noexcept
    {
        auto i{index.load(::std::memory_order_relaxed)};
        ::std::atomic_signal_fence(::std::memory_order_acquire);
        return systick[i];
    }

    ::SoC::systick_t::operator ::std::uint64_t () const noexcept { return load(); }

    ::std::uint64_t(::SoC::systick_t::operator++)() noexcept
    {
        auto new_index{index.load(::std::memory_order_relaxed) ^ 1};
        auto result{systick[new_index] += 2};
        ::std::atomic_signal_fence(::std::memory_order_release);
        index.store(new_index, ::std::memory_order_relaxed);
        return result;
    }

    extern "C" void SysTick_Handler() noexcept { ++::SoC::systick; }
}  // namespace SoC

namespace SoC::detail
{
    [[gnu::noinline]] void wait_for(::SoC::cycles cycles) noexcept
    {
        if(cycles.rep <= 1) { return; }
        else
        {
            auto cnt{cycles.rep >> 1};
            asm volatile (
                "SoC_detail_wait_for_loop:\n"
                "subs %[counter], #1\n"
                "bne SoC_detail_wait_for_loop\n"
                : [counter] "+r"(cnt)
                :
                : "cc");
        }
    }

    void wait_for(::SoC::systicks ticks) noexcept
    {
        auto tick{ticks.rep};
        if(tick == 0) [[unlikely]] { return; }
        auto start_value{SysTick->VAL};
        auto end_tick{::SoC::systick + tick};

#pragma GCC unroll 0
        // 等待直到系统时刻到达预定值
        while(::SoC::systick < end_tick) { ::SoC::wait_for_interpret(); }
        // 自旋以补偿start_value带来的时间误差
        ::SoC::wait_until([start_value, end_tick] noexcept { return SysTick->VAL < start_value || ::SoC::systick > end_tick; });
    }
}  // namespace SoC::detail

namespace SoC
{
    using namespace ::std::string_view_literals;
    using namespace ::SoC::literal;

#define ASSERT_FAILED_MESSAGE "\N{ESCAPE}[31m断言失败:\N{ESCAPE}[39m\r\n"

#ifdef USE_FULL_ASSERT
    extern "C" [[noreturn]] [[using gnu: noinline, cold]] void
        c_assert_failed(const char* file_name, ::std::uint32_t line, const char* function_name) noexcept
    {
        ::SoC::println(::SoC::log_device, ASSERT_FAILED_MESSAGE "文件: {}({}) `{}`"_fmt, file_name, line, function_name);
        ::SoC::fast_fail();
    }
#endif

    extern "C++" [[noreturn]] [[using gnu: noinline, cold]] void assert_failed(::std::string_view message,
                                                                               ::std::source_location location) noexcept
    {
        ::SoC::println(::SoC::log_device, ASSERT_FAILED_MESSAGE "{}: {}"_fmt, location, message);
        ::SoC::fast_fail();
    }
}  // namespace SoC
