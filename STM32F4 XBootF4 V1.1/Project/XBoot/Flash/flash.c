/*
***************************************Copyright (c)***************************
**                               	  �人������Ϣ�������޹�˾
**                                     		������ҵ��
**                                        
**
**                                		 
**
**--------------�ļ���Ϣ-------------------------------------------------------
**��   ��   ��: Flash.c
**��   ��   ��: ���
**����޸�����: 2018��6��4��
**��        ��: �ڲ�Flash����
**              
**--------------��ʷ�汾��Ϣ---------------------------------------------------
** ������: ���
** ��  ��: v1.0
** �ա���: 2018��6��4��
** �衡��: ��ʼ����
**
**--------------��ǰ�汾�޶�---------------------------------------------------
** �޸���: 
** �ա���: 
** �衡��: 
**
**-----------------------------------------------------------------------------
******************************************************************************/
#include  "flash.h"

/******************************************************************************
** ��������: _flash_Address_Sector
** ��������: ��ȡָ����ַ��Ӧ��flash����
** ��ڲ���: ��ַ
** �� �� ֵ: ����
**
** ������: ���
** �ա���: 2018��8��29��
**-----------------------------------------------------------------------------
******************************************************************************/
uint16_t _flash_Address_Sector(uint32_t address) {
  uint16_t sector = ADDR_WRONG_SECTOR;
  if(!IS_FLASH_ADDRESS(address)/* || address<FLASH_PARAM_START_ADDR || address>FLASH_PARAM_END_ADDR*/) {
    sector = ADDR_WRONG_SECTOR;
  } else if(address >= ADDR_FLASH_SECTOR_11) {
    sector = FLASH_Sector_11;
  } else if(address >= ADDR_FLASH_SECTOR_10) {
    sector = FLASH_Sector_10;
  } else if(address >= ADDR_FLASH_SECTOR_9) {
    sector = FLASH_Sector_9;
  } else if(address >= ADDR_FLASH_SECTOR_8) {
    sector = FLASH_Sector_8;
  } else if(address >= ADDR_FLASH_SECTOR_7) {
    sector = FLASH_Sector_7;
  } else if(address >= ADDR_FLASH_SECTOR_6) {
    sector = FLASH_Sector_6;
  } else if(address >= ADDR_FLASH_SECTOR_5) {
    sector = FLASH_Sector_5;
  } else if(address >= ADDR_FLASH_SECTOR_4) {
    sector = FLASH_Sector_4;
  } else if(address >= ADDR_FLASH_SECTOR_3) {
    sector = FLASH_Sector_3;
  } else if(address >= ADDR_FLASH_SECTOR_2) {
    sector = FLASH_Sector_2;
  } else if(address >= ADDR_FLASH_SECTOR_1) {
    sector = FLASH_Sector_1;
  } else {
    sector = FLASH_Sector_0;
  }
  return sector;
}


/******************************************************************************
** ��������: _flash_Erase_Page
** ��������: ����ָ�������Flash���˺���û��Unlock Flash��������Unlock Flash֮�����
** ��ڲ���: Ҫ����������ͷβ����ͷ��β��
** �� �� ֵ: ��
**
** ������: ���
** �ա���: 2018��6��4��
**-----------------------------------------------------------------------------
******************************************************************************/
void _flash_Erase_Sectors(uint16_t sector_start, uint16_t sector_end) {
  if(sector_start > sector_end || !(IS_FLASH_SECTOR(sector_start))) return;
  
  for(uint16_t sector=sector_start; sector<=sector_end && IS_FLASH_SECTOR(sector); sector+=8) {
    FLASH_EraseSector(sector, VoltageRange_3);
  }
}

