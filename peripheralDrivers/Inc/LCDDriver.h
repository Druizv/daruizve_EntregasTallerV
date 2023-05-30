#ifndef LCDDRIVER_H_
#define LCDDRIVER_H_

#include <stm32f4xx.h>
#include "I2CDriver.h"

#define I2C_WRITE_DATA						0
#define I2C_READ_DATA						1

#define MAIN_CLOCK_4_MHz_FOR_I2C			4
#define MAIN_CLOCK_16_MHz_FOR_I2C			16
#define MAIN_CLOCK_20_MHz_FOR_I2C			20
#define	MAIN_CLOCK_40_MHz_FOR_I2C			40

#define I2C_MODE_SM			0
#define I2C_MODE_FM			1

//Clock 16MHz
#define I2C_MODE_SM_SPEED_100KHz	80
#define I2C_MODE_FM_SPEED_400KHz	14

//Clock 80 MHz
#define I2C_MODE_SM_SPEED_100KHz_80MHz	200
#define I2C_MODE_FM_SPEED_400KHz_80MHz	33

#define I2C_MAX_RISE_TIME_SM				17
#define I2C_MAX_RISE_TIME_FM				6

void LCD_Init(I2C_Handler_t *ptrHandlerI2C);
void LCD_sendCMD (I2C_Handler_t *ptrHandlerI2C, char cmd);
void LCD_sendata (I2C_Handler_t *ptrHandlerI2C, char data);
void LCD_sendSTR(I2C_Handler_t *ptrHandlerI2C, char *str);
void LCD_Clear (I2C_Handler_t *ptrHandlerI2C);
void LCD_setCursor (I2C_Handler_t *ptrHandlerI2C, uint8_t x, uint8_t y);
void delay_50 (void);
void delay_5 (void);
void delay_1 (void);
void delay_10 (void);
void delay_X (uint16_t num);
void LCD_ClearScreen(I2C_Handler_t *ptrHandlerI2C, uint8_t);
void LCD_writeData(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite);

#define LCD_ADDRESS		0x25
#endif /* LCDDriver_H_ */
