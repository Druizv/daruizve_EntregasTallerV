#include "LiquidCrystal_PCF8574.h"
//#include "LCDDriver.h"
#include "delay.h"
#include "stdio.h"
#include <stdlib.h>
#include "RFID.h"
#include <string.h>
#include "RTC.h"
#include "keypad.h"
#include "GPIOxDriver.h"
#include "BasicTimer.h"

GPIO_Handler_t handler_i2cSCL = {0};
GPIO_Handler_t handler_i2cSDA = {0};
GPIO_Handler_t handlerSolen	  = {0};

//LEDS
GPIO_Handler_t handlerBlinkyPin = {0};
BasicTimer_Handler_t handlerBlinkyTimer	= {0};
uint8_t counter_dummy = 0;

GPIO_Handler_t handlerGreenLED	= {0};
GPIO_Handler_t handlerRedLED	= {0};

RTC_Handler_t handlerRTC = {0};

//Para generar el token
uint16_t *date = 0;
uint8_t second = 0;
uint8_t hour = 5;
uint8_t minute = 50;
uint8_t day = 19;
uint8_t month = 6;
uint8_t year = 23;
uint8_t format = 0;


char acces[64];
long seed = 123456789;
int token = 0;
char userToken[8] = {0};
char tokenArray[8];

int generateToken(long seed, int year, int month, int day, int hour, int minute);
//SPI

//Keypad
bool    keypadflag   		 = false;
char     key;
uint8_t rxData				 = 0;
char    bufferReception[64]  = {0};
int 	counterReception 	 = 0;
bool    stringComplete 		 = false;
bool prevButtonState 		 = false;

bool analize = false;

//Menu Admin
uint32_t Date[4] = {0};

uint8_t	counterDate = 0;
bool adminbool = false; //reconocido como admin
bool comAdmin = false; //accedió a comandos admin
bool agregate = false; //agragar tarjetas
bool delete = false; //borrar tarjetas
bool confDate = false;


bool PM = false;

void deleteCard(uint8_t rfid[4]);

void changeFormat(int formato3,int formato2, int formato1,int formato0,bool PM);
char* PM24 (int value, bool PM);

char lcd_data[30];

uint8_t rfid_id[4];
//uint8_t baseUser = 2;
uint8_t base_rfid[4][4] = {
		{0x7f,0x69,0xa8,0x2},
		{0x5c,0x18,0x88,0x59}
};
uint8_t base_admin[4] = {0x7f,0x69,0xa8,0x2};


//RFID
bool showMessage= true;
void mainMessage(bool showMessage);
void timer (int x);
void green(void);
void red(void);
void openDoor(void);
void addToBaseRFID(uint8_t rfid[4]);


void init_system(void);
bool checkCode(uint8_t code[4]);



