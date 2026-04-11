/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

#include "../../Drivers/FlashDriver/FlashDriver.h"
#include "../../Drivers/DriverLED/DriverLED.h"
#include "../../Drivers/BluetoothDriver/BluetoothDriver.h"
#include "../../app/Interactor/Interactor.h"

#include "../../app/Adapters/in/Controller.h"
#include "../../app/Adapters/out/Presenter.h"

#include "../../app/Entities/ChessEngine.h"
#include "../../app/Entities/Converter.h"
#include "../../Drivers/FieldDriver/FieldDriver.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Приветствие при запуске
const char* logo =
"###############################################\n"
"#    ___________      __  ___        __       #\n"
"#   / ___/_  __/     /  |/  /____   / /_ ___  #\n"
"#   \\__ \\ / /______ / /|_/ //__  \\ / __// _ \\ #\n"
"#  ___/ // //_____// /  / // _   // /_ /  __/ #\n"
"# /____//_/       /_/  /_/ \\__,_/ \\__/ \\___/  #\n"
"#          Your Silicon Grandmaster           #\n"
"###############################################\n"
" ST-Mate %s (c)\n"
" Developed by: Alex Kondratyev\n"
" System:  STM32F103C8 | 64kB Flash | 20kB RAM\n"
" Engine:  micro-Max v4.8 by H.G. Muller\n"
"###############################################\n\n";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern "C" {
int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  HAL_UART_Transmit(&huart1, (const uint8_t*)ptr, len, 100);
  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}
}

struct SystemState {
	bool fieldTest = false;
	bool needReboot = false;
	bool needForceCalibrate = false;
	bool isFlashInit = false;
	bool needSetTime = true;
	bool isActivatePRNG = false;
	bool isNeedHelp = false;
	bool isActiveHelpButton = true;

	GameMode portGameMode;
	float cpuV;
	struct tm dt;
	unsigned int seed = 0;
}state_;


void reboot()
{
	HAL_UART_Transmit(&huart1, (const uint8_t*)"ST-Mate will reboot\n", 20, 100);
	NVIC_SystemReset();
}

void printInitialisation()
{
	printf("Voltage CPU: %.3f: %s\n", state_.cpuV, (state_.cpuV > 1.71 || state_.cpuV > 3.6) ? "OK":"ERROR" );
	printf("Flash state: %s\n", state_.isFlashInit ? "OK":"ERROR");
	printf("DateTime: %02d.%02d.%d %02d:%02d:%02d\n",
			state_.dt.tm_mday,
			state_.dt.tm_mon,
			state_.dt.tm_year,
			state_.dt.tm_hour,
			state_.dt.tm_min,
			state_.dt.tm_sec);
	printf("Mode: %s\n", modeStr[state_.portGameMode]);
	printf("PRNG: %s", state_.isActivatePRNG ? "on":"off"); printf(". seed: %d\n", state_.seed);
	printf("Initialization...successes \n");
	printf(logo, version);
}

