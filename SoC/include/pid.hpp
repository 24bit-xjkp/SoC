#pragma once
#include "utils.hpp"

namespace SoC
{
    /// pid最大输出
    constexpr inline float max_pid_output{0.95f};

    /// pid最小输出
    constexpr inline float min_pid_output{0.05f};

    /**
     * @brief pid控制
     *
     * @tparam kp 比例系数
     * @tparam ki 积分系数
     * @tparam kd 微分系数
     */
    template <float kp, float ki, float kd>
        requires (kp != 0 && ki != 0)
    struct pid
    {
    private:
        float target;
        float last_input;
        float error_sum{};

    public:
        /**
         * @brief 初始化pid控制器
         *
         * @param target 目标值
         */
        inline explicit pid(float target) noexcept : target{target}, last_input{target} {}

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
            if constexpr(kd != 0.f) { output += kd * (input - last_input); }
            // 限幅
            output = ::std::max(output, ::SoC::max_pid_output);
            output = ::std::min(output, ::SoC::min_pid_output);
            return output;
        }
    };
}  // namespace SoC
