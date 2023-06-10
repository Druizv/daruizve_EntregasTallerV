/*
 * PllDriver.h
 *
 *  Created on: May 23, 2023
 *      Author: druiz
 */

#ifndef PLLDRIVER_H_
#define PLLDRIVER_H_

#include <stm32f4xx.h>


#define FREQUENCY_16MHz		0
#define FREQUENCY_80MHz		1
#define FREQUENCY_100MHz	2

#define HSI					0
#define LSE					1
#define PLL					2

#define presc1				3
#define presc2				4
#define presc3				5
#define presc4				6
#define presc5				7



typedef struct{
	uint8_t		clkSpeed;

}PLL_Handler_t;

typedef struct{

	uint8_t  clk; //configurar el tipo de reloj
	uint8_t  presc; //configurar el tipo de prescaler
}pll_MCO1;




void configPLL(PLL_Handler_t *ptrPLL_Handler);

void getConfigPLL(PLL_Handler_t *ptrPLL_Handler);

void MCOConfig (pll_MCO1 *ptrPLL_Handler);

#endif /* PLLDRIVER_H_ */
