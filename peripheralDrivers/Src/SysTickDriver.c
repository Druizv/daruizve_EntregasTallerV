/*
 * SysTickDriver.c
 *
 *  Created on: May 22, 2023
 *      Author: aristizabal
 */



#include <stm32f4xx.h>
#include "SysTickDriver.h"

uint64_t ticks = 0;
uint64_t ticks_start = 0;
uint64_t ticks_counting = 0;

void config_SysTick_ms(uint8_t systemClock){
	//Reiniciamos el valor de la variabl que cuenta el tiempo
	ticks = 0;

	//cargando el valor del limite de incrementos que representan 1 ms

	switch (systemClock) {
		case 0:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
			break;
		case 1:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
			break;
		case 2:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_100MHz_1ms;
			break;
		default:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
			break;
	}

	//Limpiamos el valor actual del SysTick
	SysTick->VAL = 0;
	//Configuramos el reloj interno como el resultado para el timer
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	//Desactivamos las interrupciones
	__disable_irq();

	//Matriculamos la interupcion en el nvic
	NVIC_EnableIRQ(SysTick_IRQn);

	//Activamos la interrupcion debida al conteo del SysTick
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

	//Activamos el timer
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	//Activamos de nuevo las interrupciones globales
	__enable_irq();

}

uint64_t getTicks_ms(void){
	return ticks;
}

void delay_ms(uint32_t wait_time_ms){

	//Captura el primer valor de tiempo para comparar
	ticks_start = getTicks_ms();

	//Captura el segundo valor de tiempo para comparar
	ticks_counting = getTicks_ms();


	//Compara si el calor de "countig" es mayor que el valor de "start + wait"
	//Actualiza el valor "counting"
	//Repite esta operacion  hasta que counting  sea mayor (se cumple el tiempo de espera)
	while(ticks_counting <(ticks_start + (uint64_t)wait_time_ms)){
		//Actualizar el valor
		ticks_counting = getTicks_ms();
	}
}


void SysTick_Handler(void){
	//Verificamos que la interrupcion se lanzo
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){

		//Limpiamos la bandera
		SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;


		// Incrementar el contador
		ticks++;


	}
}

