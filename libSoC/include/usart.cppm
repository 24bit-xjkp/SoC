/**
 * @file usart.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 usart外设
 */

module;
#include "pch.hpp"
export module SoC:usart;
import :dma;

namespace SoC::detail
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
}  // namespace SoC::detail

export namespace SoC
{

    /**
     * @brief usart工作模式
     *
     */
    enum class usart_mode : ::std::uint8_t
    {
        /// 异步模式
        async = 0,
        /// 同步模式
        sync = 1,
        /// 默认工作模式
        default_mode = async
    };

    /**
     * @brief usart数据宽度
     *
     */
    enum class usart_data_width : ::std::uint16_t
    {
        /// 8位数据宽度
        bit8 = LL_USART_DATAWIDTH_8B,
        /// 9位数据宽度
        bit9 = LL_USART_DATAWIDTH_9B,
        /// 默认数据宽度
        default_width = bit8
    };

    /**
     * @brief usart停止位数量
     *
     */
    enum class usart_stop_bit : ::std::uint8_t
    {
        /// 0.5个停止位
        bit0_5 = 0,
        /// 1个停止位
        bit1 = 1,
        /// 1.5个停止位
        bit1_5 = 2,
        /// 2个停止位
        bit2 = 3,
        /// 默认停止位数量
        default_bit = bit1
    };

    /**
     * @brief usart校验位
     *
     */
    enum class usart_parity : ::std::uint16_t
    {
        /// 无校验位
        none = LL_USART_PARITY_NONE,
        /// 偶数校验位
        even = LL_USART_PARITY_EVEN,
        /// 奇数校验位
        odd = LL_USART_PARITY_ODD,
        /// 默认校验位
        default_parity = none
    };

    /**
     * @brief usart传输方向
     *
     */
    enum class usart_direction : ::std::uint8_t
    {
        /// 无方向
        none = LL_USART_DIRECTION_NONE,
        /// 接收方向
        rx = LL_USART_DIRECTION_RX,
        /// 发送方向
        tx = LL_USART_DIRECTION_TX,
        /// 接收发送方向
        rx_tx = LL_USART_DIRECTION_TX_RX,
        /// 默认传输方向
        default_direction = rx_tx
    };

    /**
     * @brief usart硬件流量控制
     *
     */
    enum class usart_hardware_flow_control : ::std::uint16_t
    {
        /// 无硬件流量控制
        none = LL_USART_HWCONTROL_NONE,
        /// 发送RTS信号
        rts = LL_USART_HWCONTROL_RTS,
        /// 接收CTS信号
        cts = LL_USART_HWCONTROL_CTS,
        /// 发送RTS信号，接收CTS信号
        rts_cts = LL_USART_HWCONTROL_RTS_CTS,
        /// 默认硬件流量控制
        default_control = none
    };

    /**
     * @brief usart过采样倍数
     *
     */
    enum class usart_oversampling : ::std::uint8_t
    {
        /// 16倍过采样
        by16 = 4,
        /// 8倍过采样
        by8 = 3,
        /// 默认过采样倍数
        default_oversampling = by16
    };

    /**
     * @brief usart外设
     *
     */
    struct usart
    {
        using usart_enum = ::SoC::detail::usart;
        using enum usart_enum;

    private:
        ::SoC::moveable_value<::USART_TypeDef*> usart_ptr{};
        ::SoC::detail::dtor_close_clock_callback_t callback{};
        ::IRQn_Type irqn{};
        ::SoC::usart_data_width data_width{};

        void wait_until_write_complete() const noexcept;

        /**
         * @brief 断言dma外设为指定外设
         *
         * @param dma dma外设
         * @param dma_enum 指定dma外设对应的枚举
         */
        static void assert_dma(::SoC::dma& dma, ::SoC::dma::dma_enum dma_enum) noexcept;

        /// 没有选定的dma数据流，即使用模式配置
        constexpr inline static auto no_selected_stream{static_cast<::SoC::dma_stream::dma_stream_enum>(-1zu)};

