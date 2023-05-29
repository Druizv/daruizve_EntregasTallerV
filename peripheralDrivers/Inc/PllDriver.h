/*
 * PllDriver.h
 *
 *  Created on: May 23, 2023
 *      Author: aristizabal
 */

#ifndef PLLDRIVER_H_
#define PLLDRIVER_H_

#include <stm32f4xx.h>


#define FREQUENCY_16MHz		0
#define FREQUENCY_80MHz		1



typedef struct{
	uint8_t		clkSpeed;

}PLL_Handler_t;






void configPLL(PLL_Handler_t *ptrPLL_Handler);

void getConfigPLL(PLL_Handler_t *ptrPLL_Handler);



#endif /* PLLDRIVER_H_ */
