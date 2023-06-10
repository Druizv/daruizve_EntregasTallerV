/**
 ******************************************************************************
 * @file           : main.c
 * @author         : druiz
 ******************************************************************************
 */

#include <stdint.h>
#include <stm32f4xx.h>
#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include "USARTxDriver1.h"
#include "PLLDriver.h"
#include "I2CDriver1.h"
#include "arm_math.h"
#include <math.h>



/* Definicion de los elementos del sistema */
/*Handler PLL para la velocidad del micro*/
PLL_Handler_t handlerPll                 = {0};


// Handlers de los pines
GPIO_Handler_t handlerLEDBlinky          = {0};
GPIO_Handler_t handlerpinEXTI            = {0};
GPIO_Handler_t handlerPinPrueba          = {0};


// Handlers y banderas de los timers
BasicTimer_Handler_t handlerBlinkyTimer  = {0};
BasicTimer_Handler_t handler1HzTimer     = {0};
uint8_t banderaMuestreo                  = 0;
uint16_t numeroMuestreo                   = 0;




// Extis
EXTI_Config_t Exti                       = {0};


// Comunicacion USART
USART_Handler_t Usart2Comm               = {0};
GPIO_Handler_t handlerPinTX              = {0};
GPIO_Handler_t handlerPinRX              = {0};
uint16_t printMSJ                        = 0;
uint8_t usart2DataReceived               = 0;
char bufferMsj[64]                       = {0};
char bufferMsj2[64]                      = {0};
float ArregloX[2000]                     ={0};
float ArregloY[2000]                     ={0};
float ArregloZ[2000]                     ={0};
uint8_t numeroArreglo                    = 0;





//Configuracion para el I2C
GPIO_Handler_t handleI2cSDA             = {0};
GPIO_Handler_t handleI2cSCL             = {0};
I2C_Handler_t handlerAccelerometer      = {0};
uint8_t i2cBuffer    = 0;

#define ACCEL_ADDRESS          	 0x1D
#define ACCEL_XOUT_L             50
#define ACCEL_XOUT_H             51
#define ACCEL_YOUT_L             52
#define ACCEL_YOUT_H             53
#define ACCEL_ZOUT_L             54
#define ACCEL_ZOUT_H             55

#define BW_RATE                  44

#define POWER_CTL                45
#define WHO_AM_I                 0





//Definiendo las Funciones
void init_hardware (void);


