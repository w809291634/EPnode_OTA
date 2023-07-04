#include "led.h"
#include "soft_timer.h"

void led_init(void)
{
    P1SEL &= ~0x03;          //P1.0 P1.1Ϊ��ͨ I/O ��
    P1DIR |= 0x03;           //���
    
    LED2 = 1;                //��LED
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