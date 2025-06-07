#pragma once
#include "utils.hpp"

namespace SoC
{
    /// pid最大输出
    constexpr inline float max_pid_output{0.90f};

    /// pid最小输出
    constexpr inline float min_pid_output{0.f};

    /**
     * @brief pid控制
     *
     */
    struct pid
    {
    private:
        /// 目标偏置值
        float target_bias;
        /// 影子值，未添加偏置
        float shadow_target;
        /// 目标值，已添加偏置
        float target;
        /// 误差积分
        float error_sum{};
        /// 比例因子
        float kp;
        /// 积分因子
        float ki;
        /// 反向积分因子
        float kc;
        /// 目标最小值
        float target_min;
        /// 目标最大值
        float target_max;

    public:
        /**
         * @brief 初始化pid控制器
         *
         * @param target 目标值
         * @param kp 比例因子
         * @param ki 积分因子
         * @param kc 反向积分因子
         * @param target_min 目标最小值
         * @param target_max 目标最大值
         * @param target_bias 目标偏置值
         */
        inline explicit pid(float target,
                            float kp,
                            float ki,
                            float kc,
                            float target_min,
                            float target_max,
                            float target_bias) noexcept :
            target_bias{target_bias}, shadow_target{target}, target{target + target_bias}, kp{kp}, ki{ki}, kc{kc},
            target_min{target_min}, target_max{target_max}
        {
        }

        /**
         * @brief 获取pid目标值
         *
         * @return pid目标值
         */
        inline float get_target() const noexcept { return target; }

        /**
         * @brief 目标值步进
         *
         * @param delta 步进值
         */
        inline void step(float delta) noexcept { set_target(target + delta); }

        /**
         * @brief 设置pid目标值
         *
         * @param target 目标值
         */
        inline void set_target(float target) noexcept
        {
            target = ::std::min(target, target_max);
            target = ::std::max(target, target_min);
            shadow_target = target;
            this->target = shadow_target + target_bias;
        }

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
