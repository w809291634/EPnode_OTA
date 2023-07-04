#include "user_cmd.h"
#include "shell.h"
#include "uart.h"
#include "iap_config.h"
#include "sys.h"
#include "hal_flash.h"
#include "download.h"

void SoftReset(void* arg)
{ 
  (void)arg;
  HAL_DISABLE_INTERRUPTS();
  // Abort all DMA channels to insure that ongoing operations do not
  // interfere with re-configuration.
  DMAARM = 0x80 | 0x1F;
  asm("LJMP 0x0");
}

// 启动 IAP 模式
void __IAP(void * arg)
{
  char * argv[2];
  int argc =cmdline_strtok((char*)arg,argv,2);
  if(argc<2){
    debug_info(INFO"please input %s [<update> or <exit> ] \r\n",IAP_CMD);
    return;
  }
  if(strstr(argv[1],"update")){
    usart0_mode=1;
    download_part=1;
  }else if(strstr(argv[1],"exit")){
    sys_parameter.current_part=1;
    write_sys_parameter();  
  }else{
    debug_info(INFO"please input %s [<update> or <exit> ] \r\n",IAP_CMD);
  }
}

// 用户命令注册
void register_user_cmd()
{
  shell_register_command("reboot",SoftReset);
  shell_register_command(IAP_CMD,__IAP);
}
