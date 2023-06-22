#ifndef SPIDRIVER_H
#define SPIDRIVER_H

#include "stm32f4xx.h"
#include "stdint.h"
#include "GPIOxDriver.h"

#define  SPI_MODE_0    0
#define  SPI_MODE_1    1
#define  SPI_MODE_2    2
#define  SPI_MODE_3    3

#define SPI_BAUDRATE_FPCLK_2      0
#define SPI_BAUDRATE_FPCLK_4      1
#define SPI_BAUDRATE_FPCLK_8      2
#define SPI_BAUDRATE_FPCLK_16     3
#define SPI_BAUDRATE_FPCLK_32     4
#define SPI_BAUDRATE_FPCLK_64     5
#define SPI_BAUDRATE_FPCLK_128    6
#define SPI_BAUDRATE_FPCLK_256    7

#define SPI_RECEIVE_ONLY    0
#define SPI_FULL_DUPLEX     1

#define SPI_DATASIZE_8_BIT    0
#define SPI_DATASIZE_16_BIT   1


typedef struct SpiDriver
{
    uint8_t SPI_mode;
    uint8_t SPI_baudrate;
    uint8_t SPI_fullDupplexEnable;
    uint8_t SPI_datasize;
    uint8_t SPI_enableRX;
    uint8_t SPI_enableTX;
}SPI_Config_t;

typedef struct 
{
   SPI_TypeDef *ptrSPIx;
   SPI_Config_t SPI_Config;
   GPIO_Handler_t SPI_slavePin;
}SPI_handler_t;


/*Prototipos de las funciones*/\
void SPI_pins_init();
void spi_config(SPI_handler_t ptrHandlerSPI);
void spi_transmit(SPI_handler_t ptrHandlerSPI, uint8_t *ptrData, uint32_t dataSize);
void spi_receive(SPI_handler_t ptrHandlerSPI, uint8_t *ptrData, uint32_t dataSize);
void spi_selectSlave(SPI_handler_t *ptrHandlerSPI);
void spi_unSelectSlave(SPI_handler_t *ptrHandlerSPI);

#endif
