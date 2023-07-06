#include "uart.h"
#include "math.h"

#define USART0_RINGBUF_SIZE                 256    
shellinput_t shell_1;
static char shell_ringbuf[USART0_RINGBUF_SIZE]={0};
static unsigned short Read_Index;
static unsigned short Write_Index;
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

void Uart0_Send_LenString(const char *Data,unsigned short len)
{  
	while (len--)
	{
		Uart0_Send_char(*Data++);
	}
}

/* ��ȡ��������ʣ��δ��ȡ���ݳ��� */
static unsigned short UART_GetRemain(void) {
  unsigned short remain_length;             // ʣ�����ݳ���
  unsigned short write_index=Write_Index;   // ������ǰд����λ��
  //��ȡʣ�����ݳ���
  if(write_index >= Read_Index) { 
    remain_length = write_index - Read_Index;
  } else {
    remain_length = USART0_RINGBUF_SIZE - Read_Index + write_index;   // ��ʱ˵�����ڻ��������ݴ�ͷ��ʼ����
  }
  return remain_length;
}

// �ӻ�������ȡһ���ֽ�
// ע�⣺���ݴ�������shell_app_cycleֻ��ѡ����һ
int usart0_getchar(uint8_t* data)
{
  if(Write_Index!=Read_Index){
    *data=shell_ringbuf[Read_Index];
    Read_Index = (Read_Index+1)% USART0_RINGBUF_SIZE;   // ��ȡ������1
    return 1;
  }else return 0;
}

// shell����̨��ȡ��������
void shell_app_cycle(void)
{
#define SHELL_DATA_PROCESS          {shell_input(&shell_1, temp, data_len);\
                                    Read_Index = (Read_Index+data_len)% USART0_RINGBUF_SIZE;}
         
  if(Write_Index!=Read_Index){
    /* ȡ���λ�����ʣ������ */
    char temp[USART0_RINGBUF_SIZE];
    memset(temp,0,USART0_RINGBUF_SIZE);
    unsigned short data_len= UART_GetRemain();          // ��ȡ��ǰ���ݳ���
    if(Read_Index+data_len>USART0_RINGBUF_SIZE){
      // ��������ȡ���ȳ�������
      int len1=USART0_RINGBUF_SIZE-Read_Index;          // ����ĩβ������
      int len2=data_len-len1;
      memcpy(temp,shell_ringbuf + Read_Index,len1);
      memcpy(temp+len1,shell_ringbuf,len2);
    }else{
      memcpy(temp,shell_ringbuf + Read_Index,data_len);
    } 
   
    /* ���ݴ��� */
    char object[3]={0};
    object[0]=0x1B; //object[1]='['; 
    if(strstr(temp,object)){
      // ��ֹ���ݲ�ȫ���´���
      delay_ms(2);
      if(strlen(temp)>2) 
        SHELL_DATA_PROCESS
    }
    else SHELL_DATA_PROCESS
  }
}
// ����0�����жϷ�����
#pragma vector = URX0_VECTOR      
__interrupt void uart0_RxInt(void)            
{    
	if (URX0IF == 1);
	{
    shell_ringbuf[Write_Index] = (char)U0DBUF;
    Write_Index++;
    Write_Index = Write_Index % USART0_RINGBUF_SIZE;
    URX0IF = 0;
	}
}

// ����printf����
__near_func int putchar(int c)
{
    Uart0_Send_char(c);
    return(c);
}
