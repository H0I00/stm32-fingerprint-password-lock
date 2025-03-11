#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LED1 PCout(15)	// PC15
#define LED2 PCout(14)	// PC14	
#define LED3 PCout(13)	// PC13

void LED_Init(void);
void LED1_ON(void);
void LED1_OFF(void);
void LED1_Turn(void);
void LED2_ON(void);
void LED2_OFF(void);
void LED2_Turn(void);
void LED3_ON(void);
void LED3_OFF(void);
void LED3_Turn(void);
#endif
