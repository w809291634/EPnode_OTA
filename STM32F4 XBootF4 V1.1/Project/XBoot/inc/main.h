#ifndef _MAIN_H_
#define _MAIN_H_

#include "stm32f4xx.h"

#define REC_DETECT_CLK       RCC_AHB1Periph_GPIOD
#define REC_DETECT_PORT      GPIOD
#define REC_DETECT_PIN       GPIO_Pin_8
#define REC_DETECT_ACTIVE()  (GPIO_ReadInputDataBit(REC_DETECT_PORT, REC_DETECT_PIN) == Bit_RESET)
#define REC_WAIT_CNT         6666666

#define LED_RUN_CLK   RCC_AHB1Periph_GPIOD
#define LED_RUN_PORT  GPIOD
#define LED_RUN_PIN   GPIO_Pin_14
#define LED_ON()      GPIO_ResetBits(LED_RUN_PORT, LED_RUN_PIN)
#define LED_OFF()     GPIO_SetBits(LED_RUN_PORT, LED_RUN_PIN)

#define TERM_UART           USART6
#define TERM_UART_CLK_INIT  RCC_APB2PeriphClockCmd
#define TERM_UART_CLK       RCC_APB2Periph_USART6
#define TERM_UART_IRQn      USART6_IRQn

#define TERM_GPIO_CLK       RCC_AHB1Periph_GPIOC
#define TERM_GPIO_PORT      GPIOC
#define TERM_GPIO_TX_PIN    GPIO_Pin_6
#define TERM_GPIO_RX_PIN    GPIO_Pin_7
#define TERM_GPIO_TX_PINSOURCE  GPIO_PinSource6
#define TERM_GPIO_RX_PINSOURCE  GPIO_PinSource7
#define TERM_GPIO_AF        GPIO_AF_USART6

#endif //_MAIN_H_
