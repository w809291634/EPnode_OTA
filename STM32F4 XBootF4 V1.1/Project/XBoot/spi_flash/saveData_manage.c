#include "saveData_manage.h"
#include "w25qxx.h"
#include "flash_if.h"
#include "flash.h"
#include "printk.h"
#include <string.h>

#define FLASH_INFO_ADDR     0*W25QXX_SEC_SIZE
#define FLASH_ERASE_LIMIT   100000

static uint16_t saveData_Sec;   //��ǰ�洢Ӧ�����ݵ��������(0-W25QXX_SEC_MAX)
static saveData_t app_Data;     //Ӧ������
static __align(4) uint8_t code_buf[W25QXX_SEC_SIZE];
void rec_Code_Update(void) {
  printk("\n");
  for(uint32_t offset = 0; offset < USER_FLASH_SIZE; offset += W25QXX_SEC_SIZE) {
    memcpy(code_buf, (void *)(APPLICATION_ADDRESS+offset), W25QXX_SEC_SIZE);
    //����16���ֽ�Ϊ0xFFʱ����Ϊ������ѽ���
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
    //����16���ֽ�Ϊ0xFFʱ����Ϊ������ѽ���
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


//����洢���ݣ��ȳ�ʼ��SPIFlash��Ȼ���ȡ���ؾ��������������ǰ���������±��渺�ؾ������
void saveData_Clear(void) {
  //��ʼ������ȡ��ǰ�洢������Ϣ
  W25QXX_Read((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);//��ȡ��ǰ�洢Ӧ�����ݵ��������
  if(saveData_Sec > W25QXX_SEC_MAX) {   //�Ƿ��������--��Ϊ����оƬ
    saveData_Sec = 1;
    W25QXX_Write((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);
    app_Data.sector_num = saveData_Sec;
    app_Data.sector_erase_count = 0;
  } else {  //��ǰ�Ѵ洢��Ӧ������
    W25QXX_Read((uint8_t *)&app_Data, saveData_Sec*W25QXX_SEC_SIZE, sizeof(saveData_t));//��ȡ��ǰ�洢��Ӧ������
    if(app_Data.sector_num > W25QXX_SEC_MAX) {   //�Ƿ��������--��Ϊ����оƬ
      app_Data.sector_num = saveData_Sec;
      app_Data.sector_erase_count = 0;
    } else {
      if(app_Data.sector_num-saveData_Sec == 1) {//���´洢���ݡ��л�����ʱ������Ӧ�����ݺ������⣬δ�ܸ�����������
        saveData_Sec = app_Data.sector_num;
        W25QXX_Write((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);
      }
    }
  }
  //�����ָ���������
  app_Data.sector_erase_count += 1;
  if(app_Data.sector_erase_count == FLASH_ERASE_LIMIT) {//��ǰ�����ѱ���
    if(saveData_Sec+1 > W25QXX_SEC_MAX) { //����������������ȫ���ﵽ10��Σ������ѱ���
      return;
    }
    //������������
    saveData_Sec += 1;
    W25QXX_Write((uint8_t *)&saveData_Sec, FLASH_INFO_ADDR, 2);
    //�޸ĸ��ؾ�������
    app_Data.sector_num = saveData_Sec;
    app_Data.sector_erase_count = 1;
  }
  //�������
  W25QXX_Erase_Sector(saveData_Sec);
  //���ؾ�������д��
  W25QXX_Write_NoCheck((uint8_t *)&app_Data, saveData_Sec*W25QXX_SEC_SIZE, sizeof(saveData_t));
}
