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

// ϵͳ������������
sys_parameter_t sys_parameter ;

// ϵͳ��ʼ��
static void system_init()
{
  /* ��ʼ��ϵͳ����Ӳ�� */
  xtal_init();                                        // ϵͳʱ�ӳ�ʼ��
  PREFETCH_ENABLE();
  HalDmaInit();                                       // ����flash�Ķ�д
  shell_hw_init(38400);											          // ��ʼ�� ����̨����Ӳ��
  /* �ж����� APP ���� */
//  SYS_PARAMETER_READ;
//  
//  FlashRead(PARA_PARTITION_START_ADDR,(uint8_t*)&sys_parameter,sizeof(sys_parameter));
//  start_app_partition(sys_parameter.current_part);
//  
 
  
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

static char data1[2];
// ������
void main(void)
{	
  system_init();                      // ϵͳ��ʼ��
  hardware_init();                    // Ӳ����ʼ��
  app_init();                         // Ӧ�ó�ʼ��
  while(1)
  {
//    SYS_PARAMETER_READ;
//    hw_ms_delay(1000);
//    printk("0x%x 0x%x\r\n",(long)sys_parameter.current_part,(long)sys_parameter.app1_flag);
//    FlashRead(PARA_PARTITION_START_ADDR, (uint8_t*)&data1,2);
//    printf("0x%x 0x%x\r\n",data1[0],data1[1]);
    
    HalFlash_test(0x10000,CC2530_FLASH_END);
  
    softTimer_Update();               // �����ʱ��
    if(usart0_mode==0)                // ������������
      shell_app_cycle();
    else
      IAP_download();
  }
}
