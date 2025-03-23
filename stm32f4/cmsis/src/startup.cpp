#include "common.hpp"

int main();

namespace SoC
{
    extern "C"
    {
        extern "C" [[using gnu: naked]] void Reset_Handler() noexcept { asm volatile("ldr sp, =_estack\n" "b startup\n"); }

        using cursor = ::SoC::cursor_t<::std::size_t>;

        extern ::SoC::cursor _sdata;
        extern ::SoC::cursor _edata;
        extern ::SoC::cursor _sidata;

        extern ::SoC::cursor _sbss;
        extern ::SoC::cursor _ebss;

        extern ::SoC::cursor _sccmram;
        extern ::SoC::cursor _eccmram;
        extern ::SoC::cursor _siccmram;

        extern ::SoC::cursor _estack;
    }

    using no_optimize_cursor = volatile ::std::size_t*;

    void copy(::SoC::no_optimize_cursor begin, ::SoC::no_optimize_cursor end, ::SoC::no_optimize_cursor from) noexcept
    {
#ifdef __clang__
    #pragma clang loop unroll_count(1)
#else
    #pragma GCC unroll 1
#endif
        while(begin != end) { *begin++ = *from++; }
    }

    void fill(::SoC::no_optimize_cursor begin, ::SoC::no_optimize_cursor end) noexcept
    {
#ifdef __clang__
    #pragma clang loop unroll_count(1)
#else
    #pragma GCC unroll 1
#endif
        while(begin != end) { *begin++ = 0; }
    }

    extern "C" [[noreturn, gnu::used]] void startup() noexcept
    {
        ::SoC::SystemInit();
        ::SoC::copy(::SoC::_sdata, ::SoC::_edata, ::SoC::_sidata);
        ::SoC::fill(::SoC::_sbss, ::SoC::_ebss);
        ::SoC::copy(::SoC::_sccmram, ::SoC::_eccmram, ::SoC::_siccmram);

        ::SoC::_init();
        ::main();
        ::SoC::_fini();

        while(true);
    }

    extern "C" [[noreturn, gnu::weak]] void Default_Handler() noexcept { while(true); }

    extern "C" [[using gnu: weak, alias("Default_Handler")]] void NMI_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void HardFault_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void MemManage_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void BusFault_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void UsageFault_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void SVC_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DebugMon_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void PendSV_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void SysTick_Handler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void WWDG_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void PVD_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TAMP_STAMP_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void RTC_WKUP_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void FLASH_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void RCC_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void EXTI0_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void EXTI1_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void EXTI2_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void EXTI3_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void EXTI4_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream0_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream1_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream2_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream3_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream4_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream5_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream6_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void ADC_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN1_TX_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN1_RX0_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN1_RX1_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN1_SCE_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void EXTI9_5_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM1_BRK_TIM9_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM1_UP_TIM10_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM1_TRG_COM_TIM11_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM1_CC_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM2_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM3_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM4_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void I2C1_EV_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void I2C1_ER_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void I2C2_EV_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void I2C2_ER_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void SPI1_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void SPI2_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void USART1_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void USART2_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void USART3_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void EXTI15_10_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void RTC_Alarm_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void OTG_FS_WKUP_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM8_BRK_TIM12_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM8_UP_TIM13_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM8_TRG_COM_TIM14_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM8_CC_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream7_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void FSMC_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void SDIO_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM5_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void SPI3_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void UART4_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void UART5_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM6_DAC_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void TIM7_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream0_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream1_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream2_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream3_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream4_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void ETH_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void ETH_WKUP_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN2_TX_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN2_RX0_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN2_RX1_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void CAN2_SCE_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void OTG_FS_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream5_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream6_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream7_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void USART6_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void I2C3_EV_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void I2C3_ER_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_EP1_OUT_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_EP1_IN_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_WKUP_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void DCMI_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void HASH_RNG_IRQHandler() noexcept;
    extern "C" [[using gnu: weak, alias("Default_Handler")]] void FPU_IRQHandler() noexcept;

