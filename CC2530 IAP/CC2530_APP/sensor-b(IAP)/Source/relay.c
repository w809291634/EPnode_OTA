/*********************************************************************************************
* 文件：relay.c
* 作者：Lixm 2017.10.17
* 说明：继电器驱动代码
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include "relay.h"

/*********************************************************************************************
* 名称：relay_init()
* 功能：继电器传感器初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void relay_init(void)
{
  P0SEL &= ~0xC0;                                               //配置管脚为通用IO模式
  P0DIR |= 0xC0;                                                //配置控制管脚为输入模式
}
