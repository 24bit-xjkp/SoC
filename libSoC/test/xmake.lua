local config_table = {toolchains = get_config("toolchain_host")}
local is_unit_test_support = is_current_mode_support_unit_test()
if is_unit_test_support then
    add_requireconfs("*|soc_*", {plat = get_config("host"), arch = os.arch()})
    add_requires("doctest >=2.4.12", {configs = config_table})
    add_requires("fakeit", {configs = table.join(config_table, {framework = "doctest"})})
end
set_arch(os.arch())
set_plat(get_config("host"))
add_packages("doctest", "fakeit")

target("unit_test_utils")
    add_files("test_framework.cpp")
    add_deps("SoC.freestanding.unit_test")
    set_kind("shared")
    add_rules("utils.symbols.export_all", {export_classes = true})
    set_default(false)
    set_enabled(is_unit_test_support)
target_end()
target("unit_test")
    local regex = "*.cpp|test_framework.cpp"
    add_files(regex)
    add_deps("unit_test_utils")
    set_kind("binary")
    set_default(is_mode("coverage"))
    set_enabled(is_unit_test_support)

    for _, file in ipairs(os.files(regex .. "|*_interface.cpp|utils_impl.cpp|main.cpp")) do
        local name = path.basename(file)
        add_tests(name, {runargs = {"-ts=" .. name}})
    end
target_end()
