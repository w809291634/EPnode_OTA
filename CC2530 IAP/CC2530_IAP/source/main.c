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

// ϵͳ������������
sys_parameter_t sys_parameter ;

// ϵͳ��ʼ��
static void system_init()
{
  /* ��ʼ��ϵͳ����Ӳ�� */
  HAL_BOARD_INIT();                                   // ϵͳʱ�ӳ�ʼ��
  vddWait(VDD_MIN_RUN);                               // �ȴ�оƬ�Ĺ����ѹ��VDD���ﵽָ���ĵ�ѹֵ
  HalDmaInit();                                       // ����flash�Ķ�д
  shell_hw_init(38400);											          // ��ʼ�� ����̨����Ӳ��
  /* �ж����� APP ���� */
  SYS_PARAMETER_READ;
  
  FlashRead(PARA_PARTITION_START_ADDR,(uint8_t*)&sys_parameter,sizeof(sys_parameter));
  printf("current_part:%d\r\n",sys_parameter.current_part);
  printf("app1_flag:0x%02x\r\n",sys_parameter.app1_flag);
  start_app_partition(sys_parameter.current_part);
  
  /* ���� boot����Ϳ���̨ */
  shell_init("shell >" ,Uart0_Send_LenString);        // ��ʼ�� ����̨���
  shell_input_init(&shell_1,Uart0_Send_LenString);    // ��ʼ�� ����
  welcome_gets(&shell_1,0,0);                         // ������ʾ welcome'
  printk(INFO"Entered the bootloader program!\r\n");

  /* shell ����̨�����û����� */
  cmdline_gets(&shell_1,"\r",1);                      // һ�λ���
}

// Ӳ����ʼ��
static void hardware_init(void)
{
  time1Int_init(1000);        // 1ms
  led_init();
}

// Ӧ�ó�ʼ��
static void app_init()
{
  register_user_cmd();
  led_app_init(); 
}

// ������
void main(void)
{	
  system_init();                      // ϵͳ��ʼ��
  hardware_init();                    // Ӳ����ʼ��
  app_init();                         // Ӧ�ó�ʼ��
  while(1)
  {
    softTimer_Update();               // �����ʱ��
    if(usart0_mode==0)                // ������������
      shell_app_cycle();
    else
      IAP_download();
  }
}
