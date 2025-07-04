/**
 ******************************************************************************
 * @file    stm32f4xx_hal_pcd_ex.h
 * @author  MCD Application Team
 * @brief   Header file of PCD HAL Extension module.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32F4xx_HAL_PCD_EX_H
#define STM32F4xx_HAL_PCD_EX_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal_def.h"

#if defined(USB_OTG_FS) || defined(USB_OTG_HS)
    /** @addtogroup STM32F4xx_HAL_Driver
     * @{
     */

    /** @addtogroup PCDEx
     * @{
     */
    /* Exported types ------------------------------------------------------------*/
    /* Exported constants --------------------------------------------------------*/
    /* Exported macros -----------------------------------------------------------*/
    /* Exported functions --------------------------------------------------------*/
    /** @addtogroup PCDEx_Exported_Functions PCDEx Exported Functions
     * @{
     */
    /** @addtogroup PCDEx_Exported_Functions_Group1 Peripheral Control functions
     * @{
     */
    #if defined(USB_OTG_FS) || defined(USB_OTG_HS)
    HAL_StatusTypeDef HAL_PCDEx_SetTxFiFo(PCD_HandleTypeDef* hpcd, uint8_t fifo, uint16_t size);
    HAL_StatusTypeDef HAL_PCDEx_SetRxFiFo(PCD_HandleTypeDef* hpcd, uint16_t size);
    #endif /* defined (USB_OTG_FS) || defined (USB_OTG_HS) */

    #if defined(STM32F446xx) || defined(STM32F469xx) || defined(STM32F479xx) || defined(STM32F412Zx) || defined(STM32F412Vx) ||  \
        defined(STM32F412Rx) || defined(STM32F412Cx) || defined(STM32F413xx) || defined(STM32F423xx)
    HAL_StatusTypeDef HAL_PCDEx_ActivateLPM(PCD_HandleTypeDef* hpcd);
    HAL_StatusTypeDef HAL_PCDEx_DeActivateLPM(PCD_HandleTypeDef* hpcd);
    #endif /* defined(STM32F446xx) || defined(STM32F469xx) || defined(STM32F479xx) || defined(STM32F412Zx) ||                    \
              defined(STM32F412Vx) || defined(STM32F412Rx) || defined(STM32F412Cx) || defined(STM32F413xx) ||                    \
              defined(STM32F423xx) */
    #if defined(STM32F412Zx) || defined(STM32F412Vx) || defined(STM32F412Rx) || defined(STM32F412Cx) || defined(STM32F413xx) ||  \
        defined(STM32F423xx)
    HAL_StatusTypeDef HAL_PCDEx_ActivateBCD(PCD_HandleTypeDef* hpcd);
    HAL_StatusTypeDef HAL_PCDEx_DeActivateBCD(PCD_HandleTypeDef* hpcd);
    void HAL_PCDEx_BCD_VBUSDetect(PCD_HandleTypeDef* hpcd);
    #endif /* defined(STM32F412Zx) || defined(STM32F412Vx) || defined(STM32F412Rx) ||                                            \
              defined(STM32F412Cx) || defined(STM32F413xx) || defined(STM32F423xx) */
    void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef* hpcd, PCD_LPM_MsgTypeDef msg);
    void HAL_PCDEx_BCD_Callback(PCD_HandleTypeDef* hpcd, PCD_BCD_MsgTypeDef msg);

/**
 * @}
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
#endif /* defined (USB_OTG_FS) || defined (USB_OTG_HS) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* STM32F4xx_HAL_PCD_EX_H */
