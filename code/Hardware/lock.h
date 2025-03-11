#ifndef __LOCK_H
#define __LOCK_H	 
#include "sys.h"

#define lock PBout(12)	// PB12
#define buzz PBout(13)	// PB13
//void lock_Init(void);
//void lock_on(void);
//void lock_off(void);
//void buzz_on(void);
//void lock_4_seconds(void);




void Servo_Init(void);
void lock_on(void);
void lock_off(void);
void lock_4_seconds(void);
void Buzzer_Init(void);
void buzz_ok(void);
void buzz_no(void);
void PWM_Init(void);
void PWM_SetCompare2(uint16_t Compare);

#endif