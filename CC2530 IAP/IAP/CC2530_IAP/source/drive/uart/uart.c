#include "uart.h"
#include "math.h"

#define USART1_RINGBUF_SIZE                 (256)        // �����ն���ʹ��������Ҫ1050
shellinput_t shell_1;
static char shell_ringbuf[USART1_RINGBUF_SIZE]={0};
static unsigned short Read_Index;
static unsigned short Write_Index;


/*UART1*/
#define U1RxBuf_SIZE 64										    //���ݽ�������ֽ��������ܳ���63
extern unsigned char UART1_RX_STA;							//UART1����״̬�����ո���
extern unsigned char U1RX_Buf[U1RxBuf_SIZE];				    //UART1���ݽ���BUF

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
* ���ƣ�uart0_init
* ���ܣ�uart0��ʼ�������õ�λ��1
* ������baud:������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart0_init(double baud)
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
    
	URX0IE = 1;													//���ڽ����ж�ʹ��
	EA = 1;														//�����ж�
}

/*********************************************************************************************
* ���ƣ�Uart0_Send_char
* ���ܣ�����0�����ֽں���
* ������ch:Ҫ���͵��ֽ�
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void Uart0_Send_char(unsigned char ch)
{
	U0DBUF = ch;
	while(UTX0IF == 0);
	UTX0IF = 0;
}

/*********************************************************************************************
* ���ƣ�Uart0_Send_String
* ���ܣ�����0�����ַ�������
* ������*Data:Ҫ�����ַ����׵�ַ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void Uart0_Send_String(unsigned char *Data)
{  
	while (*Data != '\0')
	{
		Uart0_Send_char(*Data++);
	}
}

/*********************************************************************************************
* ���ƣ�Uart0_Send_String
* ���ܣ�����0�����ַ�������
* ������*Data:Ҫ�����ַ����׵�ַ
		len���������ݳ���
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
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
    remain_length = USART1_RINGBUF_SIZE - Read_Index + write_index;   // ��ʱ˵�����ڻ��������ݴ�ͷ��ʼ����
  }
  return remain_length;
}

// shell����̨��ȡ��������
void shell_app_cycle(void)
{
  if(Write_Index!=Read_Index){
    /* ȡ���λ�����ʣ������ */
    char temp[USART1_RINGBUF_SIZE]={0};
    unsigned short data_len= UART_GetRemain();          // ��ȡ��ǰ���ݳ���
    if(Read_Index+data_len>USART1_RINGBUF_SIZE){
      // ��������ȡ���ȳ�������
      int len1=USART1_RINGBUF_SIZE-Read_Index;          // ����ĩβ������
      int len2=data_len-len1;
      memcpy(temp,shell_ringbuf + Read_Index,len1);
      memcpy(temp+len1,shell_ringbuf,len2);
    }else{
      memcpy(temp,shell_ringbuf + Read_Index,data_len);
    } 
   
    char  object[3]={0};
    object[0]=0x1B;
    object[1]='[';
    if(strstr(temp,object) ){
      if(strlen(temp)>2){
        shell_input(&shell_1, temp, data_len);
        /* ���ݴ������ */
        Read_Index = (Read_Index+data_len)% USART1_RINGBUF_SIZE;          // �´ζ�ȡ���ݵ���ʼλ�ã���ֹ�����������������
      }
    }else{
      shell_input(&shell_1, temp, data_len);
      /* ���ݴ������ */
      Read_Index = (Read_Index+data_len)% USART1_RINGBUF_SIZE;          // �´ζ�ȡ���ݵ���ʼλ�ã���ֹ�����������������
    }
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
    Write_Index = Write_Index % USART1_RINGBUF_SIZE;
    URX0IF = 0;
	}
}

