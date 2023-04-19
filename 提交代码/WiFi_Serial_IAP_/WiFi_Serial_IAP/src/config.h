#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash操作接口文件（在库文件中），必须要包含  

#define BOOT_IAP    0xA2
#define BOOT_APP    0x3C

#define APP_OK      0x4D
#define APP_ERR     0xFF

#endif