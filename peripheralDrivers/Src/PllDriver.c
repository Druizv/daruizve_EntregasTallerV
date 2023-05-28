/*
 * PllDriver.c

 *      Author: druiz
 */


#include "stdint.h"
#include "GPIOxDriver.h"
#include "PllDriver.h"

void configPLL(PLL_Handler_t *ptrPLL_Handler){

	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;

	//Se configura la frecuencia a 16MHz, la que es por defecto

	if(ptrPLL_Handler->clkSpeed == FREQUENCY_16MHz){

		RCC->CFGR &= ~RCC_CFGR_SW;
	}

	//Configuramos 80MHz usando el PLL como entrada para el reloj HSI

	else if(ptrPLL_Handler->clkSpeed == FREQUENCY_80MHz){

		//Activamos la señal de reloj del periferico

		RCC->AHB1ENR |= RCC_APB1ENR_PWREN;


		/* Debemos cambiar el voltaje ya que tenemos una velocidad diferente
		 * PWR->CR
		 * Para 84 Mhz se configura en el valor de escala 2 referido en el manual
		 * Escribimos los bits 10 en el regulador de voltaje
		 */

		PWR->CR &= ~PWR_CR_VOS_0;
		PWR->CR |= PWR_CR_VOS_1;



		/*
		 * Programamos la latencia en el registro FLASH->ACR
		 * para estar acorde a la velocidad del procesador
		 * Programamos a 2WS de acuerdo al manual 64 =< HCLK =< 90
		 *
		 * Es necesario habilitar la cache de diferentes registros:
		 * PRFTEN, ICEN, DCEN
		 */

		FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN;
		FLASH->ACR |= FLASH_ACR_LATENCY_2WS;



		/*
		 * Debemos configurar el RCC_PLLCFGR
		 *
		 * Foc=16(PLLN / PLLM), tenemos PLLN=50 y PLLM = 5  y PLLP= 2
		 *
		 * F_PLL_OUT = 160MHz
		 *
		 */

		//Primero limpiamos
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;
		// 000110010: PLLN = 50
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_1 |RCC_PLLCFGR_PLLN_4 |RCC_PLLCFGR_PLLN_5;
		// 000101: PLLM = 5
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_0 | RCC_PLLCFGR_PLLM_2;
		// 00: PLLP = 2
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;



		//Configurando el System Clock Switch para usar el clock con el PLL
		RCC->CFGR &= ~RCC_CFGR_SW;
		RCC->CFGR |= RCC_CFGR_SW_1;


		/*
		 * Configuramos el prescaler de ser necesario
		 * En este caso ya esta listo entonces le damos valor de 1
		 */


		RCC->CFGR &= ~RCC_CFGR_HPRE;
		RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

		//Los perifericos que estan conectados al APB1 tienen una velocidad maxima de 50MHz
		//El APB2 tiene una velocidad maxima de 100MHz

		//Dividimos los 80MHz /2 para tener 40MHz en el APB1
		RCC->CFGR &= ~RCC_CFGR_PPRE1;
		RCC->CFGR |= RCC_CFGR_PPRE1_2;

		//El APB2 soporta los 80MHz
		RCC->CFGR &= ~RCC_CFGR_PPRE2;





		/*
		 * PARA PROBAR EN EL OSCILOSCOPIO/ANALIZADOR DE SEÑALES
		 * Manipulamos el registro MCO1: Microcontroller clock output 1
		 */

		// 11: PLL clock seleccionado
		RCC->CFGR &= ~RCC_CFGR_MCO1;
		RCC->CFGR |= RCC_CFGR_MCO1_0;
		RCC->CFGR |= RCC_CFGR_MCO1_1;


		// En el registro MCO1PRE esta el prescaler del MCO1
		RCC->CFGR &= ~RCC_CFGR_MCO1PRE;
		// 111: para dividir por 5
		RCC->CFGR |= RCC_CFGR_MCO1PRE;



		//Encendemos el PLL

		RCC->CR |= RCC_CR_PLLON;

		//Esperamos que el clk se estabilice

		while(!(RCC->CR & RCC_CR_PLLRDY)){
			__NOP();
		}








	}

}

void getConfigPLL(PLL_Handler_t *ptrPLL_Handler){

}
