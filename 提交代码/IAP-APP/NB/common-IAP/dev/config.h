#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash操作接口文件（在库文件中），必须要包含  

#define PARAMETER_ADDRESS     (uint32_t)0x0800FE00
#define APPLICATION_ADDRESS   (uint32_t)0x08004000

#define BOOT_IAP    0xA2
#define BOOT_APP    0x3C

#define APP_OK      0x4D
#define APP_ERR     0xFF

typedef struct {
  uint16_t boot_mode;
  uint16_t app_en;
  uint32_t flag;
  char id[40]; //ID
  char key[120]; //KEY
  char ip[64]; //
  unsigned char nbandFlag;
  int port;
  int mode;
}nb_config_t;

extern nb_config_t nbConfig;

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//指定地址开始写入指定长度的数据
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//指定地址开始读取指定长度数据
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据


int config_init(void);

void config_save(void);
#endif