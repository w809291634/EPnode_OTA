#include "led.h"
#include "soft_timer.h"

void led_init(void)
{
    P1SEL &= ~0x03;          //P1.0 P1.1为普通 I/O 口
    P1DIR |= 0x03;           //输出
    
    LED2 = 1;                //关LED
    LED1 = 1;
}

static void led_app_entry(void)
{
  LED1=!LED1;
}

void led_app_init(void)
{
  softTimer_create(LED_APP_TIMER_ID,MODE_PERIODIC,led_app_entry);
  softTimer_start(LED_APP_TIMER_ID,100);
}