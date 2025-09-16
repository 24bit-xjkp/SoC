/**
 * @file common.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief 公共模块，包括crt和startup交互所需定义
 */

export module SoC.crt:common;
import SoC.std;

namespace SoC
{
    void _init() noexcept;
    void _fini() noexcept;

    template <typename type>
    using cursor_t = type*;

    using cxa_atexit_callback_t = void (*)(void*);
    using cxa_atexit_callback_arg_t = void*;

    constexpr inline auto max_cxa_at_exit_callback{32zu};
    extern ::std::array<::SoC::cxa_atexit_callback_t, ::SoC::max_cxa_at_exit_callback> cxa_at_exit_callback_array;
    extern ::std::array<::SoC::cxa_atexit_callback_arg_t, ::SoC::max_cxa_at_exit_callback> cxa_at_exit_callback_arg_array;
    extern ::std::size_t cxa_at_exit_callback_index;

    using isr_t = void (*)() noexcept;

    extern "C"
    {
        // 定义为函数形式以便放入constexpr的isr_table中
        void _estack() noexcept;
        void SystemInit();
        [[gnu::naked, noreturn]] void Reset_Handler() noexcept;
    }
}  // namespace SoC

extern "C++" int main();
