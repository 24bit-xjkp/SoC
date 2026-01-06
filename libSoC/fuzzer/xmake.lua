set_arch(os.arch())
set_plat(get_config("host"))

local is_fuzzer_support = is_current_mode_support_fuzzer()
target("fuzzer_utils")
    add_files("utils.cppm", {public = true})
    add_deps("SoC.freestanding.fuzzer")
    set_kind("object")
    set_enabled(is_fuzzer_support)
target_end()

for _, file in ipairs(os.files("*.cpp")) do
    local name = "fuzzer_" .. path.basename(file)
    target(name)
        add_files(file)
        add_deps("fuzzer_utils")
        add_tests("fuzzer", { runargs = { "-timeout=20", "-max_total_time=14400", "-jobs=" .. 2, "-use_value_profile=1" } })
        set_enabled(is_fuzzer_support)

        on_load(function (target)
            target:set("targetdir", path.join(target:targetdir(), name))
        end)
    target_end()
end
