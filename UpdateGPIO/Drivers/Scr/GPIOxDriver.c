/*
 * GPIOxDriver.c
 *
 *  Created on: Mar 10, 2023
 *      Author: druiz
 *
 * Este archivo es la parte del programa donde escribimos adecuadamente el control,
 * Para que sea lo mas generico posible, de forma que independiente del puerto GPIO y
 * El PIN seleccionados,el programa se ejecute y configure todo correctamente.
 *
 */

#include "GPIOxDriver.h"

/*
 * Para cualquier periferico, hay varios pasos que siempre se deben seguir en un
 * orde estricto para poder que el sistema permita configurar el periferico X.
 * lo primero y mas importante es activar la señal de reloj principal hacia ese
 * elemento especifico (relacionado con el periférico RCC), a esto lo llamaremos
 * simplemente "activar el periferico o activar la señal de reloj del periferico).
 */
void GPIO_Config (GPIO_Handler_t *pGPIOHandler){

	// Variable para hacer todo paso a paso
	uint32_t auxConfig = 0;
	uint32_t auxPosition = 0;

	// 1) activar el periferico
	// Verificamos para GPIOA
	if (pGPIOHandler ->pGPIOx == GPIOA){
		// Escribimos 1 (SET) en la posicion correspondiente al GPIOA
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);
	}
	// Verificamos para GPIOB
	else if (pGPIOHandler->pGPIOx == GPIOB){
		// Escribimos 1 (SET) en la posicion correspondiente al GPIOB
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN);
	}
	// Verificamos para GPIOC
	else if (pGPIOHandler->pGPIOx == GPIOC){
		// Escribimos 1 (SET) en la posicion correspondiente al GPIOC
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN);
	}
	// Verificamos para GPIOD
	else if (pGPIOHandler->pGPIOx == GPIOD){
		// Escribimos 1 (SET) en la posicion correspondiente al GPIOD
		RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN);
	}
	// Verificamos para GPIOE
	else if (pGPIOHandler->pGPIOx == GPIOE){
	// Escribimos 1 (SET) en la posicion correspondiente al GPIOE
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN);
	}
	// Verificamos para GPIOH
	else if (pGPIOHandler->pGPIOx == GPIOH){
	// Escribimos 1 (SET) en la posicion correspondiente al GPIOH
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN);
	}

	// Despues de activado ,podemos comenzar a configurar

	//2) configurarmos el registro GPIOx_MODER
	// Aca estamos leyendo la config, moviendo "PinNumber" veces hacia la izquierda ese valor (shift left)
	// y todo eso lo cargamos en la variable auxConfig
	auxConfig = (pGPIOHandler-> GPIO_PinConfig.GPIO_PinMode << 2 * pGPIOHandler -> GPIO_PinConfig.GPIO_PinNumber);

	// Antes de cargar el nuevo valor limpiamos los bits especificos de ese registro (debemos escribir 0b00)
	// para lo cual aplicamos una mascara y una operacion bitwise AND
	pGPIOHandler -> pGPIOx -> MODER &= ~(0b11 << 2 * pGPIOHandler -> GPIO_PinConfig.GPIO_PinNumber);

	// Cargamos la auxConfig el el registro MODER
	pGPIOHandler -> pGPIOx -> MODER |= auxConfig;

	// 3) Configurando el registro GPIOx_OTYPER
	// de nuevo, leemos y movemos el valor de un numero "PinNumber" de veces
	auxConfig = (pGPIOHandler -> GPIO_PinConfig.GPIO_PinOType << pGPIOHandler ->GPIO_PinConfig.GPIO_PinNumber);

	//Limpiamos antes de cargar
	pGPIOHandler ->pGPIOx -> OTYPER &= ~(SET << pGPIOHandler -> GPIO_PinConfig.GPIO_PinNumber);

	//cargamos el resultado sobre el registro adecuado
	pGPIOHandler -> pGPIOx -> OTYPER |= auxConfig;

	// 4) Configurando ahora la velocidad
	auxConfig = ( pGPIOHandler -> GPIO_PinConfig.GPIO_PinSpeed << 2*pGPIOHandler ->GPIO_PinConfig.GPIO_PinNumber);

	// limpiamos la posicion antes de cargar la nueva configuracion
	pGPIOHandler -> pGPIOx -> OSPEEDR &= ~(0b11 << 2 * pGPIOHandler ->GPIO_PinConfig.GPIO_PinNumber);

	// Cargamos el resultado sobre el registro adecuado
	pGPIOHandler -> pGPIOx -> OSPEEDR |= auxConfig;

	// 5) Configurando si se desea pull-up, pull-down o fotante.
	auxConfig = (pGPIOHandler -> GPIO_PinConfig.GPIO_PinPuPdControl << 2*pGPIOHandler ->GPIO_PinConfig.GPIO_PinNumber);

	// Limpiando la posicion antes de cargar la nueva configuracion
	pGPIOHandler -> pGPIOx -> PUPDR &= ~(0b11 << 2 * pGPIOHandler ->GPIO_PinConfig.GPIO_PinNumber);

	// Cargamos el resultado sobre el registro adecuado
	pGPIOHandler -> pGPIOx -> PUPDR |= auxConfig;

	// Esta es la  parte para las configuraciones de las funciones alternativas... Se verá luego
	if(pGPIOHandler -> GPIO_PinConfig.GPIO_PinMode == GPIOx_MODE_ALTFN){

		// Seleccionamos primero si se debe utilizar el registro bajo (AFRL) o el alto (AFRH)
		if(pGPIOHandler -> GPIO_PinConfig.GPIO_PinNumber < 8 ){
			// Estamos en el registro AFRL, que controla los pines del PIN_0 al PIN_7
			auxPosition = 4 * pGPIOHandler -> GPIO_PinConfig.GPIO_PinNumber;

			// Limpiamos primero la posicion del registro que deseamos escribir a continuacion
			pGPIOHandler -> pGPIOx -> AFR[0] &= ~(0b111 << auxPosition);

			// Y escribimos el valor configurado en la posicion seleccionada
			pGPIOHandler -> pGPIOx -> AFR[0] |= ( pGPIOHandler -> GPIO_PinConfig.GPIO_PinAltFunMode << auxPosition);
		}
		else{
			// Estamos en el registro AFRH, que controla los pines del PIN_8 al PIN_15
			auxPosition = 4 * (pGPIOHandler ->GPIO_PinConfig.GPIO_PinNumber -8);

			// Limpiamos primero la posicion del registro que deseamos escribir a continuacion
			pGPIOHandler ->pGPIOx ->AFR[1] &= ~(0b111 << auxPosition);

			// Y escribimos el valor configurado en la posicion seleccionada
			pGPIOHandler ->pGPIOx ->AFR[1] |= (pGPIOHandler ->GPIO_PinConfig.GPIO_PinAltFunMode << auxPosition);

		}
	}
}  // Fin del GPIO_config

