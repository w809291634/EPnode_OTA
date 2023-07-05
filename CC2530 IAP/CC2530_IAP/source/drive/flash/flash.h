#ifndef __FLASH_H__
#define __FLASH_H__
#include "sys.h"
#include "hal_board_cfg.h"

void Flash_test(uint32 start_add,uint32 end_add);
uint16 FlashWrite(uint32 WriteAddr, uint8 *buf, uint16 NumToWrite);
uint16 FlashRead(uint32 address, uint8 *buffer, uint16 NumToRead);
void write_sys_parameter();
#endif
