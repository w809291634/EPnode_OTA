#ifndef __SAVE_DATA_CONFIG_H_
#define __SAVE_DATA_CONFIG_H_

#include <stdint.h>

typedef struct {
  unsigned sector_num;          //当前扇区编号
  unsigned sector_erase_count;  //当前扇区擦除次数--到100K次时切换到下一个扇区
} saveData_t;

void saveData_Clear(void);
void rec_Code_Update(void);
int8_t rec_Code_Recovery(void);

#endif
