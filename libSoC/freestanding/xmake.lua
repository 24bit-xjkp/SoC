register_target_with_unit_test("SoC.freestanding", function ()
    add_files("*.cppm", {public = true})
    add_files("*.cpp")

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
end, function ()
    add_deps("SoC.std")
    set_kind("object")
end, function ()
    add_deps("SoC.std.unit_test")
    add_defines("SOC_IN_UNIT_TEST", {public = true})
    set_kind("shared")
end)
