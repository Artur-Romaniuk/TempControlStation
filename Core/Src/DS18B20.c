/*
 * DS18B20.c
 *
 *  Created on: Jun 22, 2021
 *      Author: artur
 */

#include "DS18B20.h"

//static functions declarations//////////////
static int DS18B20_Initialize();
static void Micro_Delay(uint16_t delay);
static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
static void Set_Pin_Output_Default(DS18B20_HandleTypeDef ds18b20);
static void Set_Pin_Input_Default(DS18B20_HandleTypeDef ds18b20);
static void Write_Byte(DS18B20_HandleTypeDef ds18b20, uint8_t byte);
static uint8_t Read_Byte(DS18B20_HandleTypeDef ds18b20);

DS18B20_HandleTypeDef DS18B20_Create(DS18B20_PortType data_port, DS18B20_PinType data_pin)
{
	DS18B20_HandleTypeDef ds18b20;
	ds18b20.data_port = data_port;
	ds18b20.data_pin = data_pin;
	return ds18b20;
}

uint16_t DS18B20_Read_Temperature(DS18B20_HandleTypeDef ds18b20)
{
	if (DS18B20_Initialize(ds18b20))
	{
		return 0xffff; //error, no response from Thermometer
	}
	else
	{
		HAL_Delay(1);					//BLOCKING!
		Write_Byte(ds18b20, SKIP_ROM);	//Single device connected
		Write_Byte(ds18b20, CONVERT_T); //Start temperature conversion

		HAL_Delay(800); //BLOCKING! Gives time for temp conversion

		if (DS18B20_Initialize(ds18b20))
		{
			return 0xffff; //error, device didn't respond
		}
		else
		{
			HAL_Delay(1);						  //BLOCKING! Waits for voltage to normalize
			Write_Byte(ds18b20, SKIP_ROM);		  //Single device connected
			Write_Byte(ds18b20, READ_SCRATCHPAD); //Get temperature from the device
			uint8_t tmp1 = Read_Byte(ds18b20);
			uint8_t tmp2 = Read_Byte(ds18b20);
			uint16_t temperature = tmp1 | (tmp2 << 8);
			return temperature; //return float
		}
	}
}

//static functions///////////////////////////

static int DS18B20_Initialize(DS18B20_HandleTypeDef ds18b20)
{
	uint8_t response;
	Set_Pin_Output_Default(ds18b20);
	HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
	Micro_Delay(480); //datasheet
	Set_Pin_Input_Default(ds18b20);
	Micro_Delay(100); //datasheet
	if (!HAL_GPIO_ReadPin(ds18b20.data_port, ds18b20.data_pin))
	{
		response = 0; //correct
	}
	else
	{
		response = 1; //empty line, error
	}
	Micro_Delay(380);
	return response;
}

static void Micro_Delay(uint16_t delay)
{
	__HAL_TIM_SET_COUNTER(MICRO_DELAY_TIM_HANDLE, 0);
	while ((__HAL_TIM_GET_COUNTER(MICRO_DELAY_TIM_HANDLE)) < delay)
		;
}

static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

static void Set_Pin_Output_Default(DS18B20_HandleTypeDef ds18b20)
{
	Set_Pin_Output(ds18b20.data_port, ds18b20.data_pin);
}

static void Set_Pin_Input_Default(DS18B20_HandleTypeDef ds18b20)
{
	Set_Pin_Input(ds18b20.data_port, ds18b20.data_pin);
}

static void Write_Byte(DS18B20_HandleTypeDef ds18b20, uint8_t byte)
{
	for (int i = 0; i < 8; i++)
	{
		if ((byte & (1 << i)) != 0)
		{
			Set_Pin_Output_Default(ds18b20);
			HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
			Micro_Delay(1);
			Set_Pin_Input_Default(ds18b20);
			Micro_Delay(60);
		}
		else
		{
			Set_Pin_Output_Default(ds18b20);
			HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
			Micro_Delay(60);
			Set_Pin_Input_Default(ds18b20);
		}
	}
}

static uint8_t Read_Byte(DS18B20_HandleTypeDef ds18b20)
{
	uint8_t byte = 0;
	Set_Pin_Input_Default(ds18b20);
	for (int i = 0; i < 8; i++)
	{
		Set_Pin_Output_Default(ds18b20);
		HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
		Micro_Delay(2);
		Set_Pin_Input_Default(ds18b20);
		Micro_Delay(10);
		if (HAL_GPIO_ReadPin(ds18b20.data_port, ds18b20.data_pin))
		{
			byte |= 1 << i;
		}
		Micro_Delay(50);
	}
	return byte;
}