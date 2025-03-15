target("cmsis")
    set_kind("static")
    add_includedirs("include", {public = true})
    add_files("src/*.c", "src/*.s")
    if string.find(get_config("toolchain") or "", "clang", 1, true) ~= nil then
        add_files("src/*.o")
    end
target_end()
