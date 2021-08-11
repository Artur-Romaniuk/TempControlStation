/*
 * DS18B20.c
 *
 *  Created on: Jun 22, 2021
 *      Author: artur
 */

#include "DS18B20.h"

//static functions declarations//////////////
static int DS18B20_Initialize();
static void set_baudrate(uint32_t baudrate);
static void write_bit(int value);
static int read_bit();
// static void Micro_Delay(uint16_t delay);
// static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
// static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
// static void Set_Pin_Output_Default(DS18B20_HandleTypeDef ds18b20);
// static void Set_Pin_Input_Default(DS18B20_HandleTypeDef ds18b20);
static void Write_Byte(DS18B20_HandleTypeDef ds18b20, uint8_t byte);
static uint8_t Read_Byte(DS18B20_HandleTypeDef ds18b20);

static void set_baudrate(uint32_t baudrate)
{
	huart3.Instance = USART3;
	huart3.Init.BaudRate = baudrate;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_HalfDuplex_Init(&huart3) != HAL_OK)
	{
		Error_Handler();
	}
}

static void write_bit(int value)
{
	if (value)
	{
		uint8_t data_out = 0xff;
		HAL_UART_Transmit(&huart3, &data_out, 1, HAL_MAX_DELAY);
	}
	else
	{
		uint8_t data_out = 0x0;
		HAL_UART_Transmit(&huart3, &data_out, 1, HAL_MAX_DELAY);
	}
}

static int read_bit(void)
{
	uint8_t data_out = 0xFF;
	uint8_t data_in = 0;
	HAL_UART_Transmit(&huart3, &data_out, 1, HAL_MAX_DELAY);
	HAL_UART_Receive(&huart3, &data_in, 1, HAL_MAX_DELAY);

	return data_in & 0x01;
}

DS18B20_HandleTypeDef DS18B20_Create(UART_HandleTypeDef *uart)
{
	DS18B20_HandleTypeDef ds18b20;
	ds18b20.huart = uart;
	return ds18b20;
}

uint16_t DS18B20_Read_Temperature(DS18B20_HandleTypeDef ds18b20)
{
	if (DS18B20_Initialize(ds18b20))
	{
		return DS18B20_ERROR; //error, no response from Thermometer
	}
	else
	{
		DS18B20_DELAY(1);				//BLOCKING!
		Write_Byte(ds18b20, SKIP_ROM);	//Single device connected
		Write_Byte(ds18b20, CONVERT_T); //Start temperature conversion

		DS18B20_DELAY(800); //BLOCKING! Gives time for temp conversion

		if (DS18B20_Initialize(ds18b20))
		{
			return DS18B20_ERROR; //error, device didn't respond
		}
		else
		{
			DS18B20_DELAY(1);					  //BLOCKING! Waits for voltage to normalize
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
	// uint8_t response;
	// Set_Pin_Output_Default(ds18b20);
	// HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
	// Micro_Delay(480); //datasheet
	// Set_Pin_Input_Default(ds18b20);
	// Micro_Delay(100); //datasheet
	// if (!HAL_GPIO_ReadPin(ds18b20.data_port, ds18b20.data_pin))
	// {
	// 	response = 0; //correct
	// }
	// else
	// {
	// 	response = 1; //empty line, error
	// }
	// Micro_Delay(380);
	// return response;

	uint8_t data_out = 0xF0;
	uint8_t data_in = 0;

	set_baudrate(9600);
	HAL_UART_Transmit(&huart3, &data_out, 1, HAL_MAX_DELAY);
	HAL_UART_Receive(&huart3, &data_in, 1, HAL_MAX_DELAY);
	set_baudrate(115200);

	if (data_in != 0xF0)
		return 0;
	else
		return 1;
}

// static void Micro_Delay(uint16_t delay)
// {
// 	__HAL_TIM_SET_COUNTER(MICRO_DELAY_TIM_HANDLE, 0);
// 	while ((__HAL_TIM_GET_COUNTER(MICRO_DELAY_TIM_HANDLE)) < delay)
// 		;
// }

// static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
// {
// 	GPIO_InitTypeDef GPIO_InitStruct = {0};
// 	GPIO_InitStruct.Pin = GPIO_Pin;
// 	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
// 	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
// 	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
// }

// static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
// {
// 	GPIO_InitTypeDef GPIO_InitStruct = {0};
// 	GPIO_InitStruct.Pin = GPIO_Pin;
// 	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
// 	GPIO_InitStruct.Pull = GPIO_NOPULL;
// 	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
// }

// static void Set_Pin_Output_Default(DS18B20_HandleTypeDef ds18b20)
// {
// 	Set_Pin_Output(ds18b20.data_port, ds18b20.data_pin);
// }

// static void Set_Pin_Input_Default(DS18B20_HandleTypeDef ds18b20)
// {
// 	Set_Pin_Input(ds18b20.data_port, ds18b20.data_pin);
// }

static void Write_Byte(DS18B20_HandleTypeDef ds18b20, uint8_t byte)
{
	// for (int i = 0; i < 8; i++)
	// {
	// 	if ((byte & (1 << i)) != 0)
	// 	{
	// 		Set_Pin_Output_Default(ds18b20);
	// 		HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
	// 		Micro_Delay(1);
	// 		Set_Pin_Input_Default(ds18b20);
	// 		Micro_Delay(60);
	// 	}
	// 	else
	// 	{
	// 		Set_Pin_Output_Default(ds18b20);
	// 		HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
	// 		Micro_Delay(60);
	// 		Set_Pin_Input_Default(ds18b20);
	// 	}
	// }
	int i;
	for (i = 0; i < 8; i++)
	{
		write_bit(byte & 0x01);
		byte >>= 1;
	}
}

static uint8_t Read_Byte(DS18B20_HandleTypeDef ds18b20)
{
	// uint8_t byte = 0;
	// Set_Pin_Input_Default(ds18b20);
	// for (int i = 0; i < 8; i++)
	// {
	// 	Set_Pin_Output_Default(ds18b20);
	// 	HAL_GPIO_WritePin(ds18b20.data_port, ds18b20.data_pin, GPIO_PIN_RESET);
	// 	Micro_Delay(2);
	// 	Set_Pin_Input_Default(ds18b20);
	// 	Micro_Delay(10);
	// 	if (HAL_GPIO_ReadPin(ds18b20.data_port, ds18b20.data_pin))
	// 	{
	// 		byte |= 1 << i;
	// 	}
	// 	Micro_Delay(50);
	// }
	// return byte;

	uint8_t value = 0;
	int i;
	for (i = 0; i < 8; i++)
	{
		value >>= 1;
		if (read_bit())
			value |= 0x80;
	}
	return value;
}