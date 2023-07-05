#include "uart.h"
#include "math.h"

#define USART0_RINGBUF_SIZE                 256        
  
//���ܣ����ڲ����ʼ���
//������baud:������
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

//���ܣ�uart0��ʼ�������õ�λ��1
//������baud:������
// ���أ���
void shell_hw_init(double baud)
{
	char baud_e,baud_m;
	
	P0SEL |=  0x0C;                 							//��ʼ��UART0�˿�
	PERCFG&= ~0x01;                 							//ѡ��UART0Ϊ��ѡλ��һ
	P0DIR &= ~(1<<2);                 					  //P02,IN
	P0DIR |= (1<<3);                 							//PO3,OUT
	P2DIR &= ~0xC0;                 							//P0������Ϊ����0
	
	U0CSR = 0xC0;                   							//����ΪUARTģʽ,����ʹ�ܽ�����
	UART_BuadCount(&baud,&baud_e,&baud_m);				//���㲨����
	U0GCR = baud_e;                  
	U0BAUD = baud_m;                  						//���ò�����
    
  //�����ж����ȼ����
  IP0 |= (1<<2);
  IP1 |= (1<<2);
    
	URX0IE = 1;													        //���ڽ����ж�ʹ��
	EA = 1;														          //�����ж�
}

void Uart0_Send_char(unsigned char ch)
{
	U0DBUF = ch;
	while(UTX0IF == 0);
	UTX0IF = 0;
}

void Uart0_Send_String(unsigned char *Data)
{  
	while (*Data != '\0')
	{
		Uart0_Send_char(*Data++);
	}
}

void Uart0_Send_LenString(const char *Data,unsigned short len)
{  
	while (len--)
	{
		Uart0_Send_char(*Data++);
	}
}

// ����0�����жϷ�����
#pragma vector = URX0_VECTOR      
__interrupt void uart0_RxInt(void)            
{    
	if (URX0IF == 1);
	{
    (char)U0DBUF;

    URX0IF = 0;
	}
}

// ����printf����
__near_func int putchar(int c)
{
    Uart0_Send_char(c);
    return(c);
}