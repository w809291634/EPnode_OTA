#ifndef __UART0_H__

#define __UART0_H__

void uart0_Init();
void uart0Send(unsigned char ch);
void uart0SetInputCall(void (*f)(char ch));

#endif
