#include "uart.h"
#include "math.h"

#define USART1_RINGBUF_SIZE                 (256)        // 超级终端中使用至少需要1050
shellinput_t shell_1;
static char shell_ringbuf[USART1_RINGBUF_SIZE]={0};
static unsigned short Read_Index;
static unsigned short Write_Index;


/*UART1*/
#define U1RxBuf_SIZE 64										    //数据接收最大字节数，不能超过63
extern unsigned char UART1_RX_STA;							//UART1接收状态及接收个数
extern unsigned char U1RX_Buf[U1RxBuf_SIZE];				    //UART1数据接收BUF

/*********************************************************************************************
* 名称：UART_BuadCount
* 功能：串口波特率计算
* 参数：baud:波特率
* 返回：无
* 修改：
* 注释：根据波特率计算寄存器值
*********************************************************************************************/
void UART_BuadCount(double* baud,char* baud_e,char* baud_m)
{
	double sys_clk_baud = 32000000.0;							//系统时钟
	
	/*根据波特率选择baud_e*/
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
	
	/*计算baud_m*/
	*baud_m = (char)((((*baud)*pow(2,28))/(sys_clk_baud*pow(2,*baud_e)))-256.0);
}

/*********************************************************************************************
* 名称：uart0_init
* 功能：uart0初始化，复用到位置1
* 参数：baud:波特率
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void uart0_init(double baud)
{
	char baud_e,baud_m;
	
	P0SEL |=  0x0C;                 							//初始化UART0端口
	PERCFG&= ~0x01;                 							//选择UART0为可选位置一
	P0DIR &= ~(1<<2);                 					  //P02,IN
	P0DIR |= (1<<3);                 							//PO3,OUT
	P2DIR &= ~0xC0;                 							//P0优先作为串口0
	
	U0CSR = 0xC0;                   							//设置为UART模式,而且使能接受器
	UART_BuadCount(&baud,&baud_e,&baud_m);				//计算波特率
	U0GCR = baud_e;                  
	U0BAUD = baud_m;                  						//设置波特率
    
  //设置中断优先级最高
  IP0 |= (1<<2);
  IP1 |= (1<<2);
    
	URX0IE = 1;													//串口接收中断使能
	EA = 1;														//开总中断
}

/*********************************************************************************************
* 名称：Uart0_Send_char
* 功能：串口0发送字节函数
* 参数：ch:要发送的字节
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void Uart0_Send_char(unsigned char ch)
{
	U0DBUF = ch;
	while(UTX0IF == 0);
	UTX0IF = 0;
}

/*********************************************************************************************
* 名称：Uart0_Send_String
* 功能：串口0发送字符串函数
* 参数：*Data:要发送字符串首地址
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void Uart0_Send_String(unsigned char *Data)
{  
	while (*Data != '\0')
	{
		Uart0_Send_char(*Data++);
	}
}

/*********************************************************************************************
* 名称：Uart0_Send_String
* 功能：串口0发送字符串函数
* 参数：*Data:要发送字符串首地址
		len：发送数据长度
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void Uart0_Send_LenString(const char *Data,unsigned short len)
{  
	while (len--)
	{
		Uart0_Send_char(*Data++);
	}
}

/* 获取缓冲区中剩余未读取数据长度 */
static unsigned short UART_GetRemain(void) {
  unsigned short remain_length;             // 剩余数据长度
  unsigned short write_index=Write_Index;   // 拷贝当前写索引位置
  //获取剩余数据长度
  if(write_index >= Read_Index) { 
    remain_length = write_index - Read_Index;
  } else {
    remain_length = USART1_RINGBUF_SIZE - Read_Index + write_index;   // 此时说明串口缓存区数据从头开始缓存
  }
  return remain_length;
}

// shell控制台获取输入数据
void shell_app_cycle(void)
{
  if(Write_Index!=Read_Index){
    /* 取环形缓存区剩余数据 */
    char temp[USART1_RINGBUF_SIZE]={0};
    unsigned short data_len= UART_GetRemain();          // 获取当前数据长度
    if(Read_Index+data_len>USART1_RINGBUF_SIZE){
      // 索引处读取长度超出缓存
      int len1=USART1_RINGBUF_SIZE-Read_Index;          // 环形末尾读长度
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
        /* 数据处理结束 */
        Read_Index = (Read_Index+data_len)% USART1_RINGBUF_SIZE;          // 下次读取数据的起始位置，防止超出缓存区最大索引
      }
    }else{
      shell_input(&shell_1, temp, data_len);
      /* 数据处理结束 */
      Read_Index = (Read_Index+data_len)% USART1_RINGBUF_SIZE;          // 下次读取数据的起始位置，防止超出缓存区最大索引
    }
  }
}

