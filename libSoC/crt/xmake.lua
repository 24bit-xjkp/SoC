target("SoC.crt")
    set_kind("static")
    add_files("*.cppm|common.cppm", {public = true})
    add_files("common.cppm")
    add_files("*.cpp")

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
