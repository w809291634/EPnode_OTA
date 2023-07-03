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
#include "shell.h"

extern shellinput_t shell_1;
void shell_app_cycle(void);
void uart0_init(double baud);
void uart1_init(double baud);
void Uart0_Send_char(unsigned char ch);
void Uart0_Send_String(unsigned char *Data);
void Uart0_Send_LenString(const char *Data,unsigned short len);
int Uart0_Recv_char(void);
void Uart1_Send_char(unsigned char ch);
void Uart1_Send_String(unsigned char *Data);
void Uart1_Send_LenString(unsigned char *Data,int len);
int Uart1_Recv_char(void);
#endif