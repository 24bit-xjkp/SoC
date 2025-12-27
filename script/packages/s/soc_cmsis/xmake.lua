package("soc_cmsis")
    set_description("CMSIS package for ARM Cortex-M micro controllers.")
    add_defines("STM32F407xx")
    includes(path.join(os.scriptdir(), "../utils.lua"))
    package_register("cmsis")

    -- 添加fetch函数，避免多次安装soc_cmsis包
    on_fetch(function (package, opt)
        import("lib.detect.find_package")
        local script_dir = os.scriptdir()
        -- 使用根项目默认的build目录作为已安装包探测目录
        local global_package_dir = path.normalize(script_dir .. "/../../../../build/.packages")
        local install_dir_list = string.split(package:installdir(), "[/\\]+")
        -- 首字母, 包名称, 版本号, 构建哈希
        install_dir_list = table.slice(install_dir_list, #install_dir_list - 3)
        local fetch_path = path.join(global_package_dir, table.unpack(install_dir_list))
        local fetch_opt = {installdir = fetch_path, require_version = opt.require_version, buildhash = install_dir_list[#install_dir_list]}
        -- 查找已安装包
        return find_package("xmake::" .. package:name(), fetch_opt)
    end)
package_end()