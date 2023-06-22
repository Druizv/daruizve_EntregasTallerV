//#include "RFID.h"
//#include "spi.h"
#include "stdbool.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include "SpiDriver.h"
#include "GPIOxDriver.h"
#include "RFID.h"
/*
 * STM32 ->RFID
 * SPI  -> SPI
 * PA8  ->RST
 * PB0  ->CS
 * */
GPIO_Handler_t handlerSpiCLK = {0};
GPIO_Handler_t handlerSpiMISO = {0};
GPIO_Handler_t handlerSpiMOSI = {0};
GPIO_Handler_t handlerSpiSS = {0};

SPI_handler_t handlerSpiMode = {0};

void SPI_pins_init(){
		handlerSpiCLK.pGPIOx											= GPIOA;
		handlerSpiCLK.GPIO_PinConfig.GPIO_PinNumber						= PIN_5;
		handlerSpiCLK.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
		handlerSpiCLK.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_PUSHPULL;
		handlerSpiCLK.GPIO_PinConfig.GPIO_PinSpeed						= GPIO_OSPEED_FAST;
		handlerSpiCLK.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
		handlerSpiCLK.GPIO_PinConfig.GPIO_PinAltFunMode					= AF5;
		GPIO_Config(&handlerSpiCLK);


		handlerSpiMISO.pGPIOx											= GPIOA;
		handlerSpiMISO.GPIO_PinConfig.GPIO_PinNumber					= PIN_6;
		handlerSpiMISO.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
		handlerSpiMISO.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
		handlerSpiMISO.GPIO_PinConfig.GPIO_PinSpeed						= GPIO_OSPEED_FAST;
		handlerSpiMISO.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
		handlerSpiMISO.GPIO_PinConfig.GPIO_PinAltFunMode				= AF5;
		GPIO_Config(&handlerSpiMISO);

		handlerSpiMOSI.pGPIOx											= GPIOA;
		handlerSpiMOSI.GPIO_PinConfig.GPIO_PinNumber					= PIN_7;
		handlerSpiMOSI.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
		handlerSpiMOSI.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
		handlerSpiMOSI.GPIO_PinConfig.GPIO_PinSpeed						= GPIO_OSPEED_FAST;
		handlerSpiMOSI.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
		handlerSpiMOSI.GPIO_PinConfig.GPIO_PinAltFunMode				= AF5;
		GPIO_Config(&handlerSpiMOSI);

		handlerSpiSS.pGPIOx											= GPIOB;
		handlerSpiSS.GPIO_PinConfig.GPIO_PinNumber					= PIN_0;
		handlerSpiSS.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
		handlerSpiSS.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
		handlerSpiSS.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_FAST;
		handlerSpiSS.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
		handlerSpiSS.GPIO_PinConfig.GPIO_PinAltFunMode				= AF0;
		GPIO_Config(&handlerSpiSS);

		handlerSpiMode.ptrSPIx											= SPI1;
		handlerSpiMode.SPI_Config.SPI_mode								= SPI_MODE_3;
		handlerSpiMode.SPI_Config.SPI_fullDupplexEnable					= SPI_FULL_DUPLEX;
		handlerSpiMode.SPI_Config.SPI_datasize							= SPI_DATASIZE_8_BIT;
		handlerSpiMode.SPI_Config.SPI_baudrate                          = SPI_BAUDRATE_FPCLK_4; //se configura despues 1MH y necesito 13
		handlerSpiMode.SPI_slavePin										= handlerSpiSS;
		spi_config(handlerSpiMode);

}
/**
 * @brief initialize function
 */

/**
 * @brief read register
 */

bool rc522_toCard(
    uint8_t command, //comando enviado a la tarjeta
    uint8_t* sendData, // Datos enviados a la tarjeta
    uint8_t sendLen,  //Longitud de los datos enviados
    uint8_t* backData, //Datos recibidos de la tarjeta
    uint16_t* backLen); // Longitud de los datos recibidos

//Devuelve el booleano true o False de esta operacion

bool rc522_request(uint8_t reqMode, uint8_t *tagType); //Comando de solicitud con el objetivo de identificar si hay tarjeta
//reqMode : modo de solicitud usado para la tarjeta
//Tipo de tarjeta detectada, si es tarjeta o tag
bool rc522_antiColl(uint8_t* serNum);//operacion de seguridad por si hay varias tarjetas cerca
//serNum es un puntero donde se almacena el numero de serie de la tarjeta

void spi_cs_rfid_write(bool state)
{
	if(state) //si el estado del pin es TRUE significa que el pin esta seleccionado si es false no esta seleccionado
	  {
	    //GPIOB->ODR |= (1UL << 0);
		GPIOB->ODR |= GPIO_ODR_OD0;
	  }
	  else
	  {
	    //GPIOB->ODR &= ~(1UL << 0);
		  GPIOB->ODR &= ~GPIO_ODR_OD0;
	  }
}

