#ifndef _led_h_
#define _led_h_

#include "sys.h"

#define LED2    P1_0              //����LED2ΪP1_0�ڿ���
#define LED1    P1_1              //����LED1ΪP1_1�ڿ���

void led_init(void);
void led_app_init(void);
#endif
