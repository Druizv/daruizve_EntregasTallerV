/*I2CDriver
 *
 */

#include <stdint.h>
#include "I2CDriver.h"

/*
 * Recordar que se debe configurar los pines para el I2C (SDA y SCL),
 * para lo cual se necesita el modulo GPIO y los pines configurados
 * en el modo alternate function.
 * Además, estos pines deben ser configurados como salidas open-drain
 * y con la resistencia en modo pull-up
 */
void i2c_config(I2C_Handler_t *ptrHandlerI2C){
	//1 Activamos la señal de reloj para el módulo I2C seleccionado
	if (ptrHandlerI2C->ptrI2Cx == I2C1){
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	}
	else if (ptrHandlerI2C->ptrI2Cx == I2C2){
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	}
	else if (ptrHandlerI2C->ptrI2Cx == I2C3){
		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
	}

	//2.Reiniciamos el periférico, de forma que inicia en un estado conocido
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_SWRST;
	__NOP();
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_SWRST;

	//3. Indicamos cual es la velocidad del reloj principal, que es la señal utilizada
	//por el periferico para activar la señal de reloj para el bus I2C
	ptrHandlerI2C->ptrI2Cx->CR2 &= ~(0b111111 << I2C_CR2_FREQ_Pos);//borramos la configuracion anterior
	ptrHandlerI2C->ptrI2Cx->CR2 |= (MAIN_CLOCK_16_MHz_FOR_I2C << I2C_CR2_FREQ_Pos);

	/*Configuramos el modo I2C en el que el sistema funciona
	 * en esta configuracion se incluye tambien la velocidad del reloj
	 * y el tiempo maximo para el cambio de la señal (T-Rise)
	 * Todo comienza con los dos registros en cero
	 */
	ptrHandlerI2C->ptrI2Cx->CCR = 0;
	ptrHandlerI2C->ptrI2Cx->TRISE = 0;

	if (ptrHandlerI2C->modeI2C == I2C_MODE_SM){
		//Estamos en el modo "standar" (SM MODE)
		//Seleccionamos el modo estandar
		ptrHandlerI2C->ptrI2Cx->CCR &= ~I2C_CCR_FS;

		//configuramos el registro que se encarga de generar la señal de reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_SM_SPEED_100KHz << I2C_CCR_CCR_Pos);

		//Configuramos el registro que controla el tiempo T-Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_SM;
	}
	else{
		//Estamos en modo "Fast" (FM mode)
		//Seleccionamos el modo Fast
		ptrHandlerI2C->ptrI2Cx->CCR |= I2C_CCR_FS;

		//configuramos el registro que se encarga de generar la señal de reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (I2C_MODE_FM_SPEED_400KHz << I2C_CCR_CCR_Pos);
	}

	/* 5. Activamos el modulo I2C */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_PE;

}

/*8.Generamos la condicion de stop*/
void i2c_stopTransaction(I2C_Handler_t *ptrHandlerI2C){
	//7. Generamos la condicion de stop
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_STOP;
}
/*1. Verificamos que la linea no este ocupada - bit "busy" en  I"C CR2
 *2.Generamos la señal "start"
 *2a. Esperamos a que la bandera del evento "start" se levante
 *Mientras esperamos, el valor de SB es 0, entonces la negacion (!) es 1*/
void i2c_startTransaction(I2C_Handler_t *ptrHandlerI2C){
	/*1. Verificamos que la linea no este ocupada - bit "busy" en  I"C CR2*/
	while(ptrHandlerI2C->ptrI2Cx->SR2 & I2C_SR2_BUSY){
		__NOP();
	}

	 //2.Generamos la señal "start"
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/*2a. Esperamos a que la bandera del evento "start" se levante
	 *Mientras esperamos, el valor de SB es 0, entonces la negacion (!) es 1*/
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB)){
		__NOP();
	}

}

void i2c_reStartTransaction(I2C_Handler_t *ptrHandlerI2C){
	//generamos la señal "start"
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/*2a. Esperamos a que la bandera del evento "start" se levante
	 *Mientras esperamos, el valor de SB es 0, entonces la negacion (!) es 1*/
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB)){
		__NOP();
	}
}

