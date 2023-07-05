#include "main.h"
#include "sys.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "soft_timer.h"
#include "flash.h"
#include "hal_dma.h"
#include "hal_board_cfg.h"

// 系统初始化
static void system_init()
{
  /* 初始化系统基本硬件 */
  HAL_BOARD_INIT();                                   // 系统时钟初始化
  vddWait(VDD_MIN_RUN);                               // 等待芯片的供电电压（VDD）达到指定的电压值
  shell_hw_init(38400);											          // 初始化 控制台串口硬件
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
  }
}
