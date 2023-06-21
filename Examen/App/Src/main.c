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

uint32_t maxIndex;
float32_t maxValue;

#define ACCEL_ADDRESS          	 0x1D
#define POWER_CTL                45

#define ACCEL_ZOUT_L             54     //DATAZ0
#define ACCEL_ZOUT_H             55     //DATAZ1
#define DATA_SIZE				 512
//Blinky
GPIO_Handler_t handlerBlinkyPin = {0};	//PH1
uint8_t counter = 0;



/* Elemento para hacer la comunicacion serial */
GPIO_Handler_t handlerPinTX = {0};
GPIO_Handler_t handlerPinRX = {0};
USART_Handler_t usart1comm = {0};
PLL_Handler_t handlerPll = {0};


BasicTimer_Handler_t handlerBlinkyTimer	= {0};  //TIM2
BasicTimer_Handler_t handlerSampleTimer   = {0};     //TIM4

uint8_t rxData				 = 0;
char    bufferReception[64]  = {0};
uint8_t counterReception = 0;
char   	bufferData [64]  = {0};

//muestreo
uint8_t flagSample = 0;
uint16_t numberSample = 0;
uint8_t usart1DataReceived = 0;
uint16_t fftSize 		   = 512;
float32_t transformedSignal[DATA_SIZE];

float arrayZ[512]         ={0};


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


//Parametros
unsigned int 	firstParameter = 0;
unsigned int 	secondParameter = 0;
unsigned int	thirdParameter = 0;
char 			userMsg[64] = {0};
char 			cmd[64]; // ayuda para analizar comandos

//Elementos necesarios para configurar el ADC
ADC_Config_t  adcConfig       = {0};
bool          adcComplete  	  = false;
BasicTimer_Handler_t  handlerADC = {0};
uint8_t 	  flagADC		  = 0;

//callback ADC
uint8_t counterADC = 0;
float32_t adcDataCh1[256] = {0};
float32_t adcDataCh2[256] = {0};
uint8_t counterData = 0;
int16_t coord[2] = {0};


//PWM
PWM_Handler_t handlerPWM = {0};

//FFT
//float transformedSignal[500]  = {0};
uint32_t ifftFlag = 0;
arm_rfft_fast_instance_f32 config_Rfft_fast_f32;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;


// Definicion de las cabeceras de las funciones del main
void init_system(void);
void parseCommands (char *ptrBufferReception);

void createSignal(void);

////Elementos para generar una señal
//#define SINE_DATA_SIZE    4096   			//Tamaño del arrlego de datos
//float32_t fs =  8000.0;					//Frecuencia de muestreo
//float32_t f0 =	250.0;					//Frecuencia fundamental de la señal
//float32_t dt =	0;						//Periodo de muestreo, en este caso sera (1/fs)
//float32_t stopTime = 1.0;				//Quizas no sea necesario
//float32_t amplitud = 5;					//Amplitud de la señal generada
//float32_t sineSignal[SINE_DATA_SIZE];
//float32_t transformedSignal[SINE_DATA_SIZE];
//float32_t* ptrSineSingal;



