task("flash", function()
    on_run(function()
        import("core.base.option")
        import("core.project.project")
        import("core.project.config")
        config.load()
        local target = project.target("SoC")
        os.execv("pyocd", { "flash", "-t", option.get("type"), "-f", option.get("frequency"), target:targetfile() })
    end)
    set_category("plugin")
    set_menu {
        usage = "xmake flash [option]",
        description = "Program target to the flash of the mcu.",
        options = {
            { "f", "frequency", "kv", "10m",           "Set the frequency of the SWD clock." },
            { "t", "type",      "kv", "stm32f407zgtx", "Set the type of the mcu." }
        }
    }
end)
task_end()

task("gdb", function()
    on_run(function()
        import("core.base.option")
        import("core.project.project")
        import("core.project.config")
        config.load()
        local target = project.target("SoC")
        os.execv("pyocd",
            { "gdb", "-t", option.get("type"), "-f", option.get("frequency"), "--elf", target:targetfile() })
    end)
    set_category("plugin")
    set_menu {
        usage = "xmake gdb [option]",
        description = "Launch gdbserver.",
        options = {
            { "f", "frequency", "kv", "10m",           "Set the frequency of the SWD clock." },
            { "t", "type",      "kv", "stm32f407zgtx", "Set the type of the mcu." },
            { "p", "port",      "kv", "3333",          "The port number for the GDB server." }
        }
    }
end)
task_end()

task("erase", function()
    on_run(function()
        import("core.base.option")
        os.execv("pyocd",
            { "erase", "-t", option.get("type"), "-f", option.get("frequency"), "-c" })
    end)
    set_category("plugin")
    set_menu {
        usage = "xmake erase [option]",
        description = "Perform a chip erase on the flash of the mcu.",
        options = {
            { "f", "frequency", "kv", "10m",           "Set the frequency of the SWD clock." },
            { "t", "type",      "kv", "stm32f407zgtx", "Set the type of the mcu." },
        }
    }
end)
task_end()
