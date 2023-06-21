
#ifndef RCC100XDRIVERH
#define RCC100XDRIVERH

#include "stm32f4xx.h"

/*Funciones definidas en el driver RCC100xDriver para configurar el systemClock
 * en 100MHz y una función para configurar los timers de 50MHz a 100MHz */
void RCC_config(void);
void configTimers (void);

#endif /* RCC100XDRIVERH */