/******************************************************************************
** ��������: flash_write
** ��������: ������д��flash
** ��ڲ���: Ҫд�����ʼ��ַ��Ҫд�������ͷָ�룻Ҫд������ݳ���(�ֽ���)
** �� �� ֵ: д������0--ʧ�ܣ�1--�ɹ�
**
** ������: ���
** �ա���: 2018��6��4��
**-----------------------------------------------------------------------------
******************************************************************************/
uint8_t flash_write(uint32_t address, uint8_t *pdata, uint16_t length) {
  uint16_t sector_start = _flash_Address_Sector(address);
  uint16_t sector_end   = _flash_Address_Sector(address+length);
  if(sector_start == ADDR_WRONG_SECTOR || sector_end == ADDR_WRONG_SECTOR) return 0;
  
  FLASH_Unlock();
  
  _flash_Erase_Sectors(sector_start, sector_end);
  for(uint16_t i = 0; i < length; i++) {
    FLASH_ProgramByte(address + i, *pdata++);
  }
  
  FLASH_Lock();
  return 1;
}

/******************************************************************************
** ��������: flash_read
** ��������: ��ȡflash�е�����
** ��ڲ���: Ҫ��ȡ����ʼ��ַ�������ȡ�������ݵ�ͷָ�룻Ҫ��ȡ�����ݳ���(�ֽ���)
** �� �� ֵ: ��ȡ�����0--ʧ�ܣ�1--�ɹ�
**
** ������: ���
** �ա���: 2018��6��4��
**-----------------------------------------------------------------------------
******************************************************************************/
uint8_t flash_read(uint32_t address, uint8_t *pdata, uint16_t length) {
  if(!IS_FLASH_ADDRESS(address) || !IS_FLASH_ADDRESS(address+length)) return 0;
  
  for(uint16_t i = 0; i < length; i++) {
    *pdata++ = *(__IO uint8_t *) (address + i);
  }
  return 1;
}

/******************************************************************************
** ��������: flash_copy
** ��������: ��flashԴ��ַ�е����ݣ����Ƶ�Ŀ���ַ��
** ��ڲ���: Ҫд���Ŀ���ַ������Դ��ַ��Ҫ���Ƶ����ݳ���(�ֽ���)�����ݵ�ַ�����ݳ��ȱ�����4�ı���
** �� �� ֵ: д������0--ʧ�ܣ�1--�ɹ�
**
** ������: ���
** �ա���: 2021��3��24��
**-----------------------------------------------------------------------------
******************************************************************************/
uint8_t flash_copy(uint32_t dst, uint32_t src, uint32_t length) {
  uint16_t dst_start = _flash_Address_Sector(dst);
  uint16_t dst_end   = _flash_Address_Sector(dst+length);
  if(dst_start==ADDR_WRONG_SECTOR || dst_end==ADDR_WRONG_SECTOR || src%4!=0 || dst%4!=0 || length%4!=0)
    return 0;
  
  FLASH_Unlock();
  _flash_Erase_Sectors(dst_start, dst_end);
  for(uint32_t i = 0; i < length; i+=4) {
    FLASH_ProgramWord(dst + i, *(uint32_t *)(src + i));
  }
  FLASH_Lock();
  return 1;
}

//����Ӧ����
#include "flash_if.h"
uint8_t flash_erase_app(void) {
  FLASH_Unlock();
  _flash_Erase_Sectors(_flash_Address_Sector(APPLICATION_ADDRESS), _flash_Address_Sector(USER_FLASH_END_ADDRESS));
  FLASH_Lock();
  return 0;
}

//�������������д��
uint8_t flash_copy_nocheck(uint32_t dst, uint32_t src, uint32_t length) {
  uint16_t dst_start = _flash_Address_Sector(dst);
  uint16_t dst_end   = _flash_Address_Sector(dst+length);
  if(dst_start==ADDR_WRONG_SECTOR || dst_end==ADDR_WRONG_SECTOR || src%4!=0 || dst%4!=0 || length%4!=0)
    return 0;
  
  FLASH_Unlock();
  for(uint32_t i = 0; i < length; i+=4) {
    FLASH_ProgramWord(dst + i, *(uint32_t *)(src + i));
  }
  FLASH_Lock();
  return 1;
}
