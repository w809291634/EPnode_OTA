#include "sys.h"
#include "uart.h"
#include "download.h"
#include "app_start.h"
#include "shell.h"
#include "user_cmd.h"

sys_parameter_t sys_parameter;
halDMADesc_t dmaCh0;

// ϵͳ��ʼ��
static void system_init()
{
  /* ��ʼ��ϵͳ����Ӳ�� */
  HAL_BOARD_INIT();
  vddWait(VDD_MIN_RUN);
  HAL_DMA_SET_ADDR_DESC0(&dmaCh0);                    // ����flash�Ķ�д
  keyInit();                                          // ����ʹ�ð�������bootģʽ
  shell_hw_init(38400);
  /* �ж����� APP ���� */
  SYS_PARAMETER_READ;
  start_app_partition(sys_parameter.current_part);
  
  /* ���� boot����Ϳ���̨ */
  shell_init("shell >" ,Uart0_Send_LenString);        // ��ʼ�� ����̨���
  shell_input_init(&shell_1,Uart0_Send_LenString);    // ��ʼ�� ����
  welcome_gets(&shell_1,0,0);                         // ������ʾ welcome

  /* shell ����̨�����û����� */
  cmdline_gets(&shell_1,"\r",1);                      // һ�λ���
}

// APP��ʼ��
static void app_init()
{
  led_init();
  register_user_cmd();
}

// ������
void main(void)
{
  system_init();
  app_init();
  while(1){
    led_app();
    shell_app_cycle();
    if(usart0_mode==0)                // ������������
      shell_app_cycle();
    else
      IAP_download();
  }
}
