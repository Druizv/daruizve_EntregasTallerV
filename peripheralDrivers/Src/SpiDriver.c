
#include "SpiDriver.h"

void spi_config(SPI_handler_t ptrHandlerSPI){
	//1. activamos la señal del reloj del periferico
	if(ptrHandlerSPI.ptrSPIx == SPI1){
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	}
	else if(ptrHandlerSPI.ptrSPIx == SPI2){
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	}
	else if(ptrHandlerSPI.ptrSPIx == SPI3){
		RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
	}
	else if(ptrHandlerSPI.ptrSPIx == SPI4){
		RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;
	}
	else if(ptrHandlerSPI.ptrSPIx == SPI5){
		RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;
	}

	//2. Limpiamos el registro de configuración para comenzar de cero
	ptrHandlerSPI.ptrSPIx->CR1 = 0x00;

	//3. Configuramos la velocidad a la que se maneja el reloj del SPI
	ptrHandlerSPI.ptrSPIx->CR1 |= (ptrHandlerSPI.SPI_Config.SPI_baudrate << SPI_CR1_BR_Pos);
//	ptrHandlerSPI.ptrSPIx->CR1 &= ~(SPI_CR1_BR_1 | SPI_CR1_BR_0); SE DIVIDE EN 4
	// Configurar el baudrate a 13.56MHz
	//ptrHandlerSPI.ptrSPIx->CR1 &= ~SPI_CR1_BR_Msk;
	//ptrHandlerSPI.ptrSPIx->CR1 |= (SPI_BAUDRATE_FPCLK_DIV_2 << SPI_CR1_BR_Pos);



	//4. Configuramos el modo
	switch (ptrHandlerSPI.SPI_Config.SPI_mode){
	case 0 :
		ptrHandlerSPI.ptrSPIx->CR1 &= ~(SPI_CR1_CPHA);
		ptrHandlerSPI.ptrSPIx->CR1 &= ~(SPI_CR1_CPOL);
		break;

	case 1:
		ptrHandlerSPI.ptrSPIx->CR1 |=  (SPI_CR1_CPHA);
		ptrHandlerSPI.ptrSPIx->CR1 &= ~(SPI_CR1_CPOL);
		break;

	case 2:
		ptrHandlerSPI.ptrSPIx->CR1 &= ~(SPI_CR1_CPHA);
		ptrHandlerSPI.ptrSPIx->CR1 |= (SPI_CR1_CPOL);
		break;

	case 3:
		ptrHandlerSPI.ptrSPIx->CR1 |= (SPI_CR1_CPHA);
		ptrHandlerSPI.ptrSPIx->CR1 |= (SPI_CR1_CPOL);
		break;

	default:
		//Configuramos el modo 00 como caso por defecto
		ptrHandlerSPI.ptrSPIx->CR1 &= ~(SPI_CR1_CPHA);
		ptrHandlerSPI.ptrSPIx->CR1 &= ~(SPI_CR1_CPOL);
		break;

	}

	//5. Configuramos a fullduplex a solo recepcion
	if(ptrHandlerSPI.SPI_Config.SPI_fullDupplexEnable == SPI_FULL_DUPLEX){
		//Selecciona full-duplex
		ptrHandlerSPI.ptrSPIx->CR1 &= ~SPI_CR1_RXONLY;
		//Seleccionamos el modo unidireccional (cada linea tiene solo una direcció)
		ptrHandlerSPI.ptrSPIx->CR1 &= ~SPI_CR1_BIDIMODE; //revisar si no funciona <===========
	}
	else{
		//Selecciona solo RX activado
		ptrHandlerSPI.ptrSPIx->CR1 |= SPI_CR1_RXONLY;
	}

	//6. Modo de transferencia en MSB-first
	//ptrHandlerSPI.ptrSPIx->CR1 |= SPI_CR1_LSBFIRST;  Lo necesito apagado

	ptrHandlerSPI.ptrSPIx->CR1 &= ~SPI_CR1_LSBFIRST;

	//7. Activamos para que el equipo se comporte como maestro de la red
	ptrHandlerSPI.ptrSPIx->CR1 |= SPI_CR1_MSTR;

	//8. Configuramos el formato del dato (tamaño) para que sea de 8-bits
	if(ptrHandlerSPI.SPI_Config.SPI_datasize == SPI_DATASIZE_8_BIT){
		//Frame de datos de 8-bits
		ptrHandlerSPI.ptrSPIx->CR1 &= ~SPI_CR1_DFF;
	}
	else{
		//Configuramos el tamaño del dato de 16 bits
		ptrHandlerSPI.ptrSPIx->CR1 |= SPI_CR1_DFF;
	}

	/* Configuramos para que el control del pin SS (seleccion de slave)
	 * sea controlado por software (nosotros debemos hacer ese control)
	 * De la otra forma, sera el hardware el que controla la seleccion del slave*/
	ptrHandlerSPI.ptrSPIx->CR1 |= SPI_CR1_SSM;
	ptrHandlerSPI.ptrSPIx->CR1 |= SPI_CR1_SSI;

	//10. Activamos el periferico SPI
	ptrHandlerSPI.ptrSPIx->CR1 |= SPI_CR1_SPE;

}

