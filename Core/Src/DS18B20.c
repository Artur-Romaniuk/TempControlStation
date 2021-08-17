#include "DS18B20.h"

/************************************** Static functions **************************************/
static void Set_Baudrate(DS18B20_HandleTypeDef *ds18b20, uint32_t baudrate);
static void Write_Byte(DS18B20_HandleTypeDef *ds18b20, uint8_t byte);
static uint8_t Read_Byte(DS18B20_HandleTypeDef *ds18b20);
static void Write_Bit(DS18B20_HandleTypeDef *ds18b20, int value);
static int Read_Bit(DS18B20_HandleTypeDef *ds18b20);

/************************************** Function definitions **************************************/

/**
 * @brief Function used to create an instance of DS18B20 sensor.
 * 
 * @param uart 
 * @return DS18B20_HandleTypeDef 
 */
DS18B20_HandleTypeDef DS18B20_Create(UART_HandleTypeDef *uart)
{
	DS18B20_HandleTypeDef ds18b20;
	ds18b20.huart = uart;
	return ds18b20;
}

/**
 * @brief Function used to initialize DS18B20 sensor.
 * 
 * @param ds18b20 
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef DS18B20_Initialize(DS18B20_HandleTypeDef *ds18b20)
{
	uint8_t data_out = 0xF0;
	uint8_t data_in = 0;

	Set_Baudrate(ds18b20, 9600);
	HAL_UART_Transmit(&huart3, &data_out, 1, HAL_MAX_DELAY);
	HAL_UART_Receive(&huart3, &data_in, 1, HAL_MAX_DELAY);
	Set_Baudrate(ds18b20,115200);

	if (data_in != 0xF0)
		return HAL_OK;
	else
		return HAL_ERROR;
}

/**
 * @brief Function used to send signal to start temperature conversion.
 * 
 * @param ds18b20 
 */
void DS18B20_Start_Conversion(DS18B20_HandleTypeDef *ds18b20)
{
	Write_Byte(ds18b20, DS18B20_SKIP_ROM);	//Single device connected
	Write_Byte(ds18b20, DS18B20_CONVERT_T); //Start temperature conversion
}

/**
 * @brief Function used to read temperature from DS18B20 sensor.
 * For celsius value has to be divided by 16. For fahrenheit it has to be multiplied by 0.1125 and increased by 32. 
 * 
 * @param ds18b20 
 * @return uint16_t 
 */
uint16_t DS18B20_Read_Temperature(DS18B20_HandleTypeDef *ds18b20)
{
	Write_Byte(ds18b20, DS18B20_SKIP_ROM);		  //Single device connected
	Write_Byte(ds18b20, DS18B20_READ_SCRATCHPAD); //Get temperature from the device
	uint8_t tmp1 = Read_Byte(ds18b20);
	uint8_t tmp2 = Read_Byte(ds18b20);
	uint16_t temperature = tmp1 | (tmp2 << 8);
	return temperature; //return float
}

/************************************** Static functions **************************************/

/**
 * @brief Function used to set baudrate on UART line.
 * 
 * @param baudrate 
 */
static void Set_Baudrate(DS18B20_HandleTypeDef *ds18b20, uint32_t baudrate)
{
	ds18b20->huart->Instance = USART3;
	ds18b20->huart->Init.BaudRate = baudrate;
	ds18b20->huart->Init.WordLength = UART_WORDLENGTH_8B;
	ds18b20->huart->Init.StopBits = UART_STOPBITS_1;
	ds18b20->huart->Init.Parity = UART_PARITY_NONE;
	ds18b20->huart->Init.Mode = UART_MODE_TX_RX;
	ds18b20->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	ds18b20->huart->Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_HalfDuplex_Init(ds18b20->huart) != HAL_OK)
	{
		Error_Handler();
	}
}
/**
 * @brief Send byte to DS18B20.
 * 
 * @param ds18b20 
 * @param byte 
 */
static void Write_Byte(DS18B20_HandleTypeDef *ds18b20, uint8_t byte)
{
	for (int i = 0; i < 8; i++)
	{
		Write_Bit(ds18b20, byte & 0x01);
		byte >>= 1;
	}
}

/**
 * @brief Read byte from DS18B20.
 * 
 * @param ds18b20 
 * @return uint8_t 
 */
static uint8_t Read_Byte(DS18B20_HandleTypeDef *ds18b20)
{
	uint8_t value = 0;
	for (int i = 0; i < 8; i++)
	{
		value >>= 1;
		if (Read_Bit(ds18b20))
			value |= 0x80;
	}
	return value;
}

/**
 * @brief Write bit to DS18B20 using UART single wire half-duplex.
 * 
 * @param ds18b20 
 * @param value 
 */
static void Write_Bit(DS18B20_HandleTypeDef *ds18b20, int value)
{
	if (value)
	{
		uint8_t data_out = 0xff;
		HAL_UART_Transmit(ds18b20->huart, &data_out, 1, HAL_MAX_DELAY);
	}
	else
	{
		uint8_t data_out = 0x0;
		HAL_UART_Transmit(ds18b20->huart, &data_out, 1, HAL_MAX_DELAY);
	}
}

/**
 * @brief Read bit from DS18B20 using UART single wire half-duplex.
 * 
 * @param ds18b20 
 * @param value 
 */
static int Read_Bit(DS18B20_HandleTypeDef *ds18b20)
{
	uint8_t data_out = 0xFF;
	uint8_t data_in = 0;
	HAL_UART_Transmit(ds18b20->huart, &data_out, 1, HAL_MAX_DELAY);
	HAL_UART_Receive(ds18b20->huart, &data_in, 1, HAL_MAX_DELAY);

	return data_in & 0x01;
}