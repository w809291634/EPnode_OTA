#include "uart.h"
#include "math.h"

shellinput_t shell_1;
unsigned char usart0_mode;
  
//功能：uart0初始化，复用到位置1
//参数：baud:波特率
// 返回：无
void shell_hw_init(double baud)
{
	P0SEL |=  0x0C;                 							//初始化UART0端口
	PERCFG&= ~0x01;                 							//选择UART0为可选位置一
	P0DIR &= ~(1<<2);                 					  //P02,IN
	P0DIR |= (1<<3);                 							//PO3,OUT
	P2DIR &= ~0xC0;                 							//P0优先作为串口0
	
	U0CSR = 0xC0;                   							//设置为UART模式,而且使能接受器
  
  // 38400 U0GCR = 10; U0BAUD= 58;
	U0GCR = 10;                  
	U0BAUD = 58;                  						    //设置波特率
}

// 复位状态
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

// 发送字符
void Uart0_Send_char(unsigned char ch)
{
	U0DBUF = ch;
	while(UTX0IF == 0);
	UTX0IF = 0;
}

// 发送字符串
void Uart0_Send_LenString(const char *Data,unsigned short len)
{  
	while (len--)
	{
		Uart0_Send_char(*Data++);
	}
}

// 使用查询法读取串口数据
// boot程序建议不要使用中断法，可能APP无法使用串口中断 
int usart0_getchar(uint8_t* data)
{
  if(U0CSR & 0x04)
  {
    *data = U0DBUF;     // 读取接收寄存器的值
    return 1;
  }
  return 0;
}

// shell控制台获取输入数据
void shell_app_cycle(void)
{
  char data;
  char flag=0x1B;             // 方向键的特殊字符
  static char buf[3]={0};
  
  if(!usart0_getchar((uint8*)&data))return;
    
  /* 数据处理 */
  if(data==flag){
    memset(buf,0,3);
    buf[0]=flag;
  }
  // 处理 上下左右方向键
  if(buf[0]==flag){
    if(data==flag)return;     // 已经接收，可以丢弃
    strncat(buf,&data,1);
    if(strlen(buf)>2){
      shell_input(&shell_1, buf, 3);
      memset(buf,0,3);
    } 
  }
  // 处理字符
  else shell_input(&shell_1, &data, 1);
}

// 适配printf函数
__near_func int putchar(int c)
{
    Uart0_Send_char(c);
    return(c);
}
