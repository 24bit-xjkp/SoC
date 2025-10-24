from pathlib import Path

from IPython.core.getipython import get_ipython

project_root = Path(__file__).parents[2]
SoC_dir = project_root / "libSoC"
assets_dir = SoC_dir / "assets"


def use_svg_in_ipython() -> None:
    """在IPython中使用svg格式绘图"""

    ipython = get_ipython()
    if ipython is not None:
        ipython.run_line_magic("config", "InlineBackend.figure_formats = 'svg'")
