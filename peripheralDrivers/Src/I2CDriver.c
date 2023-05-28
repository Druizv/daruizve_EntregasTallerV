/*
 * USARTxDriver.c
 *
 *  Created on: Apr 6, 2022
 *      Author: namontoy
 */

#include <stm32f4xx.h>
#include "USARTxDriver.h"

/**
 * Configurando el puerto Serial...
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 */

uint8_t auxRxData = 0;

void USART_Config(USART_Handler_t *ptrUsartHandler){

	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las pisbles opciones que tengamos (USART1, USART2, USART6) */
    /* 1.1 Configuramos el USART1 */
	if(ptrUsartHandler->ptrUSARTx == USART1){
		// Escriba acá su código
		RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	}

    /* 1.2 Configuramos el USART2 */
    // Escriba acá su código
	else if(ptrUsartHandler->ptrUSARTx == USART2){

		RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	}

    /* 1.3 Configuramos el USART6 */
    // Escriba acá su código
	else if(ptrUsartHandler->ptrUSARTx == USART6){

		RCC->APB2ENR &= ~RCC_APB2ENR_USART6EN;
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
	}
	else{
		__NOP();
	}


	/* 2. Configuramos el tamaño del dato, la paridad y los bit de parada */
	/* En el CR1 estan parity (PCE y PS) y tamaño del dato (M) */
	/* Mientras que en CR2 estan los stopbit (STOP)*/
	/* Configuracion del Baudrate (registro BRR) */
	/* Configuramos el modo: only TX, only RX, o RXTX */
	/* Por ultimo activamos el modulo USART cuando todo esta correctamente configurado */

	// 2.1 Comienzo por limpiar los registros, para cargar la configuración desde cero
	ptrUsartHandler->ptrUSARTx->CR1 = 0;
	ptrUsartHandler->ptrUSARTx->CR2 = 0;

	// 2.2 Configuracion del Parity:
	// Verificamos si el parity esta activado o no
    // Tenga cuidado, el parity hace parte del tamaño de los datos...
	if(ptrUsartHandler->USART_Config.USART_parity != USART_PARITY_NONE){

		// Verificamos si se ha seleccionado ODD or EVEN
		if(ptrUsartHandler->USART_Config.USART_parity == USART_PARITY_EVEN){
			// Es even, entonces cargamos la configuracion adecuada
			// Escriba acá su código
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PS;

		}else{
			// Si es "else" significa que la paridad seleccionada es ODD, y cargamos esta configuracion
			// Escriba acá su código
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PS;
		}
	}else{
		// Si llegamos aca, es porque no deseamos tener el parity-check
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PCE;
	}

	// 2.3 Configuramos el tamaño del dato
    // Escriba acá su código
	if(ptrUsartHandler->USART_Config.USART_datasize == USART_DATASIZE_8BIT){
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_M; // 8 bits
	}
	//si son 9 bits o si son 8 y con paridad activada
	else if((ptrUsartHandler->USART_Config.USART_datasize == USART_DATASIZE_9BIT) || ((ptrUsartHandler->USART_Config.USART_parity != USART_PARITY_NONE)
				&&(ptrUsartHandler->USART_Config.USART_datasize == USART_DATASIZE_8BIT))){
		//se pone 1 en el registro
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M;
	}






	// 2.4 Configuramos los stop bits (SFR USART_CR2)
	switch(ptrUsartHandler->USART_Config.USART_stopbits){
	case USART_STOPBIT_1: {
		// Debemoscargar el valor 0b00 en los dos bits de STOP
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR2 &= ~USART_CR2_STOP;
		break;
	}
	case USART_STOPBIT_0_5: {
		// Debemoscargar el valor 0b01 en los dos bits de STOP
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP & (~USART_CR2_STOP_1);
		break;
	}
	case USART_STOPBIT_2: {
		// Debemoscargar el valor 0b10 en los dos bits de STOP
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP & (~USART_CR2_STOP_0);
		break;
	}
	case USART_STOPBIT_1_5: {
		// Debemoscargar el valor 0b11 en los dos bits de STOP
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP;
		break;
	}
	default: {
		// En el casopor defecto seleccionamos 1 bit de parada
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR2 &= ~USART_CR2_STOP;
		break;
	}
	}

	// 2.5 Configuracion del Baudrate (SFR USART_BRR)
	// Ver tabla de valores (Tabla 73), Frec = 16MHz, overr = 0;
	if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_9600){
		// El valor a cargar es 104.1875 -> Mantiza = 104,fraction = 0.1875
		// Mantiza = 104 = 0x68, fraction = 16 * 0.1875 = 3
		// Valor a cargar 0x0683
		// Configurando el Baudrate generator para una velocidad de 9600bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0683;
	}

	else if (ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_19200) {
		// El valor a cargar es 52.0625 -> Mantiza = 52,fraction = 0.0625
		// Mantiza = 52 = 0x34, fraction = 16 * 0.1875 = 1
		// Escriba acá su código y los comentarios que faltan
		// Valor a agregar 0x0341
		ptrUsartHandler->ptrUSARTx->BRR = 0x0341;
	}

	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_115200){
		// Escriba acá su código y los comentarios que faltan
		// El valor a cargar es 8.6875 -> Mantiza = 8,fraction = 0.6875
		// Mantiza = 8 = 0x8, fraction = 16 * 0.6875 = 11
		// Valor a cargar 0x08B
		ptrUsartHandler->ptrUSARTx->BRR = 0x008B;
	}

	// Ver tabla de valores (Tabla 73), Frec = 80MHz
	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_9600_80MHz){
		// Escriba acá su código y los comentarios que faltan
		// El valor a cargar es 520.833333333 -> Mantiza = 520, fraction = 0.833333333
		// Mantiza = 520 = 0x208, fraction = 16 * 0.833333333 = 13
		// Valor a cargar 0x02B6
		ptrUsartHandler->ptrUSARTx->BRR = 0x208D;
	}

	// Ver tabla de valores (Tabla 73), Frec = 80MHz
	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_19200_80MHz){
		// Escriba acá su código y los comentarios que faltan
		// El valor a cargar es 260.416666667 -> Mantiza = 260, fraction = 0.416666667
		// Mantiza = 260 = 104, fraction = 16 * 0.416666667 = 6
		// Valor a cargar 0x02B6
		ptrUsartHandler->ptrUSARTx->BRR = 0x1046;
	}

	// Ver tabla de valores (Tabla 73), Frec = 80MHz
	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_115200_80MHz){
		// Escriba acá su código y los comentarios que faltan
		// El valor a cargar es 43.40278 -> Mantiza = 43, fraction = 0.40278
		// Mantiza = 43 = 0x2B, fraction = 16 * 0.40278 = 6
		// Valor a cargar 0x02B6
		ptrUsartHandler->ptrUSARTx->BRR = 0x2B6;
	}

	// 2.6 Configuramos el modo: TX only, RX only, RXTX, disable
	switch(ptrUsartHandler->USART_Config.USART_mode){
	case USART_MODE_TX:
	{
		// Activamos la parte del sistema encargada de enviar
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		break;
	}
	case USART_MODE_RX:
	{
		// Activamos la parte del sistema encargada de recibir
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;
		break;
	}
	case USART_MODE_RXTX:
	{
		// Activamos ambas partes, tanto transmision como recepcion
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |= (USART_CR1_TE | USART_CR1_RE);
		break;
	}
	case USART_MODE_DISABLE:
	{
		// Desactivamos ambos canales
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		break;
	}

	default:
	{
		// Actuando por defecto, desactivamos ambos canales
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		break;
	}
	}

	// 2.7 Activamos el modulo serial.
	if(ptrUsartHandler->USART_Config.USART_mode != USART_MODE_DISABLE){
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_UE;

	}
	else {
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
	}


	/* 2.8 Activando las interrupciones*/
	__disable_irq();

	// Recepcion
	if(ptrUsartHandler->USART_Config.USART_enableIntRX == USART_RX_INTERRUPT_ENABLE){
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RXNEIE;
	}
	else {
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RXNEIE;
	}
	// Transmision
	if(ptrUsartHandler->USART_Config.USART_enableIntTX == USART_TX_INTERRUPT_ENABLE){
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TXEIE;
	}
	else {
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TXEIE;
	}

	//Matriculamos el NVIC
	if(ptrUsartHandler->ptrUSARTx == USART1){
		__NVIC_EnableIRQ(USART1_IRQn);
	}
	else if(ptrUsartHandler->ptrUSARTx == USART2){
		__NVIC_EnableIRQ(USART2_IRQn);
	}
	else if(ptrUsartHandler->ptrUSARTx == USART6){
		__NVIC_EnableIRQ(USART6_IRQn);
	}

	__enable_irq();

} //FIN del USART config


