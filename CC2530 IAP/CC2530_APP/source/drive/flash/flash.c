#include "flash.h"
#include "iap_config.h"
#include "shell.h"
#include "hal_flash.h"

// addr 写入地址 ：实际地址 0-0x40000
// buf: 数据缓存区 字缓存区，大小是 NumToWrite*4
// NumToWrite: 多少个字
// 返回: 写入多少个字
// 注意：写的时候会把所在扇区的数据删除掉一次。
// 首先需要使用 HalDmaInit(); 初始化
uint16 FlashWrite(uint32 WriteAddr, uint8 *buf, uint16 NumToWrite)
{
  uint8 spage=0xff;
  uint8 epage=0xff;
	uint32 end_addr=WriteAddr+NumToWrite*4-1;        // 写入的结束地址
  /* 地址检查 */
  if( end_addr > CC2530_FLASH_END || WriteAddr % 4 ){
    debug_err(ERR"FlashWrite Address error! addrx:0x%08x\r\n",WriteAddr);
    return 0;
  }
  
  /* 擦除flash */
  spage = (WriteAddr >> 11) & 0xFFFFF;    // 除以2048得到页码
  epage = (end_addr >> 11) & 0xFFFFF;     // 除以2048得到页码
  if(epage > HAL_FLASH_MAX_PAGE){
    debug_err(ERR"Get_Page error! epage:%d\r\n",epage);
    return 0;
  }
  while( spage<=epage ){
    HalFlashErase(spage);
    spage++;
  }
  
  /* 写flash */
  uint16 quad_addr = WriteAddr/4;
  HalFlashWrite(quad_addr, buf , NumToWrite);
  return NumToWrite;
}

// addr 读地址 ：实际地址 0-0x40000
// buf: 数据缓存区 每个元素字节
// NumToRead: 多少个字节数
// 返回: 读出字节数
uint16 FlashRead(uint32 address, uint8 *buffer, uint16 NumToRead)
{
  uint8_t* currentBuffer = buffer;
  uint32_t remainingBytes = NumToRead;
  uint32_t currentPage = (address >> 11) & 0xFFFFF;   // 除以2048得到页码
  uint32_t offset = address & 0x7FF;                  // 取低11位作为偏移量

  while (remainingBytes > 0 && currentPage < HAL_FLASH_MAX_PAGE)
  {
    uint16_t bytesToRead = (remainingBytes > HAL_FLASH_PAGE_SIZE - offset) ? (HAL_FLASH_PAGE_SIZE - offset) : remainingBytes;

    HalFlashRead(currentPage, offset, currentBuffer, bytesToRead);

    remainingBytes -= bytesToRead;
    currentBuffer += bytesToRead;
    offset = 0;
    currentPage++;
  }
  return NumToRead;
}

// 要写入到STM32 FLASH的字符串数组
const uint8 TEXT_Buffer[]={"This a FLASH TEST Program"};
#define TEXT_LENTH  sizeof(TEXT_Buffer)	 		  	          // 数组长度	
#define SIZE        (TEXT_LENTH/4+((TEXT_LENTH%4)?1:0))     // 多少个字
// flash 测试程序
// start_add: 起始地址(此地址必须为4的倍数!!) 
// end_add：结束地址
// Flash_test(0x10000,CC2530_FLASH_END); 可以进行测试
void Flash_test(uint32 start_add,uint32 end_add)
{
  static char count=0;
  char datatemp[TEXT_LENTH+1]={0};
  while(count==0  && (start_add+SIZE*4-1)<=CC2530_FLASH_END && count==0){
    memset(datatemp,0,TEXT_LENTH+1);
    long write_len=FlashWrite(start_add,(uint8*)TEXT_Buffer,SIZE);
    // hw_ms_delay(100);
    FlashRead(start_add,(uint8*)datatemp,SIZE*4);
    if(memcmp(datatemp,TEXT_Buffer,TEXT_LENTH)==0){
      debug_info(INFO"FlashRead Success,start_add:0x%08x writelen:%d str:%s\r\n",start_add,write_len,datatemp);
    }
    else{
      debug_err(ERR"Read_test error! str:%s\r\n",datatemp);
      count=1;
    } 
    start_add+=SIZE*4;
  }
  count=1;
}

// 写系统参数
void write_sys_parameter()
{
  if(SYS_PARAMETER_SIZE<=PARA_PARTITION_SIZE && SYS_PARAMETER_SIZE==SYS_PARAMETER_WRITE){
    debug_info(INFO"System Parameter Write Success!\r\n");
  }else{
    debug_err(ERR"System Parameter Write Failed!",SYS_PARAMETER_SIZE);
  }
}
