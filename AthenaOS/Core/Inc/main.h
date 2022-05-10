/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Flash_MOSI_Pin GPIO_PIN_1
#define Flash_MOSI_GPIO_Port GPIOC
#define Flash_MISO_Pin GPIO_PIN_2
#define Flash_MISO_GPIO_Port GPIOC
#define Flash_CS_Pin GPIO_PIN_3
#define Flash_CS_GPIO_Port GPIOC
#define ADC_Batt_Pin GPIO_PIN_1
#define ADC_Batt_GPIO_Port GPIOA
#define Altus_SCK_Pin GPIO_PIN_5
#define Altus_SCK_GPIO_Port GPIOA
#define Altus_MISO_Pin GPIO_PIN_6
#define Altus_MISO_GPIO_Port GPIOA
#define Altus_MOSI_Pin GPIO_PIN_7
#define Altus_MOSI_GPIO_Port GPIOA
#define Altus_CS_Pin GPIO_PIN_4
#define Altus_CS_GPIO_Port GPIOC
#define SD_MOSI_Pin GPIO_PIN_0
#define SD_MOSI_GPIO_Port GPIOB
#define Flash_SCK_Pin GPIO_PIN_10
#define Flash_SCK_GPIO_Port GPIOB
#define Status_LED_Pin GPIO_PIN_12
#define Status_LED_GPIO_Port GPIOB
#define Step_DIR_Pin GPIO_PIN_15
#define Step_DIR_GPIO_Port GPIOB
#define Extra_out_Pin GPIO_PIN_7
#define Extra_out_GPIO_Port GPIOC
#define Step_EN_Pin GPIO_PIN_9
#define Step_EN_GPIO_Port GPIOC
#define USB__Pin GPIO_PIN_11
#define USB__GPIO_Port GPIOA
#define USB_A12_Pin GPIO_PIN_12
#define USB_A12_GPIO_Port GPIOA
#define Buzz_PWM_Pin GPIO_PIN_15
#define Buzz_PWM_GPIO_Port GPIOA
#define SD_SCK_Pin GPIO_PIN_10
#define SD_SCK_GPIO_Port GPIOC
#define SD_MISO_Pin GPIO_PIN_11
#define SD_MISO_GPIO_Port GPIOC
#define SD_CS_Pin GPIO_PIN_2
#define SD_CS_GPIO_Port GPIOD
#define Step_PWM_Pin GPIO_PIN_4
#define Step_PWM_GPIO_Port GPIOB
#define Extra_SCL_Pin GPIO_PIN_6
#define Extra_SCL_GPIO_Port GPIOB
#define Extra_SDA_Pin GPIO_PIN_7
#define Extra_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