void spi_transmit(SPI_handler_t ptrHandlerSPI, uint8_t *data,uint32_t size)
{
	uint8_t auxData;
	(void) auxData;
	uint32_t i=0;

	while(i<size)
	{
		/*Wait until TXE is set*/
		while(!(SPI1->SR & (SPI_SR_TXE))){}

		/*Write the data to the data register*/
		SPI1->DR = data[i];
		i++;
	}
	/*Wait until TXE is set*/
	while(!(SPI1->SR & (SPI_SR_TXE))){}

	/*Wait for BUSY flag to reset*/
	while((SPI1->SR & (SPI_SR_BSY))){}

	/*Clear OVR flag*/
		auxData = ptrHandlerSPI.ptrSPIx->DR;
		auxData = ptrHandlerSPI.ptrSPIx->SR;
}

//void spi_transmit(SPI_handler_t ptrSPIx ,uint8_t * ptrData, uint32_t dataSize){
//	uint8_t auxData;
//	(void) auxData;
//
//	while(dataSize > 0 ){
//		//Esperamos a que el buffer este vacio
//		while(!(ptrHandlerSPI.ptrSPIx->SR & SPI_SR_TXE)){
//			__NOP();
//		}
//		//Enviamos el dato que apunta al puntero
//		ptrHandlerSPI.ptrSPIx->DR = (uint8_t) (0XFF & *ptrData);
//
//		//Actualizamos el puntero y el numero de datos que faltan por enviar
//		ptrData++;
//		dataSize--;
//	}
//
//	//esperamos a que el buffer este vacio de nuevo
//	while(!(ptrHandlerSPI.ptrSPIx->SR & SPI_SR_TXE)){
//		__NOP();
//	}
//	//Esperamos a que la bandera de ocupado (busy) baje (observar que la logica cambia)
//	while((ptrHandlerSPI.ptrSPIx->SR & SPI_SR_BSY)){
//		__NOP();
//	}
//
//	//Debemos limpiar la bandera del overRun (que a veces se levanta)
//	//Para eso debemos leer DR y luego SR del modulo SPI (pag 599)
//	auxData = ptrHandlerSPI.ptrSPIx->DR;
//	auxData = ptrHandlerSPI.ptrSPIx->SR;
//
//}

void spi_receive(SPI_handler_t ptrHandlerSPI, uint8_t * ptrData, uint32_t dataSize){
	while(dataSize){
		//esperamos a que el buffer este vacio de nuevo
		while(!(ptrHandlerSPI.ptrSPIx->SR & SPI_SR_TXE)){
			__NOP();
		}
		//Enviamos un valor Dummy
		ptrHandlerSPI.ptrSPIx->DR = 0x00;

		//Esperamos a que el buffer tenga un dato que leer
		while(!(ptrHandlerSPI.ptrSPIx->SR & SPI_SR_RXNE)){
			__NOP();
		}

		//Cargamos el valor en el puntero
		*ptrData = ptrHandlerSPI.ptrSPIx->DR;

		//Actualizamos el puntero y el tamaño de los datos
		ptrData++;
		dataSize--;
	}
}


//void spi_received(SPI_handler_t ptrHandlerSPI, uint8_t * ptrData, uint32_t dataSize){
//	while(dataSize){
//		//esperamos a que el buffer este vacio de nuevo
//		while(!(ptrHandlerSPI.ptrSPIx->SR & SPI_SR_TXE)){
//			__NOP();
//		}
//		//Enviamos un valor Dummy
//		ptrHandlerSPI.ptrSPIx->DR = 0x00;
//
//		//Esperamos a que el buffer tenga un dato que leer
//		while(!(ptrHandlerSPI.ptrSPIx->SR & SPI_SR_RXNE)){
//			__NOP();
//		}
//
//		//Cargamos el valor en el puntero
//		*ptrData = ptrHandlerSPI.ptrSPIx->DR;
//
//		//Actualizamos el puntero y el tamaño de los datos
//		ptrData++;
//		dataSize--;
//	}
//}

//Seleccionamos el esclavo llevando el pin SS a GND
void spi_selectSlave(SPI_handler_t* ptrHandlerSPI){
	GPIO_WritePin(&ptrHandlerSPI->SPI_slavePin, RESET);
	//GPIOA->BSRR |= (SET << 25);
}

void spi_unselectSlave(SPI_handler_t* ptrHandlerSPI){
	GPIO_WritePin(&ptrHandlerSPI->SPI_slavePin, SET);
	//GPIOA->BSRR |= (SET << 9);
}