int main(void){
//Activamos el coprocesador Matematico
	SCB->CPACR |= (0xF <<20);
/*Inicialización de todos los elementos del sistema*/
	init_hardware();

	while(1){
		/*if(usart2DataReceived != '\0'){
				sprintf(bufferMsj, "%c", usart2DataReceived);
				writeMsg(&Usart2Comm, bufferMsj);
				usart2DataReceived = '\0';
		}*/
		writeMsg(&Usart2Comm, bufferMsj);

		if(usart2DataReceived != '\0'){

			writeChar(&Usart2Comm, numeroArreglo);

			if(usart2DataReceived == 'w'){

				sprintf(bufferMsj, "WHO_AM_I? (r)\n");
				writeMsg(&Usart2Comm, bufferMsj);

				i2cBuffer = i2c_readSingleRegister(&handlerAccelerometer, WHO_AM_I);
				sprintf(bufferMsj, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
				writeMsg(&Usart2Comm, bufferMsj);
				usart2DataReceived = '\0';
			}
			else if (usart2DataReceived == 'p'){
				sprintf(bufferMsj, "PWR_MGMT_1 state (r)\n");
				writeMsg(&Usart2Comm, bufferMsj);

				i2cBuffer = i2c_readSingleRegister(&handlerAccelerometer, POWER_CTL);
				sprintf(bufferMsj, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
				writeMsg(&Usart2Comm, bufferMsj);
				usart2DataReceived = '\0';
			}
			else if (usart2DataReceived == 'r'){
				sprintf(bufferMsj, "PWR_MGMT_1 reset (w)\n");
				writeMsg(&Usart2Comm, bufferMsj);

				i2c_writeSingleRegister(&handlerAccelerometer, POWER_CTL , 0x2D);
				usart2DataReceived = '\0';
			}
			else if (usart2DataReceived == 'x'){
				sprintf(bufferMsj, "Axis X data (r) \n");
				writeMsg(&Usart2Comm, bufferMsj);

				uint8_t AccelX_low =  i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_L);
				uint8_t AccelX_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_H);
				int16_t AccelX = AccelX_high << 8 | AccelX_low;
				sprintf(bufferMsj, "AccelX = %.2f \n", (AccelX/256.f)*9.78);
				writeMsg(&Usart2Comm, bufferMsj);
				usart2DataReceived = '\0';
			}
			else if(usart2DataReceived == 'y'){
				sprintf(bufferMsj, "Axis Y data (r)\n");
				writeMsg(&Usart2Comm, bufferMsj);
				uint8_t AccelY_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_YOUT_L);
				uint8_t AccelY_high = i2c_readSingleRegister(&handlerAccelerometer,ACCEL_YOUT_H);
				int16_t AccelY = AccelY_high << 8 | AccelY_low;
				sprintf(bufferMsj, "AccelY = %.2f \n", (AccelY/256.f)*9.78);
				writeMsg(&Usart2Comm, bufferMsj);
				usart2DataReceived = '\0';
			}
			else if(usart2DataReceived == 'z'){
				sprintf(bufferMsj, "Axis Z data (r)\n");
				writeMsg(&Usart2Comm, bufferMsj);

				uint8_t AccelZ_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_L);
				uint8_t AccelZ_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_H);
				int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
				sprintf(bufferMsj, "AccelZ = %.2f \n", (AccelZ/256.f)*9.78);
				writeMsg(&Usart2Comm, bufferMsj);
				usart2DataReceived = '\0';
			}
			if (usart2DataReceived == 'd'){
				banderaMuestreo = 1;
				numeroMuestreo = 0;
				while(numeroMuestreo < 2000){
					uint8_t AccelX_low =  i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_L);
					uint8_t AccelX_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_H);
					int16_t AccelX = AccelX_high << 8 | AccelX_low;


					uint8_t AccelY_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_YOUT_L);
					uint8_t AccelY_high = i2c_readSingleRegister(&handlerAccelerometer,ACCEL_YOUT_H);
					int16_t AccelY = AccelY_high << 8 | AccelY_low;


					uint8_t AccelZ_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_L);
					uint8_t AccelZ_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_H);
					int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;

					ArregloX[numeroMuestreo] = (AccelX/256.f)*9.78;
					ArregloY[numeroMuestreo] = (AccelY/256.f)*9.78;
					ArregloZ[numeroMuestreo] = (AccelZ/256.f)*9.78;

				}
				banderaMuestreo = 0;
				numeroMuestreo = 0;
				for (int i = 0; i < 2000; i++) {
					sprintf(bufferMsj, "X = %.2f  ;  Y = %.2f  ;  Z = %.2f   | %u \n", ArregloX[i], ArregloY[i], ArregloZ[i], i);
					writeMsg(&Usart2Comm, bufferMsj);
				}

				usart2DataReceived = '\0';

			}
			if (usart2DataReceived == 'l'){
				banderaMuestreo = 1;
				while(numeroMuestreo < 2000){
					uint8_t AccelX_low =  i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_L);
					uint8_t AccelX_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_XOUT_H);
					int16_t AccelX = AccelX_high << 8 | AccelX_low;
					sprintf(bufferMsj, "AccelX = %.2f \n", (AccelX/256.f)*9.78);
					writeMsg(&Usart2Comm, bufferMsj);

					uint8_t AccelY_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_YOUT_L);
					uint8_t AccelY_high = i2c_readSingleRegister(&handlerAccelerometer,ACCEL_YOUT_H);
					int16_t AccelY = AccelY_high << 8 | AccelY_low;
					sprintf(bufferMsj, "AccelY = %.2f \n", (AccelY/256.f)*9.78);
					writeMsg(&Usart2Comm, bufferMsj);


					uint8_t AccelZ_low = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_L);
					uint8_t AccelZ_high = i2c_readSingleRegister(&handlerAccelerometer, ACCEL_ZOUT_H);
					int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
					sprintf(bufferMsj, "AccelZ = %.2f \n", (AccelZ/256.f)*9.78);
					writeMsg(&Usart2Comm, bufferMsj);
					numeroMuestreo = 0; // para que nunca salga del while hasta que se presione otra tecla
					if(usart2DataReceived != 'l'){
						numeroMuestreo = 2000;
					}
				}
			}
		}

	} // Fin while
	return(0);
} //Fin funcion main

