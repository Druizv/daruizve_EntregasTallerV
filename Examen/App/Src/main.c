/*
 * ExtiConfig_main.c
 *
 *  Created on: Mar 24, 2023
 *      Author: druiz
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "SysTickDriver.h"
#include "PllDriver.h"
#include "I2CDriver.h"
#include "LCDDriver.h"
#include "PwmDriver.h"
#include "RTC.h"

#include "arm_math.h"
#include <math.h>

//Blinky
GPIO_Handler_t handlerBlinkyPin = {0};	//PH1
uint8_t counter = 0;


/* Elemento para hacer la comunicacion serial */
GPIO_Handler_t handlerPinTX = {0};
GPIO_Handler_t handlerPinRX = {0};
USART_Handler_t usart2comm = {0};
PLL_Handler_t handlerPll = {0};


BasicTimer_Handler_t handlerBlinkyTimer	= {0};  //TIM2
BasicTimer_Handler_t handler1KHzTimer   = {0};     //TIM4

uint8_t rxData				 = 0;
char    bufferReception[64]  = {0};
uint8_t counterReception = 0;
char   	bufferData [64]  = {0};

//muestreo
uint8_t flagSample = 0;
uint8_t numberSample = 0;
uint8_t usart2DataReceived = 0;

bool stringComplete = false;

//RTC
RTC_Handler_t handlerRTC = {0};

uint8_t hours = 0;
uint8_t mins = 0;
uint8_t secs = 0;
uint8_t day = 0;
uint8_t month = 0;
uint8_t year = 0;
uint16_t *date = 0;

//acelerometro
GPIO_Handler_t handler_i2cSCL = {0};
GPIO_Handler_t handler_i2cSDA = {0};
I2C_Handler_t handlerAccelerometer = {0};

PLL_Handler_t handlerpll  = {0};

//Parametros
unsigned int 	firstParameter = 0;
unsigned int 	secondParameter = 0;
unsigned int	thirdParameter = 0;
char 			userMsg[64] = {0};
char 			cmd[64]; // ayuda para analizar comandos

// Definicion de las cabeceras de las funciones del main
void init_system(void);
void parseCommands (char *ptrBufferReception);


int main(void){
	//Activamos el coprocesador matematico
	SCB->CPACR |= (0xF <<20);

	//Inicializamos todos los elementos del sistema
	init_system();

	writeMsg(&usart2comm, "iniciando \n");
	while(1){

		//Se verifica constantemente si tiene una nueva entrada y de ser asi entrara en los condicionales
		if (rxData != '\0'){
			bufferReception[counterReception] = rxData;
			counterReception++;

			if (rxData == '@'){
				stringComplete = true;
				// Se agrega esta linea para crear el string con el null al final.
				bufferReception[counterReception-1] = '\0';
				counterReception = 0;
			}
			rxData = '\0';

		}

		//para que no vuelva a entrar, solo cambia debido a la interrupcion
		//rxData = '\n';
		// Se analiza el string obtenido
		if (stringComplete) {
			parseCommands(bufferReception);
			stringComplete = false;
		}

		stringComplete = false;

	}

}//Fin del Main



//Funcion que configura el hardware, timers
void init_system(void){

	/* ========================== PLL ======================================================================*/

	handlerPll.clkSpeed = FREQUENCY_100MHz;
	configPLL(&handlerPll);


	//Configuracion del BLinky
	handlerBlinkyPin.pGPIOx											= GPIOH;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinNumber					= PIN_1;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_FAST;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerBlinkyPin);

/* ==================================== Configurando los TIMERS =============================================*/
	//Configurando el TIM2 el cual le da la tasa de encendido al LED
	handlerBlinkyTimer.ptrTIMx										= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode						= BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed						= BTIMER_SPEED_100MHz_100us;
	handlerBlinkyTimer.TIMx_Config.TIMx_period						= 2500;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable				= BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerBlinkyTimer);

	handler1KHzTimer.ptrTIMx                              = TIM4;
	handler1KHzTimer.TIMx_Config.TIMx_mode                = BTIMER_MODE_UP ;
	handler1KHzTimer.TIMx_Config.TIMx_speed               = BTIMER_SPEED_100MHz_100us;
	handler1KHzTimer.TIMx_Config.TIMx_period              = 10;
	handler1KHzTimer.TIMx_Config.TIMx_interruptEnable     = BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handler1KHzTimer);


/* ==================================== Configurando los EXTI =============================================*/

/* ==================================== Configurando los USART =============================================*/
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


	/* ========================== I2C ======================================================================*/
	handler_i2cSCL.pGPIOx												= GPIOB;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinNumber						= PIN_8;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinMode							= GPIO_MODE_ALTFN;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_OPENDRAIN;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinPuPdControl        			= GPIO_PUPDR_PULLUP;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinSpeed              			= GPIO_OSPEED_FAST;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinAltFunMode					= AF4;
	GPIO_Config(&handler_i2cSCL);

	handler_i2cSDA.pGPIOx												= GPIOB;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinNumber						= PIN_9;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinMode							= GPIO_MODE_ALTFN;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_OPENDRAIN;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinPuPdControl        			= GPIO_PUPDR_PULLUP;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinSpeed              			= GPIO_OSPEED_FAST;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinAltFunMode					= AF4;
	GPIO_Config(&handler_i2cSDA);

	handlerAccelerometer.ptrI2Cx                            			= I2C1;
	handlerAccelerometer.modeI2C                            			= I2C_MODE_FM;
	handlerAccelerometer.slaveAddress                    				= ACCEL_ADDRESS;
	i2c_config(&handlerAccelerometer);

