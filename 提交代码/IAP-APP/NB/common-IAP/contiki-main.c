#include <stdio.h>
#include <contiki.h>
#include <contiki-net.h>
#include <dev/leds.h>
#include "usart.h"
#include "misc.h"

#define DEBUG 0

#if DEBUG
#define Debug      printf
#else
#define Debug(...)
#endif

unsigned int idle_count = 0;

void relay_init(void);
void uart1_init(unsigned int bpr);
void debug_init(void (*f)(char));
void clock_delay_ms(unsigned int ms);
void main(void)
{
#ifdef USER_APP_BEGIN
  NVIC_SetVectorTable(NVIC_VectTab_FLASH,(USER_APP_BEGIN - NVIC_VectTab_FLASH));
#endif
  clock_init();
  
  leds_init();
  leds_on(3);
  clock_delay_ms(200);
  leds_off(3);
  uart3_init(38400);
  debug_init(uart3_putc);
  
  Debug("\r\nStarting ");
  Debug(CONTIKI_VERSION_STRING);
  Debug(" on STM32F103x\r\n"); 
  
  process_start(&etimer_process, NULL);
  
  ctimer_init();

#if AUTOSTART_ENABLE    
  autostart_start(autostart_processes);
#endif
  
  while(1) {
    do {
    } while(process_run() > 0);
    idle_count++;
    /* Idle! */
    /* Stop processor clock */
    /* asm("wfi"::); */ 
  }
}