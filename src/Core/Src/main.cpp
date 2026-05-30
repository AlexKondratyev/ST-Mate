/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.cpp
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
struct HardwareConfig {
    GameMode  gameMode;
    bool      prngEnabled;   // SET_PRNG_Pin == LOW
    bool      helpPressed;   // GET_HELP_Pin == LOW 
};

struct RuntimeState {
    // Data
    float        cpuVoltage  = 0.0f;
    struct tm    datetime    = {};
    unsigned int seed        = 0;

    // Commands 
    bool needReboot          = false;
    bool needSetTime         = true;

    // Settings that can be changed at runtime
    bool helpButtonActive    = true;
    bool helpButtonPressed   = false;  // Current state of the button
    bool diagnosticMode      = false;
};

static HardwareConfig hwCfg;
static RuntimeState   state;

// ============================================================
// Help functions
// ============================================================

void reboot()
{
	HAL_UART_Transmit(&huart1, (const uint8_t*)"ST-Mate will reboot\n", 20, 100);
	NVIC_SystemReset();
}

static HardwareConfig readHardwareConfig()
{
    HardwareConfig cfg;
    cfg.gameMode    = (GameMode)(HAL_GPIO_ReadPin(GPIOB, GM1_Pin)
                                + HAL_GPIO_ReadPin(GPIOB, GM0_Pin));
    cfg.prngEnabled = (HAL_GPIO_ReadPin(GPIOB, SET_PRNG_Pin) == GPIO_PIN_RESET);
    cfg.helpPressed = (HAL_GPIO_ReadPin(GPIOB, GET_HELP_Pin) == GPIO_PIN_RESET);
    return cfg;
}

static void printBanner(float cpuV, bool flashOk, const struct tm& dt,
                        GameMode mode, bool prng, unsigned int seed)
{
    printf("Voltage CPU: %.3f: %s\n", cpuV,
           (cpuV > 1.71f && cpuV < 3.6f) ? "OK" : "ERROR");
    printf("Flash state: %s\n", flashOk ? "OK" : "ERROR");
    printf("DateTime: %02d.%02d.%d %02d:%02d:%02d\n",
           dt.tm_mday, dt.tm_mon, dt.tm_year,
           dt.tm_hour, dt.tm_min, dt.tm_sec);
    printf("Mode: %s\n", modeStr[mode]);
    printf("RNG: %s. seed: %u\n", prng ? "ON" : "OFF", seed);
    printf("Initialization...successes\n");
    printf(logo, version);
}

// ============================================================
// Listener — разбит на методы по команде
// ============================================================

class MainListener : public Listener
{
    void handleReboot()
    {
        state.needReboot = true;
    }

    void handleVoltage()
    {
        printf("CPU Voltage: %.3f V\n", state.cpuVoltage);
    }

    void handleService(const std::string& msg)
    {
        int param = 0;
        if (sscanf(msg.c_str(), "SERVICE:%d", &param) == 1 && param >= 0 && param <= 1) {
            state.diagnosticMode = (bool)param;
            printf("Service mode is: %s\n", param ? "on" : "off");
        }
    }

    void handleSeedQuery()
    {
        printf("Seed: %u\n", state.seed);
    }

    // Seed может быть числом или строкой
    void handleSeedSet(const std::string& msg)
    {
        const char* value = msg.c_str() + 5; // skip "SEED:"
        int num = 0;
        if (sscanf(value, "%d", &num) == 1) {
            state.seed = (unsigned int)num;
            printf("Seed set: %u\n", state.seed);
            return;
        }
        // hash для строки
        unsigned long hash = 5381;
        for (const char* p = value; *p; ++p)
            hash = ((hash << 5) + hash) + (unsigned char)*p;
        state.seed = (unsigned int)hash;
        printf("Seed set: %u (from string)\n", state.seed);
    }

    void handleHintButton(const std::string& msg)
    {
        int param = 0;
        if (sscanf(msg.c_str(), "HINTBTN:%d", &param) == 1 && param >= 0 && param <= 1) {
            state.helpButtonActive = (bool)param;
            printf("Help button is: %s\n", param ? "active" : "inactive");
        }
    }

    void handleTimeQuery()
    {
        RTC_UNIT dt = rtc_GetTime();
        printf("Time: %02d.%02d.%02d %02d:%02d:%02d\n",
               dt.date, dt.month, dt.year,
               dt.hours, dt.minutes, dt.seconds);
    }

