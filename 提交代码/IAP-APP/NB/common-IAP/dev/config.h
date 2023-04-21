#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "stm32f10x.h"  
#include "stm32f10x_flash.h"                     //flash�����ӿ��ļ����ڿ��ļ��У�������Ҫ����  

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

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //��������  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����


int config_init(void);

void config_save(void);
#endif