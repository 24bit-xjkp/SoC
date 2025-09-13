/**
 * @file i2c.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 i2c外设
 */

module;
#include <pch.hpp>
export module SoC:i2c;
import :dma;

namespace SoC::detail
{
    /**
     * @brief i2c外设枚举
     *
     */
    enum class i2c : ::std::uintptr_t
    {
        i2c1 = I2C1_BASE,
        i2c2 = I2C2_BASE,
        i2c3 = I2C3_BASE
    };
}  // namespace SoC::detail

export namespace SoC
{

    /**
     * @brief i2c工作模式
     *
     */
    enum class i2c_mode : ::std::uint8_t
    {
        /// 标准i2c模式
        i2c = LL_I2C_MODE_I2C,
        /// SMBus主机模式
        smbus_host = LL_I2C_MODE_SMBUS_HOST,
        /// SMBus从机模式
        smbus_device = LL_I2C_MODE_SMBUS_DEVICE,
        /// SMBus从机模式，支持arp协议
        smbus_device_arp = LL_I2C_MODE_SMBUS_DEVICE_ARP,
        default_mode = i2c
    };

    /**
     * @brief i2c快速模式时钟占空比
     *
     */
    enum class i2c_duty_cycle : ::std::uint16_t
    {
        /// 低电平时间/高电平时间=2
        duty_2 = LL_I2C_DUTYCYCLE_2,
        /// 低电平时间/高电平时间=16/9
        duty_16_9 = LL_I2C_DUTYCYCLE_16_9,
        default_duty = duty_2
    };

    /**
     * @brief i2c数据接收后的应答行为
     *
     */
    enum class i2c_type_ack : ::std::uint16_t
    {
        /// 发送ack信号
        ack = LL_I2C_ACK,
        /// 发生nack信号
        nack = LL_I2C_NACK,
        default_ack = ack
    };

    /**
     * @brief i2c地址位数
     *
     */
    enum class i2c_owm_address_size : ::std::uint16_t
    {
        /// 7位地址
        bit7 = LL_I2C_OWNADDRESS1_7BIT,
        /// 10位地址
        bit10 = LL_I2C_OWNADDRESS1_10BIT,
        default_bit = bit7
    };

    /**
     * @brief i2c外设
     *
     */
    struct i2c
    {
        using i2c_enum = ::SoC::detail::i2c;

    private:
        ::SoC::moveable_value<::I2C_TypeDef*> i2c_ptr;
        ::std::size_t periph;

        /**
         * @brief 通过raii自动产生起始位和停止位
         *
         */
        struct condition_guard
        {
        private:
            friend struct ::SoC::i2c;
            const ::SoC::i2c& i2c;

            inline explicit condition_guard(const ::SoC::i2c& i2c) noexcept : i2c{i2c} { i2c.start(); }

        public:
            inline condition_guard(const condition_guard&) noexcept = delete;
            inline condition_guard& operator= (const condition_guard&) noexcept = delete;
            inline condition_guard(condition_guard&&) noexcept = delete;
            inline condition_guard& operator= (condition_guard&&) noexcept = delete;

            inline ~condition_guard() noexcept { i2c.stop(); }
        };

        /// 没有选定的dma数据流，即使用模式配置
        constexpr inline static ::SoC::dma_stream::dma_stream_enum no_selected_stream{-1zu};

        /**
         * @brief 断言dma外设为指定外设
         *
         * @param dma dma外设
         */
        static void assert_dma(::SoC::dma& dma) noexcept;

    public:
        using enum i2c_enum;

        /**
         * @brief 初始化并使能i2c外设
         *
         * @param i2c i2c外设
         * @param clock_speed 时钟频率
         * @param address 主机地址
         * @param mode i2c工作模式
         * @param duty 时钟占空比
         * @param ack 数据接收后应答行为
         * @param address_size 地址位数
         */
        explicit i2c(i2c_enum i2c,
                     ::std::size_t clock_speed,
                     ::std::size_t address,
                     ::SoC::i2c_mode mode = ::SoC::i2c_mode::default_mode,
                     ::SoC::i2c_duty_cycle duty = ::SoC::i2c_duty_cycle::default_duty,
                     ::SoC::i2c_type_ack ack = ::SoC::i2c_type_ack::default_ack,
                     ::SoC::i2c_owm_address_size address_size = ::SoC::i2c_owm_address_size::default_bit) noexcept;

        /**
         * @brief 失能i2c外设并关闭时钟
         *
         */
        ~i2c() noexcept;

        inline i2c(const i2c&) noexcept = delete;
        inline i2c& operator= (const i2c&) noexcept = delete;
        i2c(i2c&& other) noexcept = default;
        inline i2c& operator= (i2c&&) noexcept = delete;