/**
 *  Función utiliada para cambiar de estado el pin entregado en el handler, asignando
 *  el valor entregado en la variable newState
 */
void GPIO_WritePin(GPIO_Handler_t *pPinHandler, uint8_t newState){
	// Limpiamos la posicion que deseamos
	// pPinHandler -> pGPIOx -> ODR &= ~(SET << pPinHandler -> GPIO_PinConfig.GPIO_PinNumber):
	if (newState == SET){
		// Trabajando con la parte baja del registro
		pPinHandler ->pGPIOx ->BSRR |= (SET << pPinHandler ->GPIO_PinConfig.GPIO_PinNumber);
	}
	else{
		// Trabajando con la parte alta del registro
		pPinHandler -> pGPIOx ->BSRR |= (SET << (pPinHandler -> GPIO_PinConfig.GPIO_PinNumber + 16));
	}
}


/**
 *  Funcion para leer el estado de un pin especifico.
 */
uint32_t GPIO_ReadPin(GPIO_Handler_t *pPinHandler){
	// Creamos una variable auxiliar la cual luego retornaremos
	uint32_t pinValue = 0;

	// Cargaremos el valor del registro IDR, desplazado a derecha tantas veces como la ubicacion
	// del pin especifico
	pinValue = (pPinHandler ->pGPIOx ->IDR >> pPinHandler ->GPIO_PinConfig.GPIO_PinNumber);

	return pinValue;
}
