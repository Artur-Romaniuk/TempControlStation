/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"

#include "HD44780_LCD.h"
#include "DS18B20.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

//enum used for changing temperature unit after button press
typedef enum
{
  DISPLAY_CELSIUS,
  DISPLAY_FAHRENHEIT
} Temperature_UnitTypeDef;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUTTON_PRESSED 1      //used with ThreadFlag in USER_BUTTON interrupt
#define DEGREE_CHARACTER 0xDF //used to display degree char
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for thermometerTask */
osThreadId_t thermometerTaskHandle;
const osThreadAttr_t thermometerTask_attributes = {
    .name = "thermometerTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for lcdTask */
osThreadId_t lcdTaskHandle;
const osThreadAttr_t lcdTask_attributes = {
    .name = "lcdTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for temperatureQueue */
osMessageQueueId_t temperatureQueueHandle;
const osMessageQueueAttr_t temperatureQueue_attributes = {
    .name = "temperatureQueue"};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartThermometerTask(void *argument);
void StartLcdTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
  /* place for user code */
}

__weak void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
  /* place for user code */
}
/* USER CODE END PREPOSTSLEEP */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of temperatureQueue */
  temperatureQueueHandle = osMessageQueueNew(16, sizeof(uint16_t), &temperatureQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of thermometerTask */
  thermometerTaskHandle = osThreadNew(StartThermometerTask, NULL, &thermometerTask_attributes);

  /* creation of lcdTask */
  lcdTaskHandle = osThreadNew(StartLcdTask, NULL, &lcdTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartThermometerTask */
/**
  * @brief  Function implementing the thermometerTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartThermometerTask */
void StartThermometerTask(void *argument)
{
  /* USER CODE BEGIN StartThermometerTask */
  DS18B20_HandleTypeDef ds18b20 = DS18B20_Create(&huart3);

  /* Infinite loop */
  for (;;)
  {
    //get temperature from sensor and send it to the LCD task
    DS18B20_Initialize(ds18b20);
    osDelay(1);
    DS18B20_Start_Conversion(ds18b20);
    osDelay(800);
    DS18B20_Initialize(ds18b20);
    osDelay(1);
    uint16_t temperature = DS18B20_Read_Temperature(ds18b20);
    osMessageQueuePut(temperatureQueueHandle, &temperature, 0, osWaitForever);
    osDelay(200);
  }
  /* USER CODE END StartThermometerTask */
}

/* USER CODE BEGIN Header_StartLcdTask */
/**
* @brief Function implementing the lcdTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLcdTask */
void StartLcdTask(void *argument)
{
  /* USER CODE BEGIN StartLcdTask */

  Lcd_HandleTypeDef lcd = Lcd_Create(&hi2c1, 0x4E); //0x4E is default slave address
  int temperature_unit = DISPLAY_CELSIUS;           //default display unit in celsius
  int error_flag = 0;                               //used to repaint whole LCD after an error
  Lcd_String(&lcd, "Temperature:");
  /* Infinite loop */
  for (;;)
  {
    if (osThreadFlagsGet() == BUTTON_PRESSED)
    {
      //on button press change temp unit and clear flag
      temperature_unit = (temperature_unit == DISPLAY_CELSIUS) ? DISPLAY_FAHRENHEIT : DISPLAY_CELSIUS;
      osThreadFlagsClear(BUTTON_PRESSED);
    }

    // //get temperature from sensor
    uint16_t temperature = 0;
    osMessageQueueGet(temperatureQueueHandle, &temperature, NULL, osWaitForever);

    if (temperature == DS18B20_ERROR) //if sensor returned error code
    {
      Lcd_Clear(&lcd);
      Lcd_Cursor(&lcd, 1, 0);
      Lcd_String(&lcd, "TMP ERROR");
      error_flag = 1;
    }
    else if (error_flag == 1) //if there was an error before
    {
      error_flag = 0;
      Lcd_Clear(&lcd);
      Lcd_Cursor(&lcd, 0, 0);
      Lcd_String(&lcd, "Temperature:");
      Lcd_Cursor(&lcd, 1, 0);
      if (temperature_unit == DISPLAY_CELSIUS)
      {
        Lcd_Float(&lcd, (float)temperature / 16);
        Lcd_String(&lcd, " C");
        Lcd_Hex(&lcd, DEGREE_CHARACTER);
      }
      else
      {
        Lcd_Float(&lcd, (float)temperature * 0.1125 + 32);
        Lcd_String(&lcd, " F");
        Lcd_Hex(&lcd, DEGREE_CHARACTER);
      }
    }
    else //normal case, updating only temp value
    {
      Lcd_Cursor(&lcd, 1, 0);
      if (temperature_unit == DISPLAY_CELSIUS)
      {
        Lcd_Float(&lcd, (float)temperature / 16);
      }
      else
      {
        Lcd_Float(&lcd, (float)temperature * 0.1125 + 32);
      }
    }
    osDelay(500);
  }
  /* USER CODE END StartLcdTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == USER_BUTTON_Pin) //on USER_BUTTON press send flag to change temp unit
  {
    osThreadFlagsSet(lcdTaskHandle, BUTTON_PRESSED);
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
