
#include "stm32f4xx.h"

#include "keypad.h"

#include "stdio.h"

#include "GPIOxDriver.h"

#include "ExtiDriver.h"

//Defino los pines a usar dentro del driver

GPIO_Handler_t handlerPC0 = {0};
GPIO_Handler_t handlerPC1 = {0};
GPIO_Handler_t handlerPC2 = {0};
GPIO_Handler_t handlerPC3 = {0};
GPIO_Handler_t handlerPC4 = {0};
GPIO_Handler_t handlerPC5 = {0};
GPIO_Handler_t handlerPC6 = {0};
GPIO_Handler_t handlerPC7 = {0};

EXTI_Config_t ExtiPC0 = {0};
EXTI_Config_t ExtiPC1 = {0};
EXTI_Config_t ExtiPC2 = {0};
EXTI_Config_t ExtiPC3 = {0};

static char decode_keypad(uint8_t col, uint8_t row);



const unsigned char keymap[4][4]=
	{
		{'1', '2', '3', 'A' },
		{'4', '5', '6', 'B' },
		{'7', '8', '9', 'C' },
		{'*', '0', '#', 'D' }


	};

//se genera un arreglo para definir los estados de la matriz 4x4
const uint32_t clo_state[4]={	(GPIO_BSRR_BR4|GPIO_BSRR_BS5|GPIO_BSRR_BS6|GPIO_BSRR_BS7),
						(GPIO_BSRR_BS4|GPIO_BSRR_BR5|GPIO_BSRR_BS6|GPIO_BSRR_BS7),
						(GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BR6|GPIO_BSRR_BS7),
						(GPIO_BSRR_BS4|GPIO_BSRR_BS5|GPIO_BSRR_BS6|GPIO_BSRR_BR7)
};




void keypad_init(void)
{

	/*Enable clock access to GPIOC*/
	//RCC->AHB1ENR|=RCC_AHB1ENR_GPIOCEN;
	handlerPC0.pGPIOx	= GPIOC;
	handlerPC1.pGPIOx	= GPIOC;
	handlerPC2.pGPIOx	= GPIOC;
	handlerPC3.pGPIOx	= GPIOC;
	handlerPC4.pGPIOx	= GPIOC;
	handlerPC5.pGPIOx	= GPIOC;
	handlerPC6.pGPIOx	= GPIOC;
	handlerPC7.pGPIOx	= GPIOC;

	/*Set pins select*/
	handlerPC0.GPIO_PinConfig.GPIO_PinNumber	= PIN_0;
	handlerPC1.GPIO_PinConfig.GPIO_PinNumber	= PIN_1;
	handlerPC2.GPIO_PinConfig.GPIO_PinNumber	= PIN_2;
	handlerPC3.GPIO_PinConfig.GPIO_PinNumber	= PIN_3;
	handlerPC4.GPIO_PinConfig.GPIO_PinNumber	= PIN_4;
	handlerPC5.GPIO_PinConfig.GPIO_PinNumber	= PIN_5;
	handlerPC6.GPIO_PinConfig.GPIO_PinNumber	= PIN_6;
	handlerPC7.GPIO_PinConfig.GPIO_PinNumber	= PIN_7;

	/*Set PC0 to PC3 as input*/
	//GPIOC->MODER &= ~(GPIO_MODER_MODE0|GPIO_MODER_MODE1|GPIO_MODER_MODE2|GPIO_MODER_MODE3);
	handlerPC0.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_IN;
	handlerPC1.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_IN;
	handlerPC2.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_IN;
	handlerPC3.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_IN;

	/* Activate internal pullup resistor for PC0 to PC3*/
	//GPIOC->PUPDR|=GPIO_PUPDR_PUPD0_0|GPIO_PUPDR_PUPD1_0|GPIO_PUPDR_PUPD2_0|GPIO_PUPDR_PUPD3_0;
	handlerPC0.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP;
	handlerPC1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP;
	handlerPC2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP;
	handlerPC3.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP;


	/*Set PC4 to PC7 as output*/
//	GPIOC->MODER |= GPIO_MODER_MODE4_0|GPIO_MODER_MODE5_0|GPIO_MODER_MODE6_0|GPIO_MODER_MODE7_0;
//	GPIOC->MODER &=~( GPIO_MODER_MODE4_1|GPIO_MODER_MODE5_1|GPIO_MODER_MODE6_1|GPIO_MODER_MODE7_1);
	handlerPC4.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_OUT;
	handlerPC5.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_OUT;
	handlerPC6.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_OUT;
	handlerPC7.GPIO_PinConfig.GPIO_PinMode	= GPIO_MODE_OUT;

	/*Set PC4 to PC7 as high*/
	GPIO_WritePin(&handlerPC4, RESET);
	GPIO_WritePin(&handlerPC5, RESET);
	GPIO_WritePin(&handlerPC6, RESET);
	GPIO_WritePin(&handlerPC7, RESET);

	/*load configuration*/
	GPIO_Config(&handlerPC0);
	GPIO_Config(&handlerPC1);
	GPIO_Config(&handlerPC2);
	GPIO_Config(&handlerPC3);
	GPIO_Config(&handlerPC4);
	GPIO_Config(&handlerPC5);
	GPIO_Config(&handlerPC6);
	GPIO_Config(&handlerPC7);

	/*Configure EXTI keys*/
	ExtiPC0.edgeType                                       = EXTERNAL_INTERRUPT_RISING_EDGE;
	ExtiPC0.pGPIOHandler                                   = &handlerPC0;
	extInt_Config(&ExtiPC0);

	ExtiPC1.edgeType                                       = EXTERNAL_INTERRUPT_RISING_EDGE;
	ExtiPC1.pGPIOHandler                                   = &handlerPC1;
	extInt_Config(&ExtiPC1);

	ExtiPC2.edgeType                                       = EXTERNAL_INTERRUPT_RISING_EDGE;
	ExtiPC2.pGPIOHandler                                   = &handlerPC2;
	extInt_Config(&ExtiPC2);

	ExtiPC3.edgeType                                       = EXTERNAL_INTERRUPT_RISING_EDGE;
	ExtiPC3.pGPIOHandler                                   = &handlerPC3;
	extInt_Config(&ExtiPC3);

}


char keypad_read(void)
{

	unsigned char key=0,data=0;

	for (int i=0;i<4;i++)
	{

		GPIOC->BSRR=clo_state[i];

		data=(GPIOC->IDR)&0xF; /*Get rid of data from bit 5 to bit31*/

		if(data != 0xF)
		{
			key=decode_keypad(i,data);
		}

	}

	return key;
}

//Activo las columnas y leo las filas
static char decode_keypad(uint8_t col, uint8_t row)
{

	if (row == 0xE) return keymap[0][col];
	if (row == 0xD) return keymap[1][col];
	if (row == 0xB) return keymap[2][col];
	if (row == 0x7) return keymap[3][col];
	return 0;
}