int main(void){
	//Activamos el coprocesador matematico
	SCB->CPACR |= (0xF <<20);

	//Inicializamos todos los elementos del sistema
	init_system();

	writeMsg(&usart1comm, "iniciando \n");
	writeMsg(&usart1comm, "\n ....................................................................................................\n"
			"\n..////////////////////////////////  ..  .  .  .. ..  .  .  .. ..  ,///////////////////////////////,.\n"
			"\n..  .  .@@@@@@@@@@@@@@@@@@@@@@@@@@@ ..  .  .  .@ ..  @  .  .. ..  @@@@@@@@@@@@@@@@@@@@@@@@@@@  .  ..\n"
			"\n  .  ..  . )@@@@@@@@@@@@@@@@@@@@@@@@, .  ..  . @@@@@@@..  .  .  |@@@@@@@@@@@@@@@@@@@@@@@@(..  .  .  \n"
			"\n  .  ..  .  . @@@@@@@@@@@@@@@@@@@@@@@@@@@&|, |@@@@@@@@@|, |@@@@@@@@@@@@@@@@@@@@@@@@@@@@.  ..  .  .  \n"
			"\n..  .  .  ..  .@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@,.  .  .  .  ..\n"
			"\n..  .  .  ..  .@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@,.  .  .  .  ..\n"
			"\n..  .  .  ..  . )@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(..  .  .  .  ..\n"
			"\n..  .  .  ..  . )@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(..  .  .  .  ..\n"
			"\n  .  ..  .  .  . @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   .  .   .  .  \n"
			"\n  .  ..  .  .  . (@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@).  .  ..  .  .  \n"
			"\n                                    &@@@@@@@@@@@@@@@@@@@@@@@@@@@&.                                  \n"
			"\n..  .  .  ..  .  .  .  ..  .  .  .  ..  ..@@@@@@@@@@@@@@@@@ . ..  .  .  .. ..  .  .  ..  .  .  .  ..\n"
			"\n                                             @@@@@@@@@@@                                            \n"
			"\n  .  ..  .  .  .   .  .  .  ..  .  .  .  ..  . @@@@@@@...  .  .  .  ..  .  .  .   .  .  .  ..  .  .  \n"
			"\n  .  ..  .  .  .  ..  .  .  ..  .  .  .  ..  .  .@@@  ..  .  .  .  ..  .  .  .  ..  .  .  ..  .  .  \n"
			"\n..  .  .  ..  .  .  .  ..  .  .  .  ..  .  .  .. .@  .  .  .. ..  .  .  .. ..  .  .  ..  .  .  .  ..\n"
			"\n..  .  .  ..  .  .  . . . .. .. .  ..  . EL SEÑOR DE LA NOCHE.. . .. .. .. ..  .  .  ..  .  .  .  ..\n");
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

	handlerSampleTimer.ptrTIMx                             				= TIM4;
	handlerSampleTimer.TIMx_Config.TIMx_mode                			= BTIMER_MODE_UP ;
	handlerSampleTimer.TIMx_Config.TIMx_speed               			= BTIMER_SPEED_100MHz_100us;
	handlerSampleTimer.TIMx_Config.TIMx_period              			= 50;
	handlerSampleTimer.TIMx_Config.TIMx_interruptEnable     			= BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerSampleTimer);


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
	handlerCLK.GPIO_PinConfig.GPIO_PinAltFunMode  = AF0;
	GPIO_Config(&handlerCLK);

/*======================================== ADC =============================================*/

	adcConfig.channels[0]  	 	= ADC_CHANNEL_0;
	adcConfig.channels[1]   	= ADC_CHANNEL_1;
	adcConfig.dataAlignment     = ADC_ALIGNMENT_RIGHT;
	adcConfig.resolution        = ADC_RESOLUTION_12_BIT;
	adcConfig.samplingPeriod    = ADC_SAMPLING_PERIOD_28_CYCLES;
	adcConfig.adcData			= 11;
	//Se carga la configuracion multichannel
	ADC_ConfigMultichannel(&adcConfig, 2);




/*  ====================================== PWM =============================================*/

	handlerPWM.ptrTIMx											= TIM5;
	handlerPWM.config.channel									= PWM_CHANNEL_1;
	handlerPWM.config.duttyCicle								= PWM_DUTTY_100_PERCENT;
	handlerPWM.config.periodo									= 220;
	handlerPWM.config.prescaler									= 50;

/* ===================== Rutinas de atencion o callbacks ===============================================*/
}
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
	// se almacena en un arreglo ambos valores y se controla el orden por medio de un counter2
	if (flagADC){
		if(counterADC == 0){
			adcDataCh1[counterData] = (float) getADC();
		}
		else{
			adcDataCh2[counterData] = (float) getADC();
			counterData++;
		}
		counterADC++;
		if(counterData > 256){
			counterData = 0;
			flagADC = 0;
		}
		if(counterADC == 2){
			counterADC = 0;
		}
	}


}
void BasicTimer3_Callback(void){

	usart1DataReceived = getRxData();
}

