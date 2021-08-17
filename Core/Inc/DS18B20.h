#ifndef INC_DS18B20_H_
#define INC_DS18B20_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "usart.h"

/************************************** Typedefs **************************************/
typedef struct
{
    UART_HandleTypeDef *huart;
} DS18B20_HandleTypeDef;

/************************************** ROM COMMANDS **************************************/
#define DS18B20_SEARCH_ROM 0xF0
#define DS18B20_READ_ROM 0x33
#define DS18B20_MATCH_ROM 0x55
#define DS18B20_SKIP_ROM 0xCC
#define DS18B20_ALARM_SEARCH 0xEC

/************************************** FUNCTION COMMANDS **************************************/
#define DS18B20_CONVERT_T 0x44
#define DS18B20_WRITE_SCRATCHPAD 0x4E
#define DS18B20_READ_SCRATCHPAD 0xBE
#define DS18B20_COPY_SCRATCHPAD 0x48
#define DS18B20_RECALL_E2 0xB8
#define DS18B20_READ_POWER 0xB4

/************************************** Public funtions **************************************/
DS18B20_HandleTypeDef DS18B20_Create(UART_HandleTypeDef *huart);
HAL_StatusTypeDef DS18B20_Initialize(DS18B20_HandleTypeDef *ds18b20);
void DS18B20_Start_Conversion(DS18B20_HandleTypeDef *ds18b20);
uint16_t DS18B20_Read_Temperature(DS18B20_HandleTypeDef *ds18b20);

#endif /* INC_DS18B20_H_ */
