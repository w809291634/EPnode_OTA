/*********************************************************************************************
* �ļ���sensor.c
* ���ߣ�Xuzhy 2018.5.16
* ˵����xLab Sensor-D����������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
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
* ȫ�ֱ���
*********************************************************************************************/
static uint8  D1 = 1;                                           // ���ӳ�ʼ״̬Ϊ��
static uint16 V1 = 1000;                                        // ����ģʽ�£���ʾ������
static uint8  V2 = 0;
static uint8  V3 = 0;                                           // ��ǰģʽ 0-���� 1-AI 2-����
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
* ���ƣ�P0_ISR
* ���ܣ�P0���ⲿ�ж�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#pragma vector = P0INT_VECTOR
__interrupt void P0_ISR(void)
{ 
    if(P0IFG&(1<<4))
    {                     
        key_val = zlg7290_get_keyval();
        P0IFG &= ~(1<<4); 
    }
    P0IF = 0;   //�жϱ�־��0
}
/*********************************************************************************************
* ���ƣ�sensorInit()
* ���ܣ�������Ӳ����ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorInit(void)
{
  // ��ʼ������������
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

  // ������ʱ���������������ϱ������¼���MY_REPORT_EVT
  osal_start_timerEx(sapi_TaskID, MY_REPORT_EVT, (uint16)((osal_rand()%10) * 1000));
}
/*********************************************************************************************
* ���ƣ�sensorLinkOn()
* ���ܣ��������ڵ������ɹ����ú���
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorLinkOn(void)
{
  sensorUpdate();
}
/*********************************************************************************************
* ���ƣ�sensorUpdate()
* ���ܣ����������ϱ�������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorUpdate(void)
{ 
  char pData[16];
  char *p = pData;
  
  ZXBeeBegin();                                                 // ��������֡��ʽ��ͷ
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
  
  sprintf(p, "%u", D1);                                  // �ϱ����Ʊ��� 
  ZXBeeAdd("D1", p);
  
  p = ZXBeeEnd();                                               // ��������֡��ʽ��β
  if (p != NULL) {												
    ZXBeeInfSend(p, strlen(p));	                                // ����Ҫ�ϴ������ݽ��д����������ͨ��zb_SendDataRequest()���͵�Э����
  }
}
/*********************************************************************************************
* ���ƣ�sensorCheck()
* ���ܣ����������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
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
      ZXBeeBegin();                                               //��ʼ����������״̬���������ͻ�����
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
      char *p = ZXBeeEnd();                                       // ��������֡��ʽ��β
      if (p != NULL) {												
        ZXBeeInfSend(p, strlen(p));	                         // ����Ҫ�ϴ������ݽ��д����������ͨ��zb_SendDataRequest()���͵�Э����
      }
      key_val = 0;
    }
    if (lastDis != (D1&0x01)) {
      lastDis = D1 & 0x01;
      if (lastDis == 0) {
         oled_turn_off();
         display_off();
      } else {
        oled_turn_on();                                          //������ʾ��
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
        OLED_Clear(); // �ر���ʾ
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
  } else if(V3 == 2) {  //OLED��ʾ4λ64�Ŵ�����
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
* ���ƣ�sensorControl()
* ���ܣ�����������
* ������cmd - ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sensorControl(uint8 cmd)
{
	//����LEDˢ����ռ�ô���cpuֱ�ӣ��˴�ˢ���ᵼ�º����ȡ���ʱ��
	//���D1����ͳһ�ŵ�sensorCheck��������
}
/*********************************************************************************************
* ���ƣ�ZXBeeUserProcess()
* ���ܣ������յ��Ŀ�������
* ������*ptag -- ������������
*       *pval -- �����������
* ���أ�<0:��֧��ָ�>=0 ָ���Ѵ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int ZXBeeUserProcess(char *ptag, char *pval)
{ 
  int val;
  int ret = 0;	
  char pData[16];
  char *p = pData;
  
  // ���ַ�������pval����ת��Ϊ���ͱ�����ֵ
  val = atoi(pval);	
  // �����������
  if (0 == strcmp("CD1", ptag)){                                // ��D1��λ���в�����CD1��ʾλ�������
    D1 &= ~val;
    sensorControl(D1);                                          // ����ִ������
    ret = sprintf(p, "%u", D1);
    ZXBeeAdd("D1", p);
  }
  if (0 == strcmp("OD1", ptag)){                                // ��D1��λ���в�����OD1��ʾλ��һ����
    D1 |= val;
    sensorControl(D1);                                          // ����ִ������
    ret = sprintf(p, "%u", D1);
    ZXBeeAdd("D1", p);
  }
  if (0 == strcmp("D1", ptag)){                                 // ��ѯִ�����������
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
* ���ƣ�MyEventProcess()
* ���ܣ��Զ����¼�����
* ������event -- �¼����
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void MyEventProcess( uint16 event )
{
  //�����ʱ��ɺ���������������ⶨʱ��
  static char check_start_flag = 0;
  if(check_start_flag == 0) {
    if (event & MY_REPORT_EVT) { 
      // ������ʱ������������������¼���MY_CHECK_EVT
      osal_start_timerEx(sapi_TaskID, MY_CHECK_EVT, 100);
      check_start_flag = 1;
    }
  }
  
  if (event & MY_REPORT_EVT) { 
    sensorUpdate();
    //������ʱ���������¼���MY_REPORT_EVT 
    osal_start_timerEx(sapi_TaskID, MY_REPORT_EVT, (uint16)(30*1000));
  }  
  if (event & MY_CHECK_EVT) { 
    sensorCheck(); 
    //������ʱ���������¼���MY_CHECK_EVT
    osal_start_timerEx(sapi_TaskID, MY_CHECK_EVT, 100);
  }
}

