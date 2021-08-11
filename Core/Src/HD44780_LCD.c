#include "HD44780_LCD.h"
const uint8_t ROW_16[] = {0x00, 0x40, 0x10, 0x50};
/************************************** Static declarations **************************************/

static void Lcd_Write_Data(Lcd_HandleTypeDef *lcd, char data);
static void Lcd_Write_Command(Lcd_HandleTypeDef *lcd, char command);

/************************************** Function definitions **************************************/

/**
 * Create new Lcd_HandleTypeDef and initialize the Lcd
 */
Lcd_HandleTypeDef Lcd_Create(I2C_HandleTypeDef *hi2c)
{
	Lcd_HandleTypeDef lcd;

	lcd.i2c = hi2c;

	Lcd_Init(&lcd);

	return lcd;
}

/**
 * Initialize 16x2-lcd without cursor
 */
void Lcd_Init(Lcd_HandleTypeDef *lcd)
{
	// 4 bit initialisation
	HAL_Delay(50); // wait for >40ms
	Lcd_Write_Command(lcd, 0x30);
	HAL_Delay(5); // wait for >4.1ms
	Lcd_Write_Command(lcd, 0x30);
	HAL_Delay(1); // wait for >100us
	Lcd_Write_Command(lcd, 0x30);
	HAL_Delay(10);
	Lcd_Write_Command(lcd, FUNCTION_SET); // 4bit mode
	HAL_Delay(10);

	// display initialisation
	Lcd_Write_Command(lcd, FUNCTION_SET | OPT_N); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	Lcd_Write_Command(lcd, DISPLAY_ON_OFF_CONTROL); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	Lcd_Write_Command(lcd, CLEAR_DISPLAY); // clear display
	HAL_Delay(1);
	Lcd_Write_Command(lcd, OPT_D | OPT_C); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	Lcd_Write_Command(lcd, DISPLAY_ON_OFF_CONTROL | OPT_D); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
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

void Lcd_Hex(Lcd_HandleTypeDef *lcd, uint8_t code)
{
	Lcd_Write_Data(lcd, code);
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
	//Lcd_Write_Command(lcd, CLEAR_DISPLAY);
	Lcd_Write_Command(lcd, 0x80);
	for (int i = 0; i < 70; i++)
	{
		Lcd_Write_Data(lcd, ' ');
	}
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
void Lcd_Write_Command(Lcd_HandleTypeDef *lcd, char command)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (command & 0xf0);
	data_l = ((command << 4) & 0xf0);
	data_t[0] = data_u | 0x0C; //en=1, rs=0
	data_t[1] = data_u | 0x08; //en=0, rs=0
	data_t[2] = data_l | 0x0C; //en=1, rs=0
	data_t[3] = data_l | 0x08; //en=0, rs=0
	HAL_I2C_Master_Transmit(lcd->i2c, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 400);
}

/**
 * Write a byte to the data register
 */
void Lcd_Write_Data(Lcd_HandleTypeDef *lcd, char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D; //en=1, rs=0
	data_t[1] = data_u | 0x09; //en=0, rs=0
	data_t[2] = data_l | 0x0D; //en=1, rs=0
	data_t[3] = data_l | 0x09; //en=0, rs=0
	HAL_I2C_Master_Transmit(lcd->i2c, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 400);
}
