#ifndef _led_h_
#define _led_h_

#include "sys.h"

#define LED2    P1_0              //定义LED2为P1_0口控制
#define LED1    P1_1              //定义LED1为P1_1口控制

void led_init(void);
void led_app_init(void);
#endif