/**/
void init_hardware (void){

/*Configuramos la velocidad del micro*/
	handlerPll.FrecuenciaClock = PLL_CLOCK_100MHz;
	configPLL(&handlerPll);


/*Configuracion del LED del blinky en el puesto PA5 */
	handlerLEDBlinky.pGPIOx = GPIOA;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinNumber           = PIN_5;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_OUT;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinOPType           = GPIO_OTYPE_PUSHPULL;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinSpeed            = GPIO_OSPEED_FAST;
	handlerLEDBlinky.GPIO_PinConfig.GPIO_PinPuPdControl      = GPIO_PUPDR_NOTHING;

	GPIO_Config(&handlerLEDBlinky);
	GPIO_WritePin(&handlerLEDBlinky, SET);

/*Configuracion PIN para el exti (debe ser un pin como entrada)*/
	handlerpinEXTI.pGPIOx = GPIOB;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinNumber           = PIN_3;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_IN;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinOPType           = GPIO_OTYPE_PUSHPULL;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinSpeed            = GPIO_OSPEED_FAST;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinPuPdControl      = GPIO_PUPDR_NOTHING;

	GPIO_Config(&handlerpinEXTI);

/* Configuracion del TIM2 para que haga un blinky cada 250ms*/
	//Si el timer esta a 80MHz deben usarse la Speed que termina en _PLL80_100us
	handlerBlinkyTimer.ptrTIMx                              = TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode                = BTIMER_MODE_UP ;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed               = BTIMER_SPEED_80MHz_100us;
	handlerBlinkyTimer.TIMx_Config.TIMx_period              = 2500;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable     = BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerBlinkyTimer);

	//Si el timer esta a 80MHz deben usarse la Speed que termina en _PLL80_100us
	handler1HzTimer.ptrTIMx                              = TIM3;
	handler1HzTimer.TIMx_Config.TIMx_mode                = BTIMER_MODE_UP ;
	handler1HzTimer.TIMx_Config.TIMx_speed               = BTIMER_SPEED_80MHz_100us;
	handler1HzTimer.TIMx_Config.TIMx_period              = 10;
	handler1HzTimer.TIMx_Config.TIMx_interruptEnable     = BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handler1HzTimer);


// Configurando la interrucion del EXTI
	Exti.edgeType                                           = EXTERNAL_INTERRUPT_FALLING_EDGE;
	Exti.pGPIOHandler                                       = &handlerpinEXTI;
	extInt_Config(&Exti);


/* Configuracion de la comunicacion serial USART*/
	//PINTX del usart para Usart6(PA11 AF8) Para Usart2(PA2 AF7)
//	handlerPinTX.pGPIOx = GPIOA;
//	handlerPinTX.GPIO_PinConfig.GPIO_PinNumber           = PIN_2;
//	handlerPinTX.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_ALTFN;
//	handlerPinTX.GPIO_PinConfig.GPIO_PinAltFunMode       = AF7;
//	GPIO_Config(&handlerPinTX);
	//PINRX del usart para Usart6(PA12 AF8) Para Usart2(PA3 AF7)
