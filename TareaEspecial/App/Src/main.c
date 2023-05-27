/*
 * ExtiConfig_main.c
 *
 *  Created on: Mar 24, 2023
 *      Author: druiz
 */
#include <stdlib.h>
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
#include "PllDriver.h"

#define ACCEL_ADDRESS          	 0x1D
#define ACCEL_XOUT_L             50     //DATAX0
#define ACCEL_XOUT_H             51     //DATAX1
#define ACCEL_YOUT_L             52     //DATAYO
#define ACCEL_YOUT_H             53     //DATAY1
#define ACCEL_ZOUT_L             54     //DATAZ0
#define ACCEL_ZOUT_H             55     //DATAZ1

#define POWER_CTL                45
#define WHO_AM_I                 0      //DEVID


void init_system(void);

uint8_t flagBK = 0;

//Definicion del timer del led
BasicTimer_Handler_t handlerBlinkyTimer  = {0};

//definicion del blinky del led
GPIO_Handler_t handlerLEDBlinky    = {0};

//Congifuracion del usart
uint8_t printMsg = 0;
USART_Handler_t Usart2Comm = {0};

//cambio de velocidad del pll
PLL_Handler_t pllFreq80MHz  = {0};

//Variables para acelerometro
GPIO_Handler_t handlerPinTX = {0};
GPIO_Handler_t handlerPinRX = {0};

BasicTimer_Handler_t handlerStateOKTimer = {0};

uint8_t rxData = 0;
char bufferData[64] = "Accel MPU test";

//configuracion para el I2C
GPIO_Handler_t handlerI2cSDA  = {0};
GPIO_Handler_t handlerI2cSCL  = {0};
I2C_Handler_t handlerAccelerometer = {0};
uint8_t i2cBuffer = 0;


