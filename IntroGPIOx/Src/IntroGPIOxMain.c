/*
 * ******************************************
 * @FIle: 			IntroGPIOxMain.c
 * @Author: 		druiz
 * Created on: 		Mar 12, 2023
 * @brief:			Archivo principal (main)
 * ******************************************
 *
 * Con este programa se desea mostrar el uso basico de los registros que controlan
 * al Micro (SFR) y la forma adecuada para utilizar los operadores &,|, ~ y =, para
 * cambiar la configuracion de algún registro.
 * Tambien es importante para entender la ultilidad de los numeros BIN y HEX
 *
 * Es necesario tener a la mano tanto el manual de referencia del micro como la hoja de
 * datos del micro (ambos son documentos diferentes y complementarios el uno del otro)
 *
 * HAL -> Hardware Abstraction Layer
 *
 * Este programa introduce el preiférico mas simple que tiene el micro, que es el
 * encargado de manejar los pines de cada puerto del micro.
 *
 * Cada PINx de cada puerto GPIO puede desarrollar funciones basicas de tipo entrada
 * y salida de datos digitales ( os ea 1's y 0's), además se les puede asignar funciones
 * especiales que generalmente estan ligadas a otro periferico adicional (se vera mas
 * adelante)
 *
 * De igual forma, varias caracteristicas pueden ser configuradas para cada PINx
 * especifico como son:
 * 	- La velocidad a la que puede responder
 * 	- Tipo de entrada (pull-up, pull-down, open drain (flotante))
 * 	- Tipo de salida ( push-pull, open drain...)
 * 	- Entrada analoga
 *
 * Debemos definir entonces todos los registros que manejan el perigerico GPIOx y luego
 * crear algunas funciones para utilizar adecuadamente el equipo.
 *
 * **************************************************************
 */

#include <stdint.h>

#include "*stm32f411xx_hal.h"
#include "GPIOxDriver.h"

// Funcion principal del programa. Es aca donde se ejecuta todo
int main(void){
	//**********************
	// Definimos el handler para el PIN que deseamos configurar
	GPIO_Handler_t handlerUserLedPin = {0};

	// Deseamos trabajar con el puerto GPIOA
	handlerUserLedPin.pGPIOx = GPIOA;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinMode			= GPIOx_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinOType			= GPIOx_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIOx_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinSpeed			= GPAIOx_OSPEEDR_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;	// Ninguna funcion

	//Cargamos la configuracion del pin especifico
	GPIO_Config(&handlerUserLedPin);

	// Hacemos que el PIN_5 quede encendido
	GPIO_WritePin(&handlerUserLedPin, SET);

	// Este es el ciclo principal, donde se ejecuta todo el programa
	while(1){
		NOP();
	}

}

