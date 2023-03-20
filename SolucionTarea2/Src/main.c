/**
 ******************************************************************************
 * @file           : main.c
 * @author         : druizve
 * @brief          : configuracion basica de proyecto
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "GPIOxDriver.h"
// Se define el pin 1
uint8_t Pin_1 = 0;
GPIO_Handler_t handlerUserLedPin1 = {0};

// se definen los pines que se emplearan para los leds del 1 al 7
GPIO_Handler_t handlerLED_1 = {0};
GPIO_Handler_t handlerLED_2 = {0};
GPIO_Handler_t handlerLED_3 = {0};
GPIO_Handler_t handlerLED_4 = {0};
GPIO_Handler_t handlerLED_5 = {0};
GPIO_Handler_t handlerLED_6 = {0};
GPIO_Handler_t handlerLED_7 = {0};

//se define el boton y un contador
GPIO_Handler_t			handlerUserButton	= {0};
uint8_t contador = 0;

// la funcion numbBin se empleara como medio para convertir los numeros del contador a binario
void numBin(uint8_t);

uint8_t valor1 = 0;
uint8_t valor2 = 0;
uint8_t valor3 = 0;
uint8_t valor4 = 0;
uint8_t valor5 = 0;
uint8_t valor6 = 0;
uint8_t valor7 = 0;

//Esta variable nos ayudara para conocer el estado del boton: suelto (0) presionado (1)
uint8_t buttonStatus = 0;

int main(void)
{


	// Deseamos trabajar con el puerto GPIOA
	handlerUserLedPin1.pGPIOx 	 				  			=	GPIOA;
	handlerUserLedPin1.GPIO_PinConfig.GPIO_PinNumber 		=	PIN_1;
	handlerUserLedPin1.GPIO_PinConfig.GPIO_PinMode	 		=	GPIO_MODE_IN;
	handlerUserLedPin1.GPIO_PinConfig.GPIO_PinOType	 		=	GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin1.GPIO_PinConfig.GPIO_PinPuPdControl	=	GPIO_PUPDR_PULLUP;
	handlerUserLedPin1.GPIO_PinConfig.GPIO_PinSpeed			=	GPIO_OSPEED_MEDIUM;
	handlerUserLedPin1.GPIO_PinConfig.GPIO_PinAltFunMode	=	AF0; // ninguna funcion

	//Cargamos la configuracion del pin especifico
		GPIO_Config(&handlerUserLedPin1);

	// Hacemos que el PIN_1 quede encendido
	GPIO_WritePin(&handlerUserLedPin1, SET);

	//Se llama la funcion ReadPin para ver el valor del pin, por defecto deberia estar en 1
	GPIO_ReadPin(&handlerUserLedPin1);
	Pin_1 = GPIO_ReadPin(&handlerUserLedPin1);

	GPIOxTooglePin(&handlerUserLedPin1);
	Pin_1 = GPIO_ReadPin(&handlerUserLedPin1);



	//definimos el punto 3
	// primero configuramos el botón
	handlerUserButton.pGPIOx 								= GPIOC;
	handlerUserButton.GPIO_PinConfig.GPIO_PinNumber			= PIN_13;
	handlerUserButton.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserButton.GPIO_PinConfig.GPIO_PinOType			= GPIO_OTYPE_PUSHPULL;
	handlerUserButton.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserButton.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerUserButton.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0; // no hara nada
	GPIO_Config(&handlerUserButton);
	GPIO_WritePin(&handlerUserButton, SET);

	// aqui se configuran los pines a los que van los led que conectaremos


	// configurando el bit-0 puerto A (GPIOA) pin 7 PA7
	handlerLED_1.pGPIOx										= GPIOA;
	handlerLED_1.GPIO_PinConfig.GPIO_PinNumber				= PIN_7;
	handlerLED_1.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerLED_1.GPIO_PinConfig.GPIO_PinOType				= GPIO_OTYPE_PUSHPULL;
	handlerLED_1.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerLED_1.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;
	handlerLED_1.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;

	//configurando el bit-1 puerto C (GPIOC) pin 8 PC8
	handlerLED_2.pGPIOx										= GPIOC;
	handlerLED_2.GPIO_PinConfig.GPIO_PinNumber				= PIN_8;
	handlerLED_2.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerLED_2.GPIO_PinConfig.GPIO_PinOType				= GPIO_OTYPE_PUSHPULL;
	handlerLED_2.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerLED_2.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;
	handlerLED_2.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;

	// configurando el bit-2 puerto C (GPIOC) pin 7 PC7
	handlerLED_3.pGPIOx										= GPIOC;
	handlerLED_3.GPIO_PinConfig.GPIO_PinNumber				= PIN_7;
	handlerLED_3.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerLED_3.GPIO_PinConfig.GPIO_PinOType				= GPIO_OTYPE_PUSHPULL;
	handlerLED_3.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerLED_3.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;
	handlerLED_3.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;

	// Configurando el bit-3 puerto A (GPIOA) pin 6 PA6
	handlerLED_4.pGPIOx										= GPIOA;
	handlerLED_4.GPIO_PinConfig.GPIO_PinNumber				= PIN_6;
	handlerLED_4.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerLED_4.GPIO_PinConfig.GPIO_PinOType				= GPIO_OTYPE_PUSHPULL;
	handlerLED_4.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerLED_4.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;
	handlerLED_4.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;

	// Configurando el bit-4 puerto B (GPIOB) pin 8 PB8
	handlerLED_5.pGPIOx										= GPIOB;
	handlerLED_5.GPIO_PinConfig.GPIO_PinNumber				= PIN_8;
	handlerLED_5.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerLED_5.GPIO_PinConfig.GPIO_PinOType				= GPIO_OTYPE_PUSHPULL;
	handlerLED_5.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerLED_5.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;
	handlerLED_5.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;

	// Configurando el bit-5 puerto C (GPIOC) pin6 PC6
	handlerLED_6.pGPIOx										= GPIOC;
	handlerLED_6.GPIO_PinConfig.GPIO_PinNumber				= PIN_6;
	handlerLED_6.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerLED_6.GPIO_PinConfig.GPIO_PinOType				= GPIO_OTYPE_PUSHPULL;
	handlerLED_6.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerLED_6.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;
	handlerLED_6.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;

	// Configurando el bit-6 puerto C (GPIOC) pin 9 PC9
	handlerLED_7.pGPIOx										= GPIOC;
	handlerLED_7.GPIO_PinConfig.GPIO_PinNumber				= PIN_9;
	handlerLED_7.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerLED_7.GPIO_PinConfig.GPIO_PinOType				= GPIO_OTYPE_PUSHPULL;
	handlerLED_7.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerLED_7.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;
	handlerLED_7.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;


    /* Loop forever */
	while(1){

		buttonStatus = GPIO_ReadPin(&handlerUserButton);

		numBin(contador);

		if(buttonStatus == 1){
			contador += 1;
		}
		if(buttonStatus == 0){
			contador -= 1;
		}

		if(contador > 60){
			contador = 1;
		}

		if(contador < 1){
			contador = 60;
		}

		for (int i=0;(i<1200000);i++){
				NOP();
		}


	}
	return 0;
}


