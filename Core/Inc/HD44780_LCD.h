#ifndef LCD_H_
#define LCD_H_

#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "main.h"

// For row start addresses
extern const uint8_t ROW_16[];

/************************************** Command register **************************************/
#define LCD_CLEAR_DISPLAY 0x01

#define LCD_RETURN_HOME 0x02

#define LCD_ENTRY_MODE_SET 0x04
#define LCD_OPT_S 0x01	 // Shift entire display to right
#define LCD_OPT_INC 0x02 // Cursor increment

#define LCD_DISPLAY_ON_OFF_CONTROL 0x08
#define LCD_OPT_D 0x04 // Turn on display
#define LCD_OPT_C 0x02 // Turn on cursor
#define LCD_OPT_B 0x01 // Turn on cursor blink

#define LCD_CURSOR_DISPLAY_SHIFT 0x10 // Move and shift cursor
#define LCD_OPT_SC 0x08
#define LCD_OPT_RL 0x04

#define LCD_FUNCTION_SET 0x20
#define LCD_OPT_DL 0x10 // Set interface data length
#define LCD_OPT_N 0x08	// Set number of display lines
#define LCD_OPT_F 0x04	// Set alternate font
#define LCD_SETCGRAM_ADDR 0x040
#define LCD_SET_DDRAM_ADDR 0x80 // Set DDRAM address

/************************************** LCD defines **************************************/
#define LCD_NIB 4
#define LCD_BYTE 8
#define LCD_DATA_REG 1
#define LCD_COMMAND_REG 0

/************************************** LCD typedefs **************************************/
typedef struct
{
	I2C_HandleTypeDef *i2c;
	uint8_t slave_address;

} Lcd_HandleTypeDef;

/************************************** Public functions **************************************/
Lcd_HandleTypeDef Lcd_Create(I2C_HandleTypeDef *i2c, uint8_t slave_address);
void Lcd_Init(Lcd_HandleTypeDef *lcd);
void Lcd_Int(Lcd_HandleTypeDef *lcd, int number);
void Lcd_Float(Lcd_HandleTypeDef *lcd, float number);
void Lcd_Float_Decimal(Lcd_HandleTypeDef *lcd, float number, int decimal_places);
void Lcd_String(Lcd_HandleTypeDef *lcd, char *string);
void Lcd_Hex(Lcd_HandleTypeDef *lcd, uint8_t code);
void Lcd_Cursor(Lcd_HandleTypeDef *lcd, uint8_t row, uint8_t col);
void Lcd_Define_Char(Lcd_HandleTypeDef *lcd, uint8_t code, uint8_t bitmap[]);
void Lcd_Clear(Lcd_HandleTypeDef *lcd);

#endif /* LCD_H_ */
