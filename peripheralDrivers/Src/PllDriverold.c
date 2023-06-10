/*
 * PllDriver.c
 *
 *  Created on: May 23, 2023
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
	// para poner el clock del micro a 100MHz
		else if(ptrHandlerPLL->FrecuenciaClock == FREQUENCY_100MHz){

			//primero configuramos el power control register para una frecuencia de PLL <= 100MHz
			// activamos el reloj
			RCC->APB1ENR |= RCC_APB1ENR_PWREN;
			// ponemos los bits de Regulator voltage scaling (VOS), debe ser 11 para <= 100MHz
			PWR->CR |= PWR_CR_VOS_0;
			PWR->CR |= PWR_CR_VOS_1;

			//Luego se modifica la latencia de memoria EN 2ws para la frecuencia adecuada
			FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN;
			FLASH->ACR &= ~FLASH_ACR_LATENCY;  //se limpian los registros
			FLASH->ACR |= FLASH_ACR_LATENCY_3WS; // 3WS

			//sabemos que la velocidad por defecto es 16MHz, por lo tanto debemos usar PLLN, PLLM y PLLP para obtener 100MHz
			// F(PLL general clock output) = 16MHz(PLLN/(PLLM*PLLP))

			// El PLLM es el Factor de división para el reloj de entrada principal del PLL, el resultado será 2MHz
			RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;  // se limpia el registro
			RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_3; // Ponemos un 8 en el PLLM

			// El PLLN es el Factor de multiplicación principal del PLL. el valor debe estar entre 100 y 432MHz
			RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;  // se limpia el registro
			RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_2;  // Se pone en 100 el registro
			RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_5;
			RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_6;
			//El resultado será 200MHz

			// PLLP es el factor de division principal, el resultado no debe exceder los 100 MHz en este dominio.
			// el PLLP se configura en 2 para que el resultado sea 100MHz
			RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP; // 2 en PLLP


			// encendemos el PLL
			RCC->CR |= RCC_CR_PLLON;

			// Esperamos que el PLL se estabilice
			while(!(RCC->CR & RCC_CR_PLLRDY)){
				__NOP();
			}

			// Activando el PLL en el multiplexor SW, para que el clock del micro sea el PLL
			RCC->CFGR &= ~RCC_CFGR_SW; // limpiamos el registro
			RCC->CFGR |= RCC_CFGR_SW_1; // ponemos un 1 en el bit 1 para quedar con 10 (MODE PLL) en el SW

			/* el prescaler del AHB en division por 1 */
			RCC -> CFGR &= ~RCC_CFGR_HPRE;
			RCC -> CFGR |= RCC_CFGR_HPRE_DIV1;

			/* el preescaler de APB1 en division por 2 para los perifericos que trabajan maximo en 50MHz */
			RCC -> CFGR &= ~RCC_CFGR_PPRE1;
			RCC -> CFGR |= RCC_CFGR_PPRE1_DIV2;

			/* el preescaler de APB2 en division por 1, queda en 100MHz */
			RCC -> CFGR &= ~RCC_CFGR_PPRE2;
			RCC -> CFGR |= RCC_CFGR_PPRE2_DIV1;

		}
		else{
			// se activa el HSI, el cual es la velocidad por defecto del clock
			RCC->CFGR &= ~RCC_CFGR_SW; // ponemos el SW en HSI (los bits en 0 ambos)
		}
	// Los perifericos timer quedan con una velocidad de 50MHz por estar en el APB1
	// Con la siguiente función les multiplicamos su reloj x2 y quedan a 100MHz sus clock
void configTimers (void){

    TIM2->CR1 &= ~TIM_CR1_CKD;
    TIM2->CR1 |=  TIM_CR1_CKD_0;

    TIM3->CR1 &= ~TIM_CR1_CKD;
    TIM3->CR1 |=  TIM_CR1_CKD_0;

    TIM4->CR1 &= ~TIM_CR1_CKD;
    TIM4->CR1 |=  TIM_CR1_CKD_0;

    TIM5->CR1 &= ~TIM_CR1_CKD;
    TIM5->CR1 |=  TIM_CR1_CKD_0;

	}



void getConfigPLL(PLL_Handler_t *ptrPLL_Handler){


}


