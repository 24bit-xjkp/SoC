includes("script/xmake/config.lua")
configure(true)
set_version("1.0.0")
set_project("stm32f4_hal_ll")

add_requires("soc_cmsis", {configs = {_custom_mode = get_config("_custom_mode")}})
target("stm32f4_hal_ll")
    add_packages("soc_cmsis")
    set_kind("static")
    add_includedirs("include", "include/Legacy")
    add_headerfiles("include/*.h")
    add_headerfiles("include/Legacy/*.h", {prefixdir = "Legacy"})
    add_defines("USE_FULL_LL_DRIVER")
    add_options("hse_value")
    add_files("src/*.c")
    set_warnings("none")
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
