#include "download.h"
#include "iap_config.h"
#include "ymodem.h"
#include "uart.h"
#include "hal_flash.h"
#include "flash.h"

extern uint8_t file_name[FILE_NAME_LENGTH];
uint8_t download_part;

// ͨ������ IAP �� flash ����һ���ļ�
void IAP_download(void)
{
  uint8_t Number[11] = "          ";
  int32_t Size = 0;
  uint32_t partition_start ,partition_size;
  uint32_t timeout=30*1000;
  
  if(download_part==1){
    /* ����1 */
    partition_start=APP1_PARTITION_START_ADDR;
    partition_size=APP1_PARTITION_SIZE;
  }
  else {
    debug_err(ERR"\n\rPlease set partition to 1!\n\r");
    goto RESET;
  }
  
  /* ��������ģʽ */
  printk("The current download partition is app%d,start_address:0x%08x,size:%d Bytes\n\r",
     (long)download_part,(long)partition_start,(long)partition_size);
  printk("Waiting for the file to be sent ... (press 'a' key to exit IAP mode)\n\r");
  
  // ��������
  Size = Ymodem_Receive(partition_start,partition_size,timeout);
  hw_ms_delay(500);
  if (Size > 0)
  {
    printk("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: %s",(char*)file_name);
    Int2Str(Number, Size);
    printk("\n\r Size: %s Bytes\r\n",(char*)Number);
    printk("-------------------\r\n");
    if(download_part==1) sys_parameter.app1_flag=APP_OK;
    write_sys_parameter();
  }
  else if (Size == 0) debug_err("\n\r"ERR"Termination by sender!\n\r");
  else if (Size == -1) debug_err("\n\r"ERR"Programming address error or File too large!\n\r");
  else if (Size == -2) debug_err("\n\r"ERR"Erase flash error!\n\r");
  else if (Size == -3) debug_err("\n\r"ERR"Programming flash error!\n\r");
  else if (Size == -10) debug_err("\n\r"ERR"Error count exceeded.\n\r");
  else if (Size == -11) debug_err("\n\r"ERR"Waiting timeout\n\r");
  else if (Size == -30) debug_war("\n\r"WARNING"Aborted by user.\n\r");
  else debug_err("\n\r"ERR"Failed to receive the file!\n\r");

RESET:
  /* �������̨ģʽ */
  usart0_mode=0;
}

