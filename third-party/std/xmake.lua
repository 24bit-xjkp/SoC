register_target_with_unit_test("SoC.std", function ()
    set_kind("object")
    local use_stdcxx = (get_config("runtimes") or ""):startswith("stdc++")
    if string.find(get_config("toolchain") or "", "gcc") or use_stdcxx then
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
end)
