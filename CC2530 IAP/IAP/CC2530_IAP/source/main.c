#include "main.h"
#include "sys.h"
#include "uart.h"
#include "shell.h"


static void hardware_init(void)
{
  xtal_init();                                                  // 系统时钟初始化
  uart0_init(38400);											                      // 初始化 控制台串口硬件
  
    /* 初始化 shell 控制台 */                
  shell_init("shell >" ,Uart0_Send_LenString);     // 初始化 控制台输出
  shell_input_init(&shell_1,Uart0_Send_LenString); // 初始化 交互
  welcome_gets(&shell_1,0,0);             // 主动显示 welcome'
  printk("Entered the bootloader program!\r\n");
}

void main(void)
{	
  hardware_init();                                              // 硬件初始化
  while(1)
  {
    shell_app_cycle();
  }
}