    public:
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
        explicit usart(usart_enum usart,
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

        usart(usart&& other) noexcept = default;
        usart& operator= (usart&&) noexcept = delete;

        ~usart() noexcept;

        struct usart_dma_stream : ::SoC::dma_stream
        {
            ::SoC::usart& usart;

            /**
             * @brief 检查usart和dma是否准备好写入数据
             *
             * @note usart发送寄存器为空且dma传输完成即认为就绪
             * @return 是否准备好写入数据
             */
            [[nodiscard]] bool is_write_ready() const noexcept { return usart.get_flag_txe() && is_transfer_complete(); }

        private:
            friend struct usart;

            usart_dma_stream(::SoC::usart& usart,
                             ::SoC::dma& dma,
                             ::SoC::dma_stream::dma_stream_enum stream,
                             ::SoC::dma_channel channel,
                             ::SoC::dma_mode mode,
                             ::SoC::dma_fifo_threshold fifo_threshold,
                             ::SoC::dma_memory_burst default_burst,
                             ::SoC::dma_memory_data_size default_data_size,
                             ::SoC::dma_priority priority) noexcept :
                ::SoC::dma_stream{dma,
                                  stream,
                                  channel,
                                  ::LL_USART_DMA_GetRegAddr(usart.usart_ptr),
                                  ::SoC::dma_direction::m2p,
                                  mode,
                                  false,
                                  true,
                                  ::SoC::dma_periph_data_size::byte,
                                  default_data_size,
                                  priority,
                                  fifo_threshold,
                                  default_burst,
                                  ::SoC::dma_periph_burst::single},
                usart{usart}
            {
            }
        };

        /**
         * @brief 获取usart外设指针
         *
         * @return usart外设指针
         */
        [[nodiscard]] inline ::USART_TypeDef* get_usart() const noexcept { return usart_ptr; }

        /**
         * @brief 获取usart外设枚举
         *
         * @return usart外设枚举
         */
        [[nodiscard]] inline usart_enum get_usart_enum() const noexcept { return ::SoC::bit_cast<usart_enum>(usart_ptr.value); }

        /**
         * @brief 获取中断枚举
         *
         * @return 中断枚举
         */
        [[nodiscard]] inline ::IRQn_Type get_irqn() const noexcept { return irqn; }

        /**
         * @brief 向usart外设写入数据
         *
         * @param byte 要写入的1字节数据
         */
        void write(::std::uint8_t byte) const noexcept;

        /**
         * @brief 向usart外设写入数据
         *
         * @param buffer 数据缓冲区首指针
         * @param end 数据缓冲区尾哨位
         */
        [[gnu::noinline]] void write(const void* buffer, const void* end) const noexcept;

        /**
         * @brief 向usart外设写入数据
         *
         * @note 仅限数据宽度为9位且未启用校验位时使用
         * @param buffer 数据缓冲区首指针
         * @param end 数据缓冲区尾哨位
         */
        [[gnu::noinline]] void write(const ::std::uint16_t* buffer, const ::std::uint16_t* end) const noexcept;

        /**
         * @brief 通过usart写入数据
         *
         * @param usart usart设备对象
         * @param buffer 缓冲区首指针
         * @param end 缓冲区尾哨位
         */
        inline static void write_wrapper(void* usart, const void* buffer, const void* end) noexcept
        {
            static_cast<::SoC::usart*>(usart)->write(buffer, end);
        }

        /**
         * @brief 从usart读取8位数据
         *
         * @note 这会忽略第9位
         * @return 读取到的数据
         */
        [[nodiscard]] ::std::uint8_t read() const noexcept;

        /**
         * @brief 从usart读取9位数据
         *
         * @note 仅限数据宽度为9位时使用
         * @return 读取到的数据
         */
        [[nodiscard]] ::std::uint16_t read9() const noexcept;

        /**
         * @brief 从串口中读取数据并填充[begin, end)范围内的缓冲区
         *
         * @param begin 缓冲区首指针
         * @param end 缓冲区尾哨位
         * @return 缓冲区当前游标
         */
        [[gnu::noinline]] void* read(void* begin, void* end) const noexcept;

        /**
         * @brief 从串口中读取数据并填充[begin, end)范围内的缓冲区
         *
         * @param begin 缓冲区首指针
         * @param end 缓冲区尾哨位
         * @return 缓冲区当前游标
         */
        [[gnu::noinline]] ::std::uint16_t* read(::std::uint16_t* begin, ::std::uint16_t* end) const noexcept;

        /**
         * @brief 开启串口中断源
         *
         * @param preempt_priority 抢占优先级
         * @param sub_priority 响应优先级
         */
        void enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) const noexcept;

        /**
         * @brief 开启串口中断源
         *
         * @param encoded_priority 编码后的中断优先级
         */
        void enable_irq(::std::size_t encoded_priority) const noexcept;

        /**
         * @brief 关闭串口中断源
         *
         * @param priority
         */
        void disable_irq() const noexcept;

        /**
         * @brief 设置发送寄存器空中断状态
         *
         * @param enable 是否使能中断
         */
        void set_it_txe(bool enable) const noexcept;

