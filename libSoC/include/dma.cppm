/**
 * @file dma.cppm
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief stm32 dma外设
 */

module;
#include <pch.hpp>
export module SoC:dma;
import :utils;

namespace SoC::detail
{
    /**
     * @brief dma外设枚举
     *
     */
    enum class dma : ::std::uintptr_t
    {
        dma1 = DMA1_BASE,
        dma2 = DMA2_BASE
    };

    /**
     * @brief dma数据流枚举
     *
     */
    enum class dma_stream : ::std::uint8_t
    {
        st0,
        st1,
        st2,
        st3,
        st4,
        st5,
        st6,
        st7
    };
}  // namespace SoC::detail

export namespace SoC
{

    /**
     * @brief dma外设
     *
     */
    struct dma
    {
    private:
        ::DMA_TypeDef* dma_ptr;

    public:
        using dma_enum = ::SoC::detail::dma;
        using enum dma_enum;

        /**
         * @brief 初始化dma外设并开启时钟
         *
         * @param dma dma外设枚举
         */
        explicit dma(dma_enum dma) noexcept;

        /**
         * @brief 关闭dma时钟
         *
         */
        ~dma() noexcept;

        inline dma(const dma&) noexcept = delete;
        inline dma& operator= (const dma&) noexcept = delete;
        /**
         * @brief 移动构造函数
         *
         * @param other 其他dma外设
         */
        dma(dma&& other) noexcept;
        inline dma& operator= (dma&&) noexcept = delete;

        /**
         * @brief 获取dma外设枚举
         *
         * @return dma外设枚举
         */
        [[nodiscard]] inline dma_enum get_dma_enum() const noexcept { return ::SoC::bit_cast<dma_enum>(dma_ptr); }

        /**
         * @brief 获取dma外设指针
         *
         * @return dma外设指针
         */
        [[nodiscard]] inline ::DMA_TypeDef* get_dma() const noexcept { return dma_ptr; }

        /**
         * @brief 使能dma时钟
         *
         */
        void enable() const noexcept;

        /**
         * @brief 失能dma时钟
         *
         */
        void disable() const noexcept;

        /**
         * @brief 判断dma时钟是否使能
         *
         * @return dma时钟是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;
    };

    /**
     * @brief dma通道枚举
     *
     */
    enum class dma_channel : ::std::size_t
    {
        ch0 = LL_DMA_CHANNEL_0,
        ch1 = LL_DMA_CHANNEL_1,
        ch2 = LL_DMA_CHANNEL_2,
        ch3 = LL_DMA_CHANNEL_3,
        ch4 = LL_DMA_CHANNEL_4,
        ch5 = LL_DMA_CHANNEL_5,
        ch6 = LL_DMA_CHANNEL_6,
        ch7 = LL_DMA_CHANNEL_7
    };

    /**
     * @brief dma传输模式
     *
     */
    enum class dma_mode : ::std::uint16_t
    {
        /// 单次传输
        normal = LL_DMA_MODE_NORMAL,
        /// 无限次循环传输
        circle = LL_DMA_MODE_CIRCULAR,
        /// 外设控制传输时机
        pfctrl = LL_DMA_MODE_PFCTRL
    };

    /**
     * @brief dma传输方向枚举
     *
     */
    enum class dma_direction : ::std::uint8_t
    {
        /// 外设到内存
        p2m = LL_DMA_DIRECTION_PERIPH_TO_MEMORY,
        /// 内存到外设
        m2p = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
        /// 内存到内存
        m2m = LL_DMA_DIRECTION_MEMORY_TO_MEMORY
    };

    /**
     * @brief dma内存侧数据传输宽度
     *
     */
    enum class dma_memory_data_size : ::std::uint16_t
    {
        /// 1字节
        byte = LL_DMA_MDATAALIGN_BYTE,
        /// 2字节
        half_word = LL_DMA_MDATAALIGN_HALFWORD,
        /// 4字节
        word = LL_DMA_MDATAALIGN_WORD
    };

