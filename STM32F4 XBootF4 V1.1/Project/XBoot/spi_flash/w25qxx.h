/*********************************************************************************************
* 文件: w25qxx.c
* 作者：zonesion 2016.12.22
* 说明：外部flash驱动头文件
* 修改：
* 注释：
*********************************************************************************************/
/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "stm32f4xx.h"
/*********************************************************************************************
* 宏定义
*********************************************************************************************/
#ifndef __W25QXX_H
#define __W25QXX_H


//W25X系列/Q系列芯片列表
//W25Q80  ID  0XEF13
//W25Q16  ID  0XEF14
//W25Q32  ID  0XEF15
//W25Q64  ID  0XEF16
//W25Q128 ID  0XEF17
#define W25Q80 	0XEF13
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17

extern u16 W25QXX_TYPE;					//定义W25QXX芯片型号

#define W25QXX_CS_RCC       RCC_AHB1Periph_GPIOA
#define W25QXX_CS_GPIO      GPIOA
#define W25QXX_CS_PIN       GPIO_Pin_15

#define W25QXX_CS_L         (W25QXX_CS_GPIO->BSRRH=W25QXX_CS_PIN)
#define W25QXX_CS_H         (W25QXX_CS_GPIO->BSRRL=W25QXX_CS_PIN)

#define W25QXX_DEV_SEC_NUM  2048        /* 设备扇区数量 */
#define W25QXX_SEC_SIZE     0x1000      /* 扇区大小 */

#define W25QXX_REC_SIZE     0x100000    /* 恢复区大小 */
#define W25QXX_REC_SEC_NUM  (W25QXX_REC_SIZE/W25QXX_SEC_SIZE)   /* 恢复区扇区数量 */
#define W25QXX_REC_SEC_START  (W25QXX_DEV_SEC_NUM-W25QXX_REC_SEC_NUM)  /* 最大可用扇区号：留1M空间给恢复区 */
#define W25QXX_REC_ADDR     (W25QXX_REC_SEC_START*W25QXX_SEC_SIZE)

#define	W25QXX_SEC_MAX      (W25QXX_DEV_SEC_NUM-W25QXX_REC_SEC_NUM-1)    /* 最大可用扇区号：留1M空间给恢复区 */
////////////////////////////////////////////////////////////////////////////

//指令表
#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown   0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID   0x90
#define W25X_JedecDeviceID		0x9F


void W25QXX_Init(void);
u16  W25QXX_ReadID(void);  	    		                //读取FLASH ID
u8   W25QXX_ReadSR(void);        		                //读取状态寄存器
void W25QXX_Write_SR(u8 sr);  			                //写状态寄存器
void W25QXX_Write_Enable(void);  		                //写使能
void W25QXX_Write_Disable(void);		                //写保护
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //读取flash
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u32 NumByteToWrite);//写入flash
void W25QXX_Erase_Chip(void);    	  	                //整片擦除
void W25QXX_Erase_Sector(u32 Dst_Addr);	                        //扇区擦除
void W25QXX_Wait_Busy(void);           	                //等待空闲
void W25QXX_PowerDown(void);        	                        //进入掉电模式
void W25QXX_WAKEUP(void);				        //唤醒
#endif
