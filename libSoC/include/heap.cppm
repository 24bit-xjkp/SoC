/**
 * @file heap.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 预定义的堆分配器
 */

export module SoC:heap;
import :utils;

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
