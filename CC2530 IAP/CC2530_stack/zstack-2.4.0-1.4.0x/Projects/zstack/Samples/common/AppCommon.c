/*********************************************************************************************
* �ļ��� AppCommon.c
* ���ߣ� Xuzhy 2018.5.16
* ˵��������Sapi�ܹ���Ӧ�ò��ļ����ο���SimpleSwitch.c
* �޸ģ�fuyou ����͸����������
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "ZComDef.h"
#include "OSAL.h"
#include "sapi.h"
#include "hal_key.h"
#include "hal_led.h"
#include "hal_adc.h"
#include "SimpleApp.h"
#include "mt.h"
#include "ZDApp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensor.h"
#include "osal_nv.h"
#include "AddrMgr.h"
#include "rtg.h"
#include "nwk_util.h"
#include "AppCommon.h"
#include "zxbee.h"
#include "at.h"
#include "zxbee-inf.h"
#include "OnBoard.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/
// Application States
#define APP_INIT                           0                    // Initial state
#define APP_START                          1                    // Device has started/joined network

// Application osal event identifiers
#define __START_EVT                        0x0010
#define __REPORT_EVT                       0x0020
#define __AT_EVT                           0x0040

#define REPORT_DELAY                      30

#define NUM_IN_CMD_SWITCH                 2
#define NUM_OUT_CMD_SWITCH                2
/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static uint8 mLinkStatus = 0;                                   // zigbee����״̬��0��δ������1������
static uint16 panid;
static uint8 logicalType;

static uint32 chs[] = {0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000,
        0x00010000, 0x00020000, 0x00040000,0x00080000,0x00100000,0x00200000,
        0x00400000,0x00800000,0x01000000,0x02000000,0x04000000}; 

const cId_t zb_InCmdList[NUM_IN_CMD_SWITCH] =
{
  ID_CMD_READ_REQ,
  ID_CMD_WRITE_REQ,
};
const cId_t zb_OutCmdList[NUM_OUT_CMD_SWITCH] =
{
  ID_CMD_READ_RES,
  ID_CMD_WRITE_RES,
};
// Define SimpleDescriptor for Switch device
const SimpleDescriptionFormat_t zb_SimpleDesc =
{
  MY_ENDPOINT_ID,             //  Endpoint
  MY_PROFILE_ID,              //  Profile ID
  DEV_ID_SENSOR,              //  Device ID
  DEVICE_VERSION_SWITCH,      //  Device Version
  0,                          //  Reserved
  NUM_IN_CMD_SWITCH,          //  Number of Input Commands
  (cId_t *) zb_InCmdList,     //  Input Command List
  NUM_OUT_CMD_SWITCH,         //  Number of Output Commands
  (cId_t *) zb_OutCmdList     //  Output Command List
};

void myReset(void)
{
  HAL_DISABLE_INTERRUPTS();
  WatchDogEnable( WDTISH );
  while(1);
}

/*********************************************************************************************
* ����ԭ��˵��
*********************************************************************************************/
void zb_HanderMsg(osal_event_hdr_t *msg);
void sensorLinkOn(void);
static void my_report_proc(void);
static char* read_nb(char *buf, int len);
static uint16 _tm_report_delay, _tm_report_cnt;
void starReportTPN(uint16 t, uint16 n);
/*********************************************************************************************
* ���ƣ�_get_at_event()
* ���ܣ�����osal at�¼���ʶ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
uint8 _get_at_event(void)
{
  return __AT_EVT;
}
/*********************************************************************************************
* ���ƣ�zb_HandleOsalEvent()
* ���ܣ�sapi�¼�����������һ�������¼�������֮�󣬵����������
* ������event - �����������¼�
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_HandleOsalEvent( uint16 event )
{
  if (event & ZB_ENTRY_EVENT) {
    uint8 startOptions;
    uint8 selType = NODE_TYPE;
    
    at_init();

    zb_ReadConfiguration( ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType );
    if ( logicalType !=ZG_DEVICETYPE_ENDDEVICE && logicalType !=ZG_DEVICETYPE_ROUTER ) {
      zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &selType);
      zb_SystemReset();
    }
	
    // Do more configuration if necessary and then restart device with auto-start bit set
    // write endpoint to simple desc...dont pass it in start req..then reset
    zb_ReadConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
    if (startOptions != ZCD_STARTOPT_AUTO_START) {
      startOptions = ZCD_STARTOPT_AUTO_START;
      zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
      zb_SystemReset();
    } 
    osal_nv_read( ZCD_NV_PANID, 0, sizeof( panid ), &panid );    
    HalLedSet( HAL_LED_2, HAL_LED_MODE_FLASH );			        //����ƿ�ʼ��˸
    
    ZXBeeInfInit();                                             // ZXBeeͨ��Э���ʼ��
#ifndef CC2530_Serial
        sensorInit();                                               // ��������ʼ��
#endif
#ifdef CC2530_Serial
     WatchDogEnable( WDTIMX );
     
#endif        
    osal_start_timerEx( sapi_TaskID, __START_EVT, 500);     
  }
  
  
  static uint16 tick = 0;
  static uint16 at_tick = 0;
  
  if (event & __START_EVT) {
    
#ifdef CC2530_Serial
    uint8 f = EA;
    EA = 0;
    WDCTL = WDCLP1;
    WDCTL = WDCLP2;
    EA = f;
    
    if (mLinkStatus && ((int16)tick-(int16)at_tick) > 5*60*2){
      myReset();
    }
#endif    
    
    tick += 1;
    if (mLinkStatus == 0 && tick>110) {
      //zb_SystemReset();
      myReset();
    }
   
    osal_start_timerEx( sapi_TaskID, __START_EVT, 500); 
  }
  if (event & __REPORT_EVT) {                                   // �����ϱ���������¼�
    if (_tm_report_cnt > 0) {
      my_report_proc();
      osal_start_timerEx( sapi_TaskID, __REPORT_EVT, _tm_report_delay * 1000);
      _tm_report_cnt--;
    }
  }
  if (event & __AT_EVT) {                                       // ����ATָ���¼�
#ifdef CC2530_Serial
    at_tick = tick;
#endif
    at_proc();
  }
  if (event & 0x000F) {                                         // �����û��Զ����¼�
#ifndef CC2530_Serial
        MyEventProcess( event );
#endif
  }
}
/*********************************************************************************************
* ���ƣ�zb_HandleKeys()
* ���ܣ�����ڵ�����İ����¼�
* ������shift��ת�Ʊ�־��keys : ���µİ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_HandleKeys( uint8 shift, uint8 keys )
{
}
/*********************************************************************************************
* ���ƣ�zb_StartConfirm()
* ���ܣ���zstackЭ��ջ������ɺ�ִ���������
* ������status��������ɺ��״̬
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_StartConfirm( uint8 status )
{
  // If the device sucessfully started, change state to running
  if ( status == ZB_SUCCESS )                                   // �����ɹ�
  {
    HalLedSet( HAL_LED_2, HAL_LED_MODE_ON );
    mLinkStatus = 1;
#ifndef CC2530_Serial
        sensorLinkOn();                                             // �����ɹ������
#else
    AT_reportedLinkStatus();
#endif
  }
  else
  {
  }
}
/*********************************************************************************************
* ���ƣ�zb_SendDataConfirm()
* ���ܣ���������ȷ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_SendDataConfirm( uint8 handle, uint8 status )
{
  void ZXBeeSendConfirm(uint8 h, uint8 st);
  ZXBeeSendConfirm(handle, status);
}
/*********************************************************************************************
* ���ƣ�zb_BindConfirm()
* ���ܣ���ȷ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_BindConfirm( uint16 commandId, uint8 status )
{  
}
/*********************************************************************************************
* ���ƣ�zb_AllowBindConfirm()
* ���ܣ����������豸��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_AllowBindConfirm( uint16 source )
{
}
/*********************************************************************************************
* ���ƣ�zb_FindDeviceConfirm()
* ���ܣ������豸��ɺ�ȷ������
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_FindDeviceConfirm( uint8 searchType, uint8 *searchKey, uint8 *result )
{
}
/*********************************************************************************************
* ���ƣ�zb_HanderMsg()
* ���ܣ�MT���ڴ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_HanderMsg(osal_event_hdr_t *msg)
{
}
/*********************************************************************************************
* ���ƣ�zb_ReceiveDataIndication()
* ���ܣ���zigbee���ܵ��ڵ㷢�͵����ݺ󣬵����������
* ������source��Դ��ַ��commandID������ID��len���յ����ݵĳ��ȣ�pData���յ�������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_ReceiveDataIndication( uint16 source, uint16 command, uint16 len, uint8 *pData  )
{
  uint16 pAddr = NLME_GetCoordShortAddr();
  
  /* ���յ����ݴ��� */
  HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
  HalLedSet( HAL_LED_1, HAL_LED_MODE_BLINK );  
  
 
  // ������յ����������ݰ�APP_DATA
  if (command == 0) { //���command==0 ˵����zxbee����
    if (logicalType != ZG_DEVICETYPE_COORDINATOR) { //ͨ��atָ��͵�����
      at_notify_data((char *)pData, len);
    }
    ZXBeeInfRecv((char*)pData, len);   //����ZXBee�ӿڴ����������
  }
}
/*********************************************************************************************
* ���ƣ�my_report_proc()
* ���ܣ��ڵ�������Ϣ�ϱ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void my_report_proc(void)
{
  unsigned char mac[Z_EXTADDR_LEN];
  static char wbuf[96];
  ZXBeeBegin();

  NLME_GetCoordExtAddr(mac);
  sprintf(wbuf, "%02X%02X", mac[1],mac[0]);

  read_nb(wbuf+strlen(wbuf), -1);
  ZXBeeAdd("PN", wbuf);
  
#ifndef CC2530_Serial
    sprintf(wbuf, "%d%d%s", NODE_CATEGORY, logicalType, NODE_NAME);
    ZXBeeAdd("TYPE", wbuf);
#else
    at_notify_data("AT+TYPE?\r\n",strlen("AT+TYPE?\r\n"));
#endif
  char *p = ZXBeeEnd();
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
}
/*********************************************************************************************
* ���ƣ�read_nb()
* ���ܣ���ѯ�ھӽڵ��ַ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static char* read_nb(char *buf, int len)
{
  int i;
  char *p;
  
  buf[0] = 0;
  p = buf;     
  for (i=0; i<MAX_NEIGHBOR_ENTRIES; i++) {
    neighborEntry_t *pnb = &neighborTable[i];
    if (pnb->panId == panid 
        && memcmp(pnb->neighborExtAddr,"\x00\x00\x00\x00\x00\x00\x00\x00", 8)!=0 
        && pnb->age <= NWK_ROUTE_AGE_LIMIT) {
          sprintf(p, "%02X%02X", pnb->neighborExtAddr[1], pnb->neighborExtAddr[0]);
          p = p + strlen(p);         
    }
  }
  return buf;
}
/*********************************************************************************************
* ���ƣ�GetCurrentLogicalType()
* ���ܣ���ȡ��ǰ�ڵ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
uint8 GetCurrentLogicalType(void)
{
  return logicalType;
}
/*********************************************************************************************
* ���ƣ�GetLinkStatus()
* ���ܣ���ȡ�ڵ�����״̬
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
uint8 GetLinkStatus(void)
{
  return mLinkStatus;
}
/*********************************************************************************************
* ���ƣ�GetPanId()
* ���ܣ���ȡ�ڵ�PANID
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
uint16 GetPanId(void)
{
   uint16 tmp16;
   osal_nv_read( ZCD_NV_PANID, 0, sizeof( tmp16 ), &tmp16 );
   return tmp16;
}
/*********************************************************************************************
* ���ƣ�SetPanId()
* ���ܣ����ýڵ�PANID
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void SetPanId(uint16 id)
{
    uint8 startOptions = ZCD_STARTOPT_DEFAULT_NETWORK_STATE;
    uint16 tmp16;
    tmp16 = GetPanId();
    if (tmp16 != id) {
        osal_nv_write(ZCD_NV_PANID, 0, osal_nv_item_len( ZCD_NV_PANID ), &id);
        zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions ); //�������״̬�����ı�
    }
}
/*********************************************************************************************
* ���ƣ�GetChannel()
* ���ܣ���ȡ�ڵ��ŵ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
uint8 GetChannel(void)
{
  uint32 tmp32;
  uint8 i;
		
  osal_nv_read( ZCD_NV_CHANLIST, 0, sizeof( tmp32 ), &tmp32 );
     
  for (i=0; i<16; i++) {
    if (tmp32 == chs[i]) break;
  }
  i += 11;
  return i;
}
/*********************************************************************************************
* ���ƣ�SetChannel()
* ���ܣ����ýڵ��ŵ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void SetChannel(uint8 val)
{
  uint8 startOptions = ZCD_STARTOPT_DEFAULT_NETWORK_STATE;
  uint32 tmp32, t32;
  tmp32 = val - 11;
  osal_nv_read( ZCD_NV_CHANLIST, 0, sizeof( tmp32 ), &t32 );
  if (tmp32 < 16) {
    if (t32 != chs[tmp32]) {
      osal_nv_write(ZCD_NV_CHANLIST, 0, osal_nv_item_len( ZCD_NV_CHANLIST ), &chs[tmp32]);
      zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions ); //�������״̬�����ı�
    }
  }
}
/*********************************************************************************************
* ���ƣ�GetLogicalType()
* ���ܣ���ȡ�ڵ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
uint8 GetLogicalType(void)
{
  uint8 st;
  zb_ReadConfiguration( ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &st );
  return st;
}
/*********************************************************************************************
* ���ƣ�SetLogicalType()
* ���ܣ����ýڵ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void SetLogicalType(uint8 t)
{
  uint8 st;
  uint8 startOptions = ZCD_STARTOPT_DEFAULT_NETWORK_STATE;
  if (logicalType == ZG_DEVICETYPE_ROUTER || logicalType == ZG_DEVICETYPE_ENDDEVICE) {
    st = GetLogicalType();
    if (t != st && (t==ZG_DEVICETYPE_ROUTER || t == ZG_DEVICETYPE_ENDDEVICE)) {
      zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &t);
      zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions ); //�������״̬�����ı�
    } 
  }
}
/*********************************************************************************************
* ���ƣ�starReportTPN()
* ���ܣ����ýڵ��������ѭ���ϱ�ʱ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void starReportTPN(uint16 t, uint16 n)
{
  _tm_report_delay = t;
  _tm_report_cnt = n;
  if (_tm_report_cnt != 0) {
    osal_start_timerEx( sapi_TaskID, __REPORT_EVT, (osal_rand()%_tm_report_delay) * (osal_rand()%1000));
  }
}
