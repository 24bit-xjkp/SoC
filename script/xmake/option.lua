option("assert", function()
    set_default(true)
    set_description("Whether to use assert in stm32 hal/ll and SoC.")
    add_defines("USE_FULL_ASSERT")
end)

option("build_unit_test", function()
    set_default(true)
    set_description("Whether to build unit test.")
end)

local option_warning_prefix = "${color.warning}WARNING:${default} "

option("unit_test_with_asan", function()
    set_default(true)
    set_description("Whether to build unit test with address sanitizer.")
end)

option("unit_test_with_ubsan", function()
    set_default(true)
    set_description("Whether to build unit test with undefined behavior sanitizer.")
end)
