#include "user_cmd.h"
#include "shell.h"
#include "uart.h"
#include "iap_config.h"
#include "sys.h"

void SoftReset(void* arg)
{ 
  (void)arg;
}

// ���� IAP ģʽ
void start_IAP_mode(void * arg)
{
  usart0_mode=1;

  char * argv[4];
  int argc =cmdline_strtok((char*)arg,argv,3);
  printk("%s %s %s\r\n",argv[0],argv[1],argv[2]);
  
}

// ���� APP
void Start_APP(void * arg)
{
  sys_parameter.current_part=1;
//  write_sys_parameter();
}

// �û�����ע��
void register_user_cmd()
{
  shell_register_command("reboot",SoftReset);
}
