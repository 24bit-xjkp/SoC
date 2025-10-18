local config_table = {toolchains = get_config("toolchain_host")}
add_requireconfs("*|soc_*", {plat = get_config("host"), arch = os.arch()})
add_requires("doctest", {configs = config_table})
add_requires("fakeit", {configs = table.join(config_table, {framework = "doctest"})})
set_arch(os.arch())
set_plat(get_config("host"))

target("unit_test_utils")
    add_files("utils.cppm", {public = true})
    add_files("main.cpp")
    add_deps("SoC.freestanding.unit_test")
    add_packages("doctest", "fakeit")
    set_kind("shared")
    add_rules("utils.symbols.export_all", {export_classes = true})
    set_enabled(get_config("build_unit_test"))
target_end()
target("unit_test")
    add_files("*.cppm")
    local regex = "*.cpp|main.cpp"
    add_files(regex)
    add_deps("unit_test_utils")
    add_packages("doctest", "fakeit")
    set_kind("binary")
    set_policy("build.c++.modules.fallbackscanner", true)
    set_enabled(get_config("build_unit_test"))

    for _, file in ipairs(os.files(regex)) do
        local name = path.basename(file)
        add_tests(name, {runargs = {"-ts=" .. name}})
    end
target_end()
