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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "version_num.h"
#include "../../Drivers/stm32f1_rtc/stm32f1_rtc.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
// Greeting on startup
static const char* logo =
"###############################################\n"
"#    ___________      __  ___        __       #\n"
"#   / ___/_  __/     /  |/  /____   / /_ ___  #\n"
"#   \\__ \\ / /______ / /|_/ //__  \\ / __// _ \\ #\n"
"#  ___/ // //_____// /  / // _   // /_ /  __/ #\n"
"# /____//_/       /_/  /_/ \\__,_/ \\__/ \\___/  #\n"
"#                                             #\n"
"###############################################\n"
" ST-Mate %s (c)\n"
" Developed by: Alexander Kondratyev\n"
" System:  STM32F103C8 | 64kB Flash | 20kB RAM\n"
" Engine:  micro-Max v4.8 by H.G. Muller\n"
"###############################################\n\n";

static const char* helpText =
"\n=== SERVICE COMMANDS ===\n"
"REBOOT          - Reboot device\n"
"SERVICE:0/1     - Disable/enable service mode\n"
"VOLT?           - Show CPU voltage\n"
"CALIBRATE       - Start sensor calibration (board must be empty)\n"
"TRIGLV?         - Show trigger level\n"
"TRIGLV:X        - Set trigger level (X = integer)\n"
"ALPHA?          - Show smoothing coefficient\n"
"ALPHA:X         - Set smoothing coefficient (X = integer, 1/alpha)\n"
"\n=== USER COMMANDS ===\n"
"HELP            - Show this help\n"
"EVENT?          - Show event name\n"
"EVENT:X         - Set event name\n"
"SITE?           - Show venue\n"
"SITE:X          - Set venue\n"
"TIME?           - Show current time\n"
"TIME:dd.mm.yy HH:MM:SS - Set time\n"
"ROUND?          - Show round number\n"
"ROUND:X         - Set round number\n"
"WHITE?          - Show white player name\n"
"WHITE:X         - Set white player name\n"
"BLACK?          - Show black player name\n"
"BLACK:X         - Set black player name\n"
"FEN?            - Show current position in FEN\n"
"PGN?            - Show current game in PGN\n"
"SEED?           - Show random seed\n"
"SEED:X          - Set random seed (integer or string)\n"
"BRIGHT?         - Show brightness level\n"
"BRIGHT:X        - Set brightness (0-100)\n"
"THEME:X         - Set color theme (0-11)\n"
"HINTBTN:0/1     - Enable/disable hint button\n";
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC_S0_Pin GPIO_PIN_4
#define ADC_S0_GPIO_Port GPIOA
#define ADC_S1_Pin GPIO_PIN_5
#define ADC_S1_GPIO_Port GPIOA
#define ADC_S2_Pin GPIO_PIN_6
#define ADC_S2_GPIO_Port GPIOA
#define ADC_S3_Pin GPIO_PIN_7
#define ADC_S3_GPIO_Port GPIOA
#define GM0_Pin GPIO_PIN_0
#define GM0_GPIO_Port GPIOB
#define GM1_Pin GPIO_PIN_1
#define GM1_GPIO_Port GPIOB
#define GET_HELP_Pin GPIO_PIN_10
#define GET_HELP_GPIO_Port GPIOB
#define SET_PRNG_Pin GPIO_PIN_11
#define SET_PRNG_GPIO_Port GPIOB
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define LED_SIG_Pin GPIO_PIN_6
#define LED_SIG_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
void TIM4_IRQHandler_Custom();
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
