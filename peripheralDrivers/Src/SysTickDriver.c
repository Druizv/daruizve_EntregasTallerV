/*este driver controla el timer que trae todo procesador ARM cortex Mx,
 * el cual hace parte independiente del fabricante del MCU
 *
 * Para encontrar cual es su registro de configuracion, debemos utilizar
 * el manual generico del procesador cortex-M4 ya que esta alli la documentacion
 * donde se encuentra este periferico.
 *
 * en el archivo core_cm4.h la estructura que define el periferico se llama Systick_type
 */

#include <stm32f4xx.h>
#include "SysTickDriver.h"

uint64_t ticks = 0;
uint64_t ticks_start = 0;
uint64_t ticks_counting = 0;

void config_SysTick_ms(uint8_t systemClock){
	//reiniciamos el valor de la variable que cuenta el tiempo
	ticks = 0;

	//cargando el valor del limite del incrementos que representan 1ms
	switch(systemClock){

	//caso para el reloj HSI -> 16MHz
	case 0:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
		break;

	//caso para el reloj HSE
	case 1:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
		break;

	//caso para el reloj PLL a 100MHz
	case 2:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_100MHz_1ms;
		break;

	//en caso que se ingrese un valor diferente
	default:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
		break;

	}

	//limpiamos el valor actual del SysTick
	SysTick->VAL = 0;

	//configuramos el reloj interno como reloj para el Timer
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	//Desactivamos las interrupciones Globales
	__disable_irq();

	//Matriculamos la interrupcion en el NVIC
	NVIC_EnableIRQ(SysTick_IRQn);

	//ACtivamos la interrupcion debida al conteo a cero del  SysTick
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
	//captura el primer valor de tiempo para comparar
	ticks_start = getTicks_ms();

	//captura el segundo valor de tiempo para comparar
	ticks_counting = getTicks_ms();

	//compara: si el valor del "counting es menor que el "start + wait"
	//Actualiza el valor del counting
	//Repite esta operacion hasta que el counting sea mayor (se cumple con el tiempo de espera)
	while (ticks_counting < (ticks_start + (uint64_t)wait_time_ms)){
		//actualiza el valor
		ticks_counting = getTicks_ms();
	}
}


void SysTick_Handler(void){
	//verificamos que la interrupcion se lanzó
	if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){

		//Limpiamos la bandera
		SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;

		//incrementamos en 1 el contador
		ticks++;


	}
}















