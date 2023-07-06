/*********************************************************************************************
* 文件：oled_iic.h
* 作者：zonesion
* 说明：oled_iic头文件
* 修改：
* 注释：
*********************************************************************************************/
#ifndef _IIC_H_
#define _IIC_H_

/*********************************************************************************************
* 外部原型函数
*********************************************************************************************/
void iic_init(void);
void iic_start(void);
void iic_stop(void);

unsigned char iic_write_byte(unsigned char data);
unsigned char iic_read_byte(unsigned char ack);

int iic_write_buf(char addr, char r, char *buf, int len);
int iic_read_buf(char addr, char r, char *buf, int len);

#endif 