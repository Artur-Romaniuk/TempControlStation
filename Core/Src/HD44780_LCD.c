#include "HD44780_LCD.h"
const uint8_t ROW_16[] = {0x00, 0x40, 0x10, 0x50};
/************************************** Static declarations **************************************/

static void Lcd_Write_Data(Lcd_HandleTypeDef *lcd, uint8_t data);
static void Lcd_Write_Command(Lcd_HandleTypeDef *lcd, uint8_t command);
static void Lcd_Write(Lcd_HandleTypeDef *lcd, uint8_t data, uint8_t len);

/************************************** Function definitions **************************************/

/**
 * Create new Lcd_HandleTypeDef and initialize the Lcd
 */
Lcd_HandleTypeDef Lcd_Create(
	Lcd_PortType port[], Lcd_PinType pin[],
	Lcd_PortType rs_port, Lcd_PinType rs_pin,
	Lcd_PortType en_port, Lcd_PinType en_pin, Lcd_ModeTypeDef mode)
{
	Lcd_HandleTypeDef lcd;

	lcd.mode = mode;

	lcd.en_pin = en_pin;
	lcd.en_port = en_port;

	lcd.rs_pin = rs_pin;
	lcd.rs_port = rs_port;

	lcd.data_pin = pin;
	lcd.data_port = port;

	Lcd_Init(&lcd);

	return lcd;
}

/**
 * Initialize 16x2-lcd without cursor
 */
void Lcd_Init(Lcd_HandleTypeDef *lcd)
{
	if (lcd->mode == LCD_4_BIT_MODE)
	{
		Lcd_Write_Command(lcd, 0x33);
		Lcd_Write_Command(lcd, 0x32);
		Lcd_Write_Command(lcd, FUNCTION_SET | OPT_N); // 4-bit mode
	}
	else
		Lcd_Write_Command(lcd, FUNCTION_SET | OPT_DL | OPT_N);

	Lcd_Write_Command(lcd, CLEAR_DISPLAY);					// Clear screen
	Lcd_Write_Command(lcd, DISPLAY_ON_OFF_CONTROL | OPT_D); // Lcd-on, cursor-off, no-blink
	Lcd_Write_Command(lcd, ENTRY_MODE_SET | OPT_INC);		// Increment cursor
}

/**
 * Write a number on the current position
 */
void Lcd_Int(Lcd_HandleTypeDef *lcd, int number)
{
	char buffer[11];
	sprintf(buffer, "%d", number);

	Lcd_String(lcd, buffer);
}

void Lcd_Float(Lcd_HandleTypeDef *lcd, float number)
{
	char buffer[11];
	int tmpInt = number;
	float tmpFloat = number - tmpInt;
	int tmpIntR = trunc(tmpFloat * 1000);
	sprintf(buffer, "%d.%04d", tmpInt, tmpIntR);
	Lcd_String(lcd, buffer);
}
/**
 * Write a string on the current position
 */
void Lcd_String(Lcd_HandleTypeDef *lcd, char *string)
{
	for (uint8_t i = 0; i < strlen(string); i++)
	{
		Lcd_Write_Data(lcd, string[i]);
	}
}

/**
 * Set the cursor position
 */
void Lcd_Cursor(Lcd_HandleTypeDef *lcd, uint8_t row, uint8_t col)
{
	Lcd_Write_Command(lcd, SET_DDRAM_ADDR + ROW_16[row] + col);
}

/**
 * Clear the screen
 */
void Lcd_Clear(Lcd_HandleTypeDef *lcd)
{
	Lcd_Write_Command(lcd, CLEAR_DISPLAY);
}

void Lcd_Define_Char(Lcd_HandleTypeDef *lcd, uint8_t code, uint8_t bitmap[])
{
	Lcd_Write_Command(lcd, SETCGRAM_ADDR + (code << 3));
	for (uint8_t i = 0; i < 8; ++i)
	{
		Lcd_Write_Data(lcd, bitmap[i]);
	}
}

/************************************** Static function definition **************************************/

/**
 * Write a byte to the command register
 */
void Lcd_Write_Command(Lcd_HandleTypeDef *lcd, uint8_t command)
{
	HAL_GPIO_WritePin(lcd->rs_port, lcd->rs_pin, LCD_COMMAND_REG); // Write to command register

	if (lcd->mode == LCD_4_BIT_MODE)
	{
		Lcd_Write(lcd, (command >> 4), LCD_NIB);
		Lcd_Write(lcd, command & 0x0F, LCD_NIB);
	}
	else
	{
		Lcd_Write(lcd, command, LCD_BYTE);
	}
}

/**
 * Write a byte to the data register
 */
void Lcd_Write_Data(Lcd_HandleTypeDef *lcd, uint8_t data)
{
	HAL_GPIO_WritePin(lcd->rs_port, lcd->rs_pin, LCD_DATA_REG); // Write to data register

	if (lcd->mode == LCD_4_BIT_MODE)
	{
		Lcd_Write(lcd, data >> 4, LCD_NIB);
		Lcd_Write(lcd, data & 0x0F, LCD_NIB);
	}
	else
	{
		Lcd_Write(lcd, data, LCD_BYTE);
	}
}

/**
 * Set len bits on the bus and toggle the enable line
 */
void Lcd_Write(Lcd_HandleTypeDef *lcd, uint8_t data, uint8_t len)
{
	for (uint8_t i = 0; i < len; i++)
	{
		HAL_GPIO_WritePin(lcd->data_port[i], lcd->data_pin[i], (data >> i) & 0x01);
	}

	HAL_GPIO_WritePin(lcd->en_port, lcd->en_pin, 1);
	DELAY(1);
	HAL_GPIO_WritePin(lcd->en_port, lcd->en_pin, 0); // Data receive on falling edge
}
