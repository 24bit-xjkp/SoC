from pathlib import Path

root = Path(__file__).parent
inc_list = [file for file in (root / "std").iterdir()]
inc_list.sort()
global_include_list = [f"#include <{file.stem}>" for file in inc_list]
export_include_list = [f'#include "std/{file.name}"' for file in inc_list]
template = f"""module;
{"\n".join(global_include_list)}

export module SoC.std;
{"\n".join(export_include_list)}
"""

interface_path = root / "clang.cppm"
interface_path.write_text(template)