/*  ====================================== PWM =============================================*/

//	handlerLCD.ptrI2Cx                            			= I2C2;
//	handlerLCD.modeI2C                            			= I2C_MODE_SM;
//	handlerLCD.slaveAddress                       			= LCD_ADDRESS;
//	i2c_config(&handlerLCD);
}

/* ===================== Rutinas de atencion o callbacks ===============================================*/

void BasicTimer2_Callback(void){
	GPIOxTogglePin(&handlerBlinkyPin);
	counter++;
}

void BasicTimer4_Callback(void){
	if(flagSample == 1){
		numberSample++;
	}

}

void callback_extInt13(void){
	__NOP();
}

/*
 * Esta funcion se ejecuta cada vez que un caracter es recibido
 * por el puerto USART1
 */

void usart1Rx_Callback(void){
	rxData = getRxData();

}

/*void BasicTimer3_Callback(void){

	usart2DataReceived = getRxData();

}*/

// funcion que administra las entradas por consola del PC
void parseCommands (char *ptrBufferReception){
	// Esta función lee la cadena de caracteres a lo que apunta el 'ptr' y la divide
	// y almacena en tres elementos diferentes: Un string llamado 'cmd', y dos números
	// integer llamados 'firstParameter' y 'secondparameter', de esta forma, es posible
	// introducir información al micro desde el puerto serial.
	sscanf(ptrBufferReception, "%s %u %u %s", cmd, &firstParameter,
			&secondParameter, userMsg);
//	if (!firstParameter && firstParameter != 0) {
//		sscanf(ptrBufferReception, "%s %s", cmd, userMsg);
//	}
	// Este primer comando imprime una lista con los otros comandos que tiene el equipo.
	if (strcmp(cmd, "help") == 0) {
		writeMsg(&usart2comm, "\n");
		writeMsg(&usart2comm, "Help menus CMDs:\n");
		writeMsg(&usart2comm, "1) help ----	print help menu\n");
		writeMsg(&usart2comm, "2) SetMCO ---Cambia el reloj del MCO\n");
		writeMsg(&usart2comm, "3) SetMCOPre --- configura el prescaler del MCO\n");
		writeMsg(&usart2comm, "4) setHour #Hour #Min #Seg ---- set the time\n");
		writeMsg(&usart2comm, "5) gethour # ---- read current hour #(us) \n");
		writeMsg(&usart2comm, "6) setDate #Day #Month Year ---- set the date \n");
		writeMsg(&usart2comm, "7) getDate ---- read date current date\n");
		writeMsg(&usart2comm, "8) setADCSS ---- configurar la velocidad de muestreo \n");
		writeMsg(&usart2comm, "9) getADCData ---- muestra el arreglo de datos \n");
		writeMsg(&usart2comm, "10) setDataAcc ---- captura datos del acelerometro \n");
		writeMsg(&usart2comm, "11) getDataFFT ---- presenta la frecuencia del acelerometro FFT\n");

	} else if (strcmp(cmd, "setMCO") == 0) {

	} else if (strcmp(cmd, "setMCOPre") == 0) {

	} else if (strcmp(cmd, "setDate") == 0) {
		writeMsg(&usart2comm, bufferData);
		handlerRTC.RTC_Config.RTC_ValueDay = (unsigned int) firstParameter;
		handlerRTC.RTC_Config.RTC_Month = (unsigned int) secondParameter;
		handlerRTC.RTC_Config.RTC_Year = (unsigned int) thirdParameter;
		RTC_Config(&handlerRTC);

		sprintf(bufferData, "la fecha es %u/%u/%u", (unsigned int) firstParameter, secondParameter, thirdParameter);
		sprintf(bufferData, "\n\rSe actualizo la fecha \n\rdia:%u  mes:%u  ano:%u\n\r", (unsigned int) firstParameter, secondParameter,thirdParameter + 2000);

	} else if (strcmp(cmd, "setHour") == 0) {
		handlerRTC.RTC_Config.RTC_Hours = (unsigned int) firstParameter;
		handlerRTC.RTC_Config.RTC_Minutes = (unsigned int) secondParameter;
		handlerRTC.RTC_Config.RTC_Seconds = (unsigned int) thirdParameter;
		RTC_Config(&handlerRTC);
		sprintf(bufferData, "la hora es %u:%u:%u", (unsigned int) firstParameter, secondParameter, thirdParameter);
		sprintf(bufferData, "\n\rSe actualizo la hora \n\rhora:%u  minuto:%u  segundo:%u\n\r", (unsigned int) firstParameter, secondParameter, thirdParameter + 2000);
		writeMsg(&usart2comm, bufferData);

	} else if (strcmp(cmd, "getDate") == 0) {
		date = read_date();
		secs = date[0];
		mins = date[1];
		hours = date[2];
		day = date[4];
		month = date[5];
		year = date[6];
		sprintf(bufferData, "\n\rLa fecha actual es: \n\rlas %u:%u:%u del %u/%u/%u\n\r", (unsigned int) hours, mins, secs, day, month, year);
		writeMsg(&usart2comm, bufferData);
		sprintf(bufferData, "%u/%u/%u", (unsigned int) day, month, year);
		sprintf(bufferData, "%u:%u:%u", (unsigned int) hours, mins, secs);

	}

	else{
		//sprintf(bufferData, "Comando no reconocido\n\r");
		//writeMsg(&usart2comm, bufferData);
		//echo, envia lo que recibe
		sprintf(bufferData, "Recibido el Char = %c \n", bufferReception);
		writeMsg(&usart2comm, bufferData);

	}

}