class MainListener : public Listener
{
	void messege(const std::string &message) override
	{
		int param = 0;
		if (message.find("REBOOT") != std::string::npos)
		{
			state_.needReboot = true;
		}
		if (message.find("VOLT?") != std::string::npos)
		{
			printf("CPU Voltage: %.3f V\n", state_.cpuV);
		}
		if (message.find("SERVICE:") != std::string::npos)
		{
		    if (sscanf(message.c_str(), "SERVICE:%d", &param) == 1)
		    {
		    	if (param >= 0 && param <= 1)
		    	{
		    		state_.fieldTest = (bool)param;
		    		printf("Service mode is: %s\n",param==1?"on":"off");
		    	}
		    }
		}
		if (message.find("SEED?") != std::string::npos)
		{
			printf("Seed: %d\n", state_.seed);
		}
		if (message.find("SEED:") != std::string::npos)
		{
		    const char* value = message.c_str() + 5; // пропускаем "SEED:"
		    // Пробуем распарсить как десятичное число
		    int num;
		    if (sscanf(value, "%d", &num) == 1)
		    {
		    	state_.seed = num;
		        printf("Seed set: %d\n", state_.seed);
		        return;
		    }
		    // Иначе считаем хеш строки)
		    unsigned long hash = 5381;
		    int c;
		    while ((c = *value++) != '\0')
		        hash = ((hash << 5) + hash) + c; // hash * 33 + c
		    state_.seed = hash;
		    printf("Seed set: %d (from string)\n", state_.seed);
		}

		if (message.find("HINTBTN:") != std::string::npos)
		{
		    if (sscanf(message.c_str(), "HINTBTN:%d", &param) == 1)
		    {
		    	if (param >= 0 && param <= 1)
		    	{
		    		state_.isActiveHelpButton = (bool)param;
		    		printf("Help button is: %s\n",param==1?"active":"inactive");
		    	}
		    }
		}
		if (message.find("TIME?") != std::string::npos)
		{
			RTC_UNIT dt = rtc_GetTime();
			printf("Time: %02d.%02d.%02d %02d:%02d:%02d\n",
				  dt.date,
				  dt.month,
				  dt.year,
				  dt.hours,
				  dt.minutes,
				  dt.seconds);
		}

		if (message.find("TIME:") != std::string::npos)
		{
		    int date, month, year, hours,  minutes, seconds;
		    // Ищем начало строки времени после "TIME:"
		    const char* msg = message.c_str() + 5; // пропускаем "TIME:"
		    if (sscanf(msg, "%2d.%2d.%2d %2d:%2d:%2d",
						  &date, &month, &year,
						  &hours, &minutes ,&seconds) == 6)
		    {
		    	RTC_UNIT dt =  {year, month, date, hours, minutes, seconds};
		        rtc_SetTime(dt);
		        state_.needSetTime = true;
		        printf("Time set: %02d.%02d.%02d %02d:%02d:%02d\n",
		               dt.date, dt.month, dt.year,
		               dt.hours, dt.minutes, dt.seconds);
		    }
		}

	}
};

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
 {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

  BluetoothDriver bluetooth;

  FlashDriver flash(&hspi2, SPI2_CS_GPIO_Port, SPI2_CS_Pin);
  state_.isFlashInit = flash.isInit();

  RTC_UNIT dtRTC = rtc_GetTime();
  state_.dt.tm_year = dtRTC.year+2000;
  state_.dt.tm_mon = dtRTC.month;
  state_.dt.tm_mday = dtRTC.date;
  state_.dt.tm_hour = dtRTC.hours;
  state_.dt.tm_min = dtRTC.minutes;
  state_.dt.tm_sec = dtRTC.seconds;

  state_.portGameMode = (GameMode)(HAL_GPIO_ReadPin(GPIOB, GM1_Pin) + HAL_GPIO_ReadPin(GPIOB, GM0_Pin));
  state_.isActivatePRNG = HAL_GPIO_ReadPin(GPIOB, SET_PRNG_Pin) == GPIO_PIN_RESET;
  state_.isNeedHelp = (HAL_GPIO_ReadPin(GPIOB, GET_HELP_Pin) == GPIO_PIN_RESET);     // check button PB10

  if (state_.isNeedHelp)
  {
	  if (state_.isActivatePRNG)
		  state_.fieldTest = true;
	  else
		  state_.needForceCalibrate = true;
  }

  MainListener mainListener;

  DriverLED led(&flash);

  Presenter presenter(&led, &flash);
  Interactor interactor(state_.portGameMode,
		  	  	  	  	  &presenter,
						  &flash);
  Controller controller(&interactor);

  Fields fields;
  FieldDriver fieldDriver(
						  fields,
		  	  	  	  	  &controller,
						  &flash
						  );
  if (state_.needForceCalibrate)
	  fieldDriver.setForceCalibrate();

  // Подписки на команды от Bluetooth модуля
  bluetooth.attach(&mainListener);
  bluetooth.attach(&led);
  bluetooth.attach(&fieldDriver);
  bluetooth.attach(&presenter);
  bluetooth.attach(&interactor);

  state_.cpuV = fieldDriver.getVRef();

  if (state_.isActivatePRNG)	// if false, seed default
	  state_.seed = fieldDriver.getSeed();
  else
	  state_.seed = 25;  		// default seed for white 1. e2e4
  interactor.setSeed(state_.seed);

  printInitialisation();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (state_.needSetTime)
	  {
		  interactor.setDateTime(state_.dt);
		  state_.needSetTime = false;
	  }

	  if (state_.needReboot)
		  reboot();

	  state_.cpuV = fieldDriver.getVRef();
	  state_.portGameMode = (GameMode)(HAL_GPIO_ReadPin(GPIOB, GM1_Pin) + HAL_GPIO_ReadPin(GPIOB, GM0_Pin));
	  state_.isNeedHelp = (HAL_GPIO_ReadPin(GPIOB, GET_HELP_Pin) == GPIO_PIN_RESET) && state_.isActiveHelpButton;     // check button PB10

	  if (state_.fieldTest)
	  {
		  fieldDriver.run();
		  for(int i = 0; i < 64; i++)
		  {
			  switch (fields.getField(i)) {
				case Fields::white: led.setPixel(i, 0, 128, 0); break;
				case Fields::black: led.setPixel(i, 128, 0, 0); break;
				case Fields::none: led.setPixel(i, 0, 0, 0); break;
			  }
		  }
		  led.refresh();
	  }
	  else
	  {
		  bluetooth.run();
		  interactor.setSeed(state_.seed);
		  interactor.setMode(state_.portGameMode);
		  interactor.setIsNeedHelp(state_.isNeedHelp);

		  fieldDriver.run();
		  interactor.run();
		  presenter.run();
		  led.refresh();
	  }
	  /* USER CODE END WHILE */

	  /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
