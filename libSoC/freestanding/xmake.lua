register_target_with_unit_test("SoC.freestanding", function ()
    add_files("*.cppm", {public = true})
    add_files("*.cpp")

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
    -- 对于嵌入式平台，设置为对象库以便和SoC.stm32合并生成一个静态库
    -- 由于静态库merge需要多开启一个ar进程，这在Windows下较慢，因此不采用静态库合并
    -- 对于单元测试，Windows下dll内必须实现全部函数，而SoC::assert_failed等函数需要外部实现
    -- 因此也设置为对象库
    set_kind("object")
end, function ()
    add_deps("SoC.std")
end, function ()
    add_deps("SoC.std.unit_test")
    add_defines("SOC_IN_UNIT_TEST", {public = true})
end)