        /**
         * @brief 获取发送寄存器空中断状态
         *
         * @return 中断是否使能
         */
        [[nodiscard]] bool get_it_txe() const noexcept;

        /**
         * @brief 获取发送寄存器空标记
         *
         * @return 发送寄存器空标记状态
         */
        [[nodiscard]] bool get_flag_txe() const noexcept;

        /**
         * @brief 判断发生的串口中断是否为发送寄存器空中断
         *
         * @return 发生的串口中断是否为发送寄存器空中断
         */
        [[nodiscard]] bool is_it_txe() const noexcept;

        /**
         * @brief 设置接收寄存器非空中断状态
         *
         * @param enable 是否使能中断
         */
        void set_it_rxne(bool enable) const noexcept;

        /**
         * @brief 获取接收寄存器非空中断状态
         *
         * @return 中断是否使能
         */
        [[nodiscard]] bool get_it_rxne() const noexcept;

        /**
         * @brief 获取接收寄存器非空标记
         *
         * @return 接收寄存器非空标记状态
         */
        [[nodiscard]] bool get_flag_rxne() const noexcept;

        /**
         * @brief 判断发生的串口中断是否为接收寄存器非空中断
         *
         * @return 发生的串口中断是否为接收寄存器非空中断
         */
        [[nodiscard]] bool is_it_rxne() const noexcept;

        /**
         * @brief 设置空闲中断状态
         *
         * @param enable 中断是否使能
         */
        void set_it_idle(bool enable) const noexcept;

        /**
         * @brief 获取空闲中断状态
         *
         * @return 中断是否使能
         */
        [[nodiscard]] bool get_it_idle() const noexcept;

        /**
         * @brief 获取空闲标记
         *
         * @return 空闲标记状态
         */
        [[nodiscard]] bool get_flag_idle() const noexcept;

        /**
         * @brief 判断发生的串口中断是否为空闲中断
         *
         * @return 发生的串口中断是否为空闲中断
         */
        [[nodiscard]] bool is_it_idle() const noexcept;

        /**
         * @brief 清除空闲标记
         *
         */
        void clear_flag_idle() const noexcept;

        /**
         * @brief 获取发送完成标记
         *
         * @return 发送完成标记状态
         */
        [[nodiscard]] bool get_flag_tc() const noexcept;

        /**
         * @brief 清除发送完成标记
         *
         */
        void clear_flag_tc() const noexcept;

        /**
         * @brief 设置发送完成中断状态
         *
         * @param enable 中断是否使能
         */
        void set_it_tc(bool enable) const noexcept;

        /**
         * @brief 获取发送完成中断状态
         *
         * @return 中断是否使能
         */
        [[nodiscard]] bool get_it_tc() const noexcept;

        /**
         * @brief 判断发生的串口中断是否为发送完成中断
         *
         * @return 发生的串口中断是否为发送完成中断
         */
        [[nodiscard]] bool is_it_tc() const noexcept;

        /**
         * @brief 使能usart外设
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能usart外设，不会关闭时钟
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断usart外设是否使能
         *
         * @return usart外设是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;

        /**
         * @brief 使能串口dma写入
         *
         * @param dma dma外设
         * @param fifo_threshold fifo队列阈值
         * @param default_burst 默认内存侧突发
         * @param default_data_size 默认内存侧数据宽度
         * @param priority dma传输优先级
         * @param mode dma模式
         * @param selected_stream 要使用的dma数据流，默认使用序号最小的数据流
         * @return dma数据流对象
         */
        [[nodiscard("该函数返回具有raii的dma数据流对象，不应该弃用返回值")]] usart_dma_stream
            enable_dma_write(::SoC::dma& dma,
                             ::SoC::dma_fifo_threshold fifo_threshold = ::SoC::dma_fifo_threshold::disable,
                             ::SoC::dma_memory_burst default_burst = ::SoC::dma_memory_burst::single,
                             ::SoC::dma_memory_data_size default_data_size = ::SoC::dma_memory_data_size::byte,
                             ::SoC::dma_priority priority = ::SoC::dma_priority::low,
                             ::SoC::dma_mode mode = ::SoC::dma_mode::normal,
                             ::SoC::dma_stream::dma_stream_enum selected_stream = no_selected_stream) noexcept;

        /**
         * @brief 失能串口dma写入
         *
         */
        void disable_dma_write() const noexcept;

        /**
         * @brief 判断串口dma写入是否使能
         *
         * @return 串口dma写入是否使能
         */
        [[nodiscard]] bool is_dma_write_enabled() const noexcept;
    };

    template <>
    constexpr inline bool async_output_device<::SoC::usart::usart_dma_stream>{true};
}  // namespace SoC
