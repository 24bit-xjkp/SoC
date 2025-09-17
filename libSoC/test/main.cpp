#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#if _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

namespace
{
    /**
     * @brief 用于在Windows平台下设置控制台编码为 UTF-8
     *
     */
    struct set_console_utf8
    {
    private:
        ::UINT origin_console_cp;
        ::UINT origin_console_output_cp;

    public:
        /**
         * @brief 构造函数，保存原始控制台编码，然后设置控制台编码为 UTF-8
         *
         */
        set_console_utf8() noexcept : origin_console_cp{::GetConsoleCP()}, origin_console_output_cp{::GetConsoleOutputCP()}
        {
            ::SetConsoleCP(CP_UTF8);
            ::SetConsoleOutputCP(CP_UTF8);
        }

        /**
         * @brief 析构函数，恢复原始控制台编码
         *
         */
        ~set_console_utf8() noexcept
        {
            ::SetConsoleCP(origin_console_cp);
            ::SetConsoleOutputCP(origin_console_output_cp);
        }

        set_console_utf8(const set_console_utf8&) = delete;
        set_console_utf8& operator= (const set_console_utf8&) = delete;
        set_console_utf8(set_console_utf8&&) = delete;
        set_console_utf8& operator= (set_console_utf8&&) = delete;
    } set_console_utf8_guard{};
}  // namespace
#endif
