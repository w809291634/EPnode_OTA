/*********************************************************************************************
* �ļ���iic.h
* ���ߣ�zonesion
* ˵����iicͷ�ļ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#ifndef _IIC_OLED_H_
#define _IIC_OLED_H_
#include "config.h"

/*********************************************************************************************
* �궨��
*********************************************************************************************/
#if RTOS_SUPPORT == 1
#include <rtthread.h>
#include "stm32f10x.h"
#include "board.h"
// PB1 SCL PB0 SDA
#define     SDA_NUM               0
#define     I2C_CLK               RCC_APB2Periph_GPIOB
#define     PIN_SCL               GPIO_Pin_1
#define     PIN_SDA               GPIO_Pin_0
#define     I2C_GPIO              GPIOB

#define     SCL(x)	              {if(x)GPIO_WriteBit(I2C_GPIO,PIN_SCL,Bit_SET);else GPIO_WriteBit(I2C_GPIO,PIN_SCL,Bit_RESET);}
#define     SDA(x)	              {if(x)GPIO_WriteBit(I2C_GPIO,PIN_SDA,Bit_SET);else GPIO_WriteBit(I2C_GPIO,PIN_SDA,Bit_RESET);}
#define     R_SDA                 GPIO_ReadInputDataBit(I2C_GPIO,PIN_SDA)
#define     SDA_OUT               {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=(u32)3<<SDA_NUM*4;}   //PB0���ģʽ 0XFFFFFFF0��ʾ�˿�0��λ
#define     SDA_IN                {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=(u32)8<<SDA_NUM*4;}	  //PB0����ģʽ
#define     delay_us(delay)       hw_us_delay(delay);

#else
#include <ioCC2530.h>
#define     SCL	                  P0_0       	                	//IICʱ�����Ŷ���
#define     SDA	                  P0_1       	                	//IIC�������Ŷ���
#define     SDA_OUT               {P0DIR |= 0X02;}              //����P0_1���ģʽ
#define     SDA_IN                {P0DIR &= ~0X02;}             //����P0_1����ģʽ
#define     delay_us(delay)       {for(signed int x=0;x<(signed int)(delay-1);x++);}  //΢����ʱ����  x����������  delay:��ʱʱ��(��ǰʱ��������)
#endif
#include <math.h>
#include <stdio.h>

/*********************************************************************************************
* �ⲿԭ�ͺ���
*********************************************************************************************/
void iic_delay_us(unsigned int i);
void iic_init(void);
void iic_start(void);
void iic_stop(void);
unsigned char iic_write_byte(unsigned char data);

#endif 