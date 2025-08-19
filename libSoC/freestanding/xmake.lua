target("SoC.freestanding")
    set_kind("object")
    add_deps("SoC.std")
    add_files("*.cppm", {public = true})
    add_files("*.cpp")

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()

target("SoC.freestanding.unit_test")
    set_kind("object")
    add_deps("SoC.std.unit_test")
    add_files("*.cppm", {public = true})
    add_files("*.cpp")
    set_arch(os.arch())
    set_plat(get_config("host"))
    set_enabled(get_config("build_unit_test"))

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
