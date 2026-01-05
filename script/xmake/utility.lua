--- 判断当前构建模式是否支持stm32目标
--- @return boolean
function is_current_mode_support_stm32_target()
    return is_mode("debug", "release", "minsizerel", "releasedbg")
end

--- 判断当前构建模式是否支持单元测试
--- @return boolean
function is_current_mode_support_unit_test()
    return is_mode("debug", "release", "minsizerel", "releasedbg", "coverage")
end

--- 判断当前构建模式是否支持模糊测试
--- @return boolean
function is_current_mode_support_fuzzer()
    return is_mode("fuzzer")
end

--- 获取默认的package自定义构建模式
--- @return string
function get_default_package_custom_mode()
    local mode = get_config("mode") or "releasedbg"
    -- 将调试模式映射为发布调试模式
    local map = {
        debug = "releasedbg",
        release = "release",
        minsizerel = "minsizerel",
        releasedbg = "releasedbg",
    }
    return map[mode]
end

---注册带有测试的目标
---@param target_name string 目标名称
---@param target_define fun():void 目标定义函数
---@param main_target_only ?fun():void 仅定义主目标时调用的回调函数
---@param test_table table<string, fun():void> 单元测试配置表
function register_target_with_test(target_name, target_define, main_target_only, test_table)
    target(target_name, function()
        target_define()
        if main_target_only then
            main_target_only()
        end
    end)
    for test_name, test_callback in pairs(test_table) do
        target(target_name .. "." .. test_name, function()
            target_define()
            set_arch(os.arch())
            set_plat(get_config("host"))
            test_callback()
        end)
    end
end
