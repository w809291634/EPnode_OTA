#include "download.h"
#include "iap_config.h"
#include "ymodem.h"
#include "uart.h"
#include "hal_flash.h"

extern uint8_t file_name[FILE_NAME_LENGTH];

// 通过串口 IAP 向 flash 下载一个文件
void IAP_download(void)
{
  uint8_t Number[11] = "          ";
  int32_t Size = 0;
  uint32_t timeout=30*1000;

  /* 进入下载模式 */
  debug_info("download partition start_address:0x%lx,size:%ld Bytes\n\r",
     APP1_PARTITION_START_ADDR,APP1_PARTITION_SIZE);
  debug_info("Waiting for the file to be sent ... (press 'a' key to exit IAP mode)\n\r");
  
  // 阻塞运行
  Size = Ymodem_Receive(APP1_PARTITION_START_ADDR,APP1_PARTITION_SIZE,timeout);
  delay_ms(500);
  if (Size > 0)
  {
    debug_info("\n\n\r Programming Successfully!\n\r--------------------------------\r\n Name: %s",(char*)file_name);
    Int2Str(Number, Size);
    debug_info("\n\r Size: %s Bytes\r\n",(char*)Number);
    debug_info("-------------------\r\n");
  }
  else if (Size == 0) debug_err("\n\r"ERR"Termination by sender!\n\r");
  else if (Size == -1) debug_err("\n\r"ERR"Programming address error or File too large!\n\r");
  else if (Size == -2) debug_err("\n\r"ERR"Erase flash error!\n\r");
  else if (Size == -3) debug_err("\n\r"ERR"Programming flash error!\n\r");
  else if (Size == -10) debug_err("\n\r"ERR"Error count exceeded.\n\r");
  else if (Size == -11) debug_err("\n\r"ERR"Waiting timeout\n\r");
  else if (Size == -30) debug_war("\n\r"WARNING"Aborted by user.\n\r");
  else debug_err("\n\r"ERR"Failed to receive the file!\n\r");

  /* 进入控制台模式 */
  usart0_mode=0;
}

