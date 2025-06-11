import math

import matplotlib.pyplot as plt
import numpy as np
from common import *
from scipy import integrate


def generate_spwm_data(stm32_frequency: float, spwm_sample_point: int, output_frequency: int, output_type: type, need_plot=True) -> None:
    """生成spwm波形文件

    Args:
        stm32_frequency (float): stm32时钟频率
        spwm_sample_point (int): spwm采样点数
        output_frequency (int): 输出正弦波频率
        partial_type (type): 输出结果的数据类型
        need_plot (bool, optional): 是否进行绘图. Defaults to True.
    """

    pi2 = 2 * np.pi
    # 每样本点对应周期数
    clock_per_sample = math.floor(stm32_frequency / output_frequency / spwm_sample_point)
    all_spwm_point = np.zeros(spwm_sample_point * clock_per_sample if need_plot else 0, dtype=np.int8)

    partial_list = np.zeros(spwm_sample_point, dtype=output_type)
    for i in range(spwm_sample_point):
        start = i / spwm_sample_point * pi2
        end = start + 1 / spwm_sample_point * pi2
        res = integrate.quad(np.sin, start, end)[0]
        partial: int = round((res * spwm_sample_point / pi2 + 1) / 2 * clock_per_sample)
        partial_list[i] = partial
        begin = i * clock_per_sample
        if need_plot:
            all_spwm_point[begin : begin + partial] = 1
            all_spwm_point[begin + partial : begin + clock_per_sample] = -1

    if need_plot:
        x = np.linspace(0, pi2, spwm_sample_point * clock_per_sample)
        all_sin_point = np.sin(x)
        plt.figure(dpi=200)
        plt.plot(x, all_sin_point, color="b", linewidth=1)
        plt.plot(x, all_spwm_point, linewidth=0.075)

    (assets_dir / "spwm.data").write_bytes(partial_list)
