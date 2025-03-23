target("cmsis")
    set_kind("static")
    on_load(function (target)
        import("utility.common")
        if not common.is_clang() then
            target:set("policy", "build.optimization.lto", false)
        end
    end)
    add_includedirs("include", {public = true})
    add_files("src/*.c", "src/*.cpp")
target_end()
