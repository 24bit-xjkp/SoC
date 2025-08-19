add_requires("doctest", {plat = get_config("host"), arch = os.arch(), configs = {toolchains = get_config("toolchain_host")}})
set_arch(os.arch())
set_plat(get_config("host"))
add_packages("doctest")

target("unit_test_utils")
    add_files("utils.cpp")
    add_deps("SoC.freestanding.unit_test")
    set_kind("static")
    set_enabled(get_config("build_unit_test"))
target_end()
target("unit_test")
    add_files("*.cpp|utils.cpp")
    add_deps("unit_test_utils")
    set_kind("binary")
    add_includedirs(".")
    set_pcxxheader("doctest_pch.hpp")
    set_enabled(get_config("build_unit_test"))

    for _, file in ipairs(os.files("*.cpp|utils.cpp")) do
        local name = path.basename(file)
        add_tests(name, {runargs = {"-tc", name.."*"}})
    end
target_end()