    using isr_t = void (*)();

    [[using gnu: section(".isr_vector"), used]] isr_t table[]{
        reinterpret_cast<isr_t>(_estack),
        Reset_Handler,
        NMI_Handler,
        HardFault_Handler,
        MemManage_Handler,
        BusFault_Handler,
        UsageFault_Handler,
        0,
        0,
        0,
        0,
        SVC_Handler,
        DebugMon_Handler,
        0,
        PendSV_Handler,
        SysTick_Handler,
        WWDG_IRQHandler,
        PVD_IRQHandler,
        TAMP_STAMP_IRQHandler,
        RTC_WKUP_IRQHandler,
        FLASH_IRQHandler,
        RCC_IRQHandler,
        EXTI0_IRQHandler,
        EXTI1_IRQHandler,
        EXTI2_IRQHandler,
        EXTI3_IRQHandler,
        EXTI4_IRQHandler,
        DMA1_Stream0_IRQHandler,
        DMA1_Stream1_IRQHandler,
        DMA1_Stream2_IRQHandler,
        DMA1_Stream3_IRQHandler,
        DMA1_Stream4_IRQHandler,
        DMA1_Stream5_IRQHandler,
        DMA1_Stream6_IRQHandler,
        ADC_IRQHandler,
        CAN1_TX_IRQHandler,
        CAN1_RX0_IRQHandler,
        CAN1_RX1_IRQHandler,
        CAN1_SCE_IRQHandler,
        EXTI9_5_IRQHandler,
        TIM1_BRK_TIM9_IRQHandler,
        TIM1_UP_TIM10_IRQHandler,
        TIM1_TRG_COM_TIM11_IRQHandler,
        TIM1_CC_IRQHandler,
        TIM2_IRQHandler,
        TIM3_IRQHandler,
        TIM4_IRQHandler,
        I2C1_EV_IRQHandler,
        I2C1_ER_IRQHandler,
        I2C2_EV_IRQHandler,
        I2C2_ER_IRQHandler,
        SPI1_IRQHandler,
        SPI2_IRQHandler,
        USART1_IRQHandler,
        USART2_IRQHandler,
        USART3_IRQHandler,
        EXTI15_10_IRQHandler,
        RTC_Alarm_IRQHandler,
        OTG_FS_WKUP_IRQHandler,
        TIM8_BRK_TIM12_IRQHandler,
        TIM8_UP_TIM13_IRQHandler,
        TIM8_TRG_COM_TIM14_IRQHandler,
        TIM8_CC_IRQHandler,
        DMA1_Stream7_IRQHandler,
        FSMC_IRQHandler,
        SDIO_IRQHandler,
        TIM5_IRQHandler,
        SPI3_IRQHandler,
        UART4_IRQHandler,
        UART5_IRQHandler,
        TIM6_DAC_IRQHandler,
        TIM7_IRQHandler,
        DMA2_Stream0_IRQHandler,
        DMA2_Stream1_IRQHandler,
        DMA2_Stream2_IRQHandler,
        DMA2_Stream3_IRQHandler,
        DMA2_Stream4_IRQHandler,
        ETH_IRQHandler,
        ETH_WKUP_IRQHandler,
        CAN2_TX_IRQHandler,
        CAN2_RX0_IRQHandler,
        CAN2_RX1_IRQHandler,
        CAN2_SCE_IRQHandler,
        OTG_FS_IRQHandler,
        DMA2_Stream5_IRQHandler,
        DMA2_Stream6_IRQHandler,
        DMA2_Stream7_IRQHandler,
        USART6_IRQHandler,
        I2C3_EV_IRQHandler,
        I2C3_ER_IRQHandler,
        OTG_HS_EP1_OUT_IRQHandler,
        OTG_HS_EP1_IN_IRQHandler,
        OTG_HS_WKUP_IRQHandler,
        OTG_HS_IRQHandler,
        DCMI_IRQHandler,
        0,
        HASH_RNG_IRQHandler,
        FPU_IRQHandler,
    };
}  // namespace SoC
