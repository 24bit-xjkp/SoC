includes("*/xmake.lua")
add_requires("soc_stm32f4_hal_ll", {configs = {hse_value = get_config("hse_value"), assert = get_config("assert"), _custom_mode = get_config("mode")}})
target("SoC.stm32")
    set_kind("static")
    add_packages("soc_stm32f4_hal_ll", {public = true})
    add_deps("SoC.freestanding", "SoC.crt")
    add_files("include/*.cppm", {public = true})
    add_files("src/*.cppm")
    add_extrafiles("assets/*")
    set_pcxxheader("include/pch.hpp")
    add_includedirs("include")

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
