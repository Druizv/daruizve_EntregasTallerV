#ifndef PLL_DRIVER_H_
#define PLL_DRIVER_H_

#include "stm32f4xx.h"

typedef struct {
    uint32_t PLLM;
    uint32_t PLLN;
    uint32_t PLLP;
} PLL_Config_t;

void configPLL(PLL_Config_t config);
PLL_Config_t getConfigPLL(void);

#endif /* PLL_DRIVER_H_ */
