if is_mode("debug") then
    add_rules("releasedbg")
end
target("stm32f4_hal_ll")
    set_kind("static")
    add_deps("cmsis")
    add_includedirs("include", "include/Legacy", {public = true})
    add_files("src/*.c")
    set_warnings("none")
target_end()