    /**
     * @brief dma外设侧数据传输宽度
     *
     */
    enum class dma_periph_data_size : ::std::uint16_t
    {
        /// 1字节
        byte = LL_DMA_PDATAALIGN_BYTE,
        /// 2字节
        half_word = LL_DMA_PDATAALIGN_HALFWORD,
        /// 4字节
        word = LL_DMA_PDATAALIGN_WORD
    };

    /**
     * @brief dma传输优先级
     *
     */
    enum class dma_priority : ::std::size_t
    {
        low = LL_DMA_PRIORITY_LOW,
        medium = LL_DMA_PRIORITY_MEDIUM,
        high = LL_DMA_PRIORITY_HIGH,
        very_high = LL_DMA_PRIORITY_VERYHIGH
    };

    /**
     * @brief dma fifo阈值
     *
     */
    enum class dma_fifo_threshold : ::std::size_t
    {
        /// 使用1/4深度的fifo队列
        quart1 = LL_DMA_FIFOTHRESHOLD_1_4,
        /// 使用1/2深度的fifo队列
        half = LL_DMA_FIFOTHRESHOLD_1_2,
        /// 使用3/4深度的fifo队列
        quart3 = LL_DMA_FIFOTHRESHOLD_3_4,
        /// 使用完整的fifo队列
        full = LL_DMA_FIFOTHRESHOLD_FULL,
        /// 不使用fifo
        disable = -1zu
    };

    /// dma fifo队列最大值
    constexpr inline ::std::size_t dma_fifo_max_size{16};

    /**
     * @brief dma内存侧突发宽度
     *
     */
    enum class dma_memory_burst : ::std::size_t
    {
        single = LL_DMA_MBURST_SINGLE,
        inc4 = LL_DMA_MBURST_INC4,
        inc8 = LL_DMA_MBURST_INC8,
        inc16 = LL_DMA_MBURST_INC16,
    };

    /**
     * @brief dma外设侧突发宽度
     *
     */
    enum class dma_periph_burst : ::std::size_t
    {
        single = LL_DMA_PBURST_SINGLE,
        inc4 = LL_DMA_PBURST_INC4,
        inc8 = LL_DMA_PBURST_INC8,
        inc16 = LL_DMA_PBURST_INC16,
    };

    /**
     * @brief dma数据流
     *
     */
    struct dma_stream
    {
        using dma_stream_enum = ::SoC::detail::dma_stream;
        using enum dma_stream_enum;

    private:
        ::SoC::moveable_value<::DMA_TypeDef*> dma_ptr;
        dma_stream_enum stream;
        ::SoC::dma_direction direction;
        ::SoC::dma_mode mode;
        ::SoC::dma_fifo_threshold fifo_threshold;
        ::SoC::dma_memory_data_size mem_data_size;
        ::SoC::dma_memory_burst mem_burst;
        ::SoC::dma_periph_data_size pf_data_size;
        ::SoC::dma_periph_burst pf_burst;
        ::IRQn_Type irqn{};

        /**
         * @brief 检测内存侧参数是否合法
         *
         * @return 是否合法
         */
        [[nodiscard]] bool check_memory_access() const noexcept;

        /**
         * @brief 检测外设侧参数是否合法
         *
         * @return 是否合法
         */
        [[nodiscard]] bool check_periph_access() const noexcept;

        /**
         * @brief 检查输入是否满足对齐要求
         *
         * @param num 缓冲区首指针或缓冲区大小
         * @return 内存地址是否满足对齐要求
         */
        [[nodiscard]] bool check_aligned(::std::uintptr_t num) const noexcept;

        /**
         * @brief 设置dma的内存访问地址
         *
         * @param begin 缓冲区首指针
         */
        void set_memory_address(const void* begin) const noexcept;

        /**
         * @brief 设置dma的传输数量
         *
         * @param begin 缓冲区大小
         * @param item_size 传输单元的大小
         */
        void set_data_item(::std::size_t size, ::std::size_t item_size) const noexcept;

        /**
         * @brief 使能dma传输
         *
         */
        void enable() const noexcept;

        /**
         * @brief 获取传输完成标志位掩码
         *
         * @return 传输完成标志位掩码
         */
        [[nodiscard]] auto get_tc_mask() const noexcept;

