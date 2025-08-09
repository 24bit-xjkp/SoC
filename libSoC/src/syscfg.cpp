/**
 * @file syscfg.cp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32系统配置控制
 */

module SoC;
import :syscfg;

namespace SoC
{
    using namespace ::std::string_view_literals;

    ::SoC::syscfg::syscfg() noexcept
    {
        if constexpr(::SoC::use_full_assert) { ::SoC::assert(!is_enabled(), "初始化前此系统控制器不应处于使能状态"sv); }
        enable();
    }

    ::SoC::syscfg::~syscfg() noexcept
    {
        if(need_stop_clock) { disable(); }
    }

    ::SoC::syscfg::syscfg(syscfg&& other) noexcept { other.need_stop_clock = false; }

    void ::SoC::syscfg::enable() const noexcept { ::LL_APB2_GRP1_EnableClock(periph); }

    void ::SoC::syscfg::disable() const noexcept { ::LL_APB2_GRP1_DisableClock(periph); }

    bool ::SoC::syscfg::is_enabled() const noexcept { return ::LL_APB2_GRP1_IsEnabledClock(periph); }
}  // namespace SoC
