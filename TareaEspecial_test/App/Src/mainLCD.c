/**
 ******************************************************************************
 * @file           : main.c
 * @author         : aristizabal by STM32CubeIDE
 * @Nombre          : Jhony Steven Aristizabal Serna
 * @brief          : main Tarea Especial
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "SysTickDriver.h"
#include "PllDriver.h"
#include "I2CDriver.h"
#include "PwmDriver.h"
#include "LCDDriver.h"

#include "arm_math.h"
#include <math.h>


#define HSI_CLOCK_CONFIGURED		0
#define HSE_CLOCK_CONFIGURED		1
#define PLL_CLOCK_CONFIGURED		2



/* Definicion de handlers */


//PINES
GPIO_Handler_t handlerBlinkyPin = {0};	//PA5
GPIO_Handler_t handlerpinEXTI            = {0};
GPIO_Handler_t handlerLEDX = {0};	//PA5
GPIO_Handler_t handlerLEDY = {0};	//PA5
GPIO_Handler_t handlerLEDZ = {0};	//PA5


//PLL
PLL_Handler_t handlerPll = {0};

// PWM
PWM_Handler_t handlerPwmLEDX;
PWM_Handler_t handlerPwmLEDY;
PWM_Handler_t handlerPwmLEDZ;
uint16_t duttyX = {0};
uint16_t duttyY = {0};
uint16_t duttyZ = {0};

//I2C
GPIO_Handler_t handler_i2cSDA = {0};	//PB8
GPIO_Handler_t handler_i2cSCL = {0}; //PB9
I2C_Handler_t handlerAccelerometer = {0};

GPIO_Handler_t handler_i2cSDA_LCD = {0};	//PB8
GPIO_Handler_t handler_i2cSCL_LCD = {0}; //PB9
I2C_Handler_t handlerLCD = {0};

//Timers
BasicTimer_Handler_t	handlerBlinkyTimer	= {0};  //TIM2
BasicTimer_Handler_t	handlerSamplingTimer	= {0};  //TIM3
BasicTimer_Handler_t	handlerPwmTimer	= {0};  //TIM4

//USART
GPIO_Handler_t handlerPinTX = {0};
GPIO_Handler_t handlerPinRX = {0};
USART_Handler_t usart6Comm = {0};
uint8_t usart2DataReceived = 0;
uint8_t rxData = 0;
char bufferData[64] = "Accel ADXL345 testing...";
uint8_t i2cBuffer = {0};
uint16_t counterSampling = {0};
uint8_t samplingFlag = {0};
float arrayX[2000] = {0};
float arrayY[2000] = {0};
float arrayZ[2000] = {0};

//Exti

EXTI_Config_t Exti		= {0};

uint8_t counter_dummy = 0;



// Definicion de las cabeceras de las funciones del main
void init_Hardware(void);
void i2cLCD(void);


/*
 * Funcion principal del programa
 * Esta funcion es el corazon del programa!!
 */

int main(void){
	//Activamos el coprocesador matematico
	SCB->CPACR |= (0xF <<20);




//	LCD_setCursor(&handlerLCD, 1, 2);
//	LCD_sendSTR(&handlerLCD, "AccY = ");
//	LCD_setCursor(&handlerLCD, 2, 2);
//	LCD_sendSTR(&handlerLCD, "Accz = ");
//	LCD_setCursor(&handlerLCD, 3, 2);
//	LCD_sendSTR(&handlerLCD, "Conf = ");
//	i2cBuffer = i2c_readSingleRegister(&handlerAccelerometer, Latent);
//	sprintf(bufferConf, " %x ", (unsigned int) i2cBuffer);
//	moveCursor_inLCD(&handlerLCD, 3, 10);
//	sendSTR_toLCD(&handlerLCD, bufferConf);




	//Inicializamos todos los elementos del sistema
	init_Hardware();
	writeMsg(&usart6Comm, bufferData);

	while(1){


	}//Fin del While
	return(0);
}//Fin del Main



//Funcion que configura el hardware, timers y extis
void init_Hardware(void){

	//Configuracion del BLinky
	handlerBlinkyPin.pGPIOx											= GPIOA;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinNumber					= PIN_5;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_FAST;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerBlinkyPin);

/* ==================================== Configurando los TIMERS =============================================*/

/* ==================================== Configurando los EXTI =============================================*/
	/*Configuracion PIN para el exti (debe ser un pin como entrada)*/
	handlerpinEXTI.pGPIOx = GPIOB;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinNumber = PIN_3;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	handlerpinEXTI.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	GPIO_Config(&handlerpinEXTI);

	Exti.edgeType                                           = EXTERNAL_INTERRUPT_FALLING_EDGE;
	Exti.pGPIOHandler                                       = &handlerpinEXTI;
	extInt_Config(&Exti);

