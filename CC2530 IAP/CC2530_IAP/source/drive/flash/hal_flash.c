/**************************************************************************************************
  Filename:       hal_flash.c
  Revised:        $Date: 2010-10-07 02:19:52 -0700 (Thu, 07 Oct 2010) $
  Revision:       $Revision: 24049 $

  Description: This file contains the interface to the H/W Flash driver.


  Copyright 2006-2010 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "hal_board_cfg.h"
#include "hal_dma.h"
#include "hal_flash.h"
#include "hal_mcu.h"
#include "hal_types.h"
#include "iap_config.h"
#include "shell.h"
#include "sys.h"

/**************************************************************************************************
 * @fn          HalFlashRead
 *
 * @brief       This function reads 'cnt' bytes from the internal flash.
 *
 * input parameters
 *
 * @param       pg - A valid flash page number.
 * @param       offset - A valid offset into the page.
 * @param       buf - A valid buffer space at least as big as the 'cnt' parameter.
 * @param       cnt - A valid number of bytes to read.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashRead(uint8 pg, uint16 offset, uint8 *buf, uint16 cnt)
{
  // Calculate the offset into the containing flash bank as it gets mapped into XDATA.
  uint8 *pData = (uint8 *)(offset + HAL_FLASH_PAGE_MAP) +
                 ((pg % HAL_FLASH_PAGE_PER_BANK) * HAL_FLASH_PAGE_SIZE);
  uint8 memctr = MEMCTR;  // Save to restore.

#if (!defined HAL_OAD_BOOT_CODE) && (!defined HAL_OTA_BOOT_CODE)
  halIntState_t is;
#endif

  pg /= HAL_FLASH_PAGE_PER_BANK;  // Calculate the flash bank from the flash page.

#if (!defined HAL_OAD_BOOT_CODE) && (!defined HAL_OTA_BOOT_CODE)
  HAL_ENTER_CRITICAL_SECTION(is);
#endif

  // Calculate and map the containing flash bank into XDATA.
  MEMCTR = (MEMCTR & 0xF8) | pg;

  while (cnt--)
  {
    *buf++ = *pData++;
  }

  MEMCTR = memctr;

#if (!defined HAL_OAD_BOOT_CODE) && (!defined HAL_OTA_BOOT_CODE)
  HAL_EXIT_CRITICAL_SECTION(is);
#endif
}

/**************************************************************************************************
 * @fn          HalFlashWrite
 *
 * @brief       This function writes 'cnt' bytes to the internal flash.
 *
 * input parameters
 *
 * @param       addr - Valid HAL flash write address: actual addr / 4 and quad-aligned.
 * @param       buf - Valid buffer space at least as big as 'cnt' X 4.
 * @param       cnt - Number of 4-byte blocks to write.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashWrite(uint16 addr, uint8 *buf, uint16 cnt)
{
#if (defined HAL_DMA) && (HAL_DMA == TRUE)
  halDMADesc_t *ch = HAL_NV_DMA_GET_DESC();

  HAL_DMA_SET_SOURCE(ch, buf);
  HAL_DMA_SET_DEST(ch, &FWDATA);
  HAL_DMA_SET_VLEN(ch, HAL_DMA_VLEN_USE_LEN);
  HAL_DMA_SET_LEN(ch, (cnt * HAL_FLASH_WORD_SIZE));
  HAL_DMA_SET_WORD_SIZE(ch, HAL_DMA_WORDSIZE_BYTE);
  HAL_DMA_SET_TRIG_MODE(ch, HAL_DMA_TMODE_SINGLE);
  HAL_DMA_SET_TRIG_SRC(ch, HAL_DMA_TRIG_FLASH);
  HAL_DMA_SET_SRC_INC(ch, HAL_DMA_SRCINC_1);
  HAL_DMA_SET_DST_INC(ch, HAL_DMA_DSTINC_0);
  // The DMA is to be polled and shall not issue an IRQ upon completion.
  HAL_DMA_SET_IRQ(ch, HAL_DMA_IRQMASK_DISABLE);
  HAL_DMA_SET_M8( ch, HAL_DMA_M8_USE_8_BITS);
  HAL_DMA_SET_PRIORITY(ch, HAL_DMA_PRI_HIGH);
  HAL_DMA_CLEAR_IRQ(HAL_NV_DMA_CH);
  HAL_DMA_ARM_CH(HAL_NV_DMA_CH);

  FADDRL = (uint8)addr;
  FADDRH = (uint8)(addr >> 8);
  FCTL |= 0x02;         // Trigger the DMA writes.
  while (FCTL & 0x80);  // Wait until writing is done.
#endif
}

/**************************************************************************************************
 * @fn          HalFlashErase
 *
 * @brief       This function erases the specified page of the internal flash.
 *
 * input parameters
 *
 * @param       pg - A valid flash page number to erase. 0-127
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashErase(uint8 pg)
{
  FADDRH = pg * (HAL_FLASH_PAGE_SIZE / HAL_FLASH_WORD_SIZE / 256);
  FCTL |= 0x01;
}

/**************************************************************************************************
*/

