#ifndef __DRIVER_H
#define __DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f4xx.h>

void RFID_Init(void);
void RFID_Read(char *buffer, uint16_t size);
void LED_On(void);
void LED_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRIVER_H */
