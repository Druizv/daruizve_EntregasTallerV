/*
 *
 *  Created on: Mar 24, 2023
 *      Author: druiz
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "PllDriver.h"

#include "arm_math.h"

GPIO_Handler_t handlerBlinkyPin = {0};

GPIO_Handler_t handlerUserButton = {0};
EXTI_Config_t handlerUserButtonExti = {0};

BasicTimer_Handler_t handlerBlinkyTimer = {0};

//Elementos para hacer la comunicacion serial
GPIO_Handler_t handlerPinTX = {0};
GPIO_Handler_t handlerPinRX = {0};
USART_Handler_t usart2comm  = {0};
uint8_t sendMsg 			= 0;
uint8_t usartDataReceived  = 0;

char bufferMsg[64]			= {0};

//Arreglos para pruebas de libreria CMSIS
float32_t scrNumber[4] 	= {-0.987, 32.26, -45.21, -987.321};
float32_t destNumber[4] = {0};
uint32_t dataSize 		= 0;

//Para utilizar la funcion seno
float32_t sineValue		= 0.0;
float32_t sineArgValue	= 0.0;

//Elementos para generar una señal
#define SINE_DATA_SIZE    4096   			//Tamaño del arrlego de datos
float32_t fs =  8000.0;					//Frecuencia de muestreo
float32_t f0 =	250.0;					//Frecuencia fundamental de la señal
float32_t dt =	0;						//Periodo de muestreo, en este caso sera (1/fs)
float32_t stopTime = 1.0;				//Quizas no sea necesario
float32_t amplitud = 5;					//Amplitud de la señal generada
float32_t sineSignal[SINE_DATA_SIZE];
float32_t transformedSignal[SINE_DATA_SIZE];
float32_t* ptrSineSingal;

uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
arm_rfft_fast_instance_f32 config_Rfft_fast_f32;
arm_cfft_radix4_instance_f32 configRadix4_f32;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;
uint16_t fftSize = 1024;

PLL_Handler_t handlerpll  = {0};
//Definiciones de las cabeceras de las funciones del main

GPIO_Handler_t handlerCLK  = {0};

void initSystem(void);
void createSignal(void);

/*Funcion principal del programa
 * Esta funcion es el corazon del programa
 */

int main(void){

	//Activamos el coprocesador matematico
	SCB->CPACR |= (0xF << 20);

	//Iniciamos todos los elementos del sistema
	initSystem();

	writeMsg(&usart2comm, "iniciando \n");
	//Loop forever
	while(1){

		//Crear la señalwriteMsg(&usart2comm, "iniciando \n");

		if(usartDataReceived == 'C'){

			createSignal();
			sprintf(bufferMsg, "Creando la seña.. \n");
			writeMsg(&usart2comm, bufferMsg);
			usartDataReceived = '\0';
		}
		//Para probar el seno
		if(usartDataReceived == 'P'){

			stopTime = 0.0;
			int i	 = 0;

			sprintf(bufferMsg, "Signal values Time - sine\n");
			writeMsg(&usart2comm, bufferMsg);

			while(stopTime < 0.01){
				stopTime = dt*i;
				i++;
				sprintf(bufferMsg, "%#.5f ; %#.6f\n", stopTime, sineSignal[i]);
				writeMsg(&usart2comm, bufferMsg);
			}
			usartDataReceived = '\0';
		}
		if(usartDataReceived == 'A'){

			stopTime = 0.0;
			int i = 0;

			sprintf(bufferMsg, "Valor Absoluto \n");
			writeMsg(&usart2comm, bufferMsg);

			arm_abs_f32(sineSignal, transformedSignal, SINE_DATA_SIZE);

			while(stopTime < 0.01){
				stopTime = dt*i;
				i++;
				sprintf(bufferMsg, "%#.5f ; %#.6f\n", stopTime, sineSignal[i]);
				writeMsg(&usart2comm, bufferMsg);
			}
			usartDataReceived = '\0';
		}
		if(usartDataReceived == 'I'){
			statusInitFFT = arm_rfft_fast_init_f32(&config_Rfft_fast_f32, fftSize);

			if(statusInitFFT == ARM_MATH_SUCCESS){
				sprintf(bufferMsg, "Initialization.... SUCESS!\n");
				writeMsg(&usart2comm, bufferMsg);
			}
			usartDataReceived = '\0';
		}
		if(usartDataReceived == 'F'){

			stopTime = 0.0;
			int i = 0;
			int j = 0;

			sprintf(bufferMsg, "FFT \n");
			writeMsg(&usart2comm, bufferMsg);

			if(statusInitFFT == ARM_MATH_SUCCESS){
				arm_rfft_fast_f32(&config_Rfft_fast_f32, sineSignal, transformedSignal, ifftFlag);

				arm_abs_f32(transformedSignal, sineSignal, fftSize);

				for( i = 1; i < fftSize; i++){
					if(i % 2){
						sprintf(bufferMsg, "%u ; %#.6f\n", j, 2*sineSignal[i]);
						writeMsg(&usart2comm, bufferMsg);
						j++;
					}
				}
			}
			else{
				writeMsg(&usart2comm, "FFT not initialized...");
			}
			usartDataReceived = '\0';
		}
	}
	return 0;
}

