--- 判断当前构建模式是否支持stm32目标
--- @return boolean
function is_current_mode_support_stm32_target()
    return is_mode("debug", "release", "minsizerel", "releasedbg")
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
