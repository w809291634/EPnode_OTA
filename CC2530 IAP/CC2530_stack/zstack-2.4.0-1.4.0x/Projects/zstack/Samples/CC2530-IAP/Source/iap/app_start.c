#include "app_start.h"
#include "sys.h"
#include "uart.h"

// ��λbootloader����
static void resetPeripheral() 
{
  // ��λ����
  shell_hw_uninit();
  // ��λGPIO
  P0 = 0x00;
  P1 = 0x00;
  P2 = 0x00;
  
  P0DIR = 0x00;
  P1DIR = 0x00;
  P2DIR = 0x00;
  
  P0INP = 0x00;
  P1INP = 0x00;
  P2INP = 0x00;
  
  P0IFG = 0x00;
  P1IFG = 0x00;
}

// ��ת����APP����
void jump_to_app(void)
{
  resetPeripheral();
  // Simulate a reset for the Application code by an absolute jump to location 0x2000.
  asm("LJMP 0x5000\n");
  HAL_SYSTEM_RESET();
}

// ����appϵͳ����
void start_app_partition(uint8_t partition)
{
  // ǿ�ƽ���boot
  if(SB1_PRESS || SB2_PRESS) return;
    
  // ����falsh��������
  switch(partition){
    case 1:{
      if(sys_parameter.app1_flag==APP_OK){
        jump_to_app();
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
