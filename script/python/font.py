import dataclasses
import typing
from pathlib import Path

import common
import numpy as np
from PIL import Image, ImageDraw, ImageFont


@dataclasses.dataclass
class font_info:
    output_size: tuple[int, int]
    font_size: float
    name: str
    xy: tuple[float, float]
    threshold: int


font_info_table: typing.Final[dict[str, font_info]] = {
    "16x16": font_info((16, 16), 17, "16x16", (3, -3), 72),
    "8x16": font_info((8, 16), 11.5, "8x16", (1, 1.125), 58),
}


def generate_font_data(font_path: str | Path, font_type: str) -> None:
    """生成字体数据文件

    Args:
        font_path (str | Path): 字体文件路径
        font_type (str): 字体类型
    """

    font_type_info = font_info_table[font_type]

    with (common.assets_dir / f"font_{font_type_info.name}.data").open("wb") as output:
        # 加载字体
        font = ImageFont.truetype(font_path, font_type_info.font_size)

        for char in R""" !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~""":
            w, h = font_type_info.output_size
            # 创建图像（灰度模式，白底）
            image = Image.new("L", (w, h), color=0)
            draw = ImageDraw.Draw(image)

            # 绘制字符，255表示白色
            x, y = font_type_info.xy
            draw.text((x, y), char, font=font, fill=255)

            bitmap: np.ndarray = (np.array(image) > font_type_info.threshold).astype(np.uint8)

            # 打印点阵
            for row in bitmap:
                print("".join(["#" if pixel else "." for pixel in row]))
            print()

            # 写入文件
            bitmap = np.packbits(bitmap[::-1, :].T, axis=1)
            output.write(bitmap[:, ::-1].tobytes())
            del image
