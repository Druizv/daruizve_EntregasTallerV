#include "main.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "string.h"

UART_HandleTypeDef huart2;
SPI_HandleTypeDef hspi1;

#define RST_PIN GPIO_PIN_9
#define SS_PIN GPIO_PIN_10

MFRC522 mfrc522;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);

void UART_Transmit(const char *data)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)data, strlen(data), HAL_MAX_DELAY);
}

void RFID_Init(void)
{
  HAL_GPIO_WritePin(GPIOA, RST_PIN, GPIO_PIN_RESET);

  HAL_SPI_MspInit(&hspi1); // Inicializa el SPI

  mfrc522.PCD_Init();
}

void RFID_Read(char *buffer, uint16_t size)
{
  if (mfrc522.PICC_IsNewCardPresent())
  {
    if (mfrc522.PICC_ReadCardSerial())
    {
      sprintf(buffer, "Card UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        char uidByteStr[4];
        sprintf(uidByteStr, "%02X ", mfrc522.uid.uidByte[i]);
        strcat(buffer, uidByteStr);
      }
      strcat(buffer, "\r\n");
      mfrc522.PICC_HaltA();
    }
  }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();

  char buffer[100];

  RFID_Init();

  while (1)
  {
    RFID_Read(buffer, sizeof(buffer));
    UART_Transmit(buffer);
    HAL_Delay(1000);
  }
}









































//---------------------------------------------------------prototipo

#include "RFID.h"
#include <stm32f4xx.h>
#include <stdio.h>
#include <string.h>
#include "GPIOxDriver.h"

UART_HandleTypeDef huart2;
GPIO_InitTypeDef GPIO_InitStruct;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

void UART_Transmit(const char *data)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)data, strlen(data), HAL_MAX_DELAY);
}

void UART_Receive(char *buffer, uint16_t size)
{
  HAL_UART_Receive(&huart2, (uint8_t *)buffer, size, HAL_MAX_DELAY);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  char buffer[50];

  RFID_Init();

  while (1)
  {
    RFID_Read(buffer, sizeof(buffer));
    if (strcmp(buffer, "Card Detected") == 0)
    {
      LED_On();
      HAL_Delay(500);
      LED_Off();
    }
  }
}
