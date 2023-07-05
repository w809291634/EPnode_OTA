#include "flash.h"
#include "iap_config.h"
#include "shell.h"
#include "hal_flash.h"

// addr д���ַ ��ʵ�ʵ�ַ 0-0x40000
// buf: ���ݻ����� �ֻ���������С�� NumToWrite*4
// NumToWrite: ���ٸ���
// ����: д����ٸ���
// ע�⣺д��ʱ������������������ɾ����һ�Ρ�
// ������Ҫʹ�� HalDmaInit(); ��ʼ��
uint16 FlashWrite(uint32 WriteAddr, uint8 *buf, uint16 NumToWrite)
{
  uint8 spage=0xff;
  uint8 epage=0xff;
	uint32 end_addr=WriteAddr+NumToWrite*4-1;        // д��Ľ�����ַ
  /* ��ַ��� */
  if( end_addr > CC2530_FLASH_END || WriteAddr % 4 ){
    debug_err(ERR"FlashWrite Address error! addrx:0x%08x\r\n",WriteAddr);
    return 0;
  }
  
  /* ����flash */
  spage = (WriteAddr >> 11) & 0xFFFFF;    // ����2048�õ�ҳ��
  epage = (end_addr >> 11) & 0xFFFFF;     // ����2048�õ�ҳ��
  if(epage > HAL_FLASH_MAX_PAGE){
    debug_err(ERR"Get_Page error! epage:%d\r\n",epage);
    return 0;
  }
  while( spage<=epage ){
    HalFlashErase(spage);
    spage++;
  }
  
  /* дflash */
  uint16 quad_addr = WriteAddr/4;
  HalFlashWrite(quad_addr, buf , NumToWrite);
  return NumToWrite;
}

// addr ����ַ ��ʵ�ʵ�ַ 0-0x40000
// buf: ���ݻ����� ÿ��Ԫ���ֽ�
// NumToRead: ���ٸ��ֽ���
// ����: �����ֽ���
uint16 FlashRead(uint32 address, uint8 *buffer, uint16 NumToRead)
{
  uint8_t* currentBuffer = buffer;
  uint32_t remainingBytes = NumToRead;
  uint32_t currentPage = (address >> 11) & 0xFFFFF;   // ����2048�õ�ҳ��
  uint32_t offset = address & 0x7FF;                  // ȡ��11λ��Ϊƫ����

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

// Ҫд�뵽STM32 FLASH���ַ�������
const uint8 TEXT_Buffer[]={"This a FLASH TEST Program"};
#define TEXT_LENTH  sizeof(TEXT_Buffer)	 		  	          // ���鳤��	
#define SIZE        (TEXT_LENTH/4+((TEXT_LENTH%4)?1:0))     // ���ٸ���
// flash ���Գ���
// start_add: ��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!) 
// end_add��������ַ
// Flash_test(0x10000,CC2530_FLASH_END); ���Խ��в���
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

// дϵͳ����
void write_sys_parameter()
{
  if(SYS_PARAMETER_SIZE<=PARA_PARTITION_SIZE && SYS_PARAMETER_SIZE==SYS_PARAMETER_WRITE){
    debug_info(INFO"System Parameter Write Success!\r\n");
  }else{
    debug_err(ERR"System Parameter Write Failed!",SYS_PARAMETER_SIZE);
  }
}
