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

typedef void (*AppStartFunction)(void);
// ��ת��ָ����ַ����
void JumpToApp1(uint32_t app_addr)
{
  // �ر��ж�
  EA = 0;

  // ���庯��ָ�룬����������ΪAPP��������ʼ��ַ
  AppStartFunction appStart = (AppStartFunction)(app_addr);

  // ִ����ת��APP����
  appStart();

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
