#ifndef __LPS25HB_H__
#define __LPS25HB_H__

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "i2c.h"

#define LPS25HB_ADDR 0xBA

/****** Registers ******/
#define LPS25HB_WHO_AM_I 0x0F
#define LPS25HB_CTRL_REG1 0x20
#define LPS25HB_CTRL_REG2 0x21
#define LPS25HB_CTRL_REG3 0x22
#define LPS25HB_CTRL_REG4 0x23
#define LPS25HB_PRESS_OUT_XL 0x28
#define LPS25HB_PRESS_OUT_L 0x29
#define LPS25HB_PRESS_OUT_H 0x2A
#define LPS25HB_TEMP_OUT_L 0x2B
#define LPS25HB_TEMP_OUT_H 0x2C
#define LPS25HB_RPDS_L 0x39
#define LPS25HB_RPDS_H 0x3A

typedef enum
{
    ONE_SHOT,
    _1_HZ,
    _7_HZ,
    _12_5_HZ,
    _25_HZ
} LPS25HB_FrequencyTypeDef;

typedef struct
{
    I2C_HandleTypeDef *i2c;
    uint8_t slave_address;
} LPS25HB_HandleTypeDef;

LPS25HB_HandleTypeDef LPS25HB_Create(I2C_HandleTypeDef *i2c, uint8_t slave_adress);
void LPS25HB_Set_Frequency(LPS25HB_HandleTypeDef *lps25hb, LPS25HB_FrequencyTypeDef freq);
float LPS25HB_Read_Temperature(LPS25HB_HandleTypeDef *lps25hb);
float LPS25HB_Read_Pressure(LPS25HB_HandleTypeDef *lps25hb);
void LPS25HB_Read_Pressure_DMA(LPS25HB_HandleTypeDef *lps25hb, int32_t *pressure);

#endif