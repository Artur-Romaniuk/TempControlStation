#include "LPS25HB.h"

/************************************** Static funtions **************************************/
static void Write_Register(LPS25HB_HandleTypeDef *lps25hb, uint8_t reg, uint8_t value);

/************************************** Function definitions **************************************/

/**
 * @brief Used to create instance of LPS25HB struct.
 * 
 * @param i2c 
 * @param slave_address 
 * @return LPS25HB_HandleTypeDef 
 */
LPS25HB_HandleTypeDef LPS25HB_Create(I2C_HandleTypeDef *i2c, uint8_t slave_address)
{
    LPS25HB_HandleTypeDef lps25hb;
    lps25hb.i2c = i2c;
    lps25hb.slave_address = slave_address;
    return lps25hb;
}

/**
 * @brief Sets LPS25HB peripheral into specified probing mode.
 * 
 * @param lps25hb 
 * @param freq 
 */
void LPS25HB_Set_Frequency(LPS25HB_HandleTypeDef *lps25hb, LPS25HB_FrequencyTypeDef freq)
{
    switch (freq)
    {
    case ONE_SHOT:
        Write_Register(lps25hb, LPS25HB_CTRL_REG1, 0x80);
        break;
    case _1_HZ:
        Write_Register(lps25hb, LPS25HB_CTRL_REG1, 0x90);
        break;
    case _7_HZ:
        Write_Register(lps25hb, LPS25HB_CTRL_REG1, 0xA0);
        break;
    case _12_5_HZ:
        Write_Register(lps25hb, LPS25HB_CTRL_REG1, 0xB0);
        break;
    case _25_HZ:
        Write_Register(lps25hb, LPS25HB_CTRL_REG1, 0xC0);
        break;
    }
}

/**
 * @brief Reads temperature from LPS25HB in blocking mode. Returns value in celsius.
 * 
 * @param lps25hb 
 * @return float 
 */
float LPS25HB_Read_Temperature(LPS25HB_HandleTypeDef *lps25hb)
{
    int16_t temp;
    HAL_I2C_Mem_Read(lps25hb->i2c, lps25hb->slave_address, LPS25HB_TEMP_OUT_L | 0x80, 1, (uint8_t *)&temp, sizeof(temp), HAL_MAX_DELAY);
    return (42.5f + (float)temp / 480.0f);
}

/**
 * @brief Reads pressure from LPS25HB in blocking mode. Returns value in hPa.
 * 
 * @param lps25hb 
 * @return float 
 */
float LPS25HB_Read_Pressure(LPS25HB_HandleTypeDef *lps25hb)
{
    int32_t press = 0;
    HAL_I2C_Mem_Read(lps25hb->i2c, lps25hb->slave_address, LPS25HB_PRESS_OUT_XL | 0x80, 1, (uint8_t *)&press, 3, 400); //reading 24 bits
    return (float)press / 4096;
}

/**
 * @brief Reads pressure from LPS25HB in DMA mode. Return value has te be divided by 4096 in order to achieve value in hPa.
 * 
 * @param lps25hb 
 * @param pressure 
 */
void LPS25HB_Read_Pressure_DMA(LPS25HB_HandleTypeDef *lps25hb, int32_t *pressure)
{
    HAL_I2C_Mem_Read_DMA(lps25hb->i2c, lps25hb->slave_address, LPS25HB_PRESS_OUT_XL | 0x80, 1, (uint8_t *)pressure, 3); //reading 24 bit
}

/************************************** Static funtions **************************************/

/**
 * @brief Static function used to make writing to peripheral memory easier.
 * 
 * @param lps25hb 
 * @param reg 
 * @param value 
 */
static void Write_Register(LPS25HB_HandleTypeDef *lps25hb, uint8_t reg, uint8_t value)
{
    HAL_I2C_Mem_Write(lps25hb->i2c, lps25hb->slave_address, reg, 1, &value, sizeof(value), HAL_MAX_DELAY);
}