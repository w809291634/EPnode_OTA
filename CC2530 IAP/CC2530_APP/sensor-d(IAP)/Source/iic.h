/*********************************************************************************************
* �ļ���oled_iic.h
* ���ߣ�zonesion
* ˵����oled_iicͷ�ļ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#ifndef _IIC_H_
#define _IIC_H_

/*********************************************************************************************
* �ⲿԭ�ͺ���
*********************************************************************************************/
void iic_init(void);
void iic_start(void);
void iic_stop(void);

unsigned char iic_write_byte(unsigned char data);
unsigned char iic_read_byte(unsigned char ack);

int iic_write_buf(char addr, char r, char *buf, int len);
int iic_read_buf(char addr, char r, char *buf, int len);

#endif 