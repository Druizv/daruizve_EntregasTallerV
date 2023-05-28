/*
 * PwmDriver.c
 *
 *  Created on: XXXX , 2022
 *      Author: namontoy
 */
#include "PwmDriver.h"

/**/
void pwm_Config(PWM_Handler_t *ptrPwmHandler){

	/* 1. Activar la señal de reloj del periférico requerido */
	if(ptrPwmHandler->ptrTIMx == TIM2){
		/* agregue acá su código */
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; //timer 2
	}
	else if(ptrPwmHandler->ptrTIMx == TIM3){
		/* agregue acá su código */
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  //timer 3
	}
	/*... agregar los demas*/
	else if(ptrPwmHandler->ptrTIMx == TIM4){
		/* agregue acá su código */
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;  //timer 4
	}

	else if(ptrPwmHandler->ptrTIMx == TIM5){
		/* agregue acá su código */
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;  //timer 5
	}

	else{
		__NOP();
	}

	/* 1. Cargamos la frecuencia deseada */
	setFrequency(ptrPwmHandler);

	/* 2. Cargamos el valor del dutty-Cycle*/
	setDuttyCycle(ptrPwmHandler);

/* 3. Configuramos los bits CCxS del registro TIMy_CCMR1, de forma que sea modo salida
 * (para cada canal hay un conjunto CCxS)
 *
 * 4. Además, en el mismo "case" podemos configurar el modo del PWM, su polaridad...
 *
 * 5. Y además activamos el preload bit, para que cada vez que exista un update-event
 * el valor cargado en el CCRx será recargado en el registro "shadow" del PWM */
 switch(ptrPwmHandler->config.channel){
 case 1:{
	 // Seleccionamos como salida el canal
	 ptrPwmHandler -> ptrTIMx -> CCMR1 &= ~TIM_CCMR1_CC1S_0;
	 ptrPwmHandler -> ptrTIMx -> CCMR1 &= ~TIM_CCMR1_CC1S_1;
	 // Configuramos el canal como PWM
	 ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC1M_0;
	 ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1M_1;
	 ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1M_2;

	 // Activamos la funcionalidad de pre-load
	 ptrPwmHandler -> ptrTIMx -> CCMR1 |= TIM_CCMR1_OC1PE;

	  enableOutput(ptrPwmHandler);

	  break;
	 }

	 case 2:{
	  // Seleccionamos como salida el canal
	  ptrPwmHandler -> ptrTIMx -> CCMR1 &= ~TIM_CCMR1_CC2S;

	  // Configuramos el canal como PWM
	  ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2M_1;
	  ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2M_2;

	  // Activamos la funcionalidad de pre-load
	  ptrPwmHandler -> ptrTIMx -> CCMR1 |= TIM_CCMR1_OC2PE;

	  enableOutput(ptrPwmHandler);
	  break;
	 }
	 case 3:{
	  // Seleccionamos como salida el canal
	  ptrPwmHandler -> ptrTIMx -> CCMR2 &= ~TIM_CCMR2_CC3S;

	  // Configuramos el canal como PWM
	  ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3M_1;
	  ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3M_2;

	  // Activamos la funcionalidad de pre-load
	  ptrPwmHandler -> ptrTIMx -> CCMR2 |= TIM_CCMR2_OC3PE;

	  enableOutput(ptrPwmHandler);
	  break;
	 }
	 case 4:{
	  // Seleccionamos como salida el canal
	  ptrPwmHandler -> ptrTIMx -> CCMR2 &= ~TIM_CCMR2_CC4S;

	  // Configuramos el canal como PWM
	  ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4M_1;
	  ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4M_2;


	  // Activamos la funcionalidad de pre-load
	  ptrPwmHandler -> ptrTIMx -> CCMR2 |= TIM_CCMR2_OC4PE;

	  enableOutput(ptrPwmHandler);
	  break;
	 }

	 default:{
	  break;
	 }

	 /* 6. Activamos la salida seleccionada */
	 //enableOutput(ptrPwmHandler);

	 }// fin del switch-case
	}

/* Función para activar el Timer y activar todo el módulo PWM */
void startPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;
}

/* Función para desactivar el Timer y detener todo el módulo PWM*/
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	ptrPwmHandler->ptrTIMx->CR1 &= ~TIM_CR1_CEN;
}

/* Función encargada de activar cada uno de los canales con los que cuenta el TimerX */
void enableOutput(PWM_Handler_t *ptrPwmHandler) {
	switch (ptrPwmHandler->config.channel) {
	case 1: {
		// Activamos la salida del canal 1
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1E;
		break;
	}

	case 2: {
		// Activamos la salida del canal 2
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC2E;
		break;
	 }
	 case 3: {
		 // Activamos la salida del canal 3
		 ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC3E;
		 break;
	 }
	 case 4: {
		 // Activamos la salida del canal 4
		 ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC4E;
		 break;
	 }

	 default: {
		 break;
	 }
	 }
}
/* 
* La frecuencia es definida por el conjunto formado por el preescaler (PSC)
* y el valor límite al que llega el Timer (ARR), con estos dos se establece
* la frecuencia.
* */
void setFrequency(PWM_Handler_t *ptrPwmHandler){
	// Cargamos el valor del prescaler, nos define la velocidad (en ns) a la cual
	// se incrementa el Timer
	ptrPwmHandler->ptrTIMx->PSC = ptrPwmHandler->config.prescaler;

	// Cargamos el valor del ARR, el cual es el límite de incrementos del Timer
	// antes de hacer un update y reload.
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo;
}


/* Función para actualizar la frecuencia, funciona de la mano con setFrequency */
void updateFrequency(PWM_Handler_t *ptrPwmHandler, uint16_t newFreq){
	// Actualizamos el registro que manipula el periodo
	ptrPwmHandler->config.periodo = newFreq;

	// Llamamos a la fucnión que cambia la frecuencia
	setFrequency(ptrPwmHandler);
}

/* El valor del dutty debe estar dado en valores de %, entre 0% y 100%*/
void setDuttyCycle(PWM_Handler_t *ptrPwmHandler){
	// Seleccionamos el canal para configurar su dutty
	switch(ptrPwmHandler->config.channel){
	case 1:{
		ptrPwmHandler->ptrTIMx->CCR1 = ptrPwmHandler->config.duttyCicle;
		break;
	 }
	 case 2:{
		 ptrPwmHandler->ptrTIMx->CCR2 = ptrPwmHandler->config.duttyCicle;
		 break;
	 }
	 case 3:{
		 ptrPwmHandler->ptrTIMx->CCR3 = ptrPwmHandler->config.duttyCicle;
		 break;
	 }
	 case 4:{
		 ptrPwmHandler->ptrTIMx->CCR4 = ptrPwmHandler->config.duttyCicle;
		 break;
	 }

	 default:{
		 break;
	 }

	 }

}


/* Función para actualizar el Dutty, funciona de la mano con setDuttyCycle */
void updateDuttyCycle(PWM_Handler_t *ptrPwmHandler, uint16_t newDutty){
	// Actualizamos el registro que manipula el dutty
	 ptrPwmHandler->config.duttyCicle = newDutty;

	 // Llamamos a la fucnión que cambia el dutty y cargamos el nuevo valor
	 setDuttyCycle(ptrPwmHandler);
}
