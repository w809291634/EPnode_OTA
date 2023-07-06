#include "sys.h"
#include "uart.h"
#include "download.h"
#include "app_start.h"
#include "shell.h"
#include "user_cmd.h"

sys_parameter_t sys_parameter;
halDMADesc_t dmaCh0;

// 系统初始化
static void system_init()
{
  /* 初始化系统基本硬件 */
  HAL_BOARD_INIT();
  vddWait(VDD_MIN_RUN);
  HAL_DMA_SET_ADDR_DESC0(&dmaCh0);                                  // 用于flash的读写
  keyInit();
  shell_hw_init(38400);
  /* 判断启动 APP 程序 */
//  SYS_PARAMETER_READ;
  jump_to_app();
  
  
  
//  
//  printf("current_part:%d\r\n",sys_parameter.current_part);
//  printf("app1_flag:0x%02x\r\n",sys_parameter.app1_flag);
//  start_app_partition(sys_parameter.current_part);
  
  /* 启动 boot程序和控制台 */
  shell_init("shell >" ,Uart0_Send_LenString);        // 初始化 控制台输出
  shell_input_init(&shell_1,Uart0_Send_LenString);    // 初始化 交互
  welcome_gets(&shell_1,0,0);                         // 主动显示 welcome'

  /* shell 控制台进行用户输入 */
  cmdline_gets(&shell_1,"\r",1);                      // 一次换行
}

// APP初始化
static void app_init()
{
  led_init();
  register_user_cmd();
}

void main(void)
{
  system_init();
  app_init();
  while(1){
    led_app();
    shell_app_cycle();
    if(usart0_mode==0)                // 串口数据走向
      shell_app_cycle();
    else
      IAP_download();
  }
}
