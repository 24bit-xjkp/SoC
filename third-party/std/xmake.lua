local test_table = {}
if is_current_mode_support_fuzzer() then
    test_table["fuzzer"] = function () end
end
if is_current_mode_support_unit_test() then
    test_table["unit_test"] = function () end
end
register_target_with_test("SoC.std", function ()
    set_kind("object")

    local is_gcc = string.find(get_config("toolchain") or "", "gcc")
    local use_stdcxx = (get_config("runtimes") or ""):startswith("stdc++")
    if is_gcc or use_stdcxx then
        add_files("gcc.cppm", { public = true })
    else
        add_files("clang.cppm", { public = true })
    end

    on_load(function (target)
        import("utility.common")
        if common.is_gcc() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
end, function ()
    set_enabled(is_current_mode_support_stm32_target())
end, test_table)
