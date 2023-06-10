#include "RFID.h"
#include <stm32f4xx.h>

extern UART_HandleTypeDef huart2; // Declaración externa de la estructura UART_HandleTypeDef utilizada en main.c

void RFID_Init(void)
{
  // Aquí puedes realizar la inicialización del hardware específico del lector RFID utilizando el UART
}

void RFID_Read(char *buffer, uint16_t size)
{
  HAL_UART_Receive(&huart2, (uint8_t *)buffer, size, HAL_MAX_DELAY);
}

void LED_On(void)
{
  // Aquí puedes realizar las operaciones necesarias para encender el LED
}

void LED_Off(void)
{
  // Aquí puedes realizar las operaciones necesarias para apagar el LED
}

