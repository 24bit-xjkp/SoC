/**
 * @file syscfg.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32系统配置控制接口
 */

module;
#include <pch.hpp>
export module SoC:syscfg;
import :utils;

namespace SoC
{
    /**
     * @brief 系统配置控制
     *
     */
    struct syscfg
    {
    private:
        /// 析构时是否需要关闭时钟
        bool need_stop_clock{true};
        /// 外设时钟号
        constexpr inline static auto periph{LL_APB2_GRP1_PERIPH_SYSCFG};

    public:
        /**
         * @brief 开启系统配置控制器时钟
         *
         */
        explicit syscfg() noexcept;

        /**
         * @brief 关闭系统配置控制器时钟
         *
         */
        ~syscfg() noexcept;

        inline syscfg(const syscfg&) noexcept = delete;
        inline syscfg& operator= (const syscfg&) = delete;
        syscfg(syscfg&&) noexcept;
        inline syscfg& operator= (syscfg&&) = delete;

        /**
         * @brief 使能系统控制器时钟
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能系统控制器时钟
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断系统控制器时钟是否使能
         *
         * @return 系统控制器时钟是否使能
         */
        bool is_enabled() const noexcept;
    };
}  // namespace SoC
