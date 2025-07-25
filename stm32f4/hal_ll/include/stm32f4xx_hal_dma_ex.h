/**
 ******************************************************************************
 * @file    stm32f4xx_hal_dma_ex.h
 * @author  MCD Application Team
 * @brief   Header file of DMA HAL extension module.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_HAL_DMA_EX_H
#define __STM32F4xx_HAL_DMA_EX_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal_def.h"

    /** @addtogroup STM32F4xx_HAL_Driver
     * @{
     */

    /** @addtogroup DMAEx
     * @{
     */

    /* Exported types ------------------------------------------------------------*/
    /** @defgroup DMAEx_Exported_Types DMAEx Exported Types
     * @brief DMAEx Exported types
     * @{
     */

    /**
     * @brief  HAL DMA Memory definition
     */
    typedef enum
    {
        MEMORY0 = 0x00U, /*!< Memory 0     */
        MEMORY1 = 0x01U  /*!< Memory 1     */
    } HAL_DMA_MemoryTypeDef;

    /**
     * @}
     */

    /* Exported functions --------------------------------------------------------*/
    /** @defgroup DMAEx_Exported_Functions DMAEx Exported Functions
     * @brief   DMAEx Exported functions
     * @{
     */

    /** @defgroup DMAEx_Exported_Functions_Group1 Extended features functions
     * @brief   Extended features functions
     * @{
     */

    /* IO operation functions *******************************************************/
    HAL_StatusTypeDef HAL_DMAEx_MultiBufferStart(DMA_HandleTypeDef* hdma,
                                                 uint32_t SrcAddress,
                                                 uint32_t DstAddress,
                                                 uint32_t SecondMemAddress,
                                                 uint32_t DataLength);
    HAL_StatusTypeDef HAL_DMAEx_MultiBufferStart_IT(DMA_HandleTypeDef* hdma,
                                                    uint32_t SrcAddress,
                                                    uint32_t DstAddress,
                                                    uint32_t SecondMemAddress,
                                                    uint32_t DataLength);
    HAL_StatusTypeDef HAL_DMAEx_ChangeMemory(DMA_HandleTypeDef* hdma, uint32_t Address, HAL_DMA_MemoryTypeDef memory);

    /**
     * @}
     */
    /**
     * @}
     */

    /* Private functions ---------------------------------------------------------*/
    /** @defgroup DMAEx_Private_Functions DMAEx Private Functions
     * @brief DMAEx Private functions
     * @{
     */
    /**
     * @}
     */

    /**
     * @}
     */

    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /*__STM32F4xx_HAL_DMA_EX_H*/
