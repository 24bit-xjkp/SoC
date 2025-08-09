/**
 * @file stm32f407_startup.cpp
 * @author 24bit-xjkp (2283572185@qq.com)
 * @brief STM32F407启动代码和中断向量表
 */

module SoC.crt;
import :common;
import :stm32f407_irq;

namespace SoC
{
    [[using gnu: section(".isr_vector"), used]] const ::SoC::isr_t isr_table[]{
        reinterpret_cast<::SoC::isr_t>(::SoC::_estack),
        ::SoC::Reset_Handler,
        ::SoC::NMI_Handler,
        ::SoC::HardFault_Handler,
        ::SoC::MemManage_Handler,
        ::SoC::BusFault_Handler,
        ::SoC::UsageFault_Handler,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        ::SoC::SVC_Handler,
        ::SoC::DebugMon_Handler,
        nullptr,
        ::SoC::PendSV_Handler,
        ::SoC::SysTick_Handler,
        ::SoC::WWDG_IRQHandler,
        ::SoC::PVD_IRQHandler,
        ::SoC::TAMP_STAMP_IRQHandler,
        ::SoC::RTC_WKUP_IRQHandler,
        ::SoC::FLASH_IRQHandler,
        ::SoC::RCC_IRQHandler,
        ::SoC::EXTI0_IRQHandler,
        ::SoC::EXTI1_IRQHandler,
        ::SoC::EXTI2_IRQHandler,
        ::SoC::EXTI3_IRQHandler,
        ::SoC::EXTI4_IRQHandler,
        ::SoC::DMA1_Stream0_IRQHandler,
        ::SoC::DMA1_Stream1_IRQHandler,
        ::SoC::DMA1_Stream2_IRQHandler,
        ::SoC::DMA1_Stream3_IRQHandler,
        ::SoC::DMA1_Stream4_IRQHandler,
        ::SoC::DMA1_Stream5_IRQHandler,
        ::SoC::DMA1_Stream6_IRQHandler,
        ::SoC::ADC_IRQHandler,
        ::SoC::CAN1_TX_IRQHandler,
        ::SoC::CAN1_RX0_IRQHandler,
        ::SoC::CAN1_RX1_IRQHandler,
        ::SoC::CAN1_SCE_IRQHandler,
        ::SoC::EXTI9_5_IRQHandler,
        ::SoC::TIM1_BRK_TIM9_IRQHandler,
        ::SoC::TIM1_UP_TIM10_IRQHandler,
        ::SoC::TIM1_TRG_COM_TIM11_IRQHandler,
        ::SoC::TIM1_CC_IRQHandler,
        ::SoC::TIM2_IRQHandler,
        ::SoC::TIM3_IRQHandler,
        ::SoC::TIM4_IRQHandler,
        ::SoC::I2C1_EV_IRQHandler,
        ::SoC::I2C1_ER_IRQHandler,
        ::SoC::I2C2_EV_IRQHandler,
        ::SoC::I2C2_ER_IRQHandler,
        ::SoC::SPI1_IRQHandler,
        ::SoC::SPI2_IRQHandler,
        ::SoC::USART1_IRQHandler,
        ::SoC::USART2_IRQHandler,
        ::SoC::USART3_IRQHandler,
        ::SoC::EXTI15_10_IRQHandler,
        ::SoC::RTC_Alarm_IRQHandler,
        ::SoC::OTG_FS_WKUP_IRQHandler,
        ::SoC::TIM8_BRK_TIM12_IRQHandler,
        ::SoC::TIM8_UP_TIM13_IRQHandler,
        ::SoC::TIM8_TRG_COM_TIM14_IRQHandler,
        ::SoC::TIM8_CC_IRQHandler,
        ::SoC::DMA1_Stream7_IRQHandler,
        ::SoC::FSMC_IRQHandler,
        ::SoC::SDIO_IRQHandler,
        ::SoC::TIM5_IRQHandler,
        ::SoC::SPI3_IRQHandler,
        ::SoC::UART4_IRQHandler,
        ::SoC::UART5_IRQHandler,
        ::SoC::TIM6_DAC_IRQHandler,
        ::SoC::TIM7_IRQHandler,
        ::SoC::DMA2_Stream0_IRQHandler,
        ::SoC::DMA2_Stream1_IRQHandler,
        ::SoC::DMA2_Stream2_IRQHandler,
        ::SoC::DMA2_Stream3_IRQHandler,
        ::SoC::DMA2_Stream4_IRQHandler,
        ::SoC::ETH_IRQHandler,
        ::SoC::ETH_WKUP_IRQHandler,
        ::SoC::CAN2_TX_IRQHandler,
        ::SoC::CAN2_RX0_IRQHandler,
        ::SoC::CAN2_RX1_IRQHandler,
        ::SoC::CAN2_SCE_IRQHandler,
        ::SoC::OTG_FS_IRQHandler,
        ::SoC::DMA2_Stream5_IRQHandler,
        ::SoC::DMA2_Stream6_IRQHandler,
        ::SoC::DMA2_Stream7_IRQHandler,
        ::SoC::USART6_IRQHandler,
        ::SoC::I2C3_EV_IRQHandler,
        ::SoC::I2C3_ER_IRQHandler,
        ::SoC::OTG_HS_EP1_OUT_IRQHandler,
        ::SoC::OTG_HS_EP1_IN_IRQHandler,
        ::SoC::OTG_HS_WKUP_IRQHandler,
        ::SoC::OTG_HS_IRQHandler,
        ::SoC::DCMI_IRQHandler,
        nullptr,
        ::SoC::HASH_RNG_IRQHandler,
        ::SoC::FPU_IRQHandler,
    };

    extern "C"
    {
        [[gnu::weak]] void Default_Handler() noexcept { while(true); }

        [[using gnu: weak, alias("Default_Handler")]] void NMI_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void HardFault_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void MemManage_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void BusFault_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void UsageFault_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void SVC_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DebugMon_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void PendSV_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void SysTick_Handler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void WWDG_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void PVD_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TAMP_STAMP_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void RTC_WKUP_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void FLASH_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void RCC_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void EXTI0_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void EXTI1_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void EXTI2_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void EXTI3_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void EXTI4_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream0_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream1_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream2_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream3_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream4_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream5_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream6_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void ADC_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN1_TX_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN1_RX0_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN1_RX1_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN1_SCE_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void EXTI9_5_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM1_BRK_TIM9_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM1_UP_TIM10_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM1_TRG_COM_TIM11_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM1_CC_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM2_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM3_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM4_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void I2C1_EV_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void I2C1_ER_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void I2C2_EV_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void I2C2_ER_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void SPI1_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void SPI2_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void USART1_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void USART2_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void USART3_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void EXTI15_10_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void RTC_Alarm_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void OTG_FS_WKUP_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM8_BRK_TIM12_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM8_UP_TIM13_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM8_TRG_COM_TIM14_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM8_CC_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA1_Stream7_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void FSMC_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void SDIO_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM5_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void SPI3_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void UART4_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void UART5_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM6_DAC_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void TIM7_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream0_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream1_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream2_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream3_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream4_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void ETH_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void ETH_WKUP_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN2_TX_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN2_RX0_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN2_RX1_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void CAN2_SCE_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void OTG_FS_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream5_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream6_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DMA2_Stream7_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void USART6_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void I2C3_EV_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void I2C3_ER_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_EP1_OUT_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_EP1_IN_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_WKUP_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void OTG_HS_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void DCMI_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void HASH_RNG_IRQHandler() noexcept;
        [[using gnu: weak, alias("Default_Handler")]] void FPU_IRQHandler() noexcept;
    }
}  // namespace SoC