//	handlerPinRX.pGPIOx = GPIOA;
//	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber           = PIN_3;
//	handlerPinRX.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_ALTFN;
//	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode       = AF7;
//	GPIO_Config(&handlerPinRX);
	//cuando el micro este a 80MHz deben usarse los baudrate terminados en _80MHz
	Usart2Comm.ptrUSARTx                                     = USART2;
	Usart2Comm.USART_Config.USART_baudrate                   = USART_BAUDRATE_115200_80MHz;
	Usart2Comm.USART_Config.USART_datasize                   = USART_DATASIZE_8BIT;
	Usart2Comm.USART_Config.USART_mode                       = USART_MODE_RXTX;
	Usart2Comm.USART_Config.USART_parity                     = USART_PARITY_NONE;
	Usart2Comm.USART_Config.USART_stopbits                   = USART_STOPBIT_1;
	Usart2Comm.USART_Config.USART_enableIntTX                = USART_TX_INTERRUPT_DISABLE;
	Usart2Comm.USART_Config.USART_enableIntRX                = USART_RX_INTERRUPT_ENABLE;
	USART_Config(&Usart2Comm);

	/*Probando el PLL*/
	handlerPinPrueba.pGPIOx = GPIOA;
	handlerPinPrueba.GPIO_PinConfig.GPIO_PinNumber = PIN_8;
	handlerPinPrueba.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerPinPrueba.GPIO_PinConfig.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerPinPrueba.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	handlerPinPrueba.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerPinPrueba.GPIO_PinConfig.GPIO_PinAltFunMode = AF0;

	//Cargamos la configuracion del Pin en el registro
	GPIO_Config(&handlerPinPrueba);

	/*Configuracion para I2C*/

	handleI2cSCL.pGPIOx                                  = GPIOB;
	handleI2cSCL.GPIO_PinConfig.GPIO_PinNumber           = PIN_8;
	handleI2cSCL.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_ALTFN;
	handleI2cSCL.GPIO_PinConfig.GPIO_PinOPType           = GPIO_OTYPE_OPENDRAIN;
	handleI2cSCL.GPIO_PinConfig.GPIO_PinSpeed            = GPIO_OSPEED_FAST;
	handleI2cSCL.GPIO_PinConfig.GPIO_PinPuPdControl      = GPIO_PUPDR_PULLUP;
	handleI2cSCL.GPIO_PinConfig.GPIO_PinAltFunMode       = AF4;
	GPIO_Config(&handleI2cSCL);

	handleI2cSDA.pGPIOx                                  = GPIOB;
	handleI2cSDA.GPIO_PinConfig.GPIO_PinNumber           = PIN_9;
	handleI2cSDA.GPIO_PinConfig.GPIO_PinMode             = GPIO_MODE_ALTFN;
	handleI2cSDA.GPIO_PinConfig.GPIO_PinOPType           = GPIO_OTYPE_OPENDRAIN;
	handleI2cSDA.GPIO_PinConfig.GPIO_PinSpeed            = GPIO_OSPEED_FAST;
	handleI2cSDA.GPIO_PinConfig.GPIO_PinPuPdControl      = GPIO_PUPDR_PULLUP;
	handleI2cSDA.GPIO_PinConfig.GPIO_PinAltFunMode       = AF4;
	GPIO_Config(&handleI2cSDA);

	handlerAccelerometer.ptrI2Cx            = I2C1;
	handlerAccelerometer.modeI2C            = I2C_MODE_FM;
	handlerAccelerometer.slaveAddress       = ACCEL_ADDRESS;
	i2c_config(&handlerAccelerometer);

	//CAmbiando el muestreo del acelerometro
	i2c_writeSingleRegister(&handlerAccelerometer, BW_RATE, 0xE);
} // Termina el int_Hardware



// aca se ejecuta el Blinky
void BasicTimer2_Callback(void){

	GPIOxTogglePin(&handlerLEDBlinky);
}
// aca se ejecuta el Blinky
void BasicTimer3_Callback(void){
	if(banderaMuestreo == 1){
		numeroMuestreo++;
	}

}


void callback_extInt3 (void){
 // aca se debe levantar una bandera
}

//recibir datos con el usart
void usart2Rx_Callback(void){
	usart2DataReceived = getRxData();
}





