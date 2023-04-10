/*
 * ExtiConfig_main.c
 *
 *  Created on: Mar 24, 2023
 *      Author: druiz
 */

#include "stdint.h"
#include "stm32f4xx.h"
#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"



//Definicion de los pines del Encoder
GPIO_Handler_t handlerEncoderDT  = {0};
GPIO_Handler_t handlerEncoderSW  = {0};
GPIO_Handler_t handlerEncoderCLK  = {0};


//Interrupcion del Encoder
EXTI_Config_t EXTISW  = {0};
EXTI_Config_t EXTICLK = {0};



void callback_extInt10(void);

void init_system(void);

void BasicTimer2_Callback(void);


//Definicion del contador
uint8_t counter = 0;

uint8_t statusSW = 0;
//Unidades y decimas del display
uint8_t uniDisplay = 0;
uint8_t decDisplay = 0;

uint8_t wormpos = 0;
//definicion de las banderas
uint8_t flagDT      = 0;
uint8_t flagSW      = 0;
uint8_t flagBK      = 0;
uint8_t flagCLK     = 0;
uint8_t flag7Seg    = 0;

//Definicion del timer del Encoder
BasicTimer_Handler_t handlerRefreshTimer = {0};

//Definicion del timer del led
BasicTimer_Handler_t handlerBlinkyTimer  = {0};

//definicion del blinky del led
GPIO_Handler_t handlerLEDBlinky    = {0};

//Configuracion de todos los segmentos deL DISPLAY
GPIO_Handler_t handlerLEDA = {0};
GPIO_Handler_t handlerLEDB = {0};
GPIO_Handler_t handlerLEDC = {0};
GPIO_Handler_t handlerLEDD = {0};
GPIO_Handler_t handlerLEDE = {0};
GPIO_Handler_t handlerLEDF = {0};
GPIO_Handler_t handlerLEDG = {0};

//se configuran las salidas para los transistores
GPIO_Handler_t handlerT1 = {0};
GPIO_Handler_t handlerT2 = {0};

//Configuracion para definir los led que generan los numeros
void numberLED (uint8_t number);
void worm(uint8_t wormpos);
void tpos(uint8_t type);

int main(void){
	//inicialización de todos los elementos del sistema
	init_system();

	while(1){
		if(flagBK == 1){
			flagBK = 0;
			GPIOxTooglePin(&handlerLEDBlinky);
		}
		if (flagSW){
			flagSW = 0;
			if(statusSW == 1){
				statusSW = 0;
			}
			else{
				statusSW = 1;
			}
		}

		if(statusSW == 0){
			if(GPIO_ReadPin(&handlerEncoderDT) == 0 && flagCLK){
				if(counter < 99){
					counter ++;
				}
				flagCLK = 0;
			}

			else if (GPIO_ReadPin(&handlerEncoderDT) == 1 && flagCLK){
				if (counter > 0){
					counter -- ;
				}
				flagCLK = 0;
			}
			if(flag7Seg == 1){
				//arroja el multiplo de 10
				decDisplay = counter / 10;
				//arroja las unidades
				uniDisplay = counter % 10;
				flag7Seg = 0;
				//se usa una bandera interna para cambiar el condicional que entre y
				//que la otra interrupcion entre al otro condicional
				if(GPIO_ReadPin(&handlerT1)== 1){
					//se desactiva el primer 7 segmentos
					GPIOxTooglePin(&handlerT1);
					//se activa el segundo
					GPIOxTooglePin(&handlerT2);
					//Se escribe en el segundo pin
					numberLED(decDisplay);


				}
				else if ( GPIO_ReadPin(&handlerT2) == 1){
					//Se desactiva el segundo 7 segmentos
					GPIOxTooglePin(&handlerT1);
					//se activa el primero
					GPIOxTooglePin(&handlerT2);
					//Se escribe en el primer 7 segmentos
					numberLED(uniDisplay);

				}
			}// se cierra el flag7seg
		}

		// se cierra el contador
		else if (statusSW == 1){
			if(GPIO_ReadPin(&handlerEncoderDT) == 0 && flagCLK){
				if(wormpos < 11){
					wormpos ++;
				}
				flagCLK = 0;
				if(wormpos == 11){
					wormpos = 0;
				}
			}
			else if (GPIO_ReadPin(&handlerEncoderDT) == 1 && flagCLK){
				if(wormpos > 0){
					wormpos -- ;
				}
				flagCLK = 0;
				if(wormpos == 0){
					wormpos = 11 ;

				}
			}
			worm(wormpos);

			//codigo de la culebrita

		}/**/
	} //se cierra el ciclo while
}// se cierra el main

			// Aca termina la funcion main

