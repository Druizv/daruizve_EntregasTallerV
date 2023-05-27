/*
 * SysTickDriver.h
 *
 *  Created on: May 22, 2023
 *      Author: aristizabal
 */

#ifndef SYSTICKDRIVER_H_
#define SYSTICKDRIVER_H_

#include <stm32f4xx.h>

#define SYSTICK_LOAD_VALUE_16MHz_1ms			16000//numero de ciclos en 1ms
#define SYSTICK_LOAD_VALUE_100MHz_1ms			100000//numero de ciclos en 1ms


void config_SysTick_ms(uint8_t systemClock);
uint64_t getTicksMs(void);
void delay_ms(uint32_t wait_time_ms);



#endif /* SYSTICKDRIVER_H_ */
