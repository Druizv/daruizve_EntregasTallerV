/*
 * ExtiConfig_main.c
 *
 *  Created on: Mar 24, 2023
 *      Author: druiz
 */

#include <stdint.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <stdbool.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "SysTickDriver.h"
#include "PwmDriver.h"
#include "I2CDriver.h"
#include "pll_driver.h"


void init_system(void);

//Definicion del timer del led
BasicTimer_Handler_t handlerBlinkyTimer  = {0};

//definicion del blinky del led
GPIO_Handler_t handlerLEDBlinky    = {0};


uint8_t printMsg = 0;
USART_Handler_t usart2com = {0};

int main(void){
	//inicialización de todos los elementos del sistema
	init_system();

	while(1){
		if (printMsg> 4){
			writeChar(&usart2com, "H");
			writeChar(&usart2com, "O");
			writeChar(&usart2com,"L");
			writeChar(&usart2com, "A");

			printMsg = 0;
		}

		printMsg++;


		}
}
void init_system(void){
	/* Habilitar el reloj del USART2 */
	RCC_AHB1ENR_P PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Inicializar la estructura GPIO_InitTypeDef */
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Configurar el pins USART Rx y Tx como modo de AF */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Especificar la configuración del USART */
	USART_Handler_t USART_InitStruct;
	USART_InitStruct.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
	USART_InitStruct.USART_Config.USART_stopbits = USART_STOPBIT_1;
	USART_InitStruct.USART_Config.USART_parity = USART_PARITY_NONE;
	USART_InitStruct.USART_Config.USART_mode = USART_MODE_RX | USART_MODE_TX;

	/* Inicializar el USART */
	USART_Config(&USART_InitStruct);


}
			// Aca termina la funcion main

/*void init_system(void){

	// Se configura el PA5 para el LED2
	handlerLEDBlinky.pGPIOx = GPIOA;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinNumber = PIN_5;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinMode   = GPIO_MODE_OUT;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinOType  = GPIO_OTYPE_PUSHPULL;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinSpeed  = GPIO_OSPEED_FAST;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	// Configuracion del LED2 en el registro
	GPIO_Config(&handlerLEDBlinky);

	GPIO_WritePin(&handlerLEDBlinky,SET);

	//configurar el TIM2 para que haga una interrupcion cada 250ms
	handlerBlinkyTimer.ptrTIMx 								= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode                = BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed				= BTIMER_SPEED_1ms;
	handlerBlinkyTimer.TIMx_Config.TIMx_period				= 250; //cada 250 ms
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable		= BTIMER_INTERRUPT_ENABLE;

	//Configurar el TIM3 para que haga interrupcion cada 30ms
	handlerRefreshTimer.ptrTIMx 							= TIM3;
	handlerRefreshTimer.TIMx_Config.TIMx_mode				= BTIMER_MODE_UP;
	handlerRefreshTimer.TIMx_Config.TIMx_speed				= BTIMER_SPEED_1ms;
	handlerRefreshTimer.TIMx_Config.TIMx_period				= 10; //cada 30 ms
	handlerRefreshTimer.TIMx_Config.TIMx_interruptEnable	= BTIMER_INTERRUPT_ENABLE;

	//Se carga la configuracion en el registro
	BasicTimer_Config(&handlerBlinkyTimer);
	BasicTimer_Config(&handlerRefreshTimer);
}

void configPLL(void){
	PLL_Config_t pll_config;
	pll_config.PLLM = 8;
	pll_config.PLLN = 320; // se cambia de 336 a 320
	pll_config.PLLP = 2;
	configPLL(pll_config);

	uint32_t usart_brr = (uint32_t) (80000000 / 115200);
	ptrUsartHandler ->ptrUSARTx->BRR = usart_brr;

}
*/