// 串口0接收中断服务函数
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
* 名称：uart1CallBack
* 功能：uart1中断回调函数
* 参数：data：串口中断接收到的数据
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void uart1CallBack(u8 data)
{
    static u8 recvFlag=0;
    static u8 DataLen=U1RxBuf_SIZE;
    
    /*数据接收协议*/
    if((UART1_RX_STA&0x80)!=0x80)							    //上次数据已处理			
    {		
        if((recvFlag&0x01)==0x01)						        //接收到"0XAF"数据头
        {
            if((UART1_RX_STA&0x7F)>U1RxBuf_SIZE)			    //RxBuf溢出，重新接收
            {				
                UART1_RX_STA = 0;
                recvFlag = 0;
                DataLen = U1RxBuf_SIZE;
            }
            
            U1RX_Buf[UART1_RX_STA&0x7F] = data;	
            UART1_RX_STA++;	
            
            //计算本次数据长度
            if((UART1_RX_STA&0x7F)==2)
            {				
                DataLen = U1RX_Buf[1]+4;
            }
            if((UART1_RX_STA&0x7F)==DataLen)				    //本次接收完成,数据未处理
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
* 名称：uart1_RxInt
* 功能：串口1接收中断服务函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
/*
UART1_RX_STA：
	bit7:接收完成标志
	bit6;接收到数据头标志
	bit5-0:接收到数据的个数
*/
unsigned char UART1_RX_STA=0;									//UART1接收状态及接收个数
unsigned char U1RX_Buf[U1RxBuf_SIZE];						//UART1数据接收BUF

/*********************************************************************************************
* 名称：uart1_init
* 功能：uart1初始化，复用到位置2
* 参数：baud:波特率
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void uart1_init(double baud)
{
	char baud_e,baud_m;
	
	/*UART1，IO初始化,P16,P17*/
	P1SEL |= ((1<<6)|(1<<7));									//选择IO功能为外设
	PERCFG |= (1<<1);											//选择复用到位置2
	P1DIR &= ~(1<<7);											//设置P17为输入
	P1DIR |= (1<<6);											//设置P16为输出

	/*UART初始化*/
	U1CSR = ((1<<7)|(1<<6));									//设置为UART模式,使能接收
	UART_BuadCount(&baud,&baud_e,&baud_m);						//计算波特率
	U1GCR = baud_e;
	U1BAUD = baud_m;											//设置波特率

    //设置中断优先级最高
    IP0 |= (1<<3);
    IP1 |= (1<<3);
    
	URX1IE = 1;													//串口接收中断使能
	EA = 1;														//开总中断
}

/*********************************************************************************************
* 名称：Uart1_Send_char
* 功能：串口1发送字节函数
* 参数：ch:要发送的字节
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void Uart1_Send_char(unsigned char ch)
{
	U1DBUF = ch;
	while(UTX1IF == 0);
	UTX1IF = 0;
}

/*********************************************************************************************
* 名称：Uart1_Send_String
* 功能：串口1发送字符串函数
* 参数：*Data:要发送字符串首地址
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void Uart1_Send_String(unsigned char *Data)
{  
	while (*Data != '\0')
	{
		Uart1_Send_char(*Data++);
	}
}

/*********************************************************************************************
* 名称：Uart0_Send_String
* 功能：串口0发送字符串函数
* 参数：*Data:要发送字符串首地址
		len：发送数据长度
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void Uart1_Send_LenString(unsigned char *Data,int len)
{  
	while (len--)
	{
		Uart1_Send_char(*Data++);
	}
}

/*********************************************************************************************
* 名称：Uart1_Recv_char
* 功能：串口1接收字节函数
* 参数：无
* 返回：接收的字节
* 修改：
* 注释：
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

	EA=0;                           							//关总中断 	
	if (URX1IF == 1);
	{
		ch = U1DBUF;
		URX1IF = 0;
        
		uart1CallBack(ch);
	}
	EA=1;                           							//开总中断 
}