    void handleTimeSet(const std::string& msg)
    {
        int date, month, year, hours, minutes, seconds;
        const char* payload = msg.c_str() + 5; // skip "TIME:"
        if (sscanf(payload, "%2d.%2d.%2d %2d:%2d:%2d",
                   &date, &month, &year, &hours, &minutes, &seconds) == 6)
        {
            RTC_UNIT dt = {year, month, date, hours, minutes, seconds};
            rtc_SetTime(dt);
            state.needSetTime = true;
            printf("Time set: %02d.%02d.%02d %02d:%02d:%02d\n",
                   dt.date, dt.month, dt.year,
                   dt.hours, dt.minutes, dt.seconds);
        }
    }

public:
    void message(const std::string& msg) override
    {
        if (msg.find("HELP")     != std::string::npos) printf(helpText);
        if (msg.find("REBOOT")   != std::string::npos) handleReboot();
        if (msg.find("VOLT?")    != std::string::npos) handleVoltage();
        if (msg.find("SERVICE:") != std::string::npos) handleService(msg);
        if (msg.find("SEED?")    != std::string::npos) handleSeedQuery();
        if (msg.find("SEED:")    != std::string::npos) handleSeedSet(msg);
        if (msg.find("HINTBTN:") != std::string::npos) handleHintButton(msg);
        if (msg.find("TIME?")    != std::string::npos) handleTimeQuery();
        if (msg.find("TIME:")    != std::string::npos) handleTimeSet(msg);
    }
};

// ============================================================
// Main loop functions
// ============================================================

static void runDiagnosticMode(FieldDriver& fieldDriver, DriverLED& led, Fields& fields)
{
    fieldDriver.run();
    for (int i = 0; i < 64; i++) {
        switch (fields.getField(i)) {
            case Fields::white: led.setPixel(i,   0, 128,   0); break;
            case Fields::black: led.setPixel(i, 128,   0,   0); break;
            case Fields::none:  led.setPixel(i,   0,   0,   0); break;
        }
    }
    led.refresh();
}

static void runGameMode(BluetoothDriver& bluetooth,
                        FieldDriver& fieldDriver,
                        Interactor& interactor,
                        Presenter& presenter,
                        DriverLED& led)
{
    // Update values that may change at runtime
    GameMode currentMode = (GameMode)(HAL_GPIO_ReadPin(GPIOB, GM1_Pin)
                                    + HAL_GPIO_ReadPin(GPIOB, GM0_Pin));
    bool helpNow = (HAL_GPIO_ReadPin(GPIOB, GET_HELP_Pin) == GPIO_PIN_RESET)
                   && state.helpButtonActive;

    state.cpuVoltage = fieldDriver.getVRef();

    bluetooth.run();
    interactor.setSeed(state.seed);
    interactor.setMode(currentMode);
    interactor.setIsNeedHelp(helpNow);

    fieldDriver.run();
    interactor.run();
    presenter.run();
    led.refresh();
}

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

  // Read hardware config (buttons, jumpers) into hwCfg struct
  hwCfg = readHardwareConfig();

  // Create main objects
  BluetoothDriver 	bluetooth;
  FlashDriver     	flash(&hspi2, SPI2_CS_GPIO_Port, SPI2_CS_Pin);
  DriverLED       	led(&flash);
  Presenter       	presenter(&led, &flash);
  Interactor     	interactor(hwCfg.gameMode, &presenter, &flash);
  Controller 		controller(&interactor);
  Fields          	fields;
  FieldDriver     	fieldDriver(fields, &controller, &flash);

  // Read initial time from RTC
  {
      RTC_UNIT dtRTC = rtc_GetTime();
      state.datetime.tm_year = dtRTC.year + 2000;
      state.datetime.tm_mon  = dtRTC.month;
      state.datetime.tm_mday = dtRTC.date;
      state.datetime.tm_hour = dtRTC.hours;
      state.datetime.tm_min  = dtRTC.minutes;
      state.datetime.tm_sec  = dtRTC.seconds;
  }

  // Define mode based on buttons
  if (hwCfg.helpPressed) {
      if (hwCfg.prngEnabled) {
          // Both HELP and SERVICE: Diagnostic mode
          state.diagnosticMode = true;
          interactor.setMode(DIAGNOSTIC);
      } else {
          // Help only: Force field calibration
          fieldDriver.setForceCalibrate();
      }
  }

  state.cpuVoltage = fieldDriver.getVRef();
  state.seed = hwCfg.prngEnabled ? fieldDriver.getSeed() : 13;
  interactor.setSeed(state.seed);

  // Subscribe to Bluetooth messages
  MainListener mainListener;
  bluetooth.attach(&mainListener);
  bluetooth.attach(&led);
  bluetooth.attach(&fieldDriver);
  bluetooth.attach(&presenter);
  bluetooth.attach(&interactor);

  printBanner(state.cpuVoltage, flash.isInit(), state.datetime,
              hwCfg.gameMode, hwCfg.prngEnabled, state.seed);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if (state.needSetTime) {
          interactor.setDateTime(state.datetime);
          state.needSetTime = false;
      }

      if (state.needReboot)
          reboot();

      if (state.diagnosticMode)
          runDiagnosticMode(fieldDriver, led, fields);
      else
          runGameMode(bluetooth, fieldDriver, interactor, presenter, led);
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

// ============================================================
// System function
// ============================================================
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
