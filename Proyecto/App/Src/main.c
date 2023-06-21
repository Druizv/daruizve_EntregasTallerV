#include "LiquidCrystal_PCF8574.h"
#include "delay.h"
#include "stdio.h"
#include "RFID.h"
#include <string.h>
#include "RTC.h"
#include "keypad.h"



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
char userToken[8] = {0};
char tokenArray[8];

//SPI

//Keypad
bool    keypadflag   		 = false;
char     key;
uint8_t rxData				 = 0;
char    bufferReception[64]  = {0};
uint8_t counterReception 	 = 0;
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

		lcd_init();

		keypad_init();

		setCursor(0,0);
		lcd_send_string("Proyecto");
		setCursor(0,1);
		lcd_send_string("Taller V");
		setCursor(0,2);
		lcd_send_string("Daniel Ruiz");
		delay(2000);
		lcd_clear();

		while(1){

//			setCursor(0,1);
//			lcd_send_string("Acerque la tarjeta");
//			delay(2000);
//
				if(rc522_checkCard(rfid_id)){
							lcd_clear();
							//char data[20];

							setCursor(0,0);
							if (memcmp(rfid_id, base_rfid, sizeof(rfid_id)) == 0) {
								lcd_send_string("RFID accepted");
								setCursor(0,1);
								lcd_send_string("Generated token");
								//sprintf(data,"0x%x 0x%x 0x%x 0x%x",rfid_id[0],rfid_id[1],rfid_id[2],rfid_id[3]);
								//lcd_send_string(data);
								delay(1000);
								token = generateToken(seed, year, month, day, hour, minute);
								snprintf(tokenArray, sizeof(tokenArray), "%04d", token);
								setCursor(0,2);
								sprintf(acces,"Token: %04d\n",token);
								lcd_send_string(acces);
								delay(2000);
								lcd_clear();
								setCursor(0, 0);
								lcd_send_string("introduzca el token");
								setCursor(0, 1);
								lcd_send_string("Luego presione A");
								keypadflag = true;
								delay(30);
							}

							else{
								lcd_send_string("RFID rejected");
								setCursor(0,1);
								date = read_date();
								day = date[4];
								month = date[5];
								year = date[6];
								minute 	= date[1];
								hour 	= date[2];
								sprintf(acces, "Fecha:%u/%u/%u", (unsigned int) day,month,year+2000);
								lcd_send_string(acces);
								setCursor(0, 2);
								sprintf(acces, "hora actual:%u:%u", (unsigned int) hour, minute);
								lcd_send_string(acces);
								delay(2000);
								lcd_clear();
							}
				}
				delay(100);


				if(keypadflag){
					key = keypad_read();
					setCursor(0, 2);
					sprintf(lcd_data,"Key pressed is %c",key);
					lcd_send_string(lcd_data);
					//userToken[counterReception] = key;
					//counterReception++;
					//key = '\0';

					if (key == 'A'){
						stringComplete = true;
						// Se agrega esta linea para crear el string con el null al final.
						userToken[counterReception-1] = '\0';
						counterReception = 0;
					}


					if(stringComplete){
						lcd_clear();
						setCursor(0, 0);
						lcd_send_string("Analizando Token");
						setCursor(0,1);
						sprintf(lcd_data,"%u %u %u %u", userToken[0], userToken[1], userToken[2], userToken[3]);
						lcd_send_string(lcd_data);
						setCursor(0, 2);
						delay(2000);
						if (memcmp(tokenArray, userToken, sizeof(tokenArray)) == 0){
							lcd_send_string("Token autorizado");
							setCursor(0, 2);
							lcd_send_string("lo hiciste papu");
							for(int i = 0; i < 8; i++){
								userToken[i] = '\0';
							}
						}
						else{
							lcd_send_string("Token erroneo");
							delay(2000);
							lcd_clear();
							for(int i = 0; i <8; i++){
								userToken[i] = '\0';
							}
						}
						keypadflag = false;


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
}


