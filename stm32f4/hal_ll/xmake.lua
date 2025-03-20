target("hal_ll")
    set_kind("static")
    add_deps("cmsis")
    on_load(function (target)
        import("utility.common")
        if not common.is_clang() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
    add_includedirs("include", "include/Legacy", {public = true})
    add_files("src/*.c")
    set_warnings("none")
target_end()