        /**
         * @brief 获取传输半完成标志位掩码
         *
         * @return 传输半完成标志位掩码
         */
        [[nodiscard]] auto get_ht_mask() const noexcept;

    public:
        /**
         * @brief 获取dma外设指针
         *
         * @return dma外设指针
         */
        [[nodiscard]] inline ::DMA_TypeDef* get_dma() const noexcept { return dma_ptr; }

        /**
         * @brief 获取dma数据流枚举
         *
         * @return dma数据流枚举
         */
        [[nodiscard]] inline dma_stream_enum get_stream() const noexcept { return stream; }

        /**
         * @brief 获取fifo队列深度枚举
         *
         * @return fifo队列深度枚举
         */
        [[nodiscard]] inline ::SoC::dma_fifo_threshold get_fifo_threshold() const noexcept { return fifo_threshold; }

        /**
         * @brief 获取内存侧数据宽度枚举
         *
         * @return 内存侧数据宽度枚举
         */
        [[nodiscard]] inline ::SoC::dma_memory_data_size get_memory_data_size() const noexcept { return mem_data_size; }

        /**
         * @brief 获取内存侧突发枚举
         *
         * @return 内存侧突发枚举
         */
        [[nodiscard]] inline ::SoC::dma_memory_burst get_memory_burst() const noexcept { return mem_burst; }

        /**
         * @brief 获取外设侧数据宽度枚举
         *
         * @return 外设侧数据宽度枚举
         */
        [[nodiscard]] inline ::SoC::dma_periph_data_size get_periph_data_size() const noexcept { return pf_data_size; }

        /**
         * @brief 获取外设侧突发枚举
         *
         * @return 外设侧突发枚举
         */
        [[nodiscard]] inline ::SoC::dma_periph_burst get_periph_burst() const noexcept { return pf_burst; }

        /**
         * @brief 获取fifo队列深度
         *
         * @return fifo队列深度，以字节为单位
         */
        [[nodiscard]] inline ::std::size_t get_fifo_size() const noexcept
        {
            switch(fifo_threshold)
            {
                case ::SoC::dma_fifo_threshold::disable: return 0;
                case ::SoC::dma_fifo_threshold::quart1: return ::SoC::dma_fifo_max_size / 4;
                case ::SoC::dma_fifo_threshold::half: return ::SoC::dma_fifo_max_size / 2;
                case ::SoC::dma_fifo_threshold::quart3: return ::SoC::dma_fifo_max_size / 4 * 3;
                case ::SoC::dma_fifo_threshold::full: return ::SoC::dma_fifo_max_size;
                default: ::std::unreachable();
            }
        }

        /**
         * @brief 获取内存侧数据宽度
         *
         * @return 内存侧数据宽度，以字节为单位
         */
        [[nodiscard]] inline ::std::size_t get_memory_data_size_num() const noexcept
        {
            switch(mem_data_size)
            {
                case ::SoC::dma_memory_data_size::byte: return 1;
                case ::SoC::dma_memory_data_size::half_word: return 2;
                case ::SoC::dma_memory_data_size::word: return 4;
                default: ::std::unreachable();
            }
        }

        /**
         * @brief 获取内存侧突发拍数
         *
         * @return 内存侧突发拍数
         */
        [[nodiscard]] inline ::std::size_t get_memory_burst_num() const noexcept
        {
            switch(mem_burst)
            {
                case ::SoC::dma_memory_burst::single: return 1;
                case ::SoC::dma_memory_burst::inc4: return 4;
                case ::SoC::dma_memory_burst::inc8: return 8;
                case ::SoC::dma_memory_burst::inc16: return 16;
                default: ::std::unreachable();
            }
        }

        /**
         * @brief 获取外设侧数据宽度
         *
         * @return 外设侧数据宽度，以字节为单位
         */
        [[nodiscard]] inline ::std::size_t get_periph_data_size_num() const noexcept
        {
            switch(pf_data_size)
            {
                case ::SoC::dma_periph_data_size::byte: return 1;
                case ::SoC::dma_periph_data_size::half_word: return 2;
                case ::SoC::dma_periph_data_size::word: return 4;
                default: ::std::unreachable();
            }
        }

