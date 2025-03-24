#pragma once
#include "utils.hpp"

namespace SoC
{
    namespace detail
    {
        /**
         * @brief usart外设基地址
         *
         */
        enum class usart : ::std::uintptr_t
        {
            usart1 = USART1_BASE,
            usart2 = USART2_BASE,
            usart3 = USART3_BASE,
            uart4 = UART4_BASE,
            uart5 = UART5_BASE,
            usart6 = USART6_BASE
        };
    }  // namespace detail

    /**
     * @brief usart工作模式
     *
     */
    enum class usart_mode : ::std::size_t
    {
        async,
        sync,
        default_mode = async
    };

    /**
     * @brief usart数据宽度
     *
     */
    enum class usart_data_width : ::std::size_t
    {
        bit8,
        bit9 = ::SoC::mask_single_one<12>,
        default_width = bit8
    };

    /**
     * @brief usart停止位数量
     *
     */
    enum class usart_stop_bit : ::std::size_t
    {
        bit0_5,
        bit1,
        bit1_5,
        bit2,
        default_bit = bit1
    };

    /**
     * @brief usart校验位
     *
     */
    enum class usart_parity : ::std::size_t
    {
        none,
        even = ::SoC::mask_single_one<10>,
        odd = ::SoC::mask_single_one<10> | ::SoC::mask_single_one<9>,
        default_parity = none
    };

    /**
     * @brief usart传输方向
     *
     */
    enum class usart_direction : ::std::size_t
    {
        none,
        rx = ::SoC::mask_single_one<2>,
        tx = ::SoC::mask_single_one<3>,
        rx_tx = rx | tx,
        default_direction = rx_tx
    };

    /**
     * @brief usart硬件流量控制
     *
     */
    enum class usart_hardware_flow_control : ::std::size_t
    {
        none,
        rts = ::SoC::mask_single_one<8>,
        cts = ::SoC::mask_single_one<9>,
        rts_cts = rts | cts,
        default_control = none
    };

    /**
     * @brief usart过采样倍数
     *
     */
    enum class usart_oversampling : ::std::size_t
    {
        by16 = 4,
        by8 = 3,
        default_oversampling = by16
    };

    /**
     * @brief usart外设
     *
     */
    struct usart
    {
    private:
        using usart_enum = ::SoC::detail::usart;

        struct dtor_callback_t
        {
            using callback_t = void (*)(::std::uint32_t);
            callback_t callback;
            ::std::uint32_t param;

            constexpr inline void operator() () const noexcept { callback(param); }
        };

        ::USART_TypeDef* usart_ptr{};
        ::SoC::usart_mode mode{};
        ::SoC::usart_data_width data_width{};
        ::SoC::usart_parity parity{};
        dtor_callback_t callback;

        void wait_until_write_complete() const noexcept;

    public:
        using enum usart_enum;

        /**
         * @brief 初始化usart外设
         *
         * @note 同步时钟、中断使能等配置需要单独进行
         * @param usart usart外设枚举
         * @param baud_rate 波特率
         * @param mode usart工作模式
         * @param data_width usart数据宽度
         * @param stop_bit usart停止位
         * @param parity usart校验位
         * @param direction usart传输方向
         * @param control usart硬件流量控制
         * @param oversampling usart过采样
         */
        explicit usart(::SoC::detail::usart usart,
                       ::std::uint32_t baud_rate,
                       ::SoC::usart_mode mode = ::SoC::usart_mode::async,
                       ::SoC::usart_data_width data_width = ::SoC::usart_data_width::bit8,
                       ::SoC::usart_stop_bit stop_bit = ::SoC::usart_stop_bit::default_bit,
                       ::SoC::usart_parity parity = ::SoC::usart_parity::default_parity,
                       ::SoC::usart_direction direction = ::SoC::usart_direction::default_direction,
                       ::SoC::usart_hardware_flow_control control = ::SoC::usart_hardware_flow_control::default_control,
                       ::SoC::usart_oversampling oversampling = ::SoC::usart_oversampling::default_oversampling) noexcept;

        constexpr inline usart(const usart&) noexcept = delete;
        constexpr inline usart& operator= (const usart&) noexcept = delete;

        usart(usart&& other) noexcept;
        usart& operator= (usart&&) noexcept;

        ~usart() noexcept;

        /**
         * @brief 获取usart外设指针
         *
         * @return ::USART_TypeDef* usart外设指针
         */
        inline ::USART_TypeDef* get_usart() const noexcept { return usart_ptr; }

        /**
         * @brief 获取usart外设工作状态
         *
         * @return ::SoC::usart_mode usart外设工作状态
         */
        inline ::SoC::usart_mode get_mode() const noexcept { return mode; }

        /**
         * @brief 向usart外设写入数据
         *
         * @param buffer 数据缓冲区首指针
         * @param end 数据缓冲区尾哨位
         * @return 写入是否成功
         */
        void write(const void* buffer, const void* end) const noexcept;

        /**
         * @brief 向usart外设写入数据
         *
         * @note 仅限数据宽度为9位且未启用校验位时使用
         * @param buffer 数据缓冲区首指针
         * @param end 数据缓冲区尾哨位
         * @return 写入是否成功
         */
        void write(const ::std::uint16_t* buffer, const ::std::uint16_t* end) const noexcept;

        /**
         * @brief 通过usart写入数据
         *
         * @param usart usart设备对象
         * @param buffer 缓冲区首指针
         * @param end 缓冲区尾哨位
         * @return 写入是否成功
         */
        inline static void write_wrapper(void* usart, const void* buffer, const void* end) noexcept
        {
            return reinterpret_cast<::SoC::usart*>(usart)->write(buffer, end);
        }

        /**
         * @brief 从usart读取数据
         *
         * @note 仅限数据宽度为8位时使用
         * @return 读取到的数据
         */
        ::std::byte read8() const noexcept;

        /**
         * @brief 从usart读取数据
         *
         * @note 仅限数据宽度为9位时使用
         * @return 读取到的数据
         */
         ::std::uint16_t read9() const noexcept;
    };
}  // namespace SoC
