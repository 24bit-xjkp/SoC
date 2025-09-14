---注册带有单元测试的目标
---@param target_name string 目标名称
---@param target_define fun():void 目标定义函数
---@param main_target_only ?fun():void 仅定义主目标时调用的回调函数
---@param unit_test_only ?fun():void 仅定义单元测试目标时调用的回调函数
function register_target_with_unit_test(target_name, target_define, main_target_only, unit_test_only)
    target(target_name, function()
        target_define()
        if main_target_only then
            main_target_only()
        end
    end)
    target(target_name .. ".unit_test", function()
        target_define()
        set_arch(os.arch())
        set_plat(get_config("host"))
        set_enabled(get_config("build_unit_test"))
        if unit_test_only then
            unit_test_only()
        end
    end)
end
