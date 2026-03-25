module SoC.unit_test;

namespace SoC
{
    extern "C++" void assert_failed(::std::string_view message, ::std::source_location location)
    {
        throw ::SoC::assert_failed_exception{::std::format("\n[ERROR] {}({}:{}): 函数 `{}` 中断言失败: {}",
                                                           location.file_name(),
                                                           location.line(),
                                                           location.column(),
                                                           location.function_name(),
                                                           message)};
    }

    extern "C++" void yield_cpu() noexcept(::SoC::optional_noexcept) { ::std::this_thread::yield(); }

    /**
     * @brief 获取当前系统时刻
     *
     * @return 当前系统时刻
     */
    extern "C++" ::std::uint64_t get_systick() noexcept(::SoC::optional_noexcept)
    {
        using ratio_t = ::std::ratio_divide<::SoC::systick::ratio, ::SoC::second::ratio>;
        // chrono下的系统时刻周期
        using chrono_systick = ::std::chrono::duration<::std::uint64_t, ratio_t>;
        auto now{::std::chrono::system_clock::now()};
        return ::std::chrono::duration_cast<chrono_systick>(now.time_since_epoch()).count();
    }
}  // namespace SoC
