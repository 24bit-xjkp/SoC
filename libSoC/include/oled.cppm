/**
 * @file oled.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 oled外设
 */

export module SoC:oled;
import :i2c;

export namespace SoC
{
    /**
     * @brief oled字体大小
     *
     */
    enum class oled_font_size : ::std::uint8_t
    {
        /// 8x16
        f8x16,
        /// 16x16
        f16x16
    };

    struct oled
    {
    private:
        constexpr inline static ::std::uint8_t slave_address{0x3c};
        constexpr inline static ::std::uint8_t command_prefix{0x00};
        constexpr inline static ::std::uint8_t data_prefix{0x40};
        inline static ::std::array<::std::array<::std::uint8_t, 128>, 8> buffer{};
        ::SoC::i2c& i2c;
        ::SoC::dma_stream dma_stream;
        ::std::uint8_t page{};
        ::std::uint8_t column{};

        /**
         * @brief 将字符串写入缓冲区并刷新
         *
         * @tparam font_size 字体大小
         * @param begin 字符串首指针
         * @param end 字符串尾哨位
         */
        template <::SoC::oled_font_size font_size>
        void do_write(const char* begin, const char* end) noexcept;

        /**
         * @brief 将字符串写入缓冲区并刷新
         *
         * @param begin 字符串首指针
         * @param end 字符串尾哨位
         */
        void write_f8x16(const char* begin, const char* end) noexcept;

        /**
         * @brief 将字符串写入缓冲区并刷新
         *
         * @param begin 字符串首指针
         * @param end 字符串尾哨位
         */
        void write_f16x16(const char* begin, const char* end) noexcept;

    public:
        /**
         * @brief 初始化oled显示屏
         *
         * @param i2c i2c外设
         * @param dma dma数据流
         */
        oled(::SoC::i2c& i2c, ::SoC::dma& dma) noexcept;

        /**
         * @brief 获取绑定的dma数据流
         *
         * @return dma数据流
         */
        inline ::SoC::dma_stream& get_dma() noexcept { return dma_stream; }

        /**
         * @brief 获取i2c外设
         *
         * @return i2c外设
         */
        inline ::SoC::i2c& get_i2c() noexcept { return i2c; }

        /**
         * @brief 写入oled命令
         *
         * @param command 要写入的命令
         */
        void write_command(::std::uint8_t command) const noexcept;

        /**
         * @brief 写入oled数据
         *
         * @param data 要写入的数据
         */
        void write_data(::std::uint8_t data) const noexcept;

        /**
         * @brief 设置光标位置
         *
         * @param page
         * @param column
         */
        void set_cursor(::std::uint8_t page, ::std::uint8_t column) noexcept;

        /**
         * @brief 将缓冲区内容刷新到oled并清空缓冲区
         *
         */
        void flush() noexcept;

        /**
         * @brief 初始化oled屏幕
         *
         */
        void init() noexcept;

        /**
         * @brief 将字符串写入缓冲区并刷新
         *
         * @tparam font_size 字体大小
         * @param begin 字符串首指针
         * @param end 字符串尾哨位
         */
        template <::SoC::oled_font_size font_size = ::SoC::oled_font_size::f16x16>
        void write(const char* begin, const char* end) noexcept
        {
            if constexpr(font_size == ::SoC::oled_font_size::f16x16) { write_f16x16(begin, end); }
            else if constexpr(font_size == ::SoC::oled_font_size::f8x16) { write_f8x16(begin, end); }
        }
    };
}  // namespace SoC