uint8_t rc522_regRead8(uint8_t reg)
{
  spi_cs_rfid_write(0); //Devuelve false a esa funcion singifica que esta listo para recibir datos
  reg = ((reg << 1) & 0x7E) | 0x80; //prepara Reg para que el bit mas significativo sea 1 y el menos no tenga cambios
  spi_transmit(handlerSpiMode,&reg,1); //Envia el registro a traves del SPI
  uint8_t dataRd=0;
  spi_receive(handlerSpiMode,&dataRd, 1); //Recibe los datos de la tarjeta
  spi_cs_rfid_write(1);    //Activa el SS y en alto (1) para que no responda a transmisiones
  return dataRd;		   //Devuelve los datos leidos
}

/**
 * @brief write register
 */
void rc522_regWrite8(uint8_t reg, uint8_t data8)
{
  spi_cs_rfid_write(0); //Aciva el periferico para la comunicacion
  uint8_t txData[2] = {0x7E&(reg << 1), data8}; //De nuevo se desplaza a la izquierda 1 vez
  //el bit mas significativo sea 1 con tamaño 2
  spi_transmit(handlerSpiMode,txData, 2);//Transmite los datos del arreglo a traves de SPI
  spi_cs_rfid_write(1); //Apaga la comunicacion SS
}

/**
 * @brief set bit
 */
void rc522_setBit(uint8_t reg, uint8_t mask)
{
  rc522_regWrite8(reg, rc522_regRead8(reg)|mask);
}

/**
 * @brief clear bit
 */
void rc522_clearBit(uint8_t reg, uint8_t mask)
{
  rc522_regWrite8(reg, rc522_regRead8(reg)&(~mask));
}

/**
 * @brief reset function
 */
void rc522_reset(void)
{
  rc522_regWrite8(0x01, 0x0F);
}

/**
 * @brief Antenna ON
 */
void rc522_antennaON(void)
{
  uint8_t temp;

  temp = rc522_regRead8(MFRC522_REG_TX_CONTROL);
  if (!(temp & 0x03)) {
    rc522_setBit(MFRC522_REG_TX_CONTROL, 0x03);
  }
}

/**
 * @brief Check card
 */
bool rc522_checkCard(uint8_t *id)
{
  bool status=false;
  //Busca tarjetas, y retorna el tipo de tarjeta
    status = rc522_request(PICC_REQIDL, id);
    if (status == true) {
      //Card detected
      //Anti-collision, return card serial number 4 bytes
      status = rc522_antiColl(id);
    }
    rc522_halt();      //Command card into hibernation

    return status;
}

/**
 * @brief Request function
 */
bool rc522_request(uint8_t reqMode, uint8_t *tagType)
{
  bool status=false;
  uint16_t backBits;
  rc522_regWrite8(MFRC522_REG_BIT_FRAMING, 0x07);
  tagType[0] = reqMode;
  status = rc522_toCard(PCD_TRANSCEIVE, tagType, 1, tagType, &backBits);
  if ((status != true) || (backBits != 0x10)) {
    status = false;
  }
  return status;
}

/**
 * @brief to Card
 */
