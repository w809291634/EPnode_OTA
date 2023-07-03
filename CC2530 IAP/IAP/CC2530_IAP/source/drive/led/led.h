/*********************************************************************************************
* 文件：led.h
* 作者：fuyou
* 说明：lite节点led头文件
* 修改：
* 注释：
*********************************************************************************************/
#ifndef _led_h_
#define _led_h_

#include "sys.h"

#define LED2    P1_0              //定义LED2为P1_0口控制
#define LED1    P1_1              //定义LED1为P1_1口控制

/*********************************************************************************************
* 名称：led_init
* 功能：led初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void led_init(void);


#endif
