#include "uart.h"
#include "math.h"

shellinput_t shell_1;
unsigned char usart0_mode;
  
//���ܣ�uart0��ʼ�������õ�λ��1
//������baud:������
// ���أ���
void shell_hw_init(double baud)
{
	P0SEL |=  0x0C;                 							//��ʼ��UART0�˿�
	PERCFG&= ~0x01;                 							//ѡ��UART0Ϊ��ѡλ��һ
	P0DIR &= ~(1<<2);                 					  //P02,IN
	P0DIR |= (1<<3);                 							//PO3,OUT
	P2DIR &= ~0xC0;                 							//P0������Ϊ����0
	
	U0CSR = 0xC0;                   							//����ΪUARTģʽ,����ʹ�ܽ�����
  
  // 38400 U0GCR = 10; U0BAUD= 58;
	U0GCR = 10;                  
	U0BAUD = 58;                  						    //���ò�����
}

// ��λ״̬
void shell_hw_uninit(void)
{
  P0SEL = 0x00;
  P0DIR = 0x00;
  P2DIR = 0x00;
  PERCFG = 0x00; 
	U0CSR = 0x00;                  						
	U0GCR = 0x02;                  
	U0BAUD = 0x00; 
}

// �����ַ�
void Uart0_Send_char(unsigned char ch)
{
	U0DBUF = ch;
	while(UTX0IF == 0);
	UTX0IF = 0;
}

// �����ַ���
void Uart0_Send_LenString(const char *Data,unsigned short len)
{  
	while (len--)
	{
		Uart0_Send_char(*Data++);
	}
}

// ʹ�ò�ѯ����ȡ��������
// boot�����鲻Ҫʹ���жϷ�������APP�޷�ʹ�ô����ж� 
int usart0_getchar(uint8_t* data)
{
  if(U0CSR & 0x04)
  {
    *data = U0DBUF;     // ��ȡ���ռĴ�����ֵ
    return 1;
  }
  return 0;
}

// shell����̨��ȡ��������
void shell_app_cycle(void)
{
  char data;
  char flag=0x1B;             // ������������ַ�
  static char buf[3]={0};
  
  if(!usart0_getchar((uint8*)&data))return;
    
  /* ���ݴ��� */
  if(data==flag){
    memset(buf,0,3);
    buf[0]=flag;
  }
  // ���� �������ҷ����
  if(buf[0]==flag){
    if(data==flag)return;     // �Ѿ����գ����Զ���
    strncat(buf,&data,1);
    if(strlen(buf)>2){
      shell_input(&shell_1, buf, 3);
      memset(buf,0,3);
    } 
  }
  // �����ַ�
  else shell_input(&shell_1, &data, 1);
}

// ����printf����
__near_func int putchar(int c)
{
    Uart0_Send_char(c);
    return(c);
}