void numBin(uint8_t contador){

		valor1 = contador & 0b0000001;
		valor2 = contador & 0b0000010;
		valor2 >>= 1;
		valor3 = contador & 0b0000100;
		valor3 >>= 2;
		valor4 = contador & 0b0001000;
		valor4 >>= 3;
		valor5 = contador & 0b0010000;
		valor5 >>= 4;
		valor6 = contador & 0b0100000;
		valor6 >>= 5;
		valor7 = contador & 0b1000000;
		valor7 >>= 6;

		if(valor1 == 1){
			GPIO_WritePin(&handlerLED_1, SET);
		}else{
			GPIO_WritePin(&handlerLED_1, RESET);
		}

		if(valor2 == 1){
			GPIO_WritePin(&handlerLED_2, SET);
		}else{
			GPIO_WritePin(&handlerLED_2, RESET);
		}

		if(valor3 == 1){
			GPIO_WritePin(&handlerLED_3, SET);
		}else{
			GPIO_WritePin(&handlerLED_3, RESET);
		}

		if(valor4 == 1){
			GPIO_WritePin(&handlerLED_4, SET);
		}else{
			GPIO_WritePin(&handlerLED_4, RESET);
		}

		if(valor5 == 1){
			GPIO_WritePin(&handlerLED_5, SET);
		}else{
			GPIO_WritePin(&handlerLED_5, RESET);
		}

		if(valor6 == 1){
			GPIO_WritePin(&handlerLED_6, SET);
		}else{
			GPIO_WritePin(&handlerLED_6, RESET);
		}

		if(valor7 == 1){
			GPIO_WritePin(&handlerLED_7, SET);
		}else{
			GPIO_WritePin(&handlerLED_7, RESET);
		}
	}
