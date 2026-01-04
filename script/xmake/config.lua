local script_dir = path.join(os.projectdir(), "script")
local toolchains_dir = path.join(script_dir, "toolchains", "xmake")
local xmake_dir = path.join(script_dir, "xmake")
includes(path.join(toolchains_dir, "*.lua"))
local xmake_script_table = { "option.lua", "register.lua", "utility.lua" }
for _, script in ipairs(xmake_script_table) do
    includes(path.join(xmake_dir, script))
end

rule("stm32_pc", function()
    on_load(function(target)
        local warning_flags = {
            "-Wno-c23-extensions",
            "-Wimplicit-fallthrough",
            "-Wno-unknown-pragmas",
            "-Wno-experimental-header-units",
        }
        if target:is_arch("arm") and target:is_plat("cross") then
            import("utility", { rootdir = xmake_dir })
            target:set("enabled", utility.is_current_mode_support_stm32_target())
            target:set("exceptions", "no-cxx")
            target:set("policy", "build.c++.modules.std", false)
            target:add("options", "assert")
            target:add("cxflags", "-mtune=cortex-m4", "-ffunction-sections", "-fdata-sections",
                table.unpack(warning_flags))
            target:add("cxxflags", "-fno-rtti", "-Wno-psabi")
            target:add("ldflags", "-Wl,--gc-sections", "-nostartfiles", { force = true })
            target:add("ldflags", "gcc::-Wno-psabi")
            target:set("toolchains", get_config("toolchain"))
        else
            target:set("exceptions", "cxx")
            target:add("defines", "USE_FULL_ASSERT")
            target:set("toolchains", get_config("toolchain_host"))
            target:set("policy", "build.c++.modules.std", true)
            target:set("policy", "build.sanitizer.address", get_config("unit_test_with_asan"))
            target:set("policy", "build.sanitizer.undefined", get_config("unit_test_with_ubsan"))
            target:set("runtimes", get_config("runtimes") == "c++_static" and "c++_shared" or "stdc++_shared")
            target:set("cxflags", "-fPIC", table.unpack(warning_flags))
        end
    end)
end)

--- 添加xmake工程通用配置
--- @param in_package boolean 是否在package中调用
function configure(in_package)
    set_xmakever("3.0.5")
    set_policy("check.auto_ignore_flags", false)
    set_policy("build.c++.modules.hide_dependencies", true)
    set_policy("build.c++.modules.non_cascading_changes", true)
    set_encodings("utf-8")
    add_moduledirs(toolchains_dir)
    add_repositories("third-party .", { rootdir = script_dir })
    set_warnings("allextra")
    set_languages("clatest", "cxxlatest")
    set_allowedmodes(support_rules_table)
    set_defaultmode("debug")
    set_config("debug_info", "minsizerel")
    local build_mode = in_package and get_config("_custom_mode") or get_config("mode")
    if build_mode then
        add_rules(build_mode)
        add_defines(string.format("SOC_BUILD_MODE_%s", string.upper(build_mode)))
    end
    add_rules("stm32_pc")
end
