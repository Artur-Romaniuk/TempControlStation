#ifndef INC_DS18B20_H_
#define INC_DS18B20_H_

#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "tim.h"

#define MICRO_DELAY_TIM_HANDLE &htim7

#define DS18B20_PortType GPIO_TypeDef *
#define DS18B20_PinType uint16_t

typedef struct
{
    DS18B20_PortType data_port;
    DS18B20_PinType data_pin;
} DS18B20_HandleTypeDef;

/*********ROM COMMANDS ***********/
#define SEARCH_ROM 0xF0
#define READ_ROM 0x33
#define MATCH_ROM 0x55
#define SKIP_ROM 0xCC
#define ALARM_SEARCH 0xEC

/*********FUNCTION COMMANDS*******/
#define CONVERT_T 0x44
#define WRITE_SCRATCHPAD 0x4E
#define READ_SCRATCHPAD 0xBE
#define COPY_SCRATCHPAD 0x48
#define RECALL_E2 0xB8
#define READ_POWER 0xB4

/*********FUNCTIONS**************/
DS18B20_HandleTypeDef DS18B20_Create(DS18B20_PortType data_port, DS18B20_PinType data_pin);
uint16_t DS18B20_Read_Temperature(DS18B20_HandleTypeDef ds18b20); //return temperature if successful, 0xffff if not. Value has to be divided by 16
//float DS18B20_Read_Temperature_NB(); //Non blocking version

#endif /* INC_DS18B20_H_ */
