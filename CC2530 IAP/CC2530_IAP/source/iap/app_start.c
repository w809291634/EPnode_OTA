#include "app_start.h"

// 复位外设寄存器
static void peripheral_reset(void)
{

}

// 复位中断控制器
void nvic_reset(void) 
{

}

// 复位中断控制器
static void NVIC_Reset(void) 
{

}

// 跳转到指定地址运行
void JumpToApp(uint32_t app_addr)
{
  pFunction Jump_To_Application; 
  uint32_t JumpAddress;	
  
  /* Jump to user application */
//  JumpAddress = *(volatile uint32_t*)(app_addr + 4);
  Jump_To_Application = (pFunction) JumpAddress;
  
  /* Initialize user application's Stack Pointer */
  Jump_To_Application();

  /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
  while (1)
  {
  }
}

typedef void (*AppStartFunction)(void);
// 跳转到指定地址运行
void JumpToApp1(uint32_t app_addr)
{
  // 关闭中断
  EA = 0;

  // 定义函数指针，并将其设置为APP分区的起始地址
  AppStartFunction appStart = (AppStartFunction)(app_addr);

  // 执行跳转到APP分区
  appStart();

  /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
  while (1)
  {
  }
}

// 启动app系统分区
void start_app_partition(uint8_t partition)
{
  switch(partition){
    case 1:{
      if(sys_parameter.app1_flag==APP_OK){
        printf(INFO"Starting partition 1!\r\n");
        printf(INFO"Next automatic start partition 1!\r\n");
        JumpToApp1(APP1_PARTITION_START_ADDR);
      }else{
        printf(INFO"Starting partition 1 fail!\r\n");
      }
    }break;
    case 0xff:break;
    default:{
      printf(INFO"partition %d not assigned!\r\n",partition);
    }break;
  }
}
