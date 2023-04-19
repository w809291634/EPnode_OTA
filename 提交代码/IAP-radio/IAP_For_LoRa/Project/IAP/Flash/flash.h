/*
***************************************Copyright (c)***************************
**                               	  武汉钛联信息技术有限公司
**                                     		工程事业部
**                                        
**
**                                		 
**
**--------------文件信息-------------------------------------------------------
**文   件   名: Flash.h
**创   建   人: 杨开琦
**最后修改日期: 2018年6月4日
**描        述: 内部Flash操作
**              
**--------------历史版本信息---------------------------------------------------
** 创建人: 杨开琦
** 版  本: v1.0
** 日　期: 2018年6月4日
** 描　述: 初始创建
**
**--------------当前版本修订---------------------------------------------------
** 修改人: 
** 日　期: 
** 描　述: 
**
**-----------------------------------------------------------------------------
******************************************************************************/
#ifndef _FLASH_H_
#define _FLASH_H_

#include "main.h"

#define FLASH_PARAM_START_ADDR  ADDR_FLASH_SECTOR_3
/******************************************************************************
** 函数名称: flash_write
** 功能描述: 将数据写入flash
** 入口参数: 要写入的起始地址；要写入的数据头指针；要写入的数据长度(字节数)
** 返 回 值: 写入结果：0--失败；1--成功
**
** 作　者: 杨开琦
** 日　期: 2018年6月4日
**-----------------------------------------------------------------------------
******************************************************************************/
uint8_t flash_write(uint32_t address, uint8_t *pdata, uint16_t length);

/******************************************************************************
** 函数名称: flash_read
** 功能描述: 读取flash中的数据
** 入口参数: 要读取的起始地址；保存读取出的数据的头指针；要读取的数据长度(字节数)
** 返 回 值: 读取结果：0--失败；1--成功
**
** 作　者: 杨开琦
** 日　期: 2018年6月4日
**-----------------------------------------------------------------------------
******************************************************************************/
uint8_t flash_read(uint32_t address, uint8_t *pdata, uint16_t length);
#endif //_FLASH_H_
