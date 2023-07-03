#include "main.h"
#include "sys.h"
#include "uart.h"
#include "shell.h"
#include "user_cmd.h"
#include "iap_config.h"
#include "time.h"
#include "soft_timer.h"

// 系统初始化
static void system_init()
{
  /* 初始化系统基本硬件 */
  xtal_init();                                        // 系统时钟初始化          
  shell_hw_init(38400);											          // 初始化 控制台串口硬件
  /* 判断启动 APP 程序 */
  
  
  
  
  /* 启动 boot程序和控制台 */
  shell_init("shell >" ,Uart0_Send_LenString);        // 初始化 控制台输出
  shell_input_init(&shell_1,Uart0_Send_LenString);    // 初始化 交互
  welcome_gets(&shell_1,0,0);                         // 主动显示 welcome'
  printk(INFO"Entered the bootloader program!\r\n");

  /* shell 控制台进行用户输入 */
  cmdline_gets(&shell_1,"\r",1);                      // 一次换行
}

static void hardware_init(void)
{
  time1Int_init(1000);       // 1ms
}

void time1out()
{
  printk("tick:%d\r\n",tickCnt_Get());
}

// 应用初始化
static void app_init()
{
  register_user_cmd();
  
  
  softTimer_create(LED_APP_TIMER_ID,MODE_PERIODIC,time1out);
  softTimer_start(LED_APP_TIMER_ID,1000);
}

void main(void)
{	
  system_init();                      // 系统初始化
  hardware_init();                    // 硬件初始化
  app_init();                         // 应用初始化
  while(1)
  {
    softTimer_Update();               // 软件定时器
    shell_app_cycle();
  }
}