void init_system(void){

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

	// Configuracion de los pines del Encoder
	handlerEncoderDT.pGPIOx								  = GPIOB;
	handlerEncoderDT.GPIO_PinConfig.GPIO_PinNumber        = PIN_4;
	handlerEncoderDT.GPIO_PinConfig.GPIO_PinMode		  = GPIO_MODE_IN;
	handlerEncoderDT.GPIO_PinConfig.GPIO_PinOType		  = GPIO_OTYPE_PUSHPULL;
	handlerEncoderDT.GPIO_PinConfig.GPIO_PinSpeed		  = GPIO_OSPEED_FAST;
	handlerEncoderDT.GPIO_PinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;

	handlerEncoderSW.pGPIOx								  = GPIOB;
	handlerEncoderSW.GPIO_PinConfig.GPIO_PinNumber        = PIN_15;
	handlerEncoderSW.GPIO_PinConfig.GPIO_PinMode		  = GPIO_MODE_IN;
	handlerEncoderSW.GPIO_PinConfig.GPIO_PinOType		  = GPIO_OTYPE_PUSHPULL;
	handlerEncoderSW.GPIO_PinConfig.GPIO_PinSpeed		  = GPIO_OSPEED_FAST;
	handlerEncoderSW.GPIO_PinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;

	// se configura el boton clk
	handlerEncoderCLK.pGPIOx							 = GPIOB;
	handlerEncoderCLK.GPIO_PinConfig.GPIO_PinNumber      = PIN_3;
	handlerEncoderCLK.GPIO_PinConfig.GPIO_PinMode		 = GPIO_MODE_IN;
	handlerEncoderCLK.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerEncoderCLK.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerEncoderCLK.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	//Se carga la configuracion de los pines
	GPIO_Config(&handlerEncoderDT);
	GPIO_Config(&handlerEncoderSW);
	GPIO_Config(&handlerEncoderCLK);

	//configuracion del exti del swich
	EXTISW.edgeType                                       = EXTERNAL_INTERRUPT_FALLING_EDGE;
	EXTISW.pGPIOHandler                                   = &handlerEncoderSW;
	extInt_Config(&EXTISW);

	//CONFIGURACION DEL EXTI DEL CLOCK
	EXTICLK.edgeType                                      = EXTERNAL_INTERRUPT_RISING_EDGE;
	EXTICLK.pGPIOHandler                                  = &handlerEncoderCLK;
	extInt_Config(&EXTICLK);

	//Se configuran los LED del 7 segmentos doble
	handlerLEDA.pGPIOx								 = GPIOA;
	handlerLEDA.GPIO_PinConfig.GPIO_PinNumber       = PIN_0;
	handlerLEDA.GPIO_PinConfig.GPIO_PinMode		 = GPIO_MODE_OUT;
	handlerLEDA.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerLEDA.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerLEDA.GPIO_PinConfig.GPIO_PinPuPdControl  = GPIO_PUPDR_NOTHING;

	handlerLEDB.pGPIOx								 = GPIOA;
	handlerLEDB.GPIO_PinConfig.GPIO_PinNumber       = PIN_1;
	handlerLEDB.GPIO_PinConfig.GPIO_PinMode		 = GPIO_MODE_OUT;
	handlerLEDB.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerLEDB.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerLEDB.GPIO_PinConfig.GPIO_PinPuPdControl  = GPIO_PUPDR_NOTHING;

	handlerLEDC.pGPIOx								 = GPIOC;
	handlerLEDC.GPIO_PinConfig.GPIO_PinNumber       = PIN_4;
	handlerLEDC.GPIO_PinConfig.GPIO_PinMode		 = GPIO_MODE_OUT;
	handlerLEDC.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerLEDC.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerLEDC.GPIO_PinConfig.GPIO_PinPuPdControl  = GPIO_PUPDR_NOTHING;

	handlerLEDD.pGPIOx								 = GPIOC;
	handlerLEDD.GPIO_PinConfig.GPIO_PinNumber       = PIN_3;
	handlerLEDD.GPIO_PinConfig.GPIO_PinMode		 = GPIO_MODE_OUT;
	handlerLEDD.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerLEDD.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerLEDD.GPIO_PinConfig.GPIO_PinPuPdControl  = GPIO_PUPDR_NOTHING;

	handlerLEDE.pGPIOx								 = GPIOA;
	handlerLEDE.GPIO_PinConfig.GPIO_PinNumber       = PIN_4;
	handlerLEDE.GPIO_PinConfig.GPIO_PinMode		 = GPIO_MODE_OUT;
	handlerLEDE.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerLEDE.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerLEDE.GPIO_PinConfig.GPIO_PinPuPdControl  = GPIO_PUPDR_NOTHING;

	handlerLEDF.pGPIOx								 = GPIOA;
	handlerLEDF.GPIO_PinConfig.GPIO_PinNumber       = PIN_6;
	handlerLEDF.GPIO_PinConfig.GPIO_PinMode		 = GPIO_MODE_OUT;
	handlerLEDF.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerLEDF.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerLEDF.GPIO_PinConfig.GPIO_PinPuPdControl  = GPIO_PUPDR_NOTHING;

	handlerLEDG.pGPIOx								 = GPIOA;
	handlerLEDG.GPIO_PinConfig.GPIO_PinNumber        = PIN_7;
	handlerLEDG.GPIO_PinConfig.GPIO_PinMode			 = GPIO_MODE_OUT;
	handlerLEDG.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerLEDG.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerLEDG.GPIO_PinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;

	//Se configuran las salidas de los transistores
	handlerT1.pGPIOx								 = GPIOA;
	handlerT1.GPIO_PinConfig.GPIO_PinNumber        = PIN_8;
	handlerT1.GPIO_PinConfig.GPIO_PinMode			 = GPIO_MODE_OUT;
	handlerT1.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerT1.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerT1.GPIO_PinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;

	//transistor de las decenas
	handlerT2.pGPIOx								 = GPIOA;
	handlerT2.GPIO_PinConfig.GPIO_PinNumber        = PIN_9;
	handlerT2.GPIO_PinConfig.GPIO_PinMode			 = GPIO_MODE_OUT;
	handlerT2.GPIO_PinConfig.GPIO_PinOType		 = GPIO_OTYPE_PUSHPULL;
	handlerT2.GPIO_PinConfig.GPIO_PinSpeed		 = GPIO_OSPEED_FAST;
	handlerT2.GPIO_PinConfig.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;

	//se carga la configuracion en el GPIO
	GPIO_Config(&handlerLEDA);
	GPIO_Config(&handlerLEDB);
	GPIO_Config(&handlerLEDC);
	GPIO_Config(&handlerLEDD);
	GPIO_Config(&handlerLEDE);
	GPIO_Config(&handlerLEDF);
	GPIO_Config(&handlerLEDG);
	//SE CARGA LA CONFIGURACION DE LOS TRANSISTORES
	GPIO_Config(&handlerT1);
	GPIO_Config(&handlerT2);
	GPIO_WritePin(&handlerT1, SET);
}
// interrupcion del encoder
void callback_extInt3(void){
	flagCLK = 1;
}
// Configuracion del Timer del blinky
void BasicTimer2_Callback(void){
	flagBK =1;
}
//Timer del 7 segmentos para refrescar
void BasicTimer3_Callback(void){
	flag7Seg =1;
}

