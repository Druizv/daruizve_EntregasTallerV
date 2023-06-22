#include "LiquidCrystal_PCF8574.h"
//#include "LCDDriver.h"
#include "delay.h"
#include "stdio.h"
#include "RFID.h"
#include <string.h>
#include "RTC.h"
#include "keypad.h"
//#include "I2CDriver.h"
#include "GPIOxDriver.h"

GPIO_Handler_t handler_i2cSCL = {0};
GPIO_Handler_t handler_i2cSDA = {0};

//I2C_Handler_t handlerLCD = {0};

//Para generar el token
uint16_t *date = 0;
uint8_t hour = 0;
uint8_t minute = 0;
uint8_t day = 0;
uint8_t month = 0;
uint8_t year = 0;

char acces[64];
long seed = 123456789;
int token = 0;
char userToken[8] = {8,8,5,9};
char tokenArray[8];

//SPI

//Keypad
bool    keypadflag   		 = false;
char     key;
uint8_t rxData				 = 0;
char    bufferReception[64]  = {0};
int 	counterReception 	 = 0;
bool    stringComplete 		 = false;

char lcd_data[30];

uint8_t rfid_id[4];
uint8_t base_rfid[4] = {0x7f,0x69,0xa8,0x2};

RTC_Handler_t handlerRTC = {0};


int generateToken(long seed, int year, int month, int day, int hour, int minute) {
	date 	= read_date();
	minute 	= date[1];
	hour 	= date[2];
	day		= date[3];
	month	= date[4];
	year	= date[5]+2000;
    int token = ((seed % 10000) + ((year + month + day + hour + minute) % 10000)) % 10000;
    return token;
}
void init_system(void);



int main(void){
	//Activamos el coprocesador matematico
	SCB->CPACR |= (0xF <<20);
		init_system();

		rc522_init();

		//LCD_Init(&handlerLCD);
		lcd_init();

		keypad_init();

//		LCD_setCursor(&handlerLCD,0,0);
//		LCD_sendSTR(&handlerLCD,"Proyecto");
//		LCD_setCursor(&handlerLCD,0,1);
//		LCD_sendSTR(&handlerLCD,"Taller V");
//		LCD_setCursor(&handlerLCD,0,2);
//		LCD_sendSTR(&handlerLCD,"Daniel Ruiz");
//		delay(2000);
//		LCD_Clear(&handlerLCD);

		setCursor(0, 0);
		lcd_send_string("proyecto");
		setCursor(0, 1);
		lcd_send_string("Taller V");
		setCursor(0, 2);
		lcd_send_string("Daniel Ruiz");
		delay(2000);
		lcd_clear();

		while(1){

//			LCD_setCursor(&handlerLCD,0,1);
//			LCD_sendSTR(&handlerLCD,"Acerque la tarjeta");
//			delay(2000);
//
				if(rc522_checkCard(rfid_id)){
							lcd_clear();
							//LCD_Clear(&handlerLCD);

							//LCD_setCursor(&handlerLCD,0,0);
							setCursor(0,0);
							if (memcmp(rfid_id, base_rfid, sizeof(rfid_id)) == 0) {

								//LCD_sendSTR(&handlerLCD,"RFID accepted");
								//LCD_setCursor(&handlerLCD,0,1);
								//LCD_sendSTR(&handlerLCD,"Generated token");
								lcd_send_string("ID aceptada");
								setCursor(0, 1);
								lcd_send_string("Generando token");
								delay(1000);
								token = generateToken(seed, year, month, day, hour, minute);
								snprintf(tokenArray, sizeof(tokenArray), "%04d", token);

								//LCD_setCursor(&handlerLCD,0,2);
								setCursor(0,2);
								sprintf(acces,"Token: %04d",token);

								//LCD_sendSTR(&handlerLCD,acces);
								lcd_send_string(acces);
								delay(2000);

								//LCD_Clear(&handlerLCD);
								lcd_clear();


								//LCD_setCursor(&handlerLCD,0, 0);
								//LCD_sendSTR(&handlerLCD,"introduzca el token");
								//LCD_setCursor(&handlerLCD,0, 1);
								//LCD_sendSTR(&handlerLCD,"Luego presione A");
								setCursor(0, 0);
								lcd_send_string("Introduzca el token");
								setCursor(0, 1);
								lcd_send_string("Luego presione A");
								keypadflag = true;
								delay(30);
							}

							else{
//								LCD_sendSTR(&handlerLCD,"RFID rejected");
//								LCD_setCursor(&handlerLCD,0,1);

								lcd_send_string("ID no valida");
								setCursor(0, 1);
								date = read_date();
								day = date[4];
								month = date[5];
								year = date[6];
								minute 	= date[1];
								hour 	= date[2];
								sprintf(acces, "Fecha:%u/%u/%u", (unsigned int) day,month,year+2000);
								lcd_send_string(acces);
								setCursor(0,2);
								//LCD_sendSTR(&handlerLCD,acces);
								//LCD_setCursor(&handlerLCD,0, 2);
								sprintf(acces, "hora actual:%u:%u", (unsigned int) hour, minute);
								lcd_send_string(acces);
								//LCD_sendSTR(&handlerLCD,acces);
								delay(2000);
								//LCD_Clear(&handlerLCD);
								lcd_clear();							}
				}
				delay(100);


				if(keypadflag){
					key = keypad_read();
					//LCD_setCursor(&handlerLCD,0, 2);
					setCursor(0, 2);
					sprintf(lcd_data,"Key pressed is %c",key);
					lcd_send_string(lcd_data);
					setCursor(0, 3);
					sprintf(lcd_data,"count: %d", counterReception);
					lcd_send_string(lcd_data);
					//LCD_sendSTR(&handlerLCD,lcd_data);
					if(key != '\0'){
						userToken[counterReception] = key;
						counterReception++;
						if (key !='A'){
							key = '\0';
						}


					}
					if (key == 'A'){
						stringComplete = true;
						// Se agrega esta linea para crear el string con el null al final.
						userToken[counterReception-1] = '\0';
						counterReception = 0;
					}


					;




					if(stringComplete){
//						LCD_Clear(&handlerLCD);
//						LCD_setCursor(&handlerLCD,0, 0);
//						LCD_sendSTR(&handlerLCD,"Analizando Token");
//						LCD_setCursor(&handlerLCD,0,1);
//
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Analiazando token");
						setCursor(0, 1);
						sprintf( "%d", userToken);
						//sprintf(lcd_data,"%u%u%u%u", userToken[0], userToken[1], userToken[2], userToken[3]);
//						LCD_sendSTR(&handlerLCD,lcd_data);
//						LCD_setCursor(&handlerLCD,0, 2);
//
						lcd_send_string(lcd_data);
						setCursor(0, 2);

						delay(2000);
						lcd_clear();
						int compare = memcmp(tokenArray, userToken, 4);
						sprintf(lcd_data, "%d",compare);
						lcd_send_string(lcd_data);
						delay (2000);
						lcd_clear();
						setCursor(0, 0);
						for(int loop = 0; loop < sizeof(tokenArray); loop++){
							sprintf(lcd_data, "token array: %c", tokenArray[loop]);
							lcd_send_string(lcd_data);
							setCursor(0, 1);
							sprintf(lcd_data, "usertoken: %c", userToken[loop]);
							lcd_send_string(lcd_data);
							delay(1000);
							lcd_clear();

						}
//						sprintf(lcd_data, "%d", tokenArray);
//						lcd_send_string(lcd_data);
//						setCursor(0, 1);
//						sprintf(lcd_data, "%d", userToken);
//						lcd_send_string(lcd_data);
//						delay(4000);
//						lcd_clear();

						if (memcmp(tokenArray, userToken, sizeof(userToken)) == 0){
//							LCD_sendSTR(&handlerLCD,"Token autorizado");
//							LCD_setCursor(&handlerLCD,0, 2);
//							LCD_sendSTR(&handlerLCD,"lo hiciste papu");

							lcd_send_string("Token autorizado");
							setCursor(0,2);
							lcd_send_string("Lo hiciste papu");
							for(int i = 0; i < 8; i++){
								userToken[i] = '\0';
							}
						}
						else{
							//LCD_sendSTR(&handlerLCD,"Token erroneo");
							lcd_send_string("Token Erroneo");
							delay(2000);
							lcd_clear();
							//LCD_Clear(&handlerLCD);
							for(int i = 0; i <8; i++){
								userToken[i] = '\0';
							}
						}
						keypadflag = false;
						stringComplete = false;


					}
				}

		}
}