int main(void){
	//Activamos el coprocesador matematico
	SCB->CPACR |= (0xF <<20);
		init_system();

		rc522_init();

		//LCD_Init(&handlerLCD);
		lcd_init();
		green();
		red();	GPIO_WritePin(&handlerGreenLED, SET);
		timer(5000);
		GPIO_WritePin(&handlerGreenLED, RESET);
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
		lcd_send_string("proyecto security");
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
				if(rc522_checkCard(rfid_id)){
							showMessage = false;
							green();
							GPIO_WritePin(&handlerGreenLED, SET);
							timer(5000);
							GPIO_WritePin(&handlerGreenLED, RESET);
							lcd_clear();
							if(memcmp(rfid_id, base_admin, sizeof(base_admin)) == 0){
								adminbool = true;
							}
							setCursor(0,0);
							bool result = checkCode(rfid_id);
							if (result && comAdmin == false) {
//								for(int k = 0; k < 8; k++){
//									rfid_id[k] = '\0';
//								}
								//LCD_sendSTR(&handlerLCD,"RFID accepted");
								//LCD_setCursor(&handlerLCD,0,1);
								//LCD_sendSTR(&handlerLCD,"Generated token");
								green();
								lcd_send_string("ID aceptada");
								setCursor(0, 1);
								lcd_send_string("Retire la tarjeta");
								setCursor(0, 2);
								lcd_send_string("Generando token");
								delay(2000);
								lcd_clear();
								token = generateToken(seed, year, month, day, hour, minute);
								snprintf(tokenArray, sizeof(tokenArray), "%04d", token);

								//LCD_setCursor(&handlerLCD,0,2);
								//setCursor(0,2);
//								sprintf(acces,"Token: %04d",token);
//								lcd_send_string(acces);
								//delay(2000);

								//LCD_Clear(&handlerLCD);
								//lcd_clear();
								setCursor(0, 0);
								lcd_send_string("Introduzca el token");
								setCursor(0, 1);
								lcd_send_string("Luego presione A");
								keypadflag = true;
								delay(200);
								date = read_date();
								day = date[4];
								month = date[5];
								year = date[6];
								minute 	= date[1];
								hour 	= date[2];

							}
							else if(result == true && adminbool == true && comAdmin == true){
								bool result = checkCode(rfid_id);
								if(memcmp(rfid_id, base_admin, sizeof(base_admin)) == 0){
									lcd_send_string("No se puede eliminar");
									setCursor(0, 1);
									lcd_send_string("Administrador");
									showMessage = true;
									comAdmin = false;
									adminbool = false;
									keypadflag = false;
								}
								if(result && comAdmin == true && delete == true){
									deleteCard(rfid_id);
									lcd_clear();
									lcd_send_string("Tarjeta eliminada");
									delay(2000);
									lcd_clear();
									for(int k = 0; k < 8; k++){
										rfid_id[k] = '\0';
									}
									adminbool = false;
									keypadflag = false;
									comAdmin = false;
									showMessage = true;
									stringComplete = false;
									counterReception = 0;
									delete = false;
									mainMessage(showMessage);
									lcd_clear();
								}
								else if (result == false && comAdmin == true && delete == true){
									deleteCard(rfid_id);
									lcd_clear();
									lcd_send_string("La tarjeta no existe");
									delay(2000);
									showMessage = true;
									comAdmin = false;
									adminbool = false;
									delete = false;
								}


							}


							else if (result == false && adminbool == false){
//								LCD_sendSTR(&handlerLCD,"RFID rejected");
//								LCD_setCursor(&handlerLCD,0,1);
								red();
								lcd_send_string("ID no valida");
//								setCursor(0, 1);
//								date = read_date();
//								day = date[4];
//								month = date[5];
//								year = date[6];
//								second = date[0];
//								minute 	= date[1];
//								hour 	= date[2];
//								format = date[3];
//								sprintf(acces, "Fecha:%u/%u/%u", (unsigned int) day,month,year+2000);
//								lcd_send_string(acces);
//								setCursor(0,2);
//								sprintf(acces, "hora actual:%u:%u", (unsigned int) hour, minute);
//								lcd_send_string(acces);
								delay(2000);

								lcd_clear();
								showMessage = true;
							}
							else if(result == false && adminbool == true){
									bool result = checkCode(rfid_id);
									if(result == false && agregate == true){
										addToBaseRFID(rfid_id);
										lcd_clear();
										lcd_send_string("Tarjeta agregada");
										delay(2000);
										lcd_clear();
										for(int k = 0; k < 8; k++){
											rfid_id[k] = '\0';
										}
										adminbool = false;
										agregate = false;
										comAdmin = false;
										showMessage = true;
										stringComplete = false;
										counterReception = 0;
										lcd_clear();
										mainMessage(showMessage);

									}
									else if (result == true && agregate == true){
										lcd_clear();
										lcd_send_string("La tarjeta ya");
										setCursor(0, 1);
										lcd_send_string("existe");
										delay(2000);
										for(int k = 0; k < 8; k++){
											rfid_id[k] = '\0';
										}
										agregate = false;
										adminbool = false;
										showMessage = true;
										comAdmin = false;
									}




							}
				}
				else{
					mainMessage(showMessage);
				}
				delay(100);


				if(keypadflag == true){
					green();
					key = keypad_read();
					setCursor(0, 3);
					sprintf(lcd_data,"count: %d", counterReception);
					lcd_send_string(lcd_data);
					//LCD_sendSTR(&handlerLCD,lcd_data);
					if(key != '\0' && prevButtonState == false && comAdmin == false){
						prevButtonState = true;
						userToken[counterReception] = key;
						counterReception++;

					}
					else if (key == '\0'){
						prevButtonState = false;
					}
					if (key == 'A' && comAdmin == false){
						stringComplete = true;
						analize = true ;
						// Se agrega esta linea para crear el string con el null al final.
						userToken[counterReception-1] = '\0';
						counterReception = 0;
					}
					if(key == 'A' && comAdmin == true ){
						counterReception = 0;
						stringComplete = false;
						confDate = true;
						key = keypad_read();
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Ingrese solo los");
						setCursor(0, 1);
						lcd_send_string("Ultimos 2 digitos");
						setCursor(0, 2);
						lcd_send_string("de Year");
						delay (2000);
						lcd_clear();
						setCursor(0, 0);
						date = read_date();
						day = date[4];
						month = date[5];
						year = date[6];
						minute 	= date[1];
						hour 	= date[2];
						sprintf(acces, "Year actual:%u", (unsigned int)year+2000);
						lcd_send_string(acces);
						setCursor(0, 1);
						lcd_send_string("presione #: next ");
						lcd_send_string(lcd_data);
						setCursor(0,2);
						date[6] = key;
						sprintf(lcd_data,"Year a Ingresar : %lu", Date[counterDate]);
						lcd_send_string(lcd_data);
						counterDate = 0;
						bool yearcomplete = true;
						//LCD_sendSTR(&handlerLCD,lcd_data);
						if (key == '#' && yearcomplete == true){
							Date[0] = date[6];
							counterDate = 0;
							lcd_clear();
							setCursor(0, 0);
							sprintf(acces, "Mes actual:%u", (unsigned int)month);
							lcd_send_string(acces);
							setCursor(0, 1);
							sprintf(acces,"Mes a ingresar : %lu",Date[1]);
							lcd_send_string(acces);
							Date[1] = key;
							bool monthcomplete = true;
							if(key == '#' && Date[1]<13 && Date[1]>0 && monthcomplete == true){
								Date[1] = date[5];
								lcd_clear();
								sprintf(acces,"Dia Actual: %u",(unsigned int)day);
								lcd_send_string(acces);
								setCursor(0, 1);
								sprintf(acces,"Dia a ingresar : %lu",Date[1]);
								lcd_send_string(acces);
								Date[2] = key;
								if(key == '#' && Date[2] >0 && Date[2]<32){
									Date[2] = date[4];
									lcd_clear();
									setCursor(0, 0);
									sprintf(acces,"hora actual: %u",date[2]);
									lcd_send_string(acces);
									setCursor(0, 1);
									sprintf(acces,"ingrese hora : %lu",Date[2]);
									lcd_send_string(acces);
									Date[2] = key;

								}

							}
							else{
								lcd_clear();
								lcd_send_string("Dato invalido");
								return key = '1';
							}

						}

					}
					else if (key =='B' && comAdmin == false){
						lcd_clear();
						keypadflag = false;
						showMessage = true;
						counterReception = 0;
						adminbool = false;
						agregate = false;
						comAdmin = false;
						showMessage = true;
						stringComplete = false;
						counterReception = 0;
						lcd_clear();
						mainMessage(showMessage);
					}
					else if (key == 'B' && adminbool == true && confDate == false && comAdmin == true){
						stringComplete = false;
						agregate = true;
						counterDate = 0;
						counterReception = 0;
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Acerque la tarjeta");
						setCursor(0, 1);
						lcd_send_string("al lector para");
						setCursor(0, 2);
						lcd_send_string("Agregar");
						if(rc522_checkCard(rfid_id));
						bool result = checkCode(rfid_id);
						if(result){
							addToBaseRFID(rfid_id);
							lcd_clear();
							lcd_send_string("Tarjeta agregada");
							delay(2000);
							lcd_clear();
							for(int k = 0; k < 8; k++){
								rfid_id[k] = '\0';
							}
							showMessage = true;
						}


					}

					else if (key == 'C' && comAdmin == false){
						lcd_clear();
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
						userToken[counterReception] = '\0';
						counterReception--;
						//LCD_sendSTR(&handlerLCD,acces);
						delay(2000);
						//LCD_Clear(&handlerLCD);
						lcd_clear();
						counterReception = 0;
						return keypadflag == true && comAdmin == false;
					}
					else if (key == 'D'){
						userToken[counterReception] = '\0';
						counterReception--;
						if(counterReception < 0){
							counterReception = 0;
						}
						setCursor(0, 3);
						sprintf(lcd_data,"count: %d", counterReception);
						lcd_send_string(lcd_data);

					}
					else if(key == '*' && adminbool == true){
						comAdmin = true;
						counterReception = 0;
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Comandos admin");
						setCursor(0, 1);
						lcd_send_string("presione");
						setCursor(0, 2);
						lcd_send_string("La tecla del ajuste");
						delay(3000);
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("A: Ajuste fecha/hora");
						setCursor(0, 1);
						lcd_send_string("B: Agregar Tarjeta");
						setCursor(0, 2);
						lcd_send_string("C: Eliminar Tarjeta");

						key = keypad_read();
						if(key != '\0' && prevButtonState == false){
							prevButtonState = true;
							Date[counterDate] = key;
						}
						else if (key == '\0'){
							prevButtonState = false;
						}
						counterDate = 0;
					}
					else if(key == '*' && adminbool == false){
						counterReception = 0;
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("No posee acceso");
						setCursor(0, 1);
						lcd_send_string("de admin");
						delay(2000);
						lcd_clear();
						showMessage = true;
					}

					if(key != '\0' && prevButtonState == false){
						prevButtonState = true;
						Date[counterDate] = key;
					}
					else if (key == '\0'){
						prevButtonState = false;
					}
					counterDate = 0;
					stringComplete = false;
					if(key == 'A' && comAdmin == true ){
						counterReception = 0;
						stringComplete = false;
						confDate = true;
						key = keypad_read();
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Ingrese solo los");
						setCursor(0, 1);
						lcd_send_string("Ultimos 2 digitos");
						setCursor(0, 2);
						lcd_send_string("de Year");
						delay (2000);
						lcd_clear();
						setCursor(0, 0);
						date = read_date();
						day = date[4];
						month = date[5];
						year = date[6];
						minute 	= date[1];
						hour 	= date[2];
						setCursor(0, 1);
						lcd_send_string("presione #: next ");
						lcd_send_string(lcd_data);
						setCursor(0,2);
						date[6] = key;
						sprintf(lcd_data,"Year a Ingresar : %lu", Date[counterDate]);
						lcd_send_string(lcd_data);
						counterDate = 0;
						bool yearcomplete = true;
						//LCD_sendSTR(&handlerLCD,lcd_data);
						if (key == '#' && yearcomplete == true){
							Date[0] = date[6];
							counterDate = 0;
							lcd_clear();
							setCursor(0, 0);
							sprintf(acces, "Mes actual:%u", (unsigned int)month);
							lcd_send_string(acces);
							setCursor(0, 1);
							sprintf(acces,"Mes a ingresar : %lu",Date[1]);
							lcd_send_string(acces);
							Date[1] = key;
							bool monthcomplete = true;
							if(key == '#' && Date[1]<13 && Date[1]>0 && monthcomplete == true){
								Date[1] = date[5];
								lcd_clear();
								sprintf(acces,"Dia Actual: %u",(unsigned int)day);
								lcd_send_string(acces);
								setCursor(0, 1);
								sprintf(acces,"Dia a ingresar : %lu",Date[1]);
								lcd_send_string(acces);
								Date[2] = key;
								if(key == '#' && Date[2] >0 && Date[2]<32){
									Date[2] = date[4];
									lcd_clear();
									setCursor(0, 0);
									sprintf(acces,"hora actual: %u",date[2]);
									lcd_send_string(acces);
									setCursor(0, 1);
									sprintf(acces,"ingrese hora : %lu",Date[2]);
									lcd_send_string(acces);
									Date[2] = key;

								}

							}
							else{
								lcd_clear();
								lcd_send_string("Dato invalido");
								return key = '1';
							}

						}

					}
					else if (key == 'A' && comAdmin == false){
						stringComplete = true;
						analize = true ;
						// Se agrega esta linea para crear el string con el null al final.
						userToken[counterReception-1] = '\0';
						counterReception = 0;
					}
					else if (key == 'B' && adminbool == true && confDate == false && comAdmin == true){
						stringComplete = false;
						agregate = true;
						counterDate = 0;
						counterReception = 0;
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Acerque la tarjeta");
						setCursor(0, 1);
						lcd_send_string("al lector para");
						setCursor(0, 2);
						lcd_send_string("Agregar");
						if(rc522_checkCard(rfid_id));
						bool result = checkCode(rfid_id);
						if(result){
							addToBaseRFID(rfid_id);
							lcd_clear();
							lcd_send_string("Tarjeta agregada");
							delay(2000);
							lcd_clear();
							for(int k = 0; k < 8; k++){
								rfid_id[k] = '\0';
							}
							showMessage = true;
						}


					}
					else if(key == 'C' && adminbool == true && confDate == false && comAdmin == true){
						counterReception = 0;
						stringComplete = false;
						delete = true;
						counterDate = 0;
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Acerque la tarjeta");
						setCursor(0, 1);
						lcd_send_string("al lector para");
						setCursor(0, 2);
						lcd_send_string("Eliminar");


					}
//					else if(key == '*' && adminbool == false){
//						counterReception = 0;
//						lcd_clear();
//						setCursor(0, 0);
//						lcd_send_string("No posee acceso");
//						setCursor(0, 1);
//						lcd_send_string("de admin");
//						delay(2000);
//						lcd_clear();
//						showMessage = true;
//					}




					if(stringComplete == true && analize == true){
						lcd_clear();
						setCursor(2, 1);
						lcd_send_string("Analizando token");
						//setCursor(0, 1);
						//sprintf( "%d", userToken);

						//lcd_send_string(lcd_data);
						//setCursor(0, 2);

						delay(2000);
						lcd_clear();
//						int compare = memcmp(tokenArray, userToken, 4);
//						sprintf(lcd_data, "%d",compare);
						lcd_send_string(lcd_data);
						delay (2000);
						lcd_clear();
						setCursor(0, 0);
						if (memcmp(tokenArray, userToken, sizeof(userToken)) == 0){
//							LCD_sendSTR(&handlerLCD,"Token autorizado");
//							LCD_setCursor(&handlerLCD,0, 2);
//							LCD_sendSTR(&handlerLCD,"lo hiciste papu");
							green();
							//openDoor();
							lcd_send_string("Token autorizado");
							setCursor(0,2);
							lcd_send_string("Abriendo puerta");
							for(int i = 0; i < 8; i++){
								userToken[i] = '\0';
							}
							GPIO_WritePin(&handlerSolen, SET);
							delay(2000);
							timer(10000);
							GPIO_WritePin(&handlerSolen, RESET);
							lcd_clear();
							showMessage = true;
							keypadflag = false;
							stringComplete = false;
							counterReception = 0;
							adminbool= false;

						}
						else{
							//LCD_sendSTR(&handlerLCD,"Token erroneo");
							red();
							lcd_send_string("Token Erroneo");
							delay(2000);
							lcd_clear();
							//LCD_Clear(&handlerLCD);
							for(int i = 0; i <8; i++){
								userToken[i] = '\0';

							}
							counterReception = 0;
						}
						keypadflag = false;
						stringComplete = false;
						showMessage = true;
						adminbool = false;


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


	// Cargo el pin que activara el solenoide para abrir la puerta
	handlerSolen.pGPIOx													= GPIOA;
	handlerSolen.GPIO_PinConfig.GPIO_PinNumber							= PIN_10;
	handlerSolen.GPIO_PinConfig.GPIO_PinMode							= GPIO_MODE_OUT;
	handlerSolen.GPIO_PinConfig.GPIO_PinOPType							= GPIO_OTYPE_PUSHPULL;
	handlerSolen.GPIO_PinConfig.GPIO_PinPuPdControl						= GPIO_PUPDR_NOTHING;
	handlerSolen.GPIO_PinConfig.GPIO_PinSpeed							= GPIO_OSPEED_FAST;
	GPIO_Config(&handlerSolen);

	//Configuramos los 3 leds: uno de estado PH1, uno de aceptacion, PA2 y uno de rechazo PA3

	//Configuracion del BLinky
	handlerBlinkyPin.pGPIOx											= GPIOH;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinNumber					= PIN_1;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_FAST;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerBlinkyPin);

	handlerGreenLED.pGPIOx											= GPIOC;
	handlerGreenLED.GPIO_PinConfig.GPIO_PinNumber					= PIN_12;
	handlerGreenLED.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_OUT;
	handlerGreenLED.GPIO_PinConfig.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
	handlerGreenLED.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_FAST;
	handlerGreenLED.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerGreenLED);

	handlerRedLED.pGPIOx											= GPIOC;
	handlerRedLED.GPIO_PinConfig.GPIO_PinNumber						= PIN_13;
	handlerRedLED.GPIO_PinConfig.GPIO_PinMode						= GPIO_MODE_OUT;
	handlerRedLED.GPIO_PinConfig.GPIO_PinOPType						= GPIO_OTYPE_PUSHPULL;
	handlerRedLED.GPIO_PinConfig.GPIO_PinSpeed						= GPIO_OSPEED_FAST;
	handlerRedLED.GPIO_PinConfig.GPIO_PinPuPdControl				= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerRedLED);

	/*====================================== Timer ===================================*/

	//Configurando el TIM2 el cual le da la tasa de encendido al LED
	handlerBlinkyTimer.ptrTIMx										= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode						= BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed						= BTIMER_SPEED_100us;
	handlerBlinkyTimer.TIMx_Config.TIMx_period						= 2500;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable				= BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerBlinkyTimer);

	handlerRTC.RTC_Config.RTC_Hours = 16;
	handlerRTC.RTC_Config.RTC_Minutes = 0;
	handlerRTC.RTC_Config.RTC_Seconds = 0;
	handlerRTC.RTC_Config.RTC_ValueDay = 28 ;
	handlerRTC.RTC_Config.RTC_Month = 6;
	handlerRTC.RTC_Config.RTC_Year = 23;
	RTC_Config(&handlerRTC);

}