bool rc522_toCard(
    uint8_t command,
    uint8_t* sendData,
    uint8_t sendLen,
    uint8_t* backData,
    uint16_t* backLen)
{
  bool status = false;
  uint8_t irqEn = 0x00;
  uint8_t waitIRq = 0x00;
  uint8_t lastBits;
  uint8_t n;
  uint16_t i;

  switch (command) {
    case PCD_AUTHENT: {
      irqEn = 0x12;
      waitIRq = 0x10;
      break;
    }
    case PCD_TRANSCEIVE: {
      irqEn = 0x77;
      waitIRq = 0x30;
      break;
    }
    default:
      break;
  }

  rc522_regWrite8(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
  rc522_clearBit(MFRC522_REG_COMM_IRQ, 0x80);
  rc522_setBit(MFRC522_REG_FIFO_LEVEL, 0x80);

  rc522_regWrite8(MFRC522_REG_COMMAND, PCD_IDLE);

  //Writing data to the FIFO
  for (i = 0; i < sendLen; i++) {
    rc522_regWrite8(MFRC522_REG_FIFO_DATA, sendData[i]);
  }

  //Execute the command
  rc522_regWrite8(MFRC522_REG_COMMAND, command);
  if (command == PCD_TRANSCEIVE) {
    rc522_setBit(MFRC522_REG_BIT_FRAMING, 0x80);   //StartSend=1,transmission of data starts
  }

  //Waiting to receive data to complete
  i = 100;  //i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
  do {
    //CommIrqReg[7..0]
    //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
    n = rc522_regRead8(MFRC522_REG_COMM_IRQ);
    i--;
  } while ((i!=0) && !(n&0x01) && !(n&waitIRq));

  rc522_clearBit(MFRC522_REG_BIT_FRAMING, 0x80);     //StartSend=0

  if (i != 0)  {
    if (!(rc522_regRead8(MFRC522_REG_ERROR) & 0x1B)) {
      status = true;
      if (n & irqEn & 0x01) {
        status = false;
      }

      if (command == PCD_TRANSCEIVE) {
        n = rc522_regRead8(MFRC522_REG_FIFO_LEVEL);
        uint8_t l = n;
        lastBits = rc522_regRead8(MFRC522_REG_CONTROL) & 0x07;
        if (lastBits) {
          *backLen = (n - 1) * 8 + lastBits;
        } else {
          *backLen = n * 8;
        }

        if (n == 0) {
          n = 1;
        }
        if (n > MFRC522_MAX_LEN) {
          n = MFRC522_MAX_LEN;
        }

        //Reading the received data in FIFO
        for (i = 0; i < n; i++) {
          uint8_t d = rc522_regRead8(MFRC522_REG_FIFO_DATA);
          if (l == 4)
            printf("%02x ", d);
          backData[i] = d;
        }
        if (l==4)
          printf("\r\n");
        return status;
      }
    } else {
      printf("error\r\n");
      status = false;
    }
  }

  return status;
}

bool rc522_antiColl(uint8_t* serNum)
{
  bool status;
  uint8_t i;
  uint8_t serNumCheck = 0;
  uint16_t unLen;
  //for (i = 0; i < 4; i++)
//    printf("Anticoll In %d: 0x%02x\r\n", i, serNum[i]);


  rc522_regWrite8(MFRC522_REG_BIT_FRAMING, 0x00);    //TxLastBists = BitFramingReg[2..0]

  serNum[0] = PICC_ANTICOLL;
  serNum[1] = 0x20;
  status = rc522_toCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

  //for (i = 0; i < 4; i++)
//      printf("Anticoll ToCard %d: 0x%02x\r\n", i, serNum[i]);

  if (status == true) {
    //Check card serial number
    for (i = 0; i < 4; i++) {
      serNumCheck ^= serNum[i];
    }
    if (serNumCheck != serNum[i]) {
      status = false;
    }
  }
  return status;
}

void rc522_halt(void)
{
  uint16_t unLen;
  uint8_t buff[4];

  buff[0] = PICC_HALT;
  buff[1] = 0;
  rc522_calculateCRC(buff, 2, &buff[2]);

  rc522_toCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

void rc522_calculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData)
{
  uint8_t i, n;

  rc522_clearBit(MFRC522_REG_DIV_IRQ, 0x04);     //CRCIrq = 0
  rc522_setBit(MFRC522_REG_FIFO_LEVEL, 0x80);      //Clear the FIFO pointer
  //Write_MFRC522(CommandReg, PCD_IDLE);

  //Writing data to the FIFO
  for (i = 0; i < len; i++) {
    rc522_regWrite8(MFRC522_REG_FIFO_DATA, *(pIndata+i));
  }
  rc522_regWrite8(MFRC522_REG_COMMAND, PCD_CALCCRC);

  //Wait CRC calculation is complete
  i = 0xFF;
  do {
    n = rc522_regRead8(MFRC522_REG_DIV_IRQ);
    i--;
  } while ((i!=0) && !(n&0x04));      //CRCIrq = 1

  //Read CRC calculation result
  pOutData[0] = rc522_regRead8(MFRC522_REG_CRC_RESULT_L);
  pOutData[1] = rc522_regRead8(MFRC522_REG_CRC_RESULT_M);
}

/**
 * @brief compare IDs
 */
bool rc522_compareIds(uint8_t *idCurrent, uint8_t *idReference)
{
  uint8_t i;
  for(i=0; i<4;i++)
  {
    if(idCurrent[i] != idReference[i])
    {
      return false;
    }
  }
  return true;
}

void rc522_init(void)
{
	/*
	 * STM32 ->RFID
	 * SPI  -> SPI
	 * PA8  ->RST
	 * PB0  ->CS
	 * */
  SPI_pins_init();


  GPIOA->MODER|=GPIO_MODER_MODE8_0;
  GPIOA->MODER&=~GPIO_MODER_MODE8_1;

  RCC->AHB1ENR|=RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER|=GPIO_MODER_MODE0_0;
  GPIOB->MODER&=~GPIO_MODER_MODE0_1;
  GPIOA->BSRR=GPIO_BSRR_BR8; //Apaga el pin
  for(volatile int i=0;i<100000;i++); //Genera tiempo
  GPIOA->BSRR=GPIO_BSRR_BS8; //enciende el pin
  for(volatile int i=0;i<100000;i++); //mas tiempo
  rc522_reset(); //Reinicia el dispositivo

  rc522_regWrite8(MFRC522_REG_T_MODE, 0x80);  //Se activa la señal del reloj
  rc522_regWrite8(MFRC522_REG_T_PRESCALER, 0xA9); //Se define el prescaler
  rc522_regWrite8(MFRC522_REG_T_RELOAD_L, 0xE8); //se define el tiempo abajo del reloj
  rc522_regWrite8(MFRC522_REG_T_RELOAD_H, 0x03); //se define el tiempo arriba del reloj


  rc522_regWrite8(MFRC522_REG_TX_AUTO, 0x40); //transmision automatica
  rc522_regWrite8(MFRC522_REG_MODE, 0x3D); //modo de operacion

  rc522_antennaON();   //Open the antenna
}