/* ==================================== Configurando los USART =============================================*/
	handlerPinTX.pGPIOx												= GPIOA;
	handlerPinTX.GPIO_PinConfig.GPIO_PinNumber						= PIN_2;
	handlerPinTX.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinTX.GPIO_PinConfig.GPIO_PinAltFunMode					= AF7;
	GPIO_Config(&handlerPinTX);

	handlerPinRX.pGPIOx												= GPIOA;
	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber						= PIN_3;
	handlerPinRX.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode					= AF7;
	GPIO_Config(&handlerPinRX);

	usart6Comm.ptrUSARTx											= USART2;
	usart6Comm.USART_Config.USART_baudrate							= USART_BAUDRATE_115200;
	usart6Comm.USART_Config.USART_datasize							= USART_DATASIZE_8BIT;
	usart6Comm.USART_Config.USART_parity							= USART_PARITY_NONE;
	usart6Comm.USART_Config.USART_stopbits							= USART_STOPBIT_1;
	usart6Comm.USART_Config.USART_mode								= USART_MODE_RXTX;
	usart6Comm.USART_Config.USART_enableIntRX						= USART_RX_INTERRUPT_ENABLE;
	usart6Comm.USART_Config.USART_enableIntTX						=USART_TX_INTERRUPT_DISABLE;
	USART_Config(&usart6Comm);

/* ========================== PLL ======================================================================*/

//	handlerPll.clkSpeed = FREQUENCY_80MHz;
//	configPLL(&handlerPll);

/* ========================== I2C ======================================================================*/
	i2cLCD();

}

/* ===================== Rutinas de atencion o callbacks ===============================================*/

/*
 * Esta funcion se ejecuta cada vez que un caracter es recibido
 * por el puerto USART6
 */

void usart6Rx_Callback(void){
	rxData = getRxData();
}


void i2cLCD(void){
	handler_i2cSCL_LCD.pGPIOx												= GPIOB;
	handler_i2cSCL_LCD.GPIO_PinConfig.GPIO_PinNumber						= PIN_10;
	handler_i2cSCL_LCD.GPIO_PinConfig.GPIO_PinMode							= GPIO_MODE_ALTFN;
	handler_i2cSCL_LCD.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_OPENDRAIN;
	handler_i2cSCL_LCD.GPIO_PinConfig.GPIO_PinPuPdControl        			= GPIO_PUPDR_PULLUP;
	handler_i2cSCL_LCD.GPIO_PinConfig.GPIO_PinSpeed              			= GPIO_OSPEED_FAST;
	handler_i2cSCL_LCD.GPIO_PinConfig.GPIO_PinAltFunMode					= AF4;
	GPIO_Config(&handler_i2cSCL_LCD);

	handler_i2cSDA_LCD.pGPIOx												= GPIOB;
	handler_i2cSDA_LCD.GPIO_PinConfig.GPIO_PinNumber						= PIN_3;
	handler_i2cSDA_LCD.GPIO_PinConfig.GPIO_PinMode							= GPIO_MODE_ALTFN;
	handler_i2cSDA_LCD.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_OPENDRAIN;
	handler_i2cSDA_LCD.GPIO_PinConfig.GPIO_PinPuPdControl        			= GPIO_PUPDR_PULLUP;
	handler_i2cSDA_LCD.GPIO_PinConfig.GPIO_PinSpeed              			= GPIO_OSPEED_FAST;
	handler_i2cSDA_LCD.GPIO_PinConfig.GPIO_PinAltFunMode					= AF9;
	GPIO_Config(&handler_i2cSDA_LCD);

	handlerLCD.ptrI2Cx                            			= I2C2;
	handlerLCD.modeI2C                            			= I2C_MODE_SM;
	handlerLCD.slaveAddress                       			= LCD_ADDRESS;

	i2c_config(&handlerLCD);

	//	// LCD
//	LCD_Clear(&handlerLCD);
//	LCD_Init(&handlerLCD);
//	delay_10();
//	LCD_Clear(&handlerLCD);
//	delay_10();
//	LCD_setCursor(&handlerLCD, 0, 2);
//	LCD_sendSTR(&handlerLCD, "AccX = ");
	// LCD
	LCD_Init(&handlerLCD);
	delay_10();
	LCD_Clear(&handlerLCD);
	delay_10();
	LCD_setCursor(&handlerLCD, 0, 0);
	LCD_sendSTR(&handlerLCD, "No quiero");
	LCD_setCursor(&handlerLCD, 0, 1);
	LCD_sendSTR(&handlerLCD, "Jugar mas");
	LCD_setCursor(&handlerLCD, 0, 2);
	LCD_sendSTR(&handlerLCD, "A la universidad");
}


