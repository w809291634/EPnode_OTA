#include "app_start.h"

// ��λ����Ĵ���
static void peripheral_reset(void)
{

}

// ��λ�жϿ�����
void nvic_reset(void) 
{

}

// ��λ�жϿ�����
static void NVIC_Reset(void) 
{

}

// ��ת��ָ����ַ����
void JumpToApp(uint32_t app_addr)
{
  pFunction Jump_To_Application; 
  uint32_t JumpAddress;	
  
  /* Jump to user application */
//  JumpAddress = *(volatile uint32_t*)(app_addr + 4);
  Jump_To_Application = (pFunction) JumpAddress;
  
  /* Initialize user application's Stack Pointer */
  Jump_To_Application();

  /* ��ת�ɹ��Ļ�������ִ�е�����û�������������Ӵ��� */
  while (1)
  {
  }
}

// ����appϵͳ����
void start_app_partition(uint8_t partition)
{
  switch(partition){
    case 1:{
      if(sys_parameter.app1_flag==APP_OK){
        printk(INFO"Starting partition 1!\r\n");
        printk(INFO"Next automatic start partition 1!\r\n");
        JumpToApp(APP1_PARTITION_START_ADDR);
      }else{
        printk(INFO"Starting partition 1 fail!\r\n");
      }
    }break;
    case 0xff:break;
    default:{
      printk(INFO"partition %d not assigned!\r\n",partition);
    }break;
  }
}
