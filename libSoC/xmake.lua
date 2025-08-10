includes("*/xmake.lua")
target("SoC.stm32")
    set_kind("static")
    add_deps("stm32f4_hal_ll", "SoC.freestanding", "SoC.crt")
    add_files("include/*.cppm", {public = true})
    add_files("src/*.cppm")
    add_extrafiles("assets/*")
    set_pcxxheader("include/pch.hpp")
    add_includedirs("include")
    set_policy("build.merge_archive", true)

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
