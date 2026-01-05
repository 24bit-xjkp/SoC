target("SoC")
    set_kind("binary")
    add_deps("SoC.stm32")
    add_files("src/*.cpp", "src/*.ld")
    set_extension(".elf")
    set_enabled(is_current_mode_support_stm32_target())

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
target_end()