void callback_extInt15(void){
	flagSW = 1;
}
void clear(void){
	GPIO_WritePin(&handlerLEDA, SET);
	GPIO_WritePin(&handlerLEDB, SET);
	GPIO_WritePin(&handlerLEDC, SET);
	GPIO_WritePin(&handlerLEDD, SET);
	GPIO_WritePin(&handlerLEDE, SET);
	GPIO_WritePin(&handlerLEDF, SET);
	GPIO_WritePin(&handlerLEDG, SET);
}

void numberLED (uint8_t number){
	clear();
	switch (number){
	case 0:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		GPIO_WritePin(&handlerLEDE, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		break;
	}
	case 1:{
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		break;
	}
	case 2:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDG, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		GPIO_WritePin(&handlerLEDE, RESET);
		break;
	}
	case 3:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDG, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		break;
	}
	case 4:{
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		GPIO_WritePin(&handlerLEDG, RESET);
		break;
	}
	case 5:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		GPIO_WritePin(&handlerLEDG, RESET);
		break;
	}
	case 6:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		GPIO_WritePin(&handlerLEDE, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		GPIO_WritePin(&handlerLEDG, RESET);
		break;
	}
	case 7:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		break;
	}
	case 8:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		GPIO_WritePin(&handlerLEDE, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		GPIO_WritePin(&handlerLEDG, RESET);
		break;
	}
	case 9:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		GPIO_WritePin(&handlerLEDG, RESET);
		break;
	}
	default:{
		GPIO_WritePin(&handlerLEDA, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		GPIO_WritePin(&handlerLEDE, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		break;
		}
	}
}

