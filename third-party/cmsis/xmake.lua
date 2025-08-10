if is_mode("debug") then
    add_rules("releasedbg")
end
target("cmsis")
    set_kind("static")
    add_includedirs("include", {public = true})
    add_files("src/*.c", "src/*.cpp")
target_end()
