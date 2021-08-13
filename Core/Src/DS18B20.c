/*
 * DS18B20.c
 *
 *  Created on: Jun 22, 2021
 *      Author: artur
 */

#include "DS18B20.h"

//static functions declarations//////////////
static void Set_Baudrate(uint32_t baudrate);
static void Write_Byte(DS18B20_HandleTypeDef ds18b20, uint8_t byte);
static uint8_t Read_Byte(DS18B20_HandleTypeDef ds18b20);
static void Write_Bit(int value);
static int Read_Bit();

DS18B20_HandleTypeDef DS18B20_Create(UART_HandleTypeDef *uart)
{
	DS18B20_HandleTypeDef ds18b20;
	ds18b20.huart = uart;
	return ds18b20;
}

HAL_StatusTypeDef DS18B20_Initialize(DS18B20_HandleTypeDef ds18b20)
{
	uint8_t data_out = 0xF0;
	uint8_t data_in = 0;

	Set_Baudrate(9600);
	HAL_UART_Transmit(&huart3, &data_out, 1, HAL_MAX_DELAY);
	HAL_UART_Receive(&huart3, &data_in, 1, HAL_MAX_DELAY);
	Set_Baudrate(115200);

	if (data_in != 0xF0)
		return HAL_OK;
	else
		return HAL_ERROR;
}

void DS18B20_Start_Conversion(DS18B20_HandleTypeDef ds18b20)
{
	Write_Byte(ds18b20, SKIP_ROM);	//Single device connected
	Write_Byte(ds18b20, CONVERT_T); //Start temperature conversion
}

uint16_t DS18B20_Read_Temperature(DS18B20_HandleTypeDef ds18b20)
{
	Write_Byte(ds18b20, SKIP_ROM);		  //Single device connected
	Write_Byte(ds18b20, READ_SCRATCHPAD); //Get temperature from the device
	uint8_t tmp1 = Read_Byte(ds18b20);
	uint8_t tmp2 = Read_Byte(ds18b20);
	uint16_t temperature = tmp1 | (tmp2 << 8);
	return temperature; //return float
}

//static functions///////////////////////////
static void Set_Baudrate(uint32_t baudrate)
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

static void Write_Byte(DS18B20_HandleTypeDef ds18b20, uint8_t byte)
{
	for (int i = 0; i < 8; i++)
	{
		Write_Bit(byte & 0x01);
		byte >>= 1;
	}
}

static uint8_t Read_Byte(DS18B20_HandleTypeDef ds18b20)
{
	uint8_t value = 0;
	for (int i = 0; i < 8; i++)
	{
		value >>= 1;
		if (Read_Bit())
			value |= 0x80;
	}
	return value;
}

static void Write_Bit(int value)
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

static int Read_Bit()
{
	uint8_t data_out = 0xFF;
	uint8_t data_in = 0;
	HAL_UART_Transmit(&huart3, &data_out, 1, HAL_MAX_DELAY);
	HAL_UART_Receive(&huart3, &data_in, 1, HAL_MAX_DELAY);

	return data_in & 0x01;
}