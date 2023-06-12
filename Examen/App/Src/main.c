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
#include "AdcDriver.h"

#include "arm_math.h"
#include <math.h>

#define ACCEL_ADDRESS          	 0x2D
#define ACCEL_XOUT_L             50     //DATAX0
#define ACCEL_XOUT_H             51     //DATAX1
#define ACCEL_YOUT_L             52     //DATAYO
#define ACCEL_YOUT_H             53     //DATAY1
#define ACCEL_ZOUT_L             54     //DATAZ0
#define ACCEL_ZOUT_H             55     //DATAZ1

//Blinky
GPIO_Handler_t handlerBlinkyPin = {0};	//PH1
uint8_t counter = 0;


/* Elemento para hacer la comunicacion serial */
GPIO_Handler_t handlerPinTX = {0};
GPIO_Handler_t handlerPinRX = {0};
USART_Handler_t usart1comm = {0};
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
float arrrayx[256]                     ={0};
float arrrayY[256]                     ={0};
float arrrayZ[256]                     ={0};

//MCO1
pll_MCO1 handlerMCO1 = {0};


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
GPIO_Handler_t handlerCLK = {0};

uint8_t flagample 		={0};

//Parametros
unsigned int 	firstParameter = 0;
unsigned int 	secondParameter = 0;
unsigned int	thirdParameter = 0;
char 			userMsg[64] = {0};
char 			cmd[64]; // ayuda para analizar comandos

//Elementos necesarios para configurar el ADC
ADC_Config_t  adcConfig       = {0};
int16_t       adcData         = 0;
bool          adcComplete  	  = false;
BasicTimer_Handler_t  handlerADC = {0};
uint8_t 	  flagADC		  = 0;

//callback ADC
uint8_t contador = 0;
int16_t coordinates[2] = {0};

// Definicion de las cabeceras de las funciones del main
void init_system(void);
void parseCommands (char *ptrBufferReception);


