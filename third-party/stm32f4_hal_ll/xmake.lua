target("stm32f4_hal_ll")
    set_kind("static")
    add_deps("cmsis")
    add_includedirs("include", "include/Legacy", {public = true})
    add_files("src/*.c")
    set_warnings("none")
    if is_mode("debug") then
        add_rules("releasedbg")
    end

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
