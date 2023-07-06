/*********************************************************************************************
* 文件：sensor.c
* 作者：Xuzhy 2018.5.16
* 说明：xLab Sensor-D传感器程序
* 修改：
* 注释：
*********************************************************************************************/

/*********************************************************************************************
* 头文件
*********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sapi.h"
#include "osal_nv.h"
#include "addrmgr.h"
#include "mt.h"
#include "hal_led.h"
#include "hal_adc.h"
#include "hal_uart.h"
#include "sensor.h"
#include "lcd.h"
#include "zlg7290.h"
#include "iic.h"
#include "oled.h"
#include "delay.h"
#include "zxbee.h"
#include "zxbee-inf.h"
/*********************************************************************************************
* 全局变量
*********************************************************************************************/
static uint8  D1 = 1;                                           // 电视初始状态为开
static uint16 V1 = 1000;                                        // 数字模式下，显示的数字
static uint8  V2 = 0;
static uint8  V3 = 0;                                           // 当前模式 0-正常 1-AI 2-数字
static unsigned char key_val = 0;

union DataV1{
  uint8 normal;
  char AIBuf[15];
}dataV1;

union LastDataV1{
  uint8 normal;
  char AIBuf[15];
}lastDataV1;

/*********************************************************************************************
* 名称：P0_ISR
* 功能：P0口外部中断
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
#pragma vector = P0INT_VECTOR
__interrupt void P0_ISR(void)
{ 
    if(P0IFG&(1<<4))
    {                     
        key_val = zlg7290_get_keyval();
        P0IFG &= ~(1<<4); 
    }
    P0IF = 0;   //中断标志清0
}
/*********************************************************************************************
* 名称：sensorInit()
* 功能：传感器硬件初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorInit(void)
{
  // 初始化传感器代码
  OLED_Init();
  oled_turn_on();
  char buff[10];
  sprintf(buff,"%d ",dataV1.normal);
  OLED_ShowString(8,1,(unsigned char*)buff,16);
  sprintf(buff,"%2d",V2);
  OLED_ShowString(104,5,(unsigned char*)buff,16);OLED_ShowString(56,3,"TV",16);
  zlg7290_init();
  segment_display(dataV1.normal*100+V2, V3);
  key_val = zlg7290_read_reg(SYETEM_REG);
  srand(osal_GetSystemClock());

  // 启动定时器，触发传感器上报数据事件：MY_REPORT_EVT
  osal_start_timerEx(sapi_TaskID, MY_REPORT_EVT, (uint16)((osal_rand()%10) * 1000));
}
/*********************************************************************************************
* 名称：sensorLinkOn()
* 功能：传感器节点入网成功调用函数
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorLinkOn(void)
{
  sensorUpdate();
}
/*********************************************************************************************
* 名称：sensorUpdate()
* 功能：处理主动上报的数据
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorUpdate(void)
{ 
  char pData[16];
  char *p = pData;
  
  ZXBeeBegin();                                                 // 智云数据帧格式包头
  if(V3 == 0)
  {
    sprintf(p, "%d", dataV1.normal);
    ZXBeeAdd("V1", p);
    
    sprintf(p, "%d", V2);
    ZXBeeAdd("V2", p);
  } else if(V3 == 2) {
    uint16 temp = V1;
    temp += 90+rand()%20;
    while(temp > 9999) {
      temp = 1000 + (temp - 9999);
    }
    V1 = temp;
    sprintf(p, "%u", V1);
    ZXBeeAdd("V1", p);
  }
  
  sprintf(p, "%d", V3);
  ZXBeeAdd("V3", p);
  
  sprintf(p, "%u", D1);                                  // 上报控制编码 
  ZXBeeAdd("D1", p);
  
  p = ZXBeeEnd();                                               // 智云数据帧格式包尾
  if (p != NULL) {												
    ZXBeeInfSend(p, strlen(p));	                                // 将需要上传的数据进行打包操作，并通过zb_SendDataRequest()发送到协调器
  }
}
/*********************************************************************************************
* 名称：sensorCheck()
* 功能：传感器监测
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorCheck(void)
{
  static unsigned char lastV3 = 0;
  static uint8 lastV2 = 0, lastDis = 1;
  static uint16 last_V1 = 1000;
  char buf[10];
  if(V3 == 0)
  {
    if (key_val != 0) {
      ZXBeeBegin();                                               //开始将传感器的状态拷贝到发送缓冲区
      sprintf(buf, "%d", key_val);
      ZXBeeAdd("A0", buf);
      if (D1 & 0x01) {
        if(key_val == RIGHT){
          V2++;
          if(V2 > 99)
            V2 = 0;
        }
        if(key_val == LEFT){
          if(V2 == 0)
            V2 = 99;
          else
            V2--;
        }
        if(key_val == UP){
          dataV1.normal++;
          if(dataV1.normal > 19)
            dataV1.normal = 0;
        }
        if(key_val == DOWN){
          if(dataV1.normal == 0)
            dataV1.normal = 19;
          else
            dataV1.normal--;
        }
      }
      if(key_val == CENTER){
        if(D1 & 0X01){
          D1 &= ~0X01;
        }
        else
        {
          D1 |= 0X01;
        }
        sprintf(buf, "%u", D1);
        ZXBeeAdd("D1", buf);
      }
      if(D1 & 0X01){
        sprintf(buf, "%d", dataV1.normal);
        ZXBeeAdd("V1", buf);
        sprintf(buf, "%d", V2);
        ZXBeeAdd("V2", buf);
      }
      char *p = ZXBeeEnd();                                       // 智云数据帧格式包尾
      if (p != NULL) {												
        ZXBeeInfSend(p, strlen(p));	                         // 将需要上传的数据进行打包操作，并通过zb_SendDataRequest()发送到协调器
      }
      key_val = 0;
    }
    if (lastDis != (D1&0x01)) {
      lastDis = D1 & 0x01;
      if (lastDis == 0) {
         oled_turn_off();
         display_off();
      } else {
        oled_turn_on();                                          //开启显示屏
        OLED_DisFill(0,0,127,63,0);
        OLED_DisFill(0,0,127,7,1);
        OLED_DisFill(0,0,7,63,1);
        OLED_DisFill(0,56,127,63,1);
        OLED_DisFill(120,0,127,63,1);
        OLED_Refresh_Gram();
        OLED_DisFill(0,0,127,63,1);
        sprintf(buf,"%d ",dataV1.normal);
        OLED_ShowString(8,1,(unsigned char*)buf,16);
        sprintf(buf,"%2d",V2);
        OLED_ShowString(104,5,(unsigned char*)buf,16);
        OLED_ShowString(56,3,"TV",16);
        segment_display(dataV1.normal*100+V2, V3);
      }
    }
    if (dataV1.normal != lastDataV1.normal ||  V2 != lastV2) {
      segment_display(dataV1.normal*100+V2, V3);
      sprintf(buf,"%d ",dataV1.normal);
      OLED_ShowString(8,1,(unsigned char*)buf,16);
      sprintf(buf,"%2d",V2);
      OLED_ShowString(104,5,(unsigned char*)buf,16);
      OLED_ShowString(56,3,"TV",16);
      lastDataV1.normal = dataV1.normal;
      lastV2 = V2;
    }
  }
  else if(V3 == 1)
  {
    static unsigned char second = 0 , showStatus = 0;
    if((D1 & 0x01) == 0x01)
    {
      if(strcmp(dataV1.AIBuf, lastDataV1.AIBuf) != 0)
      { 
        char var[3] = {0};
        char str[15] = {0};
        strncpy(var, dataV1.AIBuf, 2);
        var[2] = '\0';
        unsigned char num = atoi(var);
        if(num <= 28)
        {
          OLED_ShowString(15, 4, "             ", 16);
          OLED_ShowCHinese(30, 4, num);
          strncpy(str, dataV1.AIBuf+2, strlen(dataV1.AIBuf));
          OLED_ShowString(46, 4, (unsigned char*)str, 16);
          showStatus = 1;
          second = 0;
          strcpy(lastDataV1.AIBuf, dataV1.AIBuf);
        }
      }
      if(V2 != lastV2)
      {
        segment_display(V2, V3);
        lastV2 = V2;
      }
      if(showStatus)
      {
        if(second >= 100)
        {
          OLED_ShowString(15, 4, "             ", 16);
          second = 0;
          showStatus = 0;
          strcpy(lastDataV1.AIBuf, 0);
          strcpy(dataV1.AIBuf, 0);
        }
        else
          second++;
      }
    }
    if(lastDis != D1)
    {
      if(!(D1 & 0x01))
      {
        OLED_Clear(); // 关闭显示
        display_off();
        strcpy(lastDataV1.AIBuf, 0);
        strcpy(dataV1.AIBuf, 0);
      }
      else
      {
        segment_display(V2, V3);
        for(unsigned char i=0; i<4; i++)
        {
          unsigned char x = 0;
          x = i * 16;
          OLED_ShowCHinese(30+x, 2, 29+i);
        }
      }
      lastDis = D1;
    }
  } else if(V3 == 2) {  //OLED显示4位64号大数字
    if(last_V1 != V1) {
      sprintf(buf, "%u", V1);
      OLED_ShowString(0,0,(uint8 *)buf,64);
      last_V1 = V1;
    }
  }
  if(lastV3 != V3)
  {
    OLED_Clear();
    D1 |= 0x01;
    
    if(V3 == 2)
    {
      sprintf(buf, "%u", V1);
      OLED_ShowString(0,0,(uint8 *)buf,64);
      display_off();
      sensorUpdate();
    }
    else if(V3 == 1)
    {
      memset(dataV1.AIBuf, 0, sizeof(dataV1.AIBuf));
      V2 = 200;
      segment_display(V2, V3);
      for(unsigned char i=0; i<4; i++)
      {
        unsigned char x = 0;
        x = i * 16;
        OLED_ShowCHinese(30+x, 2, 29+i);
      }
    }
    else if(V3 == 0)
    {
      dataV1.normal = 0;
      V2 = 0;
      oled_turn_on();
      sprintf(buf,"%d ",dataV1.normal);
      OLED_ShowString(8,1,(unsigned char*)buf,16);
      sprintf(buf,"%2d",V2);
      OLED_ShowString(104,5,(unsigned char*)buf,16);
      OLED_ShowString(56,3,"TV",16);
      segment_display(dataV1.normal*100+V2, V3);
    }
    lastV3 = V3;
  }
}
/*********************************************************************************************
* 名称：sensorControl()
* 功能：传感器控制
* 参数：cmd - 控制命令
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void sensorControl(uint8 cmd)
{
	//由于LED刷屏会占用大量cpu直接，此处刷屏会导致后面读取命令超时，
	//因此D1控制统一放到sensorCheck中来处理
}
/*********************************************************************************************
* 名称：ZXBeeUserProcess()
* 功能：解析收到的控制命令
* 参数：*ptag -- 控制命令名称
*       *pval -- 控制命令参数
* 返回：<0:不支持指令，>=0 指令已处理
* 修改：
* 注释：
*********************************************************************************************/
int ZXBeeUserProcess(char *ptag, char *pval)
{ 
  int val;
  int ret = 0;	
  char pData[16];
  char *p = pData;
  
  // 将字符串变量pval解析转换为整型变量赋值
  val = atoi(pval);	
  // 控制命令解析
  if (0 == strcmp("CD1", ptag)){                                // 对D1的位进行操作，CD1表示位清零操作
    D1 &= ~val;
    sensorControl(D1);                                          // 处理执行命令
    ret = sprintf(p, "%u", D1);
    ZXBeeAdd("D1", p);
  }
  if (0 == strcmp("OD1", ptag)){                                // 对D1的位进行操作，OD1表示位置一操作
    D1 |= val;
    sensorControl(D1);                                          // 处理执行命令
    ret = sprintf(p, "%u", D1);
    ZXBeeAdd("D1", p);
  }
  if (0 == strcmp("D1", ptag)){                                 // 查询执行器命令编码
    if (0 == strcmp("?", pval)){
      ret = sprintf(p, "%u", D1);
      ZXBeeAdd("D1", p);
    } 
  }
  
  if (0 == strcmp("V1", ptag)){ 
    if (0 == strcmp("?", pval)){ 
      if(V3 == 0)
      {
        sprintf(p, "%d", dataV1.normal);
        ZXBeeAdd("V1", p);
      } else if(V3 == 2) {
        sprintf(p, "%u", V1);
        ZXBeeAdd("V1", p);
      }
    }
    else {
      if(V3 == 0)
      {
        if (val >= 0 && val <= 19)
          dataV1.normal = val;
      }
      else if(V3 == 2) {
        V1 = atoi(pval);
      } else
        strcpy(dataV1.AIBuf, pval);
    } 
  }  
  if (0 == strcmp("V2", ptag)){ 
    if (0 == strcmp("?", pval)){
      sprintf(p, "%d", V2); 
      ZXBeeAdd("V2", p);
    } 
    else {   
      if(V3 == 0)
      {
        if (val >= 0 && val <= 99) {
          V2 = val;
        }
      }
      else
      {
        if (val >= 0 && val <= 255) {
          V2 = val;
        }
      }
    }
  }
  if (0 == strcmp("V3", ptag)){ 
    if (0 == strcmp("?", pval)){
      sprintf(p, "%d", V3); 
      ZXBeeAdd("V3", p);
    } 
    else {   
      V3 = val;
    }
  }
  return ret;
}
/*********************************************************************************************
* 名称：MyEventProcess()
* 功能：自定义事件处理
* 参数：event -- 事件编号
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void MyEventProcess( uint16 event )
{
  //随机延时完成后，再启动传感器监测定时器
  static char check_start_flag = 0;
  if(check_start_flag == 0) {
    if (event & MY_REPORT_EVT) { 
      // 启动定时器，触发传感器监测事件：MY_CHECK_EVT
      osal_start_timerEx(sapi_TaskID, MY_CHECK_EVT, 100);
      check_start_flag = 1;
    }
  }
  
  if (event & MY_REPORT_EVT) { 
    sensorUpdate();
    //启动定时器，触发事件：MY_REPORT_EVT 
    osal_start_timerEx(sapi_TaskID, MY_REPORT_EVT, (uint16)(30*1000));
  }  
  if (event & MY_CHECK_EVT) { 
    sensorCheck(); 
    //启动定时器，触发事件：MY_CHECK_EVT
    osal_start_timerEx(sapi_TaskID, MY_CHECK_EVT, 100);
  }
}

