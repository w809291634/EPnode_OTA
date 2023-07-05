/*********************************************************************************************
* 文件：uart.h
* 作者：fuyou
* 说明：uart头文件
* 修改：
* 注释：
*********************************************************************************************/
#ifndef _uart_h_
#define _uart_h_
#include "sys.h"

extern unsigned char usart0_mode;

int usart0_getchar(uint8_t* data);
void shell_app_cycle(void);
void shell_hw_init(double baud);
void Uart0_Send_char(unsigned char ch);
void Uart0_Send_String(unsigned char *Data);
void Uart0_Send_LenString(const char *Data,unsigned short len);
int Uart0_Recv_char(void);
#endif