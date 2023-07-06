/*********************************************************************************************
* �ļ���at-uart.c
* ���ߣ�Xuzhy 2018.5.16
* ˵�����ڵ�ATָ��ڳ�ʼ��
* �޸ģ�fuyou ����͸������
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <iocc2530.h>
#include <stdio.h>
#include "hal_mcu.h"
#include "at-uart.h"
#include "math.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define SUPPOER_DEBUG 1
/*********************************************************************************************
* ����ԭ��˵��
*********************************************************************************************/
static void (*_input)(char ch);
/*********************************************************************************************
* ���ƣ�UART_BuadCount
* ���ܣ����ڲ����ʼ���
* ������baud:������
* ���أ���
* �޸ģ�
* ע�ͣ����ݲ����ʼ���Ĵ���ֵ
*********************************************************************************************/
void UART_BuadCount(double* baud,char* baud_e,char* baud_m)
{
	double sys_clk_baud = 32000000.0;							//ϵͳʱ��
	
	/*���ݲ�����ѡ��baud_e*/
	if(*baud<4800)
	{
		*baud_e = 6;
	}
	else if((*baud>=4800)&&(*baud<9600))
	{
		*baud_e = 7;
	}
	else if((*baud>=9600)&&(*baud<19200))
	{
		*baud_e = 8;
	}
	else if((*baud>=19200)&&(*baud<38400))
	{
		*baud_e = 9;
	}
	else if((*baud>=38400)&&(*baud<76800))
	{
		*baud_e = 10;
	}
	else if((*baud>=76800)&&(*baud<230400))
	{
		*baud_e = 11;
	}
	else
	{
		*baud_e = 12;
	}
	
	/*����baud_m*/
	*baud_m = (char)((((*baud)*pow(2,28))/(sys_clk_baud*pow(2,*baud_e)))-256.0);
}
/*********************************************************************************************
* ���ƣ�uart0Init
* ���ܣ�uart0��ʼ�������õ�λ��1
* ������baud:������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart0Init(double baud)
{
	char baud_e,baud_m;
	
	P0SEL |=  0x0C;                 							//��ʼ��UART0�˿�
	PERCFG&= ~0x01;                 							//ѡ��UART0Ϊ��ѡλ��һ
	P0DIR &= ~(1<<2);                 							//P02,IN
	P0DIR |= (1<<3);                 							//PO3,OUT
	P2DIR &= ~0xC0;                 							//P0������Ϊ����0
	
	U0CSR = 0xC0;                   							//����ΪUARTģʽ,����ʹ�ܽ�����
	UART_BuadCount(&baud,&baud_e,&baud_m);						//���㲨����
	U0GCR = baud_e;                  
	U0BAUD = baud_m;                  							//���ò�����
    
    //�����ж����ȼ����
    //IP0 |= (1<<2);
    //IP1 |= (1<<2);
    
	URX0IE = 1;													//���ڽ����ж�ʹ��
	//EA = 1;														//�����ж�
}
/*********************************************************************************************
* ���ƣ�uart1Init
* ���ܣ�uart1��ʼ�������õ�λ��2
* ������baud:������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart1Init(double baud)
{
	char baud_e,baud_m;
	
	/*UART1��IO��ʼ��,P16,P17*/
	P1SEL |= ((1<<6)|(1<<7));									//ѡ��IO����Ϊ����
	PERCFG |= (1<<1);											//ѡ���õ�λ��2
	P1DIR &= ~(1<<7);											//����P17Ϊ����
	P1DIR |= (1<<6);											//����P16Ϊ���
    
	/*UART��ʼ��*/
	U1CSR = ((1<<7)|(1<<6));									//����ΪUARTģʽ,ʹ�ܽ���
	UART_BuadCount(&baud,&baud_e,&baud_m);						//���㲨����
	U1GCR = baud_e;
	U1BAUD = baud_m;											//���ò�����
    
    //�����ж����ȼ����
    //IP0 |= (1<<3);
    //IP1 |= (1<<3);
    
	URX1IE = 1;													//���ڽ����ж�ʹ��
	//EA = 1;														//�����ж�
}
/*********************************************************************************************
* ���ƣ�Uart0SendByte
* ���ܣ�����0�����ֽں���
* ������ch:Ҫ���͵��ֽ�
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void Uart0SendByte(unsigned char ch)
{
  /*
	U0DBUF = ch;
	while(UTX0IF == 0);
	UTX0IF = 0;
  */
       U0DBUF = ch;
     while((U0CSR&0x02) == 0);
     U0CSR = U0CSR&~0x02;
}
/*********************************************************************************************
* ���ƣ�Uart1SendByte
* ���ܣ�����1�����ֽں���
* ������ch:Ҫ���͵��ֽ�
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void Uart1SendByte(unsigned char ch)
{
	/*U1DBUF = ch;
	while(UTX1IF == 0);
	UTX1IF = 0;*/
     U1DBUF = ch;
     while(U1TX_BYTE == 0);
     U1TX_BYTE = 0;
}
/*********************************************************************************************
* ���ƣ�at_uart_init()
* ���ܣ��ڵ�ATָ��ڳ�ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_uart_init(void)
{
#if defined(CC2530_Serial)
//    uart0Init(38400);
    uart1Init(38400);
#else
    uart1Init(38400);
#endif
}
/*********************************************************************************************
* ���ƣ�at_uart_write()
* ���ܣ��ڵ�ATָ���д����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_uart_write(char ch)
{
#if defined(CC2530_Serial)
//    Uart0SendByte(ch);
    Uart1SendByte(ch);
#else
    Uart1SendByte(ch);
#endif
}
/*********************************************************************************************
* ���ƣ�at_uart_set_input_call()
* ���ܣ��ڵ�ATָ��ڶ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_uart_set_input_call(void (*c)(char ch))
{
  _input = c;
}
/*********************************************************************************************
* ���ƣ�HAL_ISR_FUNCTION()
* ���ܣ��ڵ�ATָ����жϴ���
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#if defined(CC2530_Serial)
//HAL_ISR_FUNCTION(halUart0TxIsr, URX0_VECTOR)
//{
//    char ch;
//    ch = U0DBUF;
//    if (_input != NULL) {
//      _input(ch);
//    }
//}
HAL_ISR_FUNCTION(halUart1TxIsr, URX1_VECTOR)
{
  char ch;
    ch = U1DBUF;
    if (_input != NULL) {
      _input(ch);
    }
}
#else
HAL_ISR_FUNCTION(halUart1TxIsr, URX1_VECTOR)
{
  char ch;
    ch = U1DBUF;
    if (_input != NULL) {
      _input(ch);
    }
}
#endif
/*********************************************************************************************
* ���ƣ�putchar()
* ���ܣ�printf iar ���Խӿ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#if  SUPPOER_DEBUG
__near_func int  putchar(int ch) 
{
  at_uart_write(ch);
  return ch;
}
#endif