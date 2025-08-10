target("SoC.std")
    set_kind("static")
    if string.find(get_config("toolchain") or "", "gcc") then
        add_files("gcc.cppm", { public = true })
    else
        add_files("clang.cppm", { public = true })
    end
target_end()
