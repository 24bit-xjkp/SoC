target("SoC.std")
    set_kind("object")
    if string.find(get_config("toolchain") or "", "gcc") then
        add_files("gcc.cppm", { public = true })
    else
        add_files("clang.cppm", { public = true })
    end

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()

target("SoC.std.unit_test")
    set_kind("object")
    set_arch(os.arch())
    set_plat(get_config("host"))
    set_enabled(get_config("build_unit_test"))
    if string.find(get_config("toolchain") or "", "gcc") then
        add_files("gcc.cppm", { public = true })
    else
        add_files("clang.cppm", { public = true })
    end

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