bool checkCode(uint8_t code[4]) {
    for (int i = 0; i < 4; i++) {
        bool match = true;
        for (int j = 0; j < 4; j++) {
            if (base_rfid[i][j] != code[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}


void BasicTimer2_Callback(void){
	GPIOxTogglePin(&handlerBlinkyPin);
	counter_dummy++;
}



void callback_extInt0(void){
	stringComplete = true;
}

void callback_extInt1(void){
	keypadflag = true;
}



void mainMessage(bool ShowMessage){
	while (!rc522_checkCard(rfid_id)&& showMessage == true) {
		setCursor(3, 0);
		lcd_send_string("Bienvenido");
		setCursor(0, 1);
		lcd_send_string("Acerque su tarjeta");
		setCursor(0, 2);
		lcd_send_string("Para continuar");
		setCursor(0, 3);
		date = read_date();
		day = date[4];
		month = date[5];
		year = date[6];
		minute = date[1];
		hour = date[2];
		sprintf(acces, "F %u/%u/%u", (unsigned int) day, month,year + 2000);
		lcd_send_string(acces);
		setCursor(13, 3);
		sprintf(acces, "H %u:%u", (unsigned int) hour, minute);
		lcd_send_string(acces);

		delay(500);
	}
}

int generateToken(long seed, int year, int month, int day, int hour, int minute){
	date 	= read_date();
	minute 	= date[1];
	hour 	= date[2];
	day		= date[4];
	month	= date[5];
	year	= date[6]+2000;
    int datetime = ((year % 100) * 100000000) +
                   (month * 1000000) +
                   (day * 10000) +
                   (hour * 100) +
                   (minute* 8);
    token = seed;
    token = (token * datetime) % 10000;
    token = abs(token);
    return token ;
}

void timer (int x){
	for(int i = 0; i< x; i++){
		__NOP();
	}
}

void changeFormat(int formato3,int formato2, int formato1,int formato0,bool PM){
	if(formato3== 1 && formato2== 11 && formato1 == 59 && formato0 == 59 && PM == true){
		formato3= 0;
	}
	else if(formato3 == 0 && formato2 == 11 && formato1 == 59 && formato0== 59 && PM == true){
		formato3 = 1;
	}
}

char* PM24 (int value,bool PM){
	char* fm = "00";
	if(value == 1 && PM == true ){
		fm ="PM";
	}
	else if(value == 1 && PM == false){
		fm = "AM";
	}
	else if(value == 0 && PM == false){
		fm= "HH";
	}
	return fm;
}

void addToBaseRFID(uint8_t rfid[4]) {
    int emptyIndex = -1;  // Índice de la posición vacía más cercana
    // Buscar la posición vacía más cercana en base_rfid
    for (int i = 0; i < 4; i++) {
        bool emptyRow = true;
        for (int j = 0; j < 4; j++) {
            if (base_rfid[i][j] != 0) {
                emptyRow = false;
                break;
            }
        }
        if (emptyRow) {
            emptyIndex = i;
            break;
        }
    }
    // Si se encontró una posición vacía, agregar el arreglo rfid en esa posición
    if (emptyIndex != -1) {
        for (int j = 0; j < 4; j++) {
            base_rfid[emptyIndex][j] = rfid[j];
        }
    }
}

void deleteCard(uint8_t rfid[4]){
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (base_rfid[i][j] != rfid[j]) {
                break;
            }
            else if(base_rfid[i][j] == rfid[j]){
            	base_rfid[i][j] = 0;
            }
        }

    }
}


void green(void){
	GPIO_WritePin(&handlerGreenLED, SET);
	timer(20000);
	GPIO_WritePin(&handlerGreenLED, RESET);
}

void red (void){
	GPIO_WritePin(&handlerRedLED, SET);
	timer(20000);
	GPIO_WritePin(&handlerRedLED, RESET);
}

void openDoor(void){
	GPIO_WritePin(&handlerSolen, SET);
	timer(100000);
	GPIO_WritePin(&handlerSolen, RESET);
}



