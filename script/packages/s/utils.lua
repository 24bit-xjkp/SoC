--- 注册第三方包
---@param package_name string 包名
---@param on_load_callback fun(package: table)? 加载回调函数
---@param on_install_callback fun(package: table, opt: table)? 安装回调函数
function package_register(package_name, on_load_callback, on_install_callback)
    set_sourcedir(path.join(os.scriptdir(), "../../../../third-party", package_name))
    add_configs("_custom_mode", { description = "The build mode for third party package.", default = "releasedbg" })

    on_load(function(package)
        package:config_set("shared", false)
        package:config_set("pic", false)
        if on_load_callback then
            on_load_callback(package)
        end
    end)

    on_install("cross|arm", function(package)
        local opt = {}
        for name, option in pairs(import("core.project.project").options()) do
            opt[name] = option:value()
        end
        opt["_custom_mode"] = get_config("mode")
        if on_install_callback then
            on_install_callback(package, opt)
        end
        import("package.tools.xmake").install(package, opt)
    end)
end