int main(void){
	//inicialización de todos los elementos del sistema
	init_system();

	writeMsg(&Usart2Comm, bufferData);

	while(1){
		if(rxData != '\0'){
			sprintf(bufferData, "%c", rxData);
			writeMsg(&Usart2Comm, bufferData);
			rxData = '\0';
		}
		//writeMsg(&Usart2Comm, "hola");

		/*if(rxData != '\0'){
			writeChar(&Usart2Comm, rxData);
			if(rxData == 'w'){
				sprintf(bufferData, "WHO_AM_I? (r)\n");
				writeMsg(&Usart2Comm, bufferData);

				i2cBuffer = i2c_readSingleRegister(&handlerAccelerometer, WHO_AM_I);
				sprintf(bufferData, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
				writeMsg(&Usart2Comm, bufferData);
				rxData = '\0';
			}
			else if (rxData == 'p'){
				sprintf(bufferData, "POWER_CTL state (r)\n");
				writeMsg(&Usart2Comm, bufferData);

				i2cBuffer = i2c_readSingleRegister(&handlerAccelerometer, POWER_CTL);
				sprintf(bufferData, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
				writeMsg(&Usart2Comm, bufferData);
				rxData = '\0';
			}
			else if (rxData == 'r'){
				sprintf(bufferData, "POWER_CTL reset (w)\n");
				writeMsg(&Usart2Comm, bufferData);

				i2c_writeSingleRegister(&handlerAccelerometer, POWER_CTL , 0x2D);
					rxData = '\0';
			}
			else if (rxData == 'x'){
				sprintf(bufferData, "Axis X data (r) \n");
				writeMsg(&Usart2Comm, bufferData);

				uint8_t AccelX_low =  i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_L);
				uint8_t AccelX_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_H);
				int16_t AccelX = AccelX_high << 8 | AccelX_low;
				sprintf(bufferData, "AccelX = %.2f \n", (float) (AccelX/256.f)*9.78);
				writeMsg(&Usart2Comm, bufferData);
				rxData = '\0';
			}
			else if(rxData == 'y'){
				sprintf(bufferData, "Axis Y data (r)\n");
				writeMsg(&Usart2Comm, bufferData);
				uint8_t AccelY_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_YOUT_L);
				uint8_t AccelY_high = i2c_readSingleRegister(&handlerAccelerometer,ACCEL_YOUT_H);
				int16_t AccelY = AccelY_high << 8 | AccelY_low;
				sprintf(bufferData, "AccelY = %.2f \n", (AccelY/256.f)*9.78);
				writeMsg(&Usart2Comm, bufferData);
				rxData = '\0';
			}
			else if(rxData == 'z'){
				sprintf(bufferData, "Axis Z data (r)\n");
				writeMsg(&Usart2Comm, bufferData);

				uint8_t AccelZ_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_L);
				uint8_t AccelZ_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_H);
				int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
				sprintf(bufferData, "AccelZ = %.2f \n",  (AccelZ/256.f)*9.78);
				writeMsg(&Usart2Comm, bufferData);
				rxData = '\0';
			}
			else{
				rxData = '\0';
			}
		}
	return(0);*/
	}
}
//Fin del Main
/*
		if (printMsg> 4){
			writeChar(&Usart2Comm,'H');
			writeChar(&Usart2Comm,'O');
			writeChar(&Usart2Comm,'L');
			writeChar(&Usart2Comm,'A');

			printMsg = 0;
		}

		printMsg++;

*/
void init_system(void){

	// Se configura el PA5 para el LED2
	handlerLEDBlinky.pGPIOx = GPIOA;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinNumber = PIN_5;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinMode   = GPIO_MODE_OUT;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinOPType  = GPIO_OTYPE_PUSHPULL;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinSpeed  = GPIO_OSPEED_FAST;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	// Configuracion del LED2 en el registro
	GPIO_Config(&handlerLEDBlinky);


	//configurar el TIM2 para que haga una interrupcion cada 250ms
	handlerBlinkyTimer.ptrTIMx 								= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode                = BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed				= BTIMER_SPEED_80MHz_100us;
	handlerBlinkyTimer.TIMx_Config.TIMx_period				= 2500; //cada 250 ms
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable		= BTIMER_INTERRUPT_ENABLE;

	BasicTimer_Config(&handlerBlinkyTimer);
	handlerPinTX.pGPIOx 								 = GPIOA;
	handlerPinTX.GPIO_PinConfig.GPIO_PinNumber           = PIN_2;
	handlerPinTX.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_ALTFN;
	handlerPinTX.GPIO_PinConfig.GPIO_PinAltFunMode       = AF7;
	GPIO_Config(&handlerPinTX);

	//PINRX del usart para Usart6(PA12 AF8) Para Usart2(PA3 AF7)
	handlerPinRX.pGPIOx 								 = GPIOA;
	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber           = PIN_3;
	handlerPinRX.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_ALTFN;
	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode       = AF7;
	GPIO_Config(&handlerPinRX);
	//cuando el micro este a 80MHz deben usarse los baudrate terminados en _80MHz
	Usart2Comm.ptrUSARTx                                     = USART2;
	Usart2Comm.USART_Config.USART_baudrate                   = USART_BAUDRATE_80MHz_115200;
	Usart2Comm.USART_Config.USART_datasize                   = USART_DATASIZE_8BIT;
	Usart2Comm.USART_Config.USART_mode                       = USART_MODE_RXTX;
	Usart2Comm.USART_Config.USART_parity                     = USART_PARITY_NONE;
	Usart2Comm.USART_Config.USART_stopbits                   = USART_STOPBIT_1;
	Usart2Comm.USART_Config.USART_enableIntTX                = USART_TX_INTERRUPT_DISABLE;
	Usart2Comm.USART_Config.USART_enableIntRX                = USART_RX_INTERRUPT_DISABLE;

	USART_Config(&Usart2Comm);

	//se carga la configuracion del PLL a 80MHz
	pllFreq80MHz.clkSpeed                                    = FREQUENCY_80MHz;
	configPLL(&pllFreq80MHz);

	//sda y SCL cualquiera disponible, SDO, CS y VCC van a 3.3 volts y ground

	//configuracion delos pines del I2C
	handlerI2cSDA.pGPIOx                                   = GPIOB;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinNumber            = PIN_9;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinMode              = GPIO_MODE_ALTFN;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinOPType            = GPIO_OTYPE_OPENDRAIN;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinPuPdControl       = GPIO_PUPDR_PULLUP;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinSpeed             = GPIO_OSPEED_FAST;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinAltFunMode        = AF4;

	GPIO_Config(&handlerI2cSDA);

	//modo SCL
	handlerI2cSCL.pGPIOx                                   = GPIOB;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinNumber            = PIN_8;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinMode              = GPIO_MODE_ALTFN;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinOPType            = GPIO_OTYPE_OPENDRAIN;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinPuPdControl       = GPIO_PUPDR_PULLUP;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinSpeed             = GPIO_OSPEED_FAST;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinAltFunMode        = AF4;

	GPIO_Config(&handlerI2cSCL);

	handlerAccelerometer.ptrI2Cx           = I2C1;
	handlerAccelerometer.modeI2C           = I2C_MODE_FM;
	handlerAccelerometer.slaveAddress      = ACCEL_ADDRESS;

	i2c_config(&handlerAccelerometer);

}

// Configuracion del Timer del blinky
void BasicTimer2_Callback(void){
	GPIOxTogglePin(&handlerLEDBlinky);
}

void usart2RX_Callback (void){
	rxData = getRxData();
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

