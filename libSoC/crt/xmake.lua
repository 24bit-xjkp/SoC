target("SoC.crt")
    set_kind("object")
    add_deps("SoC.std")
    add_files("*.cppm", {public = true})
    add_files("*.cpp")
    if is_mode("debug") then
        add_rules("releasedbg")
    end
    set_enabled(is_current_mode_support_stm32_target())

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
