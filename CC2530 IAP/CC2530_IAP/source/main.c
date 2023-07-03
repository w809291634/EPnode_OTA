#include "main.h"
#include "sys.h"
#include "uart.h"
#include "shell.h"
#include "user_cmd.h"
#include "iap_config.h"
#include "time.h"
#include "soft_timer.h"

// ϵͳ��ʼ��
static void system_init()
{
  /* ��ʼ��ϵͳ����Ӳ�� */
  xtal_init();                                        // ϵͳʱ�ӳ�ʼ��          
  shell_hw_init(38400);											          // ��ʼ�� ����̨����Ӳ��
  /* �ж����� APP ���� */
  
  
  
  
  /* ���� boot����Ϳ���̨ */
  shell_init("shell >" ,Uart0_Send_LenString);        // ��ʼ�� ����̨���
  shell_input_init(&shell_1,Uart0_Send_LenString);    // ��ʼ�� ����
  welcome_gets(&shell_1,0,0);                         // ������ʾ welcome'
  printk(INFO"Entered the bootloader program!\r\n");

  /* shell ����̨�����û����� */
  cmdline_gets(&shell_1,"\r",1);                      // һ�λ���
}

static void hardware_init(void)
{
  time1Int_init(1000);       // 1ms
}

void time1out()
{
  printk("tick:%d\r\n",tickCnt_Get());
}

// Ӧ�ó�ʼ��
static void app_init()
{
  register_user_cmd();
  
  
  softTimer_create(LED_APP_TIMER_ID,MODE_PERIODIC,time1out);
  softTimer_start(LED_APP_TIMER_ID,1000);
}

void main(void)
{	
  system_init();                      // ϵͳ��ʼ��
  hardware_init();                    // Ӳ����ʼ��
  app_init();                         // Ӧ�ó�ʼ��
  while(1)
  {
    softTimer_Update();               // �����ʱ��
    shell_app_cycle();
  }
}
