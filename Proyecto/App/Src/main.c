#include "LiquidCrystal_PCF8574.h"
//#include "LCDDriver.h"
#include "delay.h"
#include "stdio.h"
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
bool prevButtonState 		 = false;


char lcd_data[30];

uint8_t rfid_id[4];
uint8_t base_rfid[2][4] = {
		{0x7f,0x69,0xa8,0x2},
		{0x5c,0x18,0x88,0x59}
};
uint8_t admin[4] = {0x7f,0x69,0xa8,0x2};

RTC_Handler_t handlerRTC = {0};

//RFID
bool showMessage= true;
void mainMessage(bool showMessage);
void timer (int x);
void green(void);
void red(void);
void openDoor(void);


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
bool checkCode(uint8_t code[2][4]);



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




				if(rc522_checkCard(rfid_id)){
							showMessage = false;
							green();
							GPIO_WritePin(&handlerGreenLED, SET);
							timer(5000);
							GPIO_WritePin(&handlerGreenLED, RESET);
							lcd_clear();
							//LCD_Clear(&handlerLCD);

							//LCD_setCursor(&handlerLCD,0,0);
							setCursor(0,0);
							//if (memcmp(rfid_id, base_rfid, sizeof(rfid_id)) == 0) {
							bool result = checkCode(base_rfid);
							if (result) {
								for(int k = 0; k < 8; k++){
									rfid_id[k] = '\0';
								}
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
								delay(200);
							}

							else{
//								LCD_sendSTR(&handlerLCD,"RFID rejected");
//								LCD_setCursor(&handlerLCD,0,1);
								red();
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
								lcd_clear();
							}
				}
				else{
					mainMessage(showMessage);
				}
				delay(100);


				if(keypadflag){
					green();
					key = keypad_read();
					//LCD_setCursor(&handlerLCD,0, 2);
					setCursor(0, 2);
					sprintf(lcd_data,"Key pressed is %c",key);
					lcd_send_string(lcd_data);
					setCursor(0, 3);

					sprintf(lcd_data,"count: %d", counterReception);
					lcd_send_string(lcd_data);
					//LCD_sendSTR(&handlerLCD,lcd_data);
					if(key != '\0' && prevButtonState == false){
						prevButtonState = true;
						userToken[counterReception] = key;
						counterReception++;
//
//						if (key !='A'){
//							key = '\0';
//						}


					}
					else if (key == '\0'){
						prevButtonState = false;
					}
					if (key == 'A'){
						stringComplete = true;
						// Se agrega esta linea para crear el string con el null al final.
						userToken[counterReception-1] = '\0';
						counterReception = 0;
					}
					else if (key =='B'){
						lcd_clear();
						keypadflag = false;
						showMessage = true;
					}

					else if (key == 'C'){
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
					else if(key == '*'){

					}




					if(stringComplete){
//						LCD_Clear(&handlerLCD);
//						LCD_setCursor(&handlerLCD,0, 0);
//						LCD_sendSTR(&handlerLCD,"Analizando Token");
//						LCD_setCursor(&handlerLCD,0,1);
						green();
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
//						for(int loop = 0; loop < sizeof(tokenArray); loop++){
//							sprintf(lcd_data, "token array: %c", tokenArray[loop]);
//							lcd_send_string(lcd_data);
//							setCursor(0, 1);
//							sprintf(lcd_data, "usertoken: %c", userToken[loop]);
//							lcd_send_string(lcd_data);
//							delay(500);
//							lcd_clear();
//
//						}
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
							green();
							//openDoor();
							GPIO_WritePin(&handlerSolen, SET);
							timer(100000);
							GPIO_WritePin(&handlerSolen, RESET);
							lcd_send_string("Token autorizado");
							setCursor(0,2);
							lcd_send_string("Lo hiciste papu");
							for(int i = 0; i < 8; i++){
								userToken[i] = '\0';
							}
							delay(2000);
							lcd_clear();
							showMessage = true;

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

}

bool checkCode(uint8_t code[2][4]) {
    for (int i = 0; i < 2; i++) {
        bool match = true;
        for (int j = 0; j < 4; j++) {
            if (base_rfid[i][j] != code[i][j]) {
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
		delay(100);
	}
}

void timer (int x){
	for(int i = 0; i< x; i++){
		__NOP();
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
