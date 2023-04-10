/*
 * ExtiConfig_main.c
 *
 *  Created on: Mar 24, 2023
 *      Author: druiz
 */

#include "stdint.h"
#include "stm32f4xx.h"
#include "GPIOxDriver.h"

GPIO_Handler_t handlerUserLedPin2 = {0};
GPIO_Handler_t handlerBlinkyTimer = {0};

void int_Hardware(void){

	handlerUserLedPin2.pGPIOx 								= GPIOB;
	handlerUserLedPin2.GPIO_PinConfig.GPIO_PinNumber		= PIN_1;
	handlerUserLedPin2.GPIO_PinConfig.GPIO_PinMode			= GPIOx_MODE_OUT;
	handlerUserLedPin2.GPIO_PinConfig.GPIO_PinOType			= GPIOx_OTYPE_PUSHPULL;
	handlerUserLedPin2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIOx_PUPDR_PULLUP;
	handlerUserLedPin2.GPIO_PinConfig.GPIO_PinSpeed			= GPAIOx_OSPEEDR_MEDIUM;
	handlerUserLedPin2.GPIO_PinConfig.GPIO_PinAltFunMode	= AF0;	// Ninguna funcion

	handlerBlinkyTimer.ptrTIMx = TIM2;
	handlerBlinkyTimer.TIMxConfig.TIMx_mode                 = BTIMER_MODE_UP;
	handlerBlinkyTimer.

}
