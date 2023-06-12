#include "RTC.h"

void RTC_Config(RTC_Handler_t *ptrRtcHandler) {

	/*DBP Bit of the PWR_CR must be set to enable RTC registers write access
	 * *******
	 * La escritura en los registros RTC se habilita escribiendo una clave
	 * al registro RTC_WPR : 0xCA , 0x53
	 */

	/* fecha inicial de configuracion*/

	RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Activamos el clock

	PWR->CR  |= PWR_CR_DBP; //deshabilita la proteccion contra escritura


	RCC->BDCR |= RCC_BDCR_RTCEN;		  // Habilitar cambio de fuente de reloj
	RCC->BDCR |= RCC_BDCR_LSEON;		  //Activar reloj LSE
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;		  // Se selecciona el cristal como reloj

	/* Espera hasta que este listo el RCC */
	while(!(RCC->BDCR & RCC_BDCR_LSERDY)){ //esta bandera indica si el cristal se estabilizó
		__NOP() ;
	}

	RTC->WPR = (0xCA ); // Protección contra escritura de desbloqueo de teclas
	RTC->WPR = (0x53 ); // Protección contra escritura de desbloqueo de teclas


	RTC->ISR |= RTC_ISR_INIT;			  // Se inicializa el modo
	while (!(RTC->ISR & RTC_ISR_INITF)){  //esta bandera indica si se inicio correctamente
		__NOP() ;
	}
	/*el factor de division del prescaler asincrono esta configurado a 128 y el sincrono a 256
	 * para obtener 1Hz (CK_PRE) con la frecuencia del LSE o 32.768KHz
	 */

	RTC->PRER |= RTC_PRER_PREDIV_A; //se usa el factor de division asincrono, este consume menos recursos
	RTC->PRER |= 0xFF << 0 ;

	/* Load the initial time and date in the RTC_TR , RTC_DT and configure time format
	 * through the FMT bit in RTC_CR register */
    RTC->CR |= RTC_CR_BYPSHAD; //se activa el shadow register para acceder al registro de hora y fecha

	RTC->TR = 0; //se setea la hora en cero
	RTC->DR = 0; //se setea la fecha en cero ambos estan por defecto en cero

	//se cargan los registros de binario a BCD

	RTC->DR |= ((ptrRtcHandler->RTC_Config.RTC_Year) / 10) << RTC_DR_YT_Pos;  // Year Tens in BCD
	RTC->DR |= ((ptrRtcHandler->RTC_Config.RTC_Year) % 10) << RTC_DR_YU_Pos;  // Year Units in BCD

	RTC->DR |= ((ptrRtcHandler->RTC_Config.RTC_Month) / 10) << RTC_DR_MT_Pos;  // Month Units in BCD
	RTC->DR |= ((ptrRtcHandler->RTC_Config.RTC_Month) % 10) << RTC_DR_MU_Pos;  // Month Tens in BCD

	RTC->DR |= ((ptrRtcHandler->RTC_Config.RTC_ValueDay) / 10) << RTC_DR_DT_Pos;   // Date Tens in BCD
	RTC->DR |= ((ptrRtcHandler->RTC_Config.RTC_ValueDay) % 10) << RTC_DR_DU_Pos;   // Date Units in BCD

	RTC->TR |= ((ptrRtcHandler->RTC_Config.RTC_Hours) / 10) << RTC_TR_HT_Pos;  // Hours Tens in BCD
	RTC->TR |= ((ptrRtcHandler->RTC_Config.RTC_Hours) % 10) << RTC_TR_HU_Pos;  // Hours Units in BCD

	RTC->TR |= ((ptrRtcHandler->RTC_Config.RTC_Minutes) / 10) << RTC_TR_MNT_Pos;  // Minutes Tens in BCD
	RTC->TR |= ((ptrRtcHandler->RTC_Config.RTC_Minutes) % 10) << RTC_TR_MNU_Pos;  // Minutes Units in BCD

	RTC->TR |= ((ptrRtcHandler->RTC_Config.RTC_Seconds) / 10) << RTC_TR_ST_Pos;   // Seconds Tens in BCD
	RTC->TR |= ((ptrRtcHandler->RTC_Config.RTC_Seconds) % 10) << RTC_TR_SU_Pos;   // Seconds Units in BCD

	RTC->TR |= ((ptrRtcHandler->RTC_Config.RTC_TimeNotation) << RTC_TR_PM_Pos);  // 0: is am or 24h format 1: is pm

	/* Exiting initialization mode by clearing init bit */

		RCC->BDCR |= RCC_BDCR_RTCEN;
		RTC->ISR &= ~RTC_ISR_INIT;
		PWR->CR &= ~ PWR_CR_DBP;

	    RTC->CR &= ~RTC_CR_BYPSHAD;
		RTC->WPR = (0xFF); // Key Lock write protection
}


//Definición de las variables necesarias para  el RTC

//
uint8_t RTC_BcdToByte(uint16_t BCD_Value){

    uint8_t Decimal_Value = ((BCD_Value/16*10) + (BCD_Value%16));
    return Decimal_Value;
}

uint16_t calendario[7] = {0};

void *read_date(void){

    //while((RTC->ISR & RTC_ISR_RSF)){

	 uint8_t RTC_Hours   = 0;
	 uint8_t RTC_Minutes = 0;
	 uint8_t RTC_Seconds = 0;


	 uint8_t RTC_year = 0;
	 uint8_t RTC_Month = 0;
	 uint8_t RTC_Day = 0;

	 uint32_t RTC_Time = 0;
	 RTC_Time = RTC->TR;

	 uint32_t RTC_Date = 0;
	 RTC_Date = RTC->DR;

	 RTC_Hours   = RTC_BcdToByte(((RTC_Time & 0x3F0000) >> 16)); //0011 1111 0000 0000 0000 0000 en binario
	 //corre el registro 16 posiciones a la derecha para leerlo correctamente
	 RTC_Minutes = RTC_BcdToByte(((RTC_Time & 0x007F00) >> 8));//0000 0000 0111 1111 0000 0000
	 //corre el registro 8 posiciones
	 RTC_Seconds = RTC_BcdToByte((RTC_Time  & 0x7F));

	  RTC_year   = RTC_BcdToByte(((RTC_Date & 0xFF0000) >> 16));
	  RTC_Month  = RTC_BcdToByte(((RTC_Date & 0x1F00)   >> 8));
	  RTC_Day    = RTC_BcdToByte((RTC_Date  & 0x3F));


	calendario[0] = RTC_Seconds;
	calendario[1] = RTC_Minutes;
	calendario[2] = RTC_Hours;
	calendario[4] = RTC_Day;
	calendario[5] = RTC_Month;
	calendario[6] = RTC_year;
//    }
	return calendario;
}
