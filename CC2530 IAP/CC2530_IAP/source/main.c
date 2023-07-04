#include "main.h"
#include "sys.h"
#include "uart.h"
#include "shell.h"
#include "led.h"
#include "user_cmd.h"
#include "iap_config.h"
#include "time.h"
#include "soft_timer.h"
#include "hal_flash.h"
#include "hal_dma.h"
#include "download.h"
#include "app_start.h"
#include "hal_board_cfg.h"

// 系统参数保存区域
sys_parameter_t sys_parameter ;

// 系统初始化
static void system_init()
{
  /* 初始化系统基本硬件 */
  HAL_BOARD_INIT();                                   // 系统时钟初始化
  vddWait(VDD_MIN_RUN);                               // 等待芯片的供电电压（VDD）达到指定的电压值
  HalDmaInit();                                       // 用于flash的读写
  shell_hw_init(38400);											          // 初始化 控制台串口硬件
  /* 判断启动 APP 程序 */
  SYS_PARAMETER_READ;
  
  FlashRead(PARA_PARTITION_START_ADDR,(uint8_t*)&sys_parameter,sizeof(sys_parameter));
  printf("current_part:%d\r\n",sys_parameter.current_part);
  printf("app1_flag:0x%02x\r\n",sys_parameter.app1_flag);
  start_app_partition(sys_parameter.current_part);
  
  /* 启动 boot程序和控制台 */
  shell_init("shell >" ,Uart0_Send_LenString);        // 初始化 控制台输出
  shell_input_init(&shell_1,Uart0_Send_LenString);    // 初始化 交互
  welcome_gets(&shell_1,0,0);                         // 主动显示 welcome'
  printk(INFO"Entered the bootloader program!\r\n");

  /* shell 控制台进行用户输入 */
  cmdline_gets(&shell_1,"\r",1);                      // 一次换行
}

// 硬件初始化
static void hardware_init(void)
{
  time1Int_init(1000);        // 1ms
  led_init();
}

// 应用初始化
static void app_init()
{
  register_user_cmd();
  led_app_init(); 
}

// 主程序
void main(void)
{	
  system_init();                      // 系统初始化
  hardware_init();                    // 硬件初始化
  app_init();                         // 应用初始化
  while(1)
  {
    softTimer_Update();               // 软件定时器
    if(usart0_mode==0)                // 串口数据走向
      shell_app_cycle();
    else
      IAP_download();
  }
}