/*********************************************************************************************
* ���ƣ�uart1CallBack
* ���ܣ�uart1�жϻص�����
* ������data�������жϽ��յ�������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart1CallBack(u8 data)
{
    static u8 recvFlag=0;
    static u8 DataLen=U1RxBuf_SIZE;
    
    /*���ݽ���Э��*/
    if((UART1_RX_STA&0x80)!=0x80)							    //�ϴ������Ѵ���			
    {		
        if((recvFlag&0x01)==0x01)						        //���յ�"0XAF"����ͷ
        {
            if((UART1_RX_STA&0x7F)>U1RxBuf_SIZE)			    //RxBuf��������½���
            {				
                UART1_RX_STA = 0;
                recvFlag = 0;
                DataLen = U1RxBuf_SIZE;
            }
            
            U1RX_Buf[UART1_RX_STA&0x7F] = data;	
            UART1_RX_STA++;	
            
            //���㱾�����ݳ���
            if((UART1_RX_STA&0x7F)==2)
            {				
                DataLen = U1RX_Buf[1]+4;
            }
            if((UART1_RX_STA&0x7F)==DataLen)				    //���ν������,����δ����
            {		
                UART1_RX_STA-=1;
                UART1_RX_STA |= 0x80;
                recvFlag = 0;
                DataLen = U1RxBuf_SIZE;
            }
        }
        else if(data==0xAF)
        {
            recvFlag = 0x01;
            U1RX_Buf[UART1_RX_STA&0x3F] = data;	
            UART1_RX_STA++;
        }
    }
}

/*********************************************************************************************
* ���ƣ�uart1_RxInt
* ���ܣ�����1�����жϷ�����
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
/*
UART1_RX_STA��
	bit7:������ɱ�־
	bit6;���յ�����ͷ��־
	bit5-0:���յ����ݵĸ���
*/
unsigned char UART1_RX_STA=0;									//UART1����״̬�����ո���
unsigned char U1RX_Buf[U1RxBuf_SIZE];						//UART1���ݽ���BUF

/*********************************************************************************************
* ���ƣ�uart1_init
* ���ܣ�uart1��ʼ�������õ�λ��2
* ������baud:������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void uart1_init(double baud)
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
    IP0 |= (1<<3);
    IP1 |= (1<<3);
    
	URX1IE = 1;													//���ڽ����ж�ʹ��
	EA = 1;														//�����ж�
}

/*********************************************************************************************
* ���ƣ�Uart1_Send_char
* ���ܣ�����1�����ֽں���
* ������ch:Ҫ���͵��ֽ�
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void Uart1_Send_char(unsigned char ch)
{
	U1DBUF = ch;
	while(UTX1IF == 0);
	UTX1IF = 0;
}

/*********************************************************************************************
* ���ƣ�Uart1_Send_String
* ���ܣ�����1�����ַ�������
* ������*Data:Ҫ�����ַ����׵�ַ
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void Uart1_Send_String(unsigned char *Data)
{  
	while (*Data != '\0')
	{
		Uart1_Send_char(*Data++);
	}
}

/*********************************************************************************************
* ���ƣ�Uart0_Send_String
* ���ܣ�����0�����ַ�������
* ������*Data:Ҫ�����ַ����׵�ַ
		len���������ݳ���
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void Uart1_Send_LenString(unsigned char *Data,int len)
{  
	while (len--)
	{
		Uart1_Send_char(*Data++);
	}
}

/*********************************************************************************************
* ���ƣ�Uart1_Recv_char
* ���ܣ�����1�����ֽں���
* ��������
* ���أ����յ��ֽ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int Uart1_Recv_char(void)
{
	int ch;

	while (URX1IF == 0);
	ch = U1DBUF;
	URX1IF = 0;
	
	return ch;
}

#pragma vector = URX1_VECTOR      
__interrupt void uart1_RxInt(void)            
{    
	unsigned char ch;

	EA=0;                           							//�����ж� 	
	if (URX1IF == 1);
	{
		ch = U1DBUF;
		URX1IF = 0;
        
		uart1CallBack(ch);
	}
	EA=1;                           							//�����ж� 
}
