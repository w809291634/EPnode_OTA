/*********************************************************************************************
* 文件：iic.h
* 作者：zonesion
* 说明：iic头文件
* 修改：
* 注释：
*********************************************************************************************/
#ifndef _IIC_OLED_H_
#define _IIC_OLED_H_
#include "config.h"

/*********************************************************************************************
* 宏定义
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
#define     SDA_OUT               {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=(u32)3<<SDA_NUM*4;}   //PB0输出模式 0XFFFFFFF0表示端口0复位
#define     SDA_IN                {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=(u32)8<<SDA_NUM*4;}	  //PB0输入模式
#define     delay_us(delay)       hw_us_delay(delay);

#else
#include <ioCC2530.h>
#define     SCL	                  P0_0       	                	//IIC时钟引脚定义
#define     SDA	                  P0_1       	                	//IIC数据引脚定义
#define     SDA_OUT               {P0DIR |= 0X02;}              //设置P0_1输出模式
#define     SDA_IN                {P0DIR &= ~0X02;}             //设置P0_1输入模式
#define     delay_us(delay)       {for(signed int x=0;x<(signed int)(delay-1);x++);}  //微秒延时函数  x：数据区域  delay:延时时间(当前时钟配置下)
#endif
#include <math.h>
#include <stdio.h>

/*********************************************************************************************
* 外部原型函数
*********************************************************************************************/
void iic_delay_us(unsigned int i);
void iic_init(void);
void iic_start(void);
void iic_stop(void);
unsigned char iic_write_byte(unsigned char data);

#endif 