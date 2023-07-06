

#include "hal_board_cfg.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"

#if !defined MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
#define MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA  FALSE
#endif

static uint8 sbBuf[SB_BUF_SIZE], sbCmd1, sbCmd2, sbFcs, sbIdx, sbLen, sbSte;
  

static uint8 sbCmnd(void);
static void sbResp(uint8 rsp, uint8 len);
//static uint16 calcCRC(void);
//static uint16 runPoly(uint16 crc, uint8 val);

uint8 sbExec(void)
{
  uint8 ch, rtrn = FALSE;

  while (SB_RX(&ch))
  {
    sbBuf[sbSte + sbIdx] = ch;
    switch (sbSte)
    {
    case SB_SOF_STATE:
      if (SB_SOF == ch)
      {
        sbSte = SB_LEN_STATE;
      }
      break;
    
    case SB_LEN_STATE:
      sbFcs = 0;
      sbSte = ((sbLen = ch) >= SB_BUF_SIZE) ? SB_SOF_STATE : SB_CMD1_STATE;
      break;

    case SB_CMD1_STATE:
      sbCmd1 = ch;
      sbSte = SB_CMD2_STATE;
      break;
    
    case SB_CMD2_STATE:
      sbCmd2 = ch;
      sbSte = (sbLen) ? SB_DATA_STATE : SB_FCS_STATE;
      break;

    case SB_DATA_STATE:
      if (++sbIdx == sbLen)
      {
        sbSte = SB_FCS_STATE;
      }
      break;
    
    case SB_FCS_STATE:
      if ((sbFcs == ch) && (sbCmd1 == SB_RPC_SYS_BOOT))
      {
        rtrn = sbCmnd();
      }
      else
      {
        // TODO - RemoTI did not have here or on bad length - adding could cause > 1 SB_INVALID_FCS
        //        for a single data packet which could put out of sync with PC for awhile or
        //        infinte, depending on PC-side?
        // sbResp(SB_INVALID_FCS, 1);
      }
    
      sbSte = sbIdx = 0;
      break;
    
    default:
      break;
    }
    sbFcs ^= ch;
  }

  return rtrn;
}


//uint8 sbImgValid(void)
//{
//  uint16 crc[2];
//
//
//  HalFlashRead(HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
//               HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
//               (uint8 *)crc, sizeof(crc));
//
//  if ((crc[0] == 0xFFFF) || (crc[0] == 0x0000))
//  {
//    return FALSE;
//  }
//
//  if (crc[0] != crc[1])
//  {
//    crc[1] = calcCRC();
//    HalFlashWrite((HAL_SB_CRC_ADDR / HAL_FLASH_WORD_SIZE), (uint8 *)crc, 1);
//    HalFlashRead(  HAL_SB_CRC_ADDR / HAL_FLASH_PAGE_SIZE,
//                   HAL_SB_CRC_ADDR % HAL_FLASH_PAGE_SIZE,
//                   (uint8 *)crc, sizeof(crc));
//  }
//
//  return ((crc[0] == crc[1]) && (crc[0] != 0xFFFF) && (crc[0] != 0x0000));
//}

static uint8 sbCmnd(void)
{
  uint16 tmp = BUILD_UINT16(sbBuf[SB_DATA_STATE], sbBuf[SB_DATA_STATE+1]) + SB_IMG_OSET;
  uint16 crc[2];
  uint8 len = 1;
  uint8 rsp = SB_SUCCESS;
  uint8 rtrn = FALSE;

  switch (sbCmd2)
  {
  case SB_HANDSHAKE_CMD:
    break;

  case SB_WRITE_CMD:
    if ((tmp % SB_WPG_SIZE) == 0)
    {
      HalFlashErase(tmp / SB_WPG_SIZE);
    }

    HalFlashWrite(tmp, sbBuf+SB_DATA_STATE+2, SB_RW_BUF_LEN / HAL_FLASH_WORD_SIZE);
    break;

  case SB_READ_CMD:
#if !MT_SYS_OSAL_NV_READ_CERTIFICATE_DATA
    if ((tmp / (HAL_FLASH_PAGE_SIZE / 4)) >= HAL_NV_PAGE_BEG)
    {
      rsp = SB_FAILURE;
      break;
    }
#endif
    HalFlashRead(tmp / (HAL_FLASH_PAGE_SIZE / 4),
                 (tmp % (HAL_FLASH_PAGE_SIZE / 4)) << 2,
                 sbBuf + SB_DATA_STATE + 3, SB_RW_BUF_LEN);
    sbBuf[SB_DATA_STATE+2] = sbBuf[SB_DATA_STATE+1];
    sbBuf[SB_DATA_STATE+1] = sbBuf[SB_DATA_STATE];
    len = SB_RW_BUF_LEN + 3;
    break;
    
  default:
    break;
  }
  
  sbResp(rsp, len);
  return rtrn;
}

static void sbResp(uint8 rsp, uint8 len)
{
  int8 idx;

  sbBuf[SB_CMD2_STATE] |= 0x80;
  sbBuf[SB_DATA_STATE] = rsp;
  sbBuf[SB_LEN_STATE] = len;
  rsp = len ^ SB_RPC_SYS_BOOT;
  len += SB_FCS_STATE-1;

  for (idx = SB_CMD2_STATE; idx < len; idx++)
  {
    rsp ^= sbBuf[idx];
  }
  sbBuf[idx++] = rsp;
  
  SB_TX(sbBuf, idx);
}

//static uint16 calcCRC(void)
//{
//  uint32 addr;
//  uint16 crc = 0;
//
//  // Run the CRC calculation over the active body of code.
//  for (addr = HAL_SB_IMG_ADDR; addr < HAL_SB_IMG_ADDR + HAL_SB_IMG_SIZE; addr++)
//  {
//    if (addr == HAL_SB_CRC_ADDR)
//    {
//      addr += 3;
//    }
//    else
//    {
//      uint8 buf;
//      HalFlashRead(addr / HAL_FLASH_PAGE_SIZE, addr % HAL_FLASH_PAGE_SIZE, &buf, 1);
//      crc = runPoly(crc, buf);
//    }
//  }
//
//  // IAR note explains that poly must be run with value zero for each byte of crc.
//  crc = runPoly(crc, 0);
//  crc = runPoly(crc, 0);
//
//  return crc;
//}

//static uint16 runPoly(uint16 crc, uint8 val)
//{
//  const uint16 poly = 0x1021;
//  uint8 cnt;
//
//  for (cnt = 0; cnt < 8; cnt++, val <<= 1)
//  {
//    uint8 msb = (crc & 0x8000) ? 1 : 0;
//
//    crc <<= 1;
//    if (val & 0x80)  crc |= 0x0001;
//    if (msb)         crc ^= poly;
//  }
//
//  return crc;
//}

/**************************************************************************************************
*/
