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