void worm(uint8_t wormpos){
	clear();
	switch (wormpos){
	case 0:{
		GPIO_WritePin(&handlerT1, SET);
		GPIO_WritePin(&handlerT2, RESET);
		GPIO_WritePin(&handlerLEDA, RESET);
		break;
	}
	case 1:{
		GPIO_WritePin(&handlerT1, RESET);
		GPIO_WritePin(&handlerT2, SET);
		GPIO_WritePin(&handlerLEDA, RESET);
		break;
	}
	case 2:{
		GPIO_WritePin(&handlerT1, RESET);
		GPIO_WritePin(&handlerT2, SET);
		GPIO_WritePin(&handlerLEDF, RESET);
		break;
	}

	case 3:{
		GPIO_WritePin(&handlerT1, RESET);
		GPIO_WritePin(&handlerT2, SET);
		GPIO_WritePin(&handlerLEDE, RESET);
		break;
	}
	case 4:{
		GPIO_WritePin(&handlerT1, RESET);
		GPIO_WritePin(&handlerT2, SET);
		GPIO_WritePin(&handlerLEDD, RESET);
		break;
	}
	case 5:{
		GPIO_WritePin(&handlerT1, SET);
		GPIO_WritePin(&handlerT2, RESET);
		GPIO_WritePin(&handlerLEDE, RESET);
		break;
	}
	case 6:{
		GPIO_WritePin(&handlerT1, SET);
		GPIO_WritePin(&handlerT2, RESET);
		GPIO_WritePin(&handlerLEDF, RESET);
		break;
	}
	case 7:{
		GPIO_WritePin(&handlerT1, RESET);
		GPIO_WritePin(&handlerT2, SET);
		GPIO_WritePin(&handlerLEDB, RESET);
		break;
	}
	case 8:{
		GPIO_WritePin(&handlerT1, RESET);
		GPIO_WritePin(&handlerT2, SET);
		GPIO_WritePin(&handlerLEDC, RESET);
		break;
	}
	case 9:{
		GPIO_WritePin(&handlerT1, SET);
		GPIO_WritePin(&handlerT2, RESET);
		GPIO_WritePin(&handlerLEDD, RESET);
		break;
	}
	case 10:{
		GPIO_WritePin(&handlerT1, SET);
		GPIO_WritePin(&handlerT2, RESET);
		GPIO_WritePin(&handlerLEDC, RESET);
		break;
	}
	case 11:{
		GPIO_WritePin(&handlerT1, SET);
		GPIO_WritePin(&handlerT2, RESET);
		GPIO_WritePin(&handlerLEDB, RESET);
		break;
	}
	default:{
		GPIO_WritePin(&handlerT1, SET);
		GPIO_WritePin(&handlerT2, RESET);
		GPIO_WritePin(&handlerLEDA, RESET);
		break;
	}
	}
}