int main(void){
	//Activamos el coprocesador matematico
	SCB->CPACR |= (0xF <<20);

	//Inicializamos todos los elementos del sistema
	init_system();

	writeMsg(&usart1comm, "iniciando \n");
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

	usart1comm.ptrUSARTx											= USART1;
	usart1comm.USART_Config.USART_baudrate							= USART_BAUDRATE_100MHz_115200;
	usart1comm.USART_Config.USART_datasize							= USART_DATASIZE_8BIT;
	usart1comm.USART_Config.USART_parity							= USART_PARITY_NONE;
	usart1comm.USART_Config.USART_stopbits							= USART_STOPBIT_1;
	usart1comm.USART_Config.USART_mode								= USART_MODE_RXTX;
	usart1comm.USART_Config.USART_enableIntRX						= USART_RX_INTERRUPT_ENABLE;
	usart1comm.USART_Config.USART_enableIntTX						= USART_TX_INTERRUPT_DISABLE;
	USART_Config(&usart1comm);


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


	handlerCLK.pGPIOx = GPIOA;
	handlerCLK.GPIO_PinConfig.GPIO_PinNumber = PIN_8;
	handlerCLK.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerCLK.GPIO_PinConfig.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerCLK.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	handlerCLK.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerCLK.GPIO_PinConfig.GPIO_PinAltFunMode = AF0;
	GPIO_Config(&handlerCLK);

/*======================================== ADC =============================================*/

	adcConfig.channels[0]  	 	= ADC_CHANNEL_0;
	adcConfig.channels[1]   	= ADC_CHANNEL_1;
	adcConfig.dataAlignment     = ADC_ALIGNMENT_RIGHT;
	adcConfig.resolution        = ADC_RESOLUTION_12_BIT;
	adcConfig.samplingPeriod    = ADC_SAMPLING_PERIOD_28_CYCLES;

	//Se carga la configuracion multichannel
	ADC_ConfigMultichannel(&adcConfig, 2);


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

/*Esta función se ejecuta luego de una conversión ADC
 * (es llamada por la ISR de la conversión ADC)
 */
void adcComplete_Callback(void){
	// Tomamos el valor de la conversión ADC realizada con getADC(),
	// se almacena en un arreglo ambos valores y se controla el orden por medio de un contador
	if(contador){
		adcData = (getADC()-2045)*(-1);
		contador = 0;
		coordinates[1]=adcData;
	}else{
		contador = 1;
		adcData = getADC()-2045;
		coordinates[0]=adcData;
	}
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
	// Este primer comando imprime una lista con los otros comandos que tiene el equipo.
	if (strcmp(cmd, "help") == 0) {
		writeMsg(&usart1comm, "\n");
		writeMsg(&usart1comm, "Help menus CMDs:\n");
		writeMsg(&usart1comm, "1) help ----	print help menu\n");
		writeMsg(&usart1comm, "2) SetMCO ---Cambia el reloj del MCO\n");
		writeMsg(&usart1comm, "3) SetMCOPre --- configura el prescaler del MCO\n");
		writeMsg(&usart1comm, "4) setTime #Hour #Min #Seg ---- set the time\n");
		writeMsg(&usart1comm, "5) getTime # ---- read current hour #(us) \n");
		writeMsg(&usart1comm, "6) setDate #Day #Month Year ---- set the date \n");
		writeMsg(&usart1comm, "7) getDate ---- read date current date\n");
		writeMsg(&usart1comm, "8) setADCSS ---- configurar la velocidad de muestreo \n");
		writeMsg(&usart1comm, "9) getADCData ---- muestra el arreglo de datos \n");
		writeMsg(&usart1comm, "10) setAcc ---- captura datos del acelerometro \n");
		writeMsg(&usart1comm, "11) getDataFFT ---- presenta la frecuencia del acelerometro FFT\n");

	} else if (strcmp(cmd, "setMCO") == 0) {
		/*
		 * Para HSI: 0
		 * Para LSE: 1
		 * Para PLL: 2
		 */
		if (firstParameter == 0) {
			handlerMCO1.clk = HSI;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilita el reloj HSI");
			writeMsg(&usart1comm, bufferData);
		}
		else if (firstParameter == 1) {
			handlerMCO1.clk = LSE;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilita el reloj LSE");
			writeMsg(&usart1comm, bufferData);
		}

		else if (firstParameter == 2) {
			handlerMCO1.clk = PLL;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilita el reloj PLL");
			writeMsg(&usart1comm, bufferData);
		}


	} else if (strcmp(cmd, "setMCOPre") == 0) {
		if (firstParameter == 1){
			handlerMCO1.presc = presc1;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilita Prescaler 1");
			writeMsg(&usart1comm, bufferData);
		}
		else if(firstParameter == 2){
			handlerMCO1.presc = presc2;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilitó prescarler 2");
			writeMsg(&usart1comm, bufferData);
		}
		else if(firstParameter == 3){
			handlerMCO1.presc = presc3;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilitó prescarler 3");
			writeMsg(&usart1comm, bufferData);
		}
		else if(firstParameter == 4){
			handlerMCO1.presc = presc4;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilitó prescarler 4");
			writeMsg(&usart1comm, bufferData);
		}
		else if(firstParameter == 5){
			handlerMCO1.presc = presc5;
			MCOConfig(&handlerMCO1);
			sprintf(bufferData, "Se habilitó prescarler 5");
			writeMsg(&usart1comm, bufferData);
		}



	} else if (strcmp(cmd, "setDate") == 0) {
		if (firstParameter > 30 || secondParameter > 12 || thirdParameter > 99){
			sprintf(bufferData, "Fuera de rango");
			writeMsg(&usart1comm, bufferData);
		}
		else{
			writeMsg(&usart1comm, bufferData);
			handlerRTC.RTC_Config.RTC_ValueDay = (unsigned int) firstParameter;
			handlerRTC.RTC_Config.RTC_Month = (unsigned int) secondParameter;
			handlerRTC.RTC_Config.RTC_Year = (unsigned int) thirdParameter;
			RTC_Config(&handlerRTC);
			sprintf(bufferData, "la fecha es %u/%u/%u", (unsigned int) firstParameter, secondParameter, thirdParameter);
			sprintf(bufferData, "\nSe actualizo la fecha \n\rdia:%u  mes:%u  año:%u\n", firstParameter, secondParameter, thirdParameter);

		}

	} else if (strcmp(cmd, "setTime") == 0) {
		if(firstParameter > 12 || secondParameter >59 || thirdParameter > 59){
			sprintf(bufferData, "Fuera de rango");
			writeMsg(&usart1comm, bufferData);
		}
		else{
			handlerRTC.RTC_Config.RTC_Hours = (unsigned int) firstParameter;
			handlerRTC.RTC_Config.RTC_Minutes = (unsigned int) secondParameter;
			handlerRTC.RTC_Config.RTC_Seconds = (unsigned int) thirdParameter;
			RTC_Config(&handlerRTC);
			sprintf(bufferData, "la hora es %u:%u:%u",(unsigned int) firstParameter, secondParameter, thirdParameter + 2000);
			sprintf(bufferData, "\n\rSe actualizo la hora \n\rhora:%u  minuto:%u  segundo:%u\n\r", firstParameter, secondParameter, thirdParameter);
			writeMsg(&usart1comm, bufferData);
		}


	} else if (strcmp(cmd, "getTime") == 0) {
		date 	= read_date();
		secs 	= date[0];
		mins 	= date[1];
		hours 	= date[2];
		day 	= date[4];
		month 	= date[5];
		year 	= date[6];
		sprintf(bufferData, "\n\rLa fecha actual es: \n\rlas %u:%u:%u\n\r", (unsigned int) hours, mins, secs);
		writeMsg(&usart1comm, bufferData);
		sprintf(bufferData, "%u:%u:%u", (unsigned int) hours, mins, secs);

	}
//
//	else if (strcmp(cmd, "setAcc")){
//		flagSample = 1;
//		numberSample = 0;
//		while (numberSample < 256) {
//			uint8_t AccelX_low = i2c_readSingleRegister(
//					&handlerAccelerometer, ACCEL_XOUT_L);
//			uint8_t AccelX_high = i2c_readSingleRegister(
//					&handlerAccelerometer, ACCEL_XOUT_H);
//			int16_t AccelX = AccelX_high << 8 | AccelX_low;
//
//			uint8_t AccelY_low = i2c_readSingleRegister(
//					&handlerAccelerometer, ACCEL_YOUT_L);
//			uint8_t AccelY_high = i2c_readSingleRegister(
//					&handlerAccelerometer, ACCEL_YOUT_H);
//			int16_t AccelY = AccelY_high << 8 | AccelY_low;
//
//			uint8_t AccelZ_low = i2c_readSingleRegister(
//					&handlerAccelerometer, ACCEL_ZOUT_L);
//			uint8_t AccelZ_high = i2c_readSingleRegister(
//					&handlerAccelerometer, ACCEL_ZOUT_H);
//			int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
//
//			arrrayx[numberSample] = (AccelX / 256.f) * 9.78;
//			arrrayY[numberSample] = (AccelY / 256.f) * 9.78;
//			arrrayZ[numberSample] = (AccelZ / 256.f) * 9.78;
//
//		}
//		flagSample = 0;
//		numberSample = 0;
//		for (int i = 0; i < 256; i++) {
//			sprintf(bufferData,
//					"X = %.2f  ;  Y = %.2f  ;  Z = %.2f   | %u \n",
//					arrrayx[i], arrrayY[i], arrrayZ[i], i);
//			writeMsg(&usart1comm, bufferData);
//		}

//	}
	else{
		sprintf(bufferData, "Comando no reconocido\n\r");
		writeMsg(&usart1comm, bufferData);


	}

}