        /**
         * @brief 获取i2c外设指针
         *
         * @return i2c外设指针
         */
        [[nodiscard]] inline ::I2C_TypeDef* get_i2c() const noexcept { return i2c_ptr; }

        /**
         * @brief 获取i2c外设枚举
         *
         * @return i2c外设枚举
         */
        [[nodiscard]] inline i2c_enum get_i2c_enum() const noexcept { return ::SoC::bit_cast<i2c_enum>(i2c_ptr.value); }

        /**
         * @brief 使能i2c外设
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能i2c外设
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断i2c外设是否使能
         *
         * @return i2c外设是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;

        /**
         * @brief 设置数据接受后的应答行为
         *
         * @param ack 数据接收后应答行为
         */
        void set_ack(::SoC::i2c_type_ack ack) const noexcept;

        /**
         * @brief 获取i2c繁忙标志
         *
         * @return i2c繁忙标志
         */
        [[nodiscard]] bool get_flag_busy() const noexcept;

        /**
         * @brief 获取i2c起始位标志
         *
         * @return i2c起始位标志
         */
        [[nodiscard]] bool get_flag_sb() const noexcept;

        /**
         * @brief 获取i2c地址标志
         *
         * @return i2c地址标志
         */
        [[nodiscard]] bool get_flag_addr() const noexcept;

        /**
         * @brief 清除i2c地址标志
         *
         */
        void clear_flag_addr() const noexcept;

        /**
         * @brief 获取i2c传输完成标志
         *
         * @return i2c传输完成标志
         */
        [[nodiscard]] bool get_flag_btf() const noexcept;

        /**
         * @brief 获取i2c停止标志
         *
         * @return i2c停止标志
         */
        [[nodiscard]] bool get_flag_stop() const noexcept;

        /**
         * @brief 清除i2c停止标志
         *
         */
        void clear_flag_stop() const noexcept;

        /**
         * @brief 获取i2c发送寄存器空标志
         *
         * @return i2c发送寄存器空标志
         */
        [[nodiscard]] bool get_flag_txe() const noexcept;

        /**
         * @brief 发生通信起始位
         *
         */
        void start() const noexcept;

        /**
         * @brief 发生通信停止位
         *
         */
        void stop() const noexcept;

        /**
         * @brief 获取一个具有raii的防卫变量，在构造时发送起始位并阻塞直到起始标志置位，在析构时发送停止位并阻塞直到传输完成
         *
         * @return 防卫变量
         */
        [[nodiscard("该函数返回具有raii的防卫变量，不应该弃用返回值")]] inline ::SoC::i2c::condition_guard
            get_condition_guard() const noexcept
        {
            return ::SoC::i2c::condition_guard{*this};
        }

        /**
         * @brief 阻塞直到i2c总线空闲
         *
         */
        void wait_until_idle() const noexcept;

        /**
         * @brief 阻塞直到i2c发送寄存器为空
         *
         */
        void wait_until_txe() const noexcept;

        /**
         * @brief 写入1字节到i2c外设
         *
         * @note 不会发出起始位和停止位，不会进行阻塞
         * @param value 要写入的字节
         */
        void write(::std::uint8_t value) const noexcept;

        /**
         * @brief 发送从机地址并等待回应
         *
         * @param address 从机地址，不需要左移
         */
        void write_address(::std::size_t address) const noexcept;

        /**
         * @brief 将[begin, end)内的字节写入i2c外设
         *
         * @note 会阻塞直到总线空闲，同时会发出起始位和停止位
         * @param address 从机地址，不需要左移
         * @param begin 缓冲区首地址
         * @param end 缓冲区尾哨位
         */
        void write(::std::size_t address, const void* begin, const void* end) const noexcept;

        /**
         * @brief 使能i2c外设dma写入
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
        [[nodiscard("该函数返回具有raii的dma数据流对象，不应该弃用返回值")]] ::SoC::dma_stream
            enable_dma_write(::SoC::dma& dma,
                             ::SoC::dma_fifo_threshold fifo_threshold = ::SoC::dma_fifo_threshold::disable,
                             ::SoC::dma_memory_burst default_burst = ::SoC::dma_memory_burst::single,
                             ::SoC::dma_memory_data_size default_data_size = ::SoC::dma_memory_data_size::byte,
                             ::SoC::dma_priority priority = ::SoC::dma_priority::low,
                             ::SoC::dma_mode mode = ::SoC::dma_mode::normal,
                             ::SoC::dma_stream::dma_stream_enum selected_stream = no_selected_stream) const noexcept;

        /**
         * @brief 失能i2c外设dma写入
         *
         */
        void disable_dma_write() const noexcept;

        /**
         * @brief 判断i2c外设dma写入是否使能
         *
         * @return i2c外设dma写入是否使能
         */
        [[nodiscard]] bool is_dma_write_enabled() const noexcept;
    };
}  // namespace SoC