        /**
         * @brief 获取外设侧突发拍数
         *
         * @return 外设侧突发拍数
         */
        [[nodiscard]] inline ::std::size_t get_periph_burst_num() const noexcept
        {
            switch(pf_burst)
            {
                case ::SoC::dma_periph_burst::single: return 1;
                case ::SoC::dma_periph_burst::inc4: return 4;
                case ::SoC::dma_periph_burst::inc8: return 8;
                case ::SoC::dma_periph_burst::inc16: return 16;
                default: ::std::unreachable();
            }
        }

        /**
         * @brief 创建dma数据流对象，不会开启dma传输
         *
         * @param dma dma外设
         * @param stream dma数据流
         * @param channel dma通道
         * @param periph 外设寄存器地址
         * @param direction dma传输方向
         * @param mode dma传输模式
         * @param pf_increase 外设地址是否递增
         * @param mem_increase 内存地址是否递增
         * @param pf_data_size 外设数据宽度
         * @param mem_data_size 内存数据宽度
         * @param priority dma传输优先级
         * @param fifo_threshold fifo队列深度
         * @param mem_burst 内存侧突发
         * @param pf_burst 外设侧突发
         * @note direction不能为m2m
         */
        explicit dma_stream(::SoC::dma& dma,
                            dma_stream_enum stream,
                            ::SoC::dma_channel channel,
                            ::std::uintptr_t periph,
                            ::SoC::dma_direction direction,
                            ::SoC::dma_mode mode,
                            bool pf_increase,
                            bool mem_increase,
                            ::SoC::dma_periph_data_size pf_data_size,
                            ::SoC::dma_memory_data_size mem_data_size,
                            ::SoC::dma_priority priority,
                            ::SoC::dma_fifo_threshold fifo_threshold,
                            ::SoC::dma_memory_burst mem_burst,
                            ::SoC::dma_periph_burst pf_burst) noexcept;

        /**
         * @brief 关闭dma数据流
         *
         */
        ~dma_stream() noexcept;

        inline dma_stream(const dma_stream&) noexcept = delete;
        inline dma_stream& operator= (const dma_stream&) noexcept = delete;
        dma_stream(dma_stream&& other) noexcept = default;
        inline dma_stream& operator= (dma_stream&&) noexcept = delete;

        /**
         * @brief 设置内存侧数据宽度
         *
         * @param mem_data_size 内存侧数据宽度枚举
         */
        void set_memory_data_size(::SoC::dma_memory_data_size mem_data_size) noexcept;

        /**
         * @brief 设置内存侧突发
         *
         * @param mem_burst 内存侧突发
         */
        void set_memory_burst(::SoC::dma_memory_burst mem_burst) noexcept;

        /**
         * @brief 设置外设侧数据宽度
         *
         * @param pf_data_size 外设侧数据宽度枚举
         */
        void set_periph_data_size(::SoC::dma_periph_data_size pf_data_size) noexcept;

        /**
         * @brief 设置外设侧突发
         *
         * @param pf_burst 外设侧突发
         */
        void set_periph_burst(::SoC::dma_periph_burst pf_burst) noexcept;

        /**
         * @brief 设置fifo
         *
         * @param fifo_threshold fifo队列深度
         */
        void set_fifo(::SoC::dma_fifo_threshold fifo_threshold) noexcept;

        /**
         * @brief 设置dma优先级
         *
         * @param priority dma优先级
         */
        void set_priority(::SoC::dma_priority priority) const noexcept;

        /**
         * @brief 设置传输模式
         *
         * @param mode dma传输模式
         */
        void set_mode(::SoC::dma_mode mode) const noexcept;

        /**
         * @brief 判断dma数据流是否使能
         *
         * @return dma数据流是否使能
         */
        [[nodiscard]] bool is_enabled() const noexcept;

        /**
         * @brief 失能dma数据流
         *
         */
        void disable() const noexcept;

        /**
         * @brief 配置内存到外设的数据传输，并使能dma数据流
         *
         * @note 在使能前会清除传输完成标记
         * @param begin 缓冲区首指针
         * @param end 缓冲区尾哨位
         */
        void write(const void* begin, const void* end) noexcept;

