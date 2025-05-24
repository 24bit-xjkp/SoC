task("flash", function()
    on_run(function()
        import("core.base.option")
        import("core.project.project")
        import("core.project.config")
        import("core.base.task")
        task.run("build")
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

task("com", function()
    on_run(function()
        import("core.base.option")
        local args = { "-b", option.get("baud-rate"), "-f", option.get("flow-control"), "--databits", "8", "--parity",
            option.get("parity"), "--stopbits", "1" }
        if option.get("echo") then
            table.insert(args, "--echo")
        end
        table.insert(args, option.get("device"))
        os.execv("picocom", args, { envs = { LANG = "en_US" } })
    end)

    set_menu {
        usage = "xmake com [option]",
        description = "Connect to a serial device using picocom.",
        options = {
            { "b", "baud-rate", "kv", "115200",       "Set the baud rate of the uart port." },
            { "d", "device",    "kv", "/dev/ttyACM0", "Set the path of the serial device." },
            { "f", "flow-control", "kv", "n", "Set the flow control of the uart port.",
                "\t- x: software flow control, xon/xoff",
                "\t- h: hardware flow control",
                "\t- n: no flow control" },
            { "p", "parity", "kv", "n", "Set the parity of the uart port.",
                "\t- o: odd parity",
                "\t- e: even parity",
                "\t- n: no parity" },
            { nil, "echo", "k", nil, "Enable local echo for tty." }
        }
    }
end)
