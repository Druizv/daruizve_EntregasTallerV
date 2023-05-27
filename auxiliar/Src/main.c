/* Driver for PLL Configuration and LED Blinking */

#include <stm32f4xx.h>

#include "pll_driver.h"

#define LED_PIN (1 << 5) // Assuming LED is connected to PORTA Pin 5

void SysTick_Handler(void) {
  static int count = 0;
  if (++count >= 500) { // Blink at 250ms rate
    GPIOA->ODR ^= LED_PIN; // Toggle LED
    count = 0;
  }
}

int main(void) {
    PLL_Config_t pllConfig = { .PLLM = 8, .PLLN = 160, .PLLP = 2 };
    configPLL(pllConfig);

    // Configure LED pin as output
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER |= (1 << 10); // Set Pin 5 as output

    // Configure SysTick timer
    SysTick_Config(SystemCoreClock / 1000); // Generate interrupts every 1ms

    while(1);
}