//Activamos la indicacion para no-ACK (indicacion para el slave de terminar)
void i2c_sendNoACK(I2C_Handler_t *ptrHandlerI2C){
	//Debemos escribir 0 en la posicion ACK del registro de contro 1
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_ACK;
}

//Activamos la indicacion para ACK (indicacion para el slave de continuar)
void i2c_sendACK(I2C_Handler_t *ptrHandlerI2C){
	//Debemos escribir 0 en la posicion ACK del registro de contro 1
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_ACK;
}


void sendSlaveAddressRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress,uint8_t readOrWrite){
	/*0.Definimos la variable auxiliar*/
	uint8_t auxByte = 0;
	(void) auxByte;
	/*3.Enviamos la direccion del Slave y el bit que indica que desamos escribir 0
	 * En el siguiente paso se enviara la direccion de memoria que se desea escribir*/
	ptrHandlerI2C->ptrI2Cx->DR = (slaveAddress << 1) | readOrWrite;

	/*3.1 esperamos hasta que la bandera del evento "addr" se levante
	 * esto nos indica que la direccion fue enviada satisfactoriamente */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 &= I2C_SR1_ADDR)){
		__NOP();
	}

	/*Debemos limpiar la bandera de la recepcion ACK de la "addr", para lo cual
	 * debemos leer en secuencia primero el I2C CR1 y luego el I2C CR2*/
	auxByte = ptrHandlerI2C->ptrI2Cx->CR1;
	auxByte = ptrHandlerI2C->ptrI2Cx->CR2;
}

void sendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddr){
	/*escribimos la direccion de memoria que deseamos leer*/
	ptrHandlerI2C->ptrI2Cx->DR = memAddr;

	//4.1 esperamos hasta que el byte sea transmitido
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_TXE)){
		__NOP();
	}
}

void sendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite){
	/*5. Cargamos el valor que deseamos escribir*/
	ptrHandlerI2C->ptrI2Cx->DR = dataToWrite;

	/*6. esperamos hasta que el byte sea transmitido */
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_BTF)){
		__NOP();
	}
}

uint8_t readDataByte(I2C_Handler_t *ptrHandlerI2C){
	//9. Esperamos hasta que el byte sea recibido
	while(!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_RXNE)){
		__NOP();
	}
	ptrHandlerI2C->dataI2C = ptrHandlerI2C->ptrI2Cx->DR;
	return ptrHandlerI2C->dataI2C;
}


uint8_t i2c_readSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead){

	//0.Creamos una variable auxiliar para recibir el dato que leemos
	uint8_t auxRead = 0;

	//1.Generamos la condicion de "start"
	i2c_startTransaction(ptrHandlerI2C);

	//2.Enviamos la direccion del esclavo y la indicacion de ESCRIBIR
	i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,I2C_WRITE_DATA);

	//3.Enviamos la direccion de memoria que deseamos leer
	i2c_sendMemoryAddress(ptrHandlerI2C, regToRead);

	//4.creamos una condicion de restart
	i2c_reStartTransaction(ptrHandlerI2C);

	//5.Enviamos la direccion del esclavo y la indicacion de LEER
	i2c_sendSlaveAddresRW(ptrHandlerI2C,ptrHandlerI2C->slaveAddress,I2C_READ_DATA);

	//6.Leemos el dato que envia el esclavo
	auxRead = i2c_readDataByte(ptrHandlerI2C);

	//7.Generamos la condicion de NoACK, para que el master no responda y el slave solo envie 1 byte
	i2c_sendNoAck(ptrHandlerI2C);

	//8.Generamos la condicion de stop para que el slave se detenga despues de enviar 1 byte
	i2c_stopTransaction(ptrHandlerI2C);

	return auxRead;

}
void writeSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue){
	//1.Generamos la condicion de start
	i2c_startTransaction(ptrHandlerI2C);

	//2.Enviamos la direccion de el esclavo y la indicacion de ESCRIBIR
	i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);

	//3.Enviamos la direccion de memoria que deseamos escribir
	i2c_sendMemoryAddress(ptrHandlerI2C, regToRead);

	//4.Enviamos el valor que deseamos escribir en el registro seleccionado
	i2c_sendDataByte(ptrHandlerI2C, newValue);

	//5.Generamos la condicion de stop para que el slave se detenga despues de enviar 1 byte
	i2c_stopTransaction(ptrHandlerI2C);

}





