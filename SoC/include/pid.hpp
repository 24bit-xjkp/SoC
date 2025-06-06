#pragma once
#include "utils.hpp"

namespace SoC
{
    /// pid最大输出
    constexpr inline float max_pid_output{0.90f};

    /// pid最小输出
    constexpr inline float min_pid_output{0.05f};

    /**
     * @brief pid控制
     *
     */
    struct pid
    {
    private:
        float target;
        float error_sum{};
        float kp;
        float ki;
        float kc;

    public:
        /**
         * @brief 初始化pid控制器
         *
         * @param target 目标值
         */
        inline explicit pid(float target, float kp, float ki, float kc) noexcept : target{target}, kp{kp}, ki{ki}, kc{kc} {}

        /**
         * @brief 设置pid目标值
         *
         * @param target 目标值
         */
        inline void set_target(float target) noexcept { this->target = target; }

        /**
         * @brief 计算pid输出
         *
         * @param input 输入值
         * @return float pid输出
         */
        inline float operator() (float input) noexcept
        {
            auto error{target - input};
            error_sum += error;
            auto output{kp * error + ki * error_sum};
            auto old_output{output};
            // 限幅
            output = ::std::min(output, ::SoC::max_pid_output);
            output = ::std::max(output, ::SoC::min_pid_output);
            error_sum += (output - old_output) * kc;
            return output;
        }
    };
}  // namespace SoC
