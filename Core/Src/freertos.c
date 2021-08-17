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
#include "LPS25HB.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/**
 * @brief Enum used for changing temperature unit after button press.
 * 
 */
typedef enum
{
  DISPLAY_CELSIUS,
  DISPLAY_FAHRENHEIT
} Temperature_UnitTypeDef;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BLUE_BUTTON_PRESSED 0x01         ///Macro used with LcdThreadFlag in USER_BUTTON interrupt.
#define LPS25HB_DMA_Complete 0x02        ///Macro used with LcdThreadFlag in DMA interrupt
#define DS18B20_Conversion_Complete 0x04 ///Macro used with LcdThreadFlag with DS18B20 sensor.

#define DEGREE_CHARACTER 0xDF ///Define used to display degree char.
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/**
 * @brief Global variable used by pressureTask to write pressure value from the LPS25HB and by LCD to display current value. 
 * 
 */
int32_t pressure = 0;
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
/* Definitions for pressureTask */
osThreadId_t pressureTaskHandle;
const osThreadAttr_t pressureTask_attributes = {
    .name = "pressureTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityBelowNormal7,
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
void StartPressureTask(void *argument);

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

  /* creation of pressureTask */
  pressureTaskHandle = osThreadNew(StartPressureTask, NULL, &pressureTask_attributes);

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
    volatile HAL_StatusTypeDef status = HAL_OK;
    //get temperature from sensor and send it to the LCD task
    status += DS18B20_Initialize(&ds18b20);
    osDelay(1);
    if (status == HAL_OK)
    {
      DS18B20_Start_Conversion(&ds18b20);
      osDelay(800);
    }
    status += DS18B20_Initialize(&ds18b20);
    if (status == HAL_OK)
    {
      osDelay(1);
      uint16_t temperature = DS18B20_Read_Temperature(&ds18b20);
      osMessageQueuePut(temperatureQueueHandle, &temperature, 0, 10); //sends temperature value do LCD
      osThreadFlagsSet(lcdTaskHandle, DS18B20_Conversion_Complete);   //tels LCD that temperature is ready to be displayed
    }
    osDelay(100);
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

  Lcd_HandleTypeDef lcd = Lcd_Create(&hi2c3, 0x4E); //0x4E is default slave address
  int temperature_unit = DISPLAY_CELSIUS;           //default display unit in celsius
  uint16_t temperature = 0;
  int error_flag = 0; //used to repaint whole LCD after an error
  volatile osStatus_t ds18b20_status = osOK;
  Lcd_String(&lcd, "Temp:");
  Lcd_Cursor(&lcd, 0, 14);
  Lcd_Hex(&lcd, DEGREE_CHARACTER);
  Lcd_Cursor(&lcd, 1, 0);
  Lcd_String(&lcd, "Pres:");
  Lcd_Cursor(&lcd, 1, 13);
  Lcd_String(&lcd, "hPa");
  /* Infinite loop */
  for (;;)
  {
    if ((osThreadFlagsGet() & LPS25HB_DMA_Complete) == LPS25HB_DMA_Complete) //if there is updated pressure value
    {
      Lcd_Cursor(&lcd, 1, 5);
      Lcd_Float_Decimal(&lcd, (float)pressure / 4096, 2); //dividing result value in order to achieve hPa
      osThreadFlagsClear(LPS25HB_DMA_Complete);
    }
    if ((osThreadFlagsGet() & BLUE_BUTTON_PRESSED) == BLUE_BUTTON_PRESSED)
    {
      //on blue button press change temp unit
      temperature_unit = (temperature_unit == DISPLAY_CELSIUS) ? DISPLAY_FAHRENHEIT : DISPLAY_CELSIUS;
      osThreadFlagsClear(BLUE_BUTTON_PRESSED);
    }
    if ((osThreadFlagsGet() & DS18B20_Conversion_Complete) == DS18B20_Conversion_Complete)
    {
      //if there is new temperature value
      ds18b20_status = osMessageQueueGet(temperatureQueueHandle, &temperature, NULL, 10); //get temperature from sensor
      osThreadFlagsClear(DS18B20_Conversion_Complete);
    }
    if (ds18b20_status != osOK) //if temperature sensor returned error code
    {
      Lcd_Clear(&lcd);
      Lcd_Cursor(&lcd, 0, 0);
      Lcd_String(&lcd, "TMP ERROR");
      error_flag = 1;
    }
    else
    {
      if (error_flag == 1) //if there was an error before, but now there is a correct reading
      {                    //used to redraw lcd screen after "TMP ERROR" message
        error_flag = 0;
        Lcd_Clear(&lcd);
        Lcd_Cursor(&lcd, 0, 0);
        Lcd_String(&lcd, "Temp:");
        Lcd_Cursor(&lcd, 0, 14);
        Lcd_Hex(&lcd, DEGREE_CHARACTER);
      }
      Lcd_Cursor(&lcd, 0, 5);

      if (temperature_unit == DISPLAY_CELSIUS)
      {
        Lcd_Float(&lcd, (float)temperature / 16); //celsius conversion
        Lcd_String(&lcd, " C");
      }
      else
      {
        Lcd_Float(&lcd, (float)temperature * 0.1125 + 32); //fahrenheit conversion
        Lcd_String(&lcd, " F");
      }
    }

    osDelay(50);
  }
  /* USER CODE END StartLcdTask */
}

/* USER CODE BEGIN Header_StartPressureTask */
/**
* @brief Function implementing the pressureTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPressureTask */
void StartPressureTask(void *argument)
{
  /* USER CODE BEGIN StartPressureTask */
  LPS25HB_HandleTypeDef lps25hb = LPS25HB_Create(&hi2c3, LPS25HB_ADDR); //creating instance of the peripheral
  LPS25HB_Set_Frequency(&lps25hb, _1_HZ);                               //setting freq
  osDelay(100);

  /* Infinite loop */
  for (;;)
  {
    LPS25HB_Read_Pressure_DMA(&lps25hb, &pressure);
    osDelay(1000); //1 Hz
  }
  /* USER CODE END StartPressureTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c == &hi2c3)
  {
    osThreadFlagsSet(lcdTaskHandle, LPS25HB_DMA_Complete); //sends msg to LCD about new pressure value
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == USER_BUTTON_Pin) //on USER_BUTTON press send flag to change temp unit
  {
    osThreadFlagsSet(lcdTaskHandle, BLUE_BUTTON_PRESSED);
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
