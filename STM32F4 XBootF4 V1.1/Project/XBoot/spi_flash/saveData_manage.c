#include "saveData_manage.h"
#include "w25qxx.h"
#include "flash_if.h"
#include "flash.h"
#include "printk.h"
#include <string.h>

#define FLASH_INFO_ADDR     0*W25QXX_SEC_SIZE
#define FLASH_ERASE_LIMIT   100000

static uint16_t saveData_Sec;   //当前存储应用数据的扇区编号(0-W25QXX_SEC_MAX)
static saveData_t app_Data;     //应用数据
static __align(4) uint8_t code_buf[W25QXX_SEC_SIZE];
void rec_Code_Update(void) {
  printk("\n");
  for(uint32_t offset = 0; offset < USER_FLASH_SIZE; offset += W25QXX_SEC_SIZE) {
    memcpy(code_buf, (void *)(APPLICATION_ADDRESS+offset), W25QXX_SEC_SIZE);
    //连续16个字节为0xFF时，认为代码段已结束
    if(*(uint32_t *)&code_buf[0] == 0xFFFFFFFF && *(uint32_t *)&code_buf[4] == 0xFFFFFFFF &&\
       *(uint32_t *)&code_buf[8] == 0xFFFFFFFF && *(uint32_t *)&code_buf[12]== 0xFFFFFFFF) {
      break;
    }
    printk("Code copying to REC region: %d%%\r", 100*(offset/W25QXX_SEC_SIZE)/(USER_FLASH_SIZE/W25QXX_SEC_SIZE));
    W25QXX_Erase_Sector((W25QXX_REC_ADDR+offset)/W25QXX_SEC_SIZE);
    W25QXX_Write_NoCheck(code_buf, W25QXX_REC_ADDR+offset, W25QXX_SEC_SIZE);
  }
  printk("Code copying to REC region: 100%%\n");
}

int8_t rec_Code_Recovery(void) {
  flash_erase_app();
  for(uint32_t offset = 0; offset < USER_FLASH_SIZE; offset += W25QXX_SEC_SIZE) {
    W25QXX_Read(code_buf, W25QXX_REC_ADDR+offset, W25QXX_SEC_SIZE);
    //连续16个字节为0xFF时，认为代码段已结束
    if(*(uint32_t *)&code_buf[0] == 0xFFFFFFFF && *(uint32_t *)&code_buf[4] == 0xFFFFFFFF &&\
       *(uint32_t *)&code_buf[8] == 0xFFFFFFFF && *(uint32_t *)&code_buf[12]== 0xFFFFFFFF) {
      break;
    }
    if(flash_copy_nocheck(APPLICATION_ADDRESS+offset, (uint32_t)&code_buf, W25QXX_SEC_SIZE) == 0) {
      return -1;
    }
  }
  return 0;
}


//清除存储数据：先初始化SPIFlash，然后读取负载均衡参数，擦除当前扇区后，重新保存负载均衡参数
void saveData_Clear(void) {
  //初始化，获取当前存储扇区信息
  W25QXX_Read((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);//读取当前存储应用数据的扇区编号
  if(saveData_Sec > W25QXX_SEC_MAX) {   //非法扇区编号--认为是新芯片
    saveData_Sec = 1;
    W25QXX_Write((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);
    app_Data.sector_num = saveData_Sec;
    app_Data.sector_erase_count = 0;
  } else {  //此前已存储过应用数据
    W25QXX_Read((uint8_t *)&app_Data, saveData_Sec*W25QXX_SEC_SIZE, sizeof(saveData_t));//读取此前存储的应用数据
    if(app_Data.sector_num > W25QXX_SEC_MAX) {   //非法扇区编号--认为是新芯片
      app_Data.sector_num = saveData_Sec;
      app_Data.sector_erase_count = 0;
    } else {
      if(app_Data.sector_num-saveData_Sec == 1) {//更新存储数据、切换扇区时，更新应用数据后发生意外，未能更新扇区索引
        saveData_Sec = app_Data.sector_num;
        W25QXX_Write((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);
      }
    }
  }
  //参数恢复出厂设置
  app_Data.sector_erase_count += 1;
  if(app_Data.sector_erase_count == FLASH_ERASE_LIMIT) {//当前扇区已报废
    if(saveData_Sec+1 > W25QXX_SEC_MAX) { //所有扇区擦除次数全部达到10万次，磁盘已报废
      return;
    }
    //更新扇区索引
    saveData_Sec += 1;
    W25QXX_Write((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);
    //修改负载均衡数据
    app_Data.sector_num = saveData_Sec;
    app_Data.sector_erase_count = 1;
  }
  //数据清除
  W25QXX_Erase_Sector(saveData_Sec);
  //负载均衡数据写入
  W25QXX_Write_NoCheck((uint8_t *)&app_Data, saveData_Sec*W25QXX_SEC_SIZE, sizeof(saveData_t));
}