//
// addr Ğ´ÈëµØÖ· £ºÊµ¼ÊµØÖ· 0-0x40000
// buf: Êı¾İ»º´æÇø ×Ö»º´æÇø£¬´óĞ¡ÊÇ NumToWrite*4
// NumToWrite: ¶àÉÙ¸ö×Ö
// ·µ»Ø: Ğ´Èë¶àÉÙ¸ö×Ö
// ×¢Òâ£ºĞ´µÄÊ±ºò»á°ÑËùÔÚÉÈÇøµÄÊı¾İÉ¾³ıµôÒ»´Î¡£
// Ê×ÏÈĞèÒªÊ¹ÓÃ HalDmaInit(); ³õÊ¼»¯
uint16 FlashWrite(uint32 WriteAddr, uint8 *buf, uint16 NumToWrite)
{
  uint8 spage=0xff;
  uint8 epage=0xff;
	uint32 end_addr=WriteAddr+NumToWrite*4-1;        // Ğ´ÈëµÄ½áÊøµØÖ·
  /* µØÖ·¼ì²é */
  if( end_addr > CC2530_FLASH_END || WriteAddr % 4 ){
    debug_err(ERR"FlashWrite Address error! addrx:0x%08x\r\n",WriteAddr);
    return 0;
  }
  
  /* ²Á³ıflash */
  spage = (WriteAddr >> 11) & 0xFFFFF;    // ³ıÒÔ2048µÃµ½Ò³Âë
  epage = (end_addr >> 11) & 0xFFFFF;     // ³ıÒÔ2048µÃµ½Ò³Âë
  if(epage > HAL_FLASH_MAX_PAGE){
    debug_err(ERR"Get_Page error! epage:%d\r\n",epage);
    return 0;
  }
  while( spage<=epage ){
    HalFlashErase(spage);
    spage++;
  }
  
  /* Ğ´flash */
  uint16 quad_addr = WriteAddr/4;
  HalFlashWrite(quad_addr, buf , NumToWrite);
  return NumToWrite;
}

// addr ¶ÁµØÖ· £ºÊµ¼ÊµØÖ· 0-0x40000
// buf: Êı¾İ»º´æÇø Ã¿¸öÔªËØ×Ö½Ú
// NumToRead: ¶àÉÙ¸ö×Ö½ÚÊı
// ·µ»Ø: ¶Á³ö×Ö½ÚÊı
uint16 FlashRead(uint32 address, uint8 *buffer, uint16 NumToRead)
{
  uint8_t* currentBuffer = buffer;
  uint32_t remainingBytes = NumToRead;
  uint32_t currentPage = (address >> 11) & 0xFFFFF;   // ³ıÒÔ2048µÃµ½Ò³Âë
  uint32_t offset = address & 0x7FF;                  // È¡µÍ11Î»×÷ÎªÆ«ÒÆÁ¿

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

// ÒªĞ´Èëµ½STM32 FLASHµÄ×Ö·û´®Êı×é
const uint8 TEXT_Buffer[]={"This a FLASH TEST Program"};
#define TEXT_LENTH  sizeof(TEXT_Buffer)	 		  	          // Êı×é³¤¶È	
#define SIZE        (TEXT_LENTH/4+((TEXT_LENTH%4)?1:0))     // ¶àÉÙ¸ö×Ö
// flash ²âÊÔ³ÌĞò
// start_add: ÆğÊ¼µØÖ·(´ËµØÖ·±ØĞëÎª4µÄ±¶Êı!!) 
// end_add£º½áÊøµØÖ·
// HalFlash_test(0x10000,CC2530_FLASH_END); ¿ÉÒÔ½øĞĞ²âÊÔ
void HalFlash_test(uint32 start_add,uint32 end_add)
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

// Ğ´ÏµÍ³²ÎÊı
void write_sys_parameter()
{
  if(SYS_PARAMETER_SIZE<=PARA_PARTITION_SIZE && SYS_PARAMETER_SIZE==SYS_PARAMETER_WRITE){
    debug_info(INFO"System Parameter Write Success!\r\n");
  }else{
    debug_err(ERR"System Parameter Write Failed!",SYS_PARAMETER_SIZE);
  }
}
