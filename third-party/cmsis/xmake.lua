includes("script/xmake/config.lua")
configure(true)
set_version("1.0.0")
set_project("cmsis")

target("cmsis")
    set_kind("static")
    add_includedirs("include")
    add_headerfiles("include/*.h")
    add_files("src/*.c")
    add_defines("STM32F407xx")

    if get_config("_custom_mode") == "debug" then
        add_rules("releasedbg")
    end

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