//Funciones encargadas de inizializar los elementos del sistema

void initSystem(void){

	/*Configuramos la velocidad del micro*/
	handlerpll.clkSpeed = FREQUENCY_100MHz;
	configPLL(&handlerpll);

	//configuracion del pin para el LED blinky
	handlerBlinkyPin.pGPIOx											= GPIOH;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinNumber					= PIN_1;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_FAST;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerBlinkyPin);


	handlerCLK.pGPIOx = GPIOA;
	handlerCLK.GPIO_PinConfig.GPIO_PinNumber = PIN_8;
	handlerCLK.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerCLK.GPIO_PinConfig.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerCLK.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	handlerCLK.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerCLK.GPIO_PinConfig.GPIO_PinAltFunMode = AF0;
	GPIO_Config(&handlerCLK);

	//Configuracion del timer 2 para que funcione con el blinky
	handlerBlinkyTimer.ptrTIMx										= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode						= BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed						= BTIMER_SPEED_100MHz_100us;
	handlerBlinkyTimer.TIMx_Config.TIMx_period						= 2500;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable				= BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerBlinkyTimer);

	//El pin UserButton es una entrada simple que entregará la interrupcion EXTI-13
	//Observe que el pin seleccionado es el PIN_13 por lo tanto el callback
	//que se debe configurar es el callback_extInt13()
	handlerUserButton.pGPIOx										= GPIOC;
	handlerUserButton.GPIO_PinConfig.GPIO_PinNumber					= PIN_13;
	handlerUserButton.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_IN;
	handlerUserButton.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerUserButton);

	handlerUserButtonExti.pGPIOHandler								= &handlerUserButton;
	handlerUserButtonExti.edgeType									= EXTERNAL_INTERRUPT_RISING_EDGE;
	extInt_Config(&handlerUserButtonExti);

	//Configuracion de pin para la comunicacion serial
	handlerPinTX.pGPIOx												= GPIOA;
	handlerPinTX.GPIO_PinConfig.GPIO_PinNumber						= PIN_9;
	handlerPinTX.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinTX.GPIO_PinConfig.GPIO_PinAltFunMode					= AF7;
	GPIO_Config(&handlerPinTX);

	handlerPinRX.pGPIOx												= GPIOA;
	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber						= PIN_10;
	handlerPinRX.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode					= AF7;
	GPIO_Config(&handlerPinRX);

	usart2comm.ptrUSARTx											= USART1;
	usart2comm.USART_Config.USART_baudrate							= USART_BAUDRATE_100MHz_115200;
	usart2comm.USART_Config.USART_datasize							= USART_DATASIZE_8BIT;
	usart2comm.USART_Config.USART_parity							= USART_PARITY_NONE;
	usart2comm.USART_Config.USART_stopbits							= USART_STOPBIT_1;
	usart2comm.USART_Config.USART_mode								= USART_MODE_RXTX;
	usart2comm.USART_Config.USART_enableIntRX						= USART_RX_INTERRUPT_ENABLE;
	usart2comm.USART_Config.USART_enableIntTX						= USART_TX_INTERRUPT_DISABLE;
	USART_Config(&usart2comm);
}

void createSignal(void){

	/*Esta es la señal creada en Matlab
	 * sineSignal = amplitud * sin(2*pi*f0*t);
	 */
	//Creando la señal necesitamos el periodo dt
	dt = 1/fs;

	//LLenamos el arreglo con la señal seno
	for(int i = 0; i < SINE_DATA_SIZE; i++){
		sineSignal[i] = amplitud * arm_sin_f32(2*M_PI*f0*(dt*i));
	}
}


//Callback del timer 2 - Hacemos un blinky

void BasicTimer2_Callback(void){
	GPIOxTogglePin(&handlerBlinkyPin);
	sendMsg++;
}

//Callback del usuario - Hacer algo

void callback_extInt13(void){
	__NOP();
}

//Esta funcion se ejecuta una vez que un caracter
//Sea recibido por el  puerto USART 2

void usart1RX_Callback(void){

	usartDataReceived = getRxData();
}