/* funcion para escribir un solo char */
int writeChar(USART_Handler_t *ptrUsartHandler, char dataToSend ){
	while( !(ptrUsartHandler->ptrUSARTx->SR & USART_SR_TXE)){
		__NOP();
	}

	// Escriba acá su código
	ptrUsartHandler->ptrUSARTx->DR = dataToSend;

	return dataToSend;
}

/** Funcion para escribir todo un string */
void writeMsg(USART_Handler_t *ptrUsartHandler, char *msgToSend){
	int caracter = 0;
	while(msgToSend[caracter] != '\0'){
		writeChar(ptrUsartHandler, msgToSend[caracter]);
		caracter++;
	}
}





/** Función para recibir datos */
uint8_t getRxData(void){

	return auxRxData;
}


/* handler de la interrupción del USART1 */
void USART1_IRQHandler(void){
	// Evaluamos si la interrupción que se dio es por RX
	if(USART1->SR & USART_SR_RXNE){
		auxRxData = (uint8_t) USART1->DR;
		usart1Rx_Callback();
	}
}

/* handler de la interrupción del USART2 */
void USART2_IRQHandler(void){
	// Evaluamos si la interrupción que se dio es por RX
	if(USART2->SR & USART_SR_RXNE){
		auxRxData = (uint8_t) USART2->DR;
		usart2Rx_Callback();
	}
}

/* handler de la interrupción del USART6 */
void USART6_IRQHandler(void){
	// Evaluamos si la interrupción que se dio es por RX
	if(USART6->SR & USART_SR_RXNE){
		auxRxData = (uint8_t) USART6->DR;
		usart6Rx_Callback();
	}
}

/** Funciones callback weak, que pueden ser sobre-escritas*/
__attribute__((weak)) void usart1Rx_Callback(void){
		/* 	NOTE: This function should not be modified, when the callback is needed,
		  		  the BasicTimer_Callback could be implemented in the main file
		 */
	__NOP();
}
__attribute__((weak)) void usart2Rx_Callback(void){
		/* 	NOTE: This function should not be modified, when the callback is needed,
		  		  the BasicTimer_Callback could be implemented in the main file
		 */
	__NOP();
}
__attribute__((weak)) void usart6Rx_Callback(void){
		/* 	NOTE: This function should not be modified, when the callback is needed,
		  		  the BasicTimer_Callback could be implemented in the main file
		 */
	__NOP();
}
