#include "pll_driver.h"

void configPLL(PLL_Config_t config) {
    // Enable HSI clock
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));

    // Configure PLL
    RCC->PLLCFGR = 0;

    RCC->PLLCFGR |= config.PLLM & RCC_PLLCFGR_PLLM;
    RCC->PLLCFGR |= (config.PLLN << 6) & RCC_PLLCFGR_PLLN;
    RCC->PLLCFGR |= (((config.PLLP >> 1) - 1) << 16) & RCC_PLLCFGR_PLLP;
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    // Set clock source to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));

    // Configure system clock prescaler
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    // Disable HSI clock
    RCC->CR &= ~RCC_CR_HSION;
}

PLL_Config_t getConfigPLL() {
    PLL_Config_t config = {0};
    config.PLLM = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
    config.PLLN = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6;
    config.PLLP = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> 16) + 1;
    return config;
}


