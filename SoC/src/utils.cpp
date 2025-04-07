#include "../include/utils.hpp"

namespace SoC
{
    extern "C" void SysTick_Handler() noexcept { ::SoC::systick++; }
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
        // 清除溢出标记
        volatile auto _{SysTick->CTRL};

#pragma GCC nounroll
        // 等待直到系统时刻到达预定值
        while(::SoC::systick < end_tick) { ::SoC::wait_for_interpret(); }
        // 自旋以补偿start_value带来的时间误差
        ::SoC::wait_until([start_value, end_tick] noexcept { return SysTick->VAL < start_value || ::SoC::systick > end_tick; });
    }
}  // namespace SoC::detail

namespace SoC
{
    ::std::pair<int, float> normalize(float in) noexcept
    {
        constexpr auto log10_2{::std::numbers::ln2_v<float> / ::std::numbers::ln10_v<float>};
        constexpr auto log2_10{::std::numbers::ln10_v<float> / ::std::numbers::ln2_v<float>};
        auto log2{::std::log2(in)};
        auto log10{::std::floor(log2 * log10_2)};
        log2 -= log10 * log2_10;
        in = ::std::exp2(log2);
        return {static_cast<::std::int32_t>(log10), in};
    }

    char* ftoa(char* buffer, float in, ::std::size_t precision) noexcept
    {
        if(in < 0)
        {
            *buffer++ = '-';
            in = ::std::abs(in);
        }
        if(in < 1e-8)
        {
            *buffer++ = '0';
            return buffer;
        }
        constexpr char table[]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
        using namespace ::SoC::literal;
        constexpr float pow_table[]{1, 10, 100, 1_K, 10_K, 100_K, 1_M, 10_M, 100_M};
        auto&& [log10, res]{::SoC::normalize(in)};
        auto integer{static_cast<::std::size_t>(res)};
        *buffer++ = table[static_cast<::std::size_t>(integer)];
        res -= integer;
        if(precision != 0)
        {
            *buffer++ = '.';
            res *= pow_table[precision];
            buffer = ::std::to_chars(buffer, buffer + 32, static_cast<::std::int32_t>(res)).ptr;
        }
        if(log10 != 0)
        {
            *buffer++ = 'e';
            buffer = ::std::to_chars(buffer, buffer + 32, log10).ptr;
        }
        return buffer;
    }
}  // namespace SoC
