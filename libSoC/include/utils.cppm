/**
 * @file utils.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32的实用工具实现
 */

module;
#include <pch.hpp>
export module SoC:utils;
export import SoC.freestanding;
import SoC.std;

namespace SoC::detail
{
    /**
     * @brief 等待指定tick
     *
     * @param ticks 要等待的tick
     */
    void wait_for(::SoC::systicks ticks) noexcept;

    /**
     * @brief 等待指定周期数
     *
     * @param cycles 要等待的周期数
     */
    void wait_for(::SoC::cycles cycles) noexcept;
}  // namespace SoC::detail

export namespace SoC
{
    /**
     * @brief 等待直到发生中断
     *
     */
    [[using gnu: always_inline, artificial]] inline void wait_for_interpret() noexcept
    {
#ifdef __clang__
        ::__wfi();
#else
        __WFI();
#endif
    }

    /**
     * @brief 等待指定时间
     *
     * @param duration 要等待的时间
     */
    inline void wait_for(::SoC::detail::is_duration auto duration) noexcept
    {
        // 提供一个快速通道，降低延迟偏差
        if constexpr(::std::same_as<decltype(duration), ::SoC::cycles>)
        {
            using namespace ::SoC::literal;
            auto cycles{duration.rep};
            constexpr auto mini_wait_cycles{10zu};

            if(cycles < mini_wait_cycles) { return; }
            else if(cycles < 2_K) { return ::SoC::detail::wait_for(duration); }
        }

        auto ticks{duration.template duration_cast<::SoC::systicks>()};
        auto tmp{duration - ticks};
        auto cycles{tmp.template duration_cast<::SoC::cycles>()};

        ::SoC::detail::wait_for(ticks);
        ::SoC::detail::wait_for(cycles);
    }

    /// 系统时刻数
    struct systick_t
    {
    private:
        ::std::atomic_uint32_t index{};
        ::std::uint64_t systick[2]{0, 1};

    public:
        /**
         * @brief 递增系统时刻
         *
         * @return 递增后的系统时刻
         */
        ::std::uint64_t operator++ () noexcept;

        /**
         * @brief 读取系统时刻
         *
         * @return ::std::uint64_t 系统时刻
         */
        ::std::uint64_t load() const noexcept;

        /**
         * @brief 读取系统时刻
         *
         */
        operator ::std::uint64_t () const noexcept;
    } inline constinit systick{};
}  // namespace SoC

namespace SoC
{
    extern "C"
    {
        /// 主内存堆起始地址
        extern ::std::uintptr_t _ram_heap_start[];
        /// 主内存堆终止地址
        extern ::std::uintptr_t _ram_heap_end[];
        /// ccmram堆起始地址
        extern ::std::uintptr_t _ccmram_heap_start[];
        /// ccmram堆终止地址
        extern ::std::uintptr_t _ccmram_heap_end[];
    }

    /**
     * @brief 从主内存创建堆
     *
     * @return 用户堆
     */
    export inline ::SoC::heap make_ram_heap() noexcept { return ::SoC::heap{::SoC::_ram_heap_start, ::SoC::_ram_heap_end}; }

    /**
     * @brief 从ccmram创建堆
     *
     * @return ccmram堆
     */
    export inline ::SoC::heap make_ccmram_heap() noexcept
    {
        return ::SoC::heap{::SoC::_ccmram_heap_start, ::SoC::_ccmram_heap_end};
    }
}  // namespace SoC
