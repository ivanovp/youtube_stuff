/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
extern UART_HandleTypeDef huart2;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void displayHandler(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define DIGIT2_Pin GPIO_PIN_6
#define DIGIT2_GPIO_Port GPIOA
#define DIGIT1_Pin GPIO_PIN_7
#define DIGIT1_GPIO_Port GPIOA
#define LED_GREEN_4_Pin GPIO_PIN_10
#define LED_GREEN_4_GPIO_Port GPIOB
#define LED_GREEN_7_Pin GPIO_PIN_7
#define LED_GREEN_7_GPIO_Port GPIOC
#define LED_GREEN_5_Pin GPIO_PIN_8
#define LED_GREEN_5_GPIO_Port GPIOA
#define LED_GREEN_6_Pin GPIO_PIN_9
#define LED_GREEN_6_GPIO_Port GPIOA
#define LED_GREEN_0_Pin GPIO_PIN_10
#define LED_GREEN_0_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LED_GREEN_1_Pin GPIO_PIN_3
#define LED_GREEN_1_GPIO_Port GPIOB
#define LED_GREEN_3_Pin GPIO_PIN_4
#define LED_GREEN_3_GPIO_Port GPIOB
#define LED_GREEN_2_Pin GPIO_PIN_5
#define LED_GREEN_2_GPIO_Port GPIOB
#define DIGIT0_Pin GPIO_PIN_6
#define DIGIT0_GPIO_Port GPIOB
#define DIGIT3_Pin GPIO_PIN_9
#define DIGIT3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