        /**
         * @brief 配置外设到内存的数据传输，并使能dma数据流
         *
         * @note 在使能前会清除传输完成标记
         * @param begin 缓冲区首指针
         * @param end 缓冲区尾哨位
         */
        void read(void* begin, void* end) noexcept;

        /**
         * @brief 获取传输完成标记
         *
         * @return 传输完成标记
         */
        [[nodiscard]] bool get_flag_tc() const noexcept;

        /**
         * @brief 清除传输完成标记
         *
         */
        void clear_flag_tc() const noexcept;

        /**
         * @brief 获取传输半完成标记
         *
         * @return 传输半完成标记
         */
        [[nodiscard]] bool get_flag_ht() const noexcept;

        /**
         * @brief 清除传输半完成标记
         *
         */
        void clear_flag_ht() const noexcept;

        /**
         * @brief 判断是否传输完成
         *
         * @note tc置位或dma数据流失能认为传输完成
         * @return 是否传输完成
         */
        [[nodiscard]] bool is_transfer_complete() const noexcept;

        /**
         * @brief 获取中断号
         *
         * @return dma数据流对应的中断号
         */
        ::IRQn_Type get_irqn() noexcept;

        /**
         * @brief 开启dma中断
         *
         * @param preempt_priority 抢占中断优先级
         * @param sub_priority 响应中断优先级
         */
        void enable_irq(::std::size_t preempt_priority, ::std::size_t sub_priority) noexcept;

        /**
         * @brief 开启dma中断
         *
         * @param encoded_priority 编码后的中断优先级
         */
        void enable_irq(::std::size_t encoded_priority) noexcept;

        /**
         * @brief 关闭dma中断
         *
         */
        void disable_irq() noexcept;

        /**
         * @brief 设置是否使能dma传输完成中断源
         *
         * @param enable 是否使能中断源
         */
        void set_it_tc(bool enable) const noexcept;

        /**
         * @brief 获取是否使能dma传输完成中断源
         *
         * @return 是否使能中断源
         */
        [[nodiscard]] bool get_it_tc() const noexcept;

        /**
         * @brief 判断发生的dma中断是否为传输完成中断
         *
         * @return 是否为传输完成中断
         */
        [[nodiscard]] bool is_it_tc() const noexcept;

        /**
         * @brief 设置是否使能dma传输半完成中断源
         *
         * @param enable 是否使能中断源
         */
        void set_it_ht(bool enable) const noexcept;

        /**
         * @brief 获取是否使能dma传输半完成中断源
         *
         * @return 是否使能中断源
         */
        [[nodiscard]] bool get_it_ht() const noexcept;

        /**
         * @brief 判断发生的dma中断是否为传输半完成中断
         *
         * @return 是否为传输半完成中断
         */
        [[nodiscard]] bool is_it_ht() const noexcept;
    };
}  // namespace SoC

namespace SoC
{
    namespace detail
    {
        /**
         * @brief 禁止dma数据流作为IO设备，用于提供更好的报错
         *
         * @tparam type dma数据流类型
         */
        template <typename type>
        struct forbidden_dma_stream_as_io_device
        {
            constexpr inline static bool with_periph_object{!::std::same_as<type, ::SoC::dma_stream>};
            static_assert(with_periph_object, "dma数据流需要配合外设对象一起才能正确处理时序，因此禁止单独作为IO设备使用");
        };
    }  // namespace detail

    /**
     * @brief 禁止dma数据流作为输出设备
     *
     * @note dma数据流需要配合外设对象一起才能正确处理时序，因此禁止单独作为输出设备使用
     */
    export template <::std::same_as<::SoC::dma_stream> device_t>
    constexpr inline bool forbidden_output_device<device_t>{
        ::SoC::detail::forbidden_dma_stream_as_io_device<device_t>::with_periph_object};

    /**
     * @brief 禁止dma数据流作为输入设备
     *
     * @note dma数据流需要配合外设对象一起才能正确处理时序，因此禁止单独作为输入设备使用
     */
    export template <::std::same_as<::SoC::dma_stream> device_t>
    constexpr inline bool forbidden_input_device<device_t>{
        ::SoC::detail::forbidden_dma_stream_as_io_device<device_t>::with_periph_object};
}  // namespace SoC