// funcion que administra las entradas por consola del PC
void parseCommands (char *ptrBufferReception){
	// Esta función lee la cadena de caracteres a lo que apunta el 'ptr' y la divide
	// y almacena en tres elementos diferentes: Un string llamado 'cmd', y dos números
	// integer llamados 'firstParameter' y 'secondparameter', de esta forma, es posible
	// introducir información al micro desde el puerto serial.
	sscanf(ptrBufferReception, "%s %u %u %u %s", cmd, &firstParameter,
			&secondParameter,&thirdParameter, userMsg);
	// Este primer comando imprime una lista con los otros comandos que tiene el equipo.
	if (strcmp(cmd, "help") == 0) {
		writeMsg(&usart1comm, "\n");
		writeMsg(&usart1comm, "Help menus CMDs:\n");
		writeMsg(&usart1comm, "1) help ----	print help menu\n");
		writeMsg(&usart1comm, "2) SetMCO ---Cambia el reloj del MCO\n");
		writeMsg(&usart1comm, "3) SetMCOPre --- configura el prescaler del MCO\n");
		writeMsg(&usart1comm, "4) setTime #HH #MM #SS ----Configura la fecha\n");
		writeMsg(&usart1comm, "5) getTime  ---- Pesenta la hora actual \n");
		writeMsg(&usart1comm, "6) setDate #DD #MM YY ---- congigura la fecha \n");
		writeMsg(&usart1comm, "7) getDate ---- Presenta la fecha actual\n");
		writeMsg(&usart1comm, "8) setADCSS ---- configurar la velocidad de muestreo (No entregado)\n");
		writeMsg(&usart1comm, "9) getADC ---- muestra el arreglo de datos \n");
		writeMsg(&usart1comm, "10) getAcc---- captura datos del acelerometro \n");
		writeMsg(&usart1comm, "11) FFT ---- presenta la frecuencia del acelerometro FFT\n");
		writeMsg(&usart1comm, "12) special ---- comando especial ya que el resto no dio bien");

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


	}
	else if (strcmp(cmd, "setMCOPre") == 0) {
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



	}
	else if (strcmp(cmd, "setTime") == 0) {
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


		}
	else if (strcmp(cmd, "getTime") == 0) {
			date 	= read_date();
			secs 	= date[0];
			mins 	= date[1];
			hours 	= date[2];
			sprintf(bufferData, "\n\rLa hora actual es: \n\rlas %u:%u:%u\n\r", (unsigned int) hours, mins, secs);
			writeMsg(&usart1comm, bufferData);
			sprintf(bufferData, "%u:%u:%u", (unsigned int) hours, mins, secs);

		}
	else if (strcmp(cmd, "setDate") == 0) {
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
			sprintf(bufferData, "\nSe actualizo la fecha \n\rdia:%u  mes:%u  año:%u\n", firstParameter, secondParameter, thirdParameter+2000);

		}

	}
	else if (strcmp(cmd,"getDate") == 0){
		date = read_date();
		day = date[4];
		month = date[5];
		year = date[6];
		sprintf(bufferData, "\n\rLa fecha actual es: \n%u/%u/%u\n", (unsigned int) day,month,year+2000);
		writeMsg(&usart1comm, bufferData);

	}

	else if (strcmp(cmd,"getADC") == 0){
		flagADC = 1;
		for (int c = 0; c < 256; c++) {
			sprintf(bufferData, "canal 1 = %.4f   | canal 2 = %.4f | %u\n",adcDataCh1[c],adcDataCh2[c],c);
			writeMsg(&usart1comm, bufferData);
		}
	}

	else if (strcmp(cmd, "getAcc") == 0){
		flagSample = 1;
		numberSample = 0;
		//numberSample++;
		i2c_writeSingleRegister(&handlerAccelerometer, POWER_CTL , 0x2D);
		while (numberSample < 512) {
			uint8_t AccelZ_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_L);
			uint8_t AccelZ_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_H);
			int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;

			arrayZ[numberSample] = (AccelZ / 256.f) * 9.78;

		}

		flagSample = 0;
		numberSample = 0;

		for (int i = 0; i < 512; i++) {
			sprintf(bufferData, "Z = %.4f   | %u \n", arrayZ[i], i);
			writeMsg(&usart1comm, bufferData);
		}

	}
	else if (strcmp(cmd,"FFT") == 0){
		int j = 0;
		int k = 0;
		statusInitFFT = arm_rfft_fast_init_f32(&config_Rfft_fast_f32, fftSize);
		if(statusInitFFT == ARM_MATH_SUCCESS){
			sprintf(bufferData, "Initialization.... SUCESS!\n");
			writeMsg(&usart1comm, bufferData);

			arm_rfft_fast_f32(&config_Rfft_fast_f32,arrayZ, transformedSignal, ifftFlag);
			arm_abs_f32(transformedSignal, arrayZ, fftSize);

		}

			for( j = 1; j < fftSize; j++){
				if(j % 2){
					sprintf(bufferData, "%u ; %#.4f\n", k, 2*arrayZ[j]);
					writeMsg(&usart1comm, bufferData);
					k++;
				}
			}

			// Encontrar el índice y el valor máximo en el espectro de frecuencia
			float32_t maximo[1] = {0};
			int l = 0;
			for(l = 1; l< fftSize; l++ ){
				if (transformedSignal[l]> maximo[0]){
					maximo[0] = transformedSignal[l];
				}
			}

			float32_t sampleRate = 200.0f; //frecuencia de muestreo
			float32_t dominantFreq = (maximo[0] *sampleRate ) / 512;

			sprintf(bufferData,"dominante :%.2fHz | maximo:%f\n",dominantFreq, maximo[0]);
			writeMsg(&usart1comm, bufferData);

	}

	else if (strcmp(cmd, "special") == 0) {
		writeMsg(&usart1comm, "\n ....................................................................................................\n"
				"\n..////////////////////////////////  ..  .  .  .. ..  .  .  .. ..  ,///////////////////////////////,.\n"
				"\n..  .  .@@@@@@@@@@@@@@@@@@@@@@@@@@@ ..  .  .  .@ ..  @  .  .. ..  @@@@@@@@@@@@@@@@@@@@@@@@@@@  .  ..\n"
				"\n  .  ..  . )@@@@@@@@@@@@@@@@@@@@@@@@, .  ..  . @@@@@@@..  .  .  |@@@@@@@@@@@@@@@@@@@@@@@@(..  .  .  \n"
				"\n  .  ..  .  . @@@@@@@@@@@@@@@@@@@@@@@@@@@&|, |@@@@@@@@@|, |@@@@@@@@@@@@@@@@@@@@@@@@@@@@.  ..  .  .  \n"
				"\n..  .  .  ..  .@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@,.  .  .  .  ..\n"
				"\n..  .  .  ..  .@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@,.  .  .  .  ..\n"
				"\n..  .  .  ..  . )@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(..  .  .  .  ..\n"
				"\n..  .  .  ..  . )@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@(..  .  .  .  ..\n"
				"\n  .  ..  .  .  . @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   .  .   .  .  \n"
				"\n  .  ..  .  .  . (@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@).  .  ..  .  .  \n"
				"\n                                    &@@@@@@@@@@@@@@@@@@@@@@@@@@@&.                                  \n"
				"\n..  .  .  ..  .  .  .  ..  .  .  .  ..  ..@@@@@@@@@@@@@@@@@ . ..  .  .  .. ..  .  .  ..  .  .  .  ..\n"
				"\n                                             @@@@@@@@@@@                                            \n"
				"\n  .  ..  .  .  .   .  .  .  ..  .  .  .  ..  . @@@@@@@...  .  .  .  ..  .  .  .   .  .  .  ..  .  .  \n"
				"\n  .  ..  .  .  .  ..  .  .  ..  .  .  .  ..  .  .@@@  ..  .  .  .  ..  .  .  .  ..  .  .  ..  .  .  \n"
				"\n..  .  .  ..  .  .  .  ..  .  .  .  ..  .  .  .. .@  .  .  .. ..  .  .  .. ..  .  .  ..  .  .  .  ..\n"
				"\n..  .  .  ..  .  .  . . . .. .. .  ..  . EL SEÑOR DE LA NOCHE.. . .. .. .. ..  .  .  ..  .  .  .  ..\n");

	}

	else{
		sprintf(bufferData, "Comando no reconocido\n\r");
		writeMsg(&usart1comm, bufferData);
	}

}


