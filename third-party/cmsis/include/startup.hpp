#pragma once
#include <cstddef>
#include <cstdint>

namespace SoC
{
    void _init() noexcept;
    void _fini() noexcept;

    template <typename type>
    using cursor_t = type[];

    extern "C"
    {
        extern "C" void SystemInit();

        [[gnu::weak]] void NMI_Handler() noexcept;
        [[gnu::weak]] void HardFault_Handler() noexcept;
        [[gnu::weak]] void MemManage_Handler() noexcept;
        [[gnu::weak]] void BusFault_Handler() noexcept;
        [[gnu::weak]] void UsageFault_Handler() noexcept;
        [[gnu::weak]] void SVC_Handler() noexcept;
        [[gnu::weak]] void DebugMon_Handler() noexcept;
        [[gnu::weak]] void PendSV_Handler() noexcept;
        [[gnu::weak]] void SysTick_Handler() noexcept;
        [[gnu::weak]] void WWDG_IRQHandler() noexcept;
        [[gnu::weak]] void PVD_IRQHandler() noexcept;
        [[gnu::weak]] void TAMP_STAMP_IRQHandler() noexcept;
        [[gnu::weak]] void RTC_WKUP_IRQHandler() noexcept;
        [[gnu::weak]] void FLASH_IRQHandler() noexcept;
        [[gnu::weak]] void RCC_IRQHandler() noexcept;
        [[gnu::weak]] void EXTI0_IRQHandler() noexcept;
        [[gnu::weak]] void EXTI1_IRQHandler() noexcept;
        [[gnu::weak]] void EXTI2_IRQHandler() noexcept;
        [[gnu::weak]] void EXTI3_IRQHandler() noexcept;
        [[gnu::weak]] void EXTI4_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream0_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream1_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream2_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream3_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream4_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream5_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream6_IRQHandler() noexcept;
        [[gnu::weak]] void ADC_IRQHandler() noexcept;
        [[gnu::weak]] void CAN1_TX_IRQHandler() noexcept;
        [[gnu::weak]] void CAN1_RX0_IRQHandler() noexcept;
        [[gnu::weak]] void CAN1_RX1_IRQHandler() noexcept;
        [[gnu::weak]] void CAN1_SCE_IRQHandler() noexcept;
        [[gnu::weak]] void EXTI9_5_IRQHandler() noexcept;
        [[gnu::weak]] void TIM1_BRK_TIM9_IRQHandler() noexcept;
        [[gnu::weak]] void TIM1_UP_TIM10_IRQHandler() noexcept;
        [[gnu::weak]] void TIM1_TRG_COM_TIM11_IRQHandler() noexcept;
        [[gnu::weak]] void TIM1_CC_IRQHandler() noexcept;
        [[gnu::weak]] void TIM2_IRQHandler() noexcept;
        [[gnu::weak]] void TIM3_IRQHandler() noexcept;
        [[gnu::weak]] void TIM4_IRQHandler() noexcept;
        [[gnu::weak]] void I2C1_EV_IRQHandler() noexcept;
        [[gnu::weak]] void I2C1_ER_IRQHandler() noexcept;
        [[gnu::weak]] void I2C2_EV_IRQHandler() noexcept;
        [[gnu::weak]] void I2C2_ER_IRQHandler() noexcept;
        [[gnu::weak]] void SPI1_IRQHandler() noexcept;
        [[gnu::weak]] void SPI2_IRQHandler() noexcept;
        [[gnu::weak]] void USART1_IRQHandler() noexcept;
        [[gnu::weak]] void USART2_IRQHandler() noexcept;
        [[gnu::weak]] void USART3_IRQHandler() noexcept;
        [[gnu::weak]] void EXTI15_10_IRQHandler() noexcept;
        [[gnu::weak]] void RTC_Alarm_IRQHandler() noexcept;
        [[gnu::weak]] void OTG_FS_WKUP_IRQHandler() noexcept;
        [[gnu::weak]] void TIM8_BRK_TIM12_IRQHandler() noexcept;
        [[gnu::weak]] void TIM8_UP_TIM13_IRQHandler() noexcept;
        [[gnu::weak]] void TIM8_TRG_COM_TIM14_IRQHandler() noexcept;
        [[gnu::weak]] void TIM8_CC_IRQHandler() noexcept;
        [[gnu::weak]] void DMA1_Stream7_IRQHandler() noexcept;
        [[gnu::weak]] void FSMC_IRQHandler() noexcept;
        [[gnu::weak]] void SDIO_IRQHandler() noexcept;
        [[gnu::weak]] void TIM5_IRQHandler() noexcept;
        [[gnu::weak]] void SPI3_IRQHandler() noexcept;
        [[gnu::weak]] void UART4_IRQHandler() noexcept;
        [[gnu::weak]] void UART5_IRQHandler() noexcept;
        [[gnu::weak]] void TIM6_DAC_IRQHandler() noexcept;
        [[gnu::weak]] void TIM7_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream0_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream1_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream2_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream3_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream4_IRQHandler() noexcept;
        [[gnu::weak]] void ETH_IRQHandler() noexcept;
        [[gnu::weak]] void ETH_WKUP_IRQHandler() noexcept;
        [[gnu::weak]] void CAN2_TX_IRQHandler() noexcept;
        [[gnu::weak]] void CAN2_RX0_IRQHandler() noexcept;
        [[gnu::weak]] void CAN2_RX1_IRQHandler() noexcept;
        [[gnu::weak]] void CAN2_SCE_IRQHandler() noexcept;
        [[gnu::weak]] void OTG_FS_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream5_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream6_IRQHandler() noexcept;
        [[gnu::weak]] void DMA2_Stream7_IRQHandler() noexcept;
        [[gnu::weak]] void USART6_IRQHandler() noexcept;
        [[gnu::weak]] void I2C3_EV_IRQHandler() noexcept;
        [[gnu::weak]] void I2C3_ER_IRQHandler() noexcept;
        [[gnu::weak]] void OTG_HS_EP1_OUT_IRQHandler() noexcept;
        [[gnu::weak]] void OTG_HS_EP1_IN_IRQHandler() noexcept;
        [[gnu::weak]] void OTG_HS_WKUP_IRQHandler() noexcept;
        [[gnu::weak]] void OTG_HS_IRQHandler() noexcept;
        [[gnu::weak]] void DCMI_IRQHandler() noexcept;
        [[gnu::weak]] void HASH_RNG_IRQHandler() noexcept;
        [[gnu::weak]] void FPU_IRQHandler() noexcept;
    }
}  // namespace SoC
