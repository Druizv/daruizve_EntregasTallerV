/****
 * Nombre		: pll_driver.c                                      *
 * author		: Daniel Ruiz                                    *                                                     *
 * Asignatura 	: Taller V
 *
 * Configuracion  a 80MHz                                                        *
 *
 */

#include "pll_driver.h"

void RCC_config(void){
	// desactivamos las interrupciones globales
	__disable_irq();

	/**/
	FLASH->ACR &= ~FLASH_ACR_LATENCY;
	FLASH->ACR |=  FLASH_ACR_LATENCY_3WS;

	/* cargamos el valor de 8 en el PLLM*/
	RCC -> PLLCFGR &= ~RCC_PLLCFGR_PLLM;
	RCC -> PLLCFGR |= RCC_PLLCFGR_PLLM_2;


	/* cargamos el valor de 100 en el PLN*/
	RCC -> PLLCFGR &= ~RCC_PLLCFGR_PLLN;
	RCC -> PLLCFGR |= RCC_PLLCFGR_PLLN_4;
	RCC -> PLLCFGR |= RCC_PLLCFGR_PLLN_6;//6400 y necesito 5120

	/* Ponemos el registro PLLQ en 4 */
	RCC -> PLLCFGR &= ~RCC_PLLCFGR_PLLQ;
	RCC -> PLLCFGR |= RCC_PLLCFGR_PLLQ_2;

	/* Se carga 2 en el PLLP */
	RCC -> PLLCFGR &= ~RCC_PLLCFGR_PLLP;

	/* Se activa el PLL */
	RCC -> CR |= RCC_CR_PLLON;


	/* Le indicamos al sistema que use el clock pll como systemclock*/
	RCC -> CFGR &= ~RCC_CFGR_SW;
	RCC -> CFGR |= RCC_CFGR_SW_PLL;

	/*RCC -> CFGR &= ~RCC_CFGR_SWS;
	RCC -> CFGR |= RCC_CFGR_SWS_PLL;*/

	/* el prescaler del AHB en division por 1 */
	RCC -> CFGR &= ~RCC_CFGR_HPRE;
	RCC -> CFGR |= RCC_CFGR_HPRE_DIV1;

	/* el preescaler de APB1  */
	RCC -> CFGR &= ~RCC_CFGR_PPRE1;
	RCC -> CFGR |= RCC_CFGR_PPRE1_DIV2;

	/* el preescaler de APB1 en division por 1, queda en 80MHz */
	RCC -> CFGR &= ~RCC_CFGR_PPRE2;
	RCC -> CFGR |= RCC_CFGR_PPRE2_DIV1;

	/* Se configura el MCO1 para tener la salida del clock por este*/
	RCC -> CFGR &= ~RCC_CFGR_MCO1;
	RCC -> CFGR |= RCC_CFGR_MCO1;

	/*division por 5 para tener a la salida 20MHz */
	RCC -> CFGR &= ~RCC_CFGR_MCO1PRE;
	RCC -> CFGR |= RCC_CFGR_MCO1PRE;

	__enable_irq();
}
