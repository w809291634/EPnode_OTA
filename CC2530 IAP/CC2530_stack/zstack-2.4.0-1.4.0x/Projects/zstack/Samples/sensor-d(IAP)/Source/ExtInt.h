/*********************************************************************************************
* 文件：ExtInt.h
* 作者：fuyou
* 说明：外部中断头文件
* 修改：
* 注释：
*********************************************************************************************/
#ifndef _ExtInt_h_
#define _ExtInt_h_

#include "ioCC2530.h"

typedef enum
{
    Ext_P0=0,
    Ext_P1=1,
    Ext_P2=2,
}Ext_Px;

typedef enum
{
    Ext_Pin0=0,
    Ext_Pin1=1,
    Ext_Pin2=2,
    Ext_Pin3=3,
    Ext_Pin4=4,
    Ext_Pin5=5,
    Ext_Pin6=6,
    Ext_Pin7=7,
}Ext_Pinx;

typedef enum
{
    Ext_up=0,
    Ext_dowm=1,
}Ext_mode;

/*********************************************************************************************
* 名称：extInt_init
* 功能：外部中断初始化
* 参数：port：端口选择，0-2对应P0-P2。
*       pin：io口选择，0-7对应Px_0-Px_7
*       mode：中断触发方式，0-->上升沿触发，1-->下降沿触发
* 返回：无pin：
* 修改：
* 注释：port为2时，pin范围0-4
*********************************************************************************************/
void ExtInt_init(Ext_Px port,Ext_Pinx pin,Ext_mode mode);

#endif