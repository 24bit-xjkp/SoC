target("SoC.std")
    set_kind("static")
    add_files("*.cppm", {public = true})
    set_policy("build.c++.modules.fallbackscanner", true)

    on_load(function(target)
        import("utility.common")
        if common.is_gcc() then
            -- gcc下需要先构建完该目标，否则会报错
            target:set("policy", "build.fence", true)
        end
    end)
target_end()
