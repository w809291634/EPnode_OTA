#ifndef __BOARD_H_
#define __BOARD_H_
#include "stm32f10x.h"
#include <rtthread.h>

#define USER_APP_BEGIN  (uint32_t)0x08004000
void rt_hw_udelay(rt_uint32_t us);


#endif
