#include "main.h"
#include "sys.h"
#include "uart.h"
#include "shell.h"


static void hardware_init(void)
{
  xtal_init();                                                  // ϵͳʱ�ӳ�ʼ��
  uart0_init(38400);											                      // ��ʼ�� ����̨����Ӳ��
  
    /* ��ʼ�� shell ����̨ */                
  shell_init("shell >" ,Uart0_Send_LenString);     // ��ʼ�� ����̨���
  shell_input_init(&shell_1,Uart0_Send_LenString); // ��ʼ�� ����
  welcome_gets(&shell_1,0,0);             // ������ʾ welcome'
  printk("Entered the bootloader program!\r\n");
}

void main(void)
{	
  hardware_init();                                              // Ӳ����ʼ��
  while(1)
  {
    shell_app_cycle();
  }
}
