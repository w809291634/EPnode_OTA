#include "uart.h"
#include "math.h"

#define USART0_RINGBUF_SIZE                 256        
shellinput_t shell_1;
static char shell_ringbuf[USART0_RINGBUF_SIZE]={0};
static unsigned short Read_Index;
static unsigned short Write_Index;
unsigned char usart0_mode;
  
//功能：串口波特率计算
//参数：baud:波特率
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

//功能：uart0初始化，复用到位置1
//参数：baud:波特率
// 返回：无
void shell_hw_init(double baud)
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
    
	URX0IE = 1;													        //串口接收中断使能
	EA = 1;														          //开总中断
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

/* 获取缓冲区中剩余未读取数据长度 */
static unsigned short UART_GetRemain(void) {
  unsigned short remain_length;             // 剩余数据长度
  unsigned short write_index=Write_Index;   // 拷贝当前写索引位置
  //获取剩余数据长度
  if(write_index >= Read_Index) { 
    remain_length = write_index - Read_Index;
  } else {
    remain_length = USART0_RINGBUF_SIZE - Read_Index + write_index;   // 此时说明串口缓存区数据从头开始缓存
  }
  return remain_length;
}

// 从缓存区读取一个字节
// 注意：数据处理结果和shell_app_cycle只能选择其一
int usart0_getchar(uint8_t* data)
{
  if(Write_Index!=Read_Index){
    *data=shell_ringbuf[Read_Index];
    Read_Index = (Read_Index+1)% USART0_RINGBUF_SIZE;   // 读取索引加1
    return 1;
  }else return 0;
}

// shell控制台获取输入数据
void shell_app_cycle(void)
{
#define SHELL_DATA_PROCESS          {shell_input(&shell_1, temp, data_len);\
                                    Read_Index = (Read_Index+data_len)% USART0_RINGBUF_SIZE;}
         
  if(Write_Index!=Read_Index){
    /* 取环形缓存区剩余数据 */
    char temp[USART0_RINGBUF_SIZE];
    memset(temp,0,USART0_RINGBUF_SIZE);
    unsigned short data_len= UART_GetRemain();          // 获取当前数据长度
    if(Read_Index+data_len>USART0_RINGBUF_SIZE){
      // 索引处读取长度超出缓存
      int len1=USART0_RINGBUF_SIZE-Read_Index;          // 环形末尾读长度
      int len2=data_len-len1;
      memcpy(temp,shell_ringbuf + Read_Index,len1);
      memcpy(temp+len1,shell_ringbuf,len2);
    }else{
      memcpy(temp,shell_ringbuf + Read_Index,data_len);
    } 
   
    /* 数据处理 */
    char object[3]={0};
    object[0]=0x1B; //object[1]='['; 
    if(strstr(temp,object)){
      // 防止数据不全导致错误
      delay_ms(2);
      if(strlen(temp)>2) 
        SHELL_DATA_PROCESS
    }
    else SHELL_DATA_PROCESS
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
    Write_Index = Write_Index % USART0_RINGBUF_SIZE;
    URX0IF = 0;
	}
}

// 适配printf函数
__near_func int putchar(int c)
{
    Uart0_Send_char(c);
    return(c);
}