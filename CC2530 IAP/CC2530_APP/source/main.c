#include "main.h"
#include "sys.h"
#include "uart.h"
#include "led.h"
#include "time.h"
#include "soft_timer.h"
#include "flash.h"
#include "hal_dma.h"
#include "hal_board_cfg.h"

// ϵͳ��ʼ��
static void system_init()
{
  /* ��ʼ��ϵͳ����Ӳ�� */
  HAL_BOARD_INIT();                                   // ϵͳʱ�ӳ�ʼ��
  vddWait(VDD_MIN_RUN);                               // �ȴ�оƬ�Ĺ����ѹ��VDD���ﵽָ���ĵ�ѹֵ
  shell_hw_init(38400);											          // ��ʼ�� ����̨����Ӳ��
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
  }
}