void init_system(void){
	handlerRTC.RTC_Config.RTC_Hours = (unsigned int) 5;
	handlerRTC.RTC_Config.RTC_Minutes = (unsigned int) 40;
	handlerRTC.RTC_Config.RTC_ValueDay = (unsigned int) 19;
	handlerRTC.RTC_Config.RTC_Month = (unsigned int) 6;
	handlerRTC.RTC_Config.RTC_Year = (unsigned int) 23;
	handlerRTC.RTC_Config.RTC_TimeNotation = (unsigned int) 0;
	RTC_Config(&handlerRTC);

	handler_i2cSCL.pGPIOx												= GPIOB;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinNumber						= PIN_8;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinMode							= GPIO_MODE_ALTFN;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_OPENDRAIN;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinPuPdControl        			= GPIO_PUPDR_PULLUP;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinSpeed              			= GPIO_OSPEED_FAST;
	handler_i2cSCL.GPIO_PinConfig.GPIO_PinAltFunMode					= AF4;
	GPIO_Config(&handler_i2cSCL);

	handler_i2cSDA.pGPIOx												= GPIOB;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinNumber						= PIN_9;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinMode							= GPIO_MODE_ALTFN;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_OPENDRAIN;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinPuPdControl        			= GPIO_PUPDR_PULLUP;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinSpeed              			= GPIO_OSPEED_FAST;
	handler_i2cSDA.GPIO_PinConfig.GPIO_PinAltFunMode					= AF4;
	GPIO_Config(&handler_i2cSDA);

	// configuracion de la LCD con la direccion del esclavo
//	handlerLCD.ptrI2Cx		= I2C1;
//	handlerLCD.modeI2C		= personal;
//	handlerLCD.slaveAddress	= LCD_ADDRESS;
//	i2c_config(&handlerLCD);

}


