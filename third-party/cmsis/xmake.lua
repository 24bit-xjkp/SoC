target("cmsis")
    set_kind("object")
    add_includedirs("include", {public = true})
    add_files("src/*.c")

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
