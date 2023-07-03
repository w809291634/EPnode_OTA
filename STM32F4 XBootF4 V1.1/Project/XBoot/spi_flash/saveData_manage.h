#ifndef __SAVE_DATA_CONFIG_H_
#define __SAVE_DATA_CONFIG_H_

#include <stdint.h>

typedef struct {
  unsigned sector_num;          //��ǰ�������
  unsigned sector_erase_count;  //��ǰ������������--��100K��ʱ�л�����һ������
} saveData_t;

void saveData_Clear(void);
void rec_Code_Update(void);
int8_t rec_Code_Recovery(void);

#endif
