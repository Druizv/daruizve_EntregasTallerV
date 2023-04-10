/*
 * ClaseGP2_main.c
 *
 *  Created on: Mar 24, 2023
 *      Author: druiz
 */
#include "stdint.h"
#include "stm32f4xx.h"

//definicion de los elementos del programa
uint32_t counterG2 = 0;
uint32_t auxVariableG2 = 0;

uint32_t *ptr_CounterG2;

uint32_t byteVariableG2;
uint32_t *ptr_ByteVariable;

int main (void){
	//trabajando con las variables y los punteros
	counterG2 = 3456789;
	auxVariableG2 = counterG2;

	ptr_CounterG2 = &counterG2;

	*ptr_CounterG2 = 9876543;

	ptr_CounterG2++;
	*ptr_CounterG2 = 9876543;

	while (1){

	}
}
