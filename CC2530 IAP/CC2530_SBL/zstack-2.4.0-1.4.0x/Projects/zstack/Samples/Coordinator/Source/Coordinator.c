/*********************************************************************************************
* �ļ��� Coordinator.c
* ���ߣ� Xuzhy 2018.5.16
* ˵��������Sapi�ܹ���Ӧ�ò��ļ����ο���SimpleCollector
* �޸ģ�
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
#include "DebugTrace.h"
#include "SimpleApp.h"
#include "hal_flash.h"
#include "ZDApp.h"
#if defined( MT_TASK )
#include "osal_nv.h"
#endif
#include "mt_app.h"
#include "mt_uart.h"
#include "mt.h"
#include "rtg.h"
#include "mac_radio_defs.h"
#include "AddrMgr.h"
#include "nwk_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define USE_SYS_FIND_DEVICE   0
#define NODE_NAME  "000"
#define NODE_CATEGORY 1

#define OSAL_NV_PAGE_BEG        HAL_NV_PAGE_BEG
#define OSAL_NV_PAGE_END       (OSAL_NV_PAGE_BEG + HAL_NV_PAGE_CNT - 1)

// Application States
#define APP_INIT                           0
#define APP_START                          1

// Application osal event identifiers
#define _START_EVT                 0x0010
#define __REPORT_EVT               0x0020

// Same definitions as in SimpleSensor.c

#define REPORT_DELAY    30

#define NUM_OUT_CMD_COLLECTOR                2
#define NUM_IN_CMD_COLLECTOR                 3

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
#if defined( MT_TASK )
extern uint8 aExtendedAddress[8];
#endif

static uint16 panid;
static uint8 logicalType;
static uint16 _tm_cnt, _tm_delay;
static char wbuf[256];

// List of output and input commands for Switch device
const cId_t zb_InCmdList[NUM_IN_CMD_COLLECTOR] =
{
  ID_CMD_READ_RES,
  ID_CMD_WRITE_RES,
  ID_CMD_REPORT,
};
const cId_t zb_OutCmdList[NUM_OUT_CMD_COLLECTOR] =
{
  ID_CMD_READ_REQ,
  ID_CMD_WRITE_REQ,
};

// Define SimpleDescriptor for Switch device
const SimpleDescriptionFormat_t zb_SimpleDesc =
{
  MY_ENDPOINT_ID,             //  Endpoint
  MY_PROFILE_ID,              //  Profile ID
  DEV_ID_COLLECTOR,           //  Device ID
  DEVICE_VERSION_COLLECTOR,   //  Device Version
  0,                          //  Reserved
  NUM_IN_CMD_COLLECTOR,       //  Number of Input Commands
  (cId_t *) zb_InCmdList,     //  Input Command List
  NUM_OUT_CMD_COLLECTOR,      //  Number of Output Commands
  (cId_t *) zb_OutCmdList     //  Output Command List
};
/*********************************************************************************************
* ����ԭ��˵��
*********************************************************************************************/
static void my_report_proc(void);
static void process_set_command_call(int (*fun)(char *ptag, char *pval, char *pout));
static void process_package(char *pkg, int len);
static int process_command_callback(char *ptag, char *pval, char *pout);
void zb_HanderMsg(osal_event_hdr_t *pMsg);
static void processCommand(uint16 cmd, byte *dat, uint8 len);
void my_FindDevice(uint8 searchType, uint8 *searchKey);
int my_FindDeviceProc( uint16 source, uint16 command, uint16 len, uint8 *pData);
static char* read_nb(char *buf, int len);
static int (*process_command_call)(char *ptag, char *pval, char *pout);
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

    zb_ReadConfiguration( ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType );
    if ( logicalType != ZG_DEVICETYPE_COORDINATOR )
    {
      logicalType = ZG_DEVICETYPE_COORDINATOR;
      zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
      zb_SystemReset();
    }
    zb_ReadConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
    if ((startOptions & ZCD_STARTOPT_AUTO_START) == 0) {
      zb_StartRequest();
    }
    osal_nv_read( ZCD_NV_PANID, 0, sizeof( panid ), &panid );
    HalLedSet( HAL_LED_2, HAL_LED_MODE_FLASH );
    process_set_command_call(process_command_callback);
  }

  if (event & __REPORT_EVT) {                                   // �����ϱ���������¼�
    if (_tm_cnt > 0) {
      my_report_proc();
      osal_start_timerEx( sapi_TaskID, __REPORT_EVT, _tm_delay * 1000);
      _tm_cnt --;
    }
  }
}
/*********************************************************************************************
* ���ƣ�zb_HanderMsg()
* ���ܣ�MT���ڴ������������յ���λ����Э�����Ĵ�������
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_HanderMsg(osal_event_hdr_t *msg)
{
  mtSysAppMsg_t *pMsg = (mtSysAppMsg_t*)msg;
  
  uint16 dAddr;
  uint16 cmd;
  uint16 addr = NLME_GetShortAddr();
  
  HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
  HalLedSet( HAL_LED_1, HAL_LED_MODE_BLINK );
  if (pMsg->hdr.event == MT_SYS_APP_MSG) {
    //if (pMsg->appDataLen < 4) return;
    dAddr = pMsg->appData[0]<<8 | pMsg->appData[1];
    cmd = pMsg->appData[2]<<8 | pMsg->appData[3];
    if (dAddr != 0) {
      zb_SendDataRequest(dAddr, cmd, pMsg->appDataLen-4, pMsg->appData+4, 0, AF_ACK_REQUEST, AF_DEFAULT_RADIUS );
    }
    if (dAddr == 0 || dAddr == 0xffff) {
      processCommand(cmd, pMsg->appData+4, pMsg->appDataLen-4);
    }
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
  byte res[Z_EXTADDR_LEN+2];
  
  if (ZB_IEEE_SEARCH == searchType) {                            //ͨ��mac��ַѰ�Ҷ�Ӧ�Ľڵ�
    osal_memcpy(res, searchKey, Z_EXTADDR_LEN);
    res[Z_EXTADDR_LEN] = result[1];
    res[Z_EXTADDR_LEN+1] = result[0];
    MT_ReverseBytes( res, Z_EXTADDR_LEN );
    zb_ReceiveDataIndication(0, 0x0101, 8+2,  res);
  }
  if (ZB_NWKA_SEARCH == searchType) {          //ͨ�������ַѰ�Ҷ�Ӧ�Ľڵ�    
    res[0] = searchKey[1];
    res[1] = searchKey[0];
    osal_memcpy(res+2, result, Z_EXTADDR_LEN);
    MT_ReverseBytes( res+2, Z_EXTADDR_LEN );      //mac��ַ��ת
    zb_ReceiveDataIndication(0, 0x0102, 8+2,  res);//�����ַ��ǰ��mac��ַ�ں��͸�����
  }
}
/*********************************************************************************************
* ���ƣ� zb_ReceiveDataIndication()
* ���ܣ� ��zigbee���ܵ��ڵ㷢�͵����ݺ󣬵����������
* ������ source��Դ��ַ��commandID������ID��len���յ����ݵĳ��ȣ�pData���յ�������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void zb_ReceiveDataIndication( uint16 source, uint16 command, uint16 len, uint8 *pData  )
{
    HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
    HalLedSet( HAL_LED_1, HAL_LED_MODE_BLINK );
    mtOSALSerialData_t* msg = (mtOSALSerialData_t*)osal_msg_allocate(sizeof(mtOSALSerialData_t)+len+4);
    if (msg) {
      msg->hdr.event = MT_SYS_APP_RSP_MSG;
      msg->hdr.status = len+4;
      msg->msg = (byte*)(msg+1);
      msg->msg[0] = (source>>8)&0xff;
      msg->msg[1] = source&0xff;
      msg->msg[2] = (command>>8)&0xff;
      msg->msg[3] = command&0xff;
      osal_memcpy(msg->msg+4, pData, len);
      osal_msg_send( MT_TaskID, (uint8 *)msg );
    } 
}
/*********************************************************************************************
* ���ƣ�paramWrite()
* ���ܣ�����д��ӿڣ��ϰ汾������
* ������ pid:������ʶ
*        dat��д�뻺��
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static int paramWrite(uint16 pid, byte *dat)
{
  int len = 0;
  switch (pid) {
  default:
    break;
  }
  return len;
}
/*********************************************************************************************
* ���ƣ�paramRead()
* ���ܣ�������ȡ�ӿڣ��ϰ汾������
* ������ pid��������ʶ
*        dat: ��ȡ���� 
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static int paramRead(uint16 pid, byte *dat)
{
  int len = 0;
  switch (pid) {
  case 0x0001:
    dat[0] = 0x12; dat[1] = 0x09;
    len = 2;
    break;
  case 0x0002:
    dat[0] = 0x11; dat[1] = 0x44;
    len = 2;
    break;
  case 0x0003:
    dat[0] = 0x00; dat[1] = 0x01;
    len = 2;
    break;
  case 0x0004:
    dat[0] = dat[1] = dat[2] = dat[3] = dat[4] = dat[5] = 1;
    len = 6;
    break;
  case 0x0005:
    dat[0] = DEV_ID_COLLECTOR;
    len = 1;
    break;
    /* -----------  ������� ------------------- */  
  case 0x0014: //mac��ַ
    /*osal_nv_read( ZCD_NV_EXTADDR, 0, Z_EXTADDR_LEN, pBuf ); rm by liren */
    ZMacGetReq( ZMacExtAddr, dat ); // add by liren
    // Outgoing extended address needs to be reversed
    MT_ReverseBytes( dat, Z_EXTADDR_LEN );
    len = Z_EXTADDR_LEN;
    break;
  case 0x0015:
    {
      uint8 assocCnt = 0;
      uint16 *assocList;
      int i;
#if defined(RTR_NWK) && !defined( NONWK )
      assocList = AssocMakeList( &assocCnt );
#else
      assocCnt = 0;
      assocList = NULL;
#endif
      dat[0] = assocCnt;
      for (i=0; i<assocCnt&&i<16; i++) {
        dat[1+2*i] = HI_UINT16(assocList[i]);
        dat[1+2*i+1] = LO_UINT16(assocList[i]);
      }
      len = 1 + 2 * assocCnt;
      break;
    }
  }
  /* ------------------------------------ */
  
  return len;
}
/*********************************************************************************************
* ���ƣ�processCommand()
* ���ܣ�zigbee�������
* ������ 
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void processCommand(uint16 cmd, byte *pData, uint8 len)
{
  int i;
  uint16 pid;
  byte dat[64];
  byte rlen = 1;
  int ret;
  
  switch (cmd) {
  case 0x0000:                                  //ZXBee����
    process_package((char*)pData, len);
    break;
  case 0x0101:                                  //ͨ��mac��ַѰ�Ҷ�Ӧ�Ľڵ�
    {
      uint8 *pExtAddr = pData;
      MT_ReverseBytes( pExtAddr, Z_EXTADDR_LEN );
      ZMacGetReq( ZMacExtAddr, dat );            //��ȡ��ǰ�ڵ��mac��ַ
#if USE_SYS_FIND_DEVICE
      zb_FindDeviceRequest(ZB_IEEE_SEARCH, pExtAddr);
#else
      if (TRUE == osal_memcmp(pExtAddr, dat, Z_EXTADDR_LEN) ||   //���mac��ַƥ��
          TRUE == osal_memcmp(pData, "\x00\x00\x00\x00\x00\x00\x00\x00", Z_EXTADDR_LEN))
      {
        ret = 0;
        zb_FindDeviceConfirm(ZB_IEEE_SEARCH, pExtAddr, (unsigned char *)&ret);       
      } else {
        my_FindDevice(ZB_IEEE_SEARCH, pExtAddr);
      }
#endif
    }
    break;
    
  case 0x0102:                                  //ͨ�������ַѰ�Ҷ�Ӧ�Ľڵ�
    {
      uint16 shortAddr = (pData[0]<<8) | pData[1];
      uint16 sa = NLME_GetShortAddr();   //��ȡ��ǰ�ڵ�������ַ
      if (shortAddr == sa) {             //��������ַƥ��
        ZMacGetReq( ZMacExtAddr, dat );  //��ȡ��ǰ�ڵ��mac��ַ 
        zb_FindDeviceConfirm(ZB_NWKA_SEARCH, (unsigned char *)&sa, dat);
      } else {
#if USE_SYS_FIND_DEVICE
        ZDP_IEEEAddrReq( shortAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );
#else
        my_FindDevice(ZB_NWKA_SEARCH, (uint8*)pData);
#endif
      }
    }
    break;
    
  case ID_CMD_WRITE_REQ:                        //�ϰ汾д��ָ��
    for (i=0; i<len; i+=2) {
      pid = pData[i]<<8 | pData[i+1];
      ret = paramWrite(pid, &pData[i+2]);
      if (ret <= 0) {
        dat[0] = 1;
        zb_ReceiveDataIndication( 0, ID_CMD_WRITE_RES, 1, dat );
        return;
      } 
      i += ret;
    }
    dat[0] = 0;
    zb_ReceiveDataIndication( 0, ID_CMD_WRITE_RES, 1, dat);
    break;
  case ID_CMD_READ_REQ:                         //�ϰ汾��ȡָ��
    for (i=0; i<len; i+=2) {
      pid = pData[i]<<8 | pData[i+1];
      dat[rlen++] = pData[i];
      dat[rlen++] = pData[i+1];
      ret = paramRead(pid, dat+rlen);
      if (ret <= 0) {
        dat[0] = 1;
        zb_ReceiveDataIndication( 0, ID_CMD_READ_RES, 1, dat );
        return;
      }
      rlen += ret;
    }
    dat[0] = 0;
    zb_ReceiveDataIndication( 0, ID_CMD_READ_RES, rlen, dat );
    break;
  }    
}
/*********************************************************************************************
* ���ƣ�process_set_command_call()
* ���ܣ�����ָ�����
* ������fun��ָ����� 
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void process_set_command_call(int (*fun)(char *ptag, char *pval, char *pout)) 
{
  process_command_call = fun;
}
/*********************************************************************************************
* ���ƣ�process_package()
* ���ܣ������������
* ������ 
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void process_package(char *pkg, int len)
{  
  char *p;
  char *ptag = NULL;
  char *pval = NULL;
  
  char *pwbuf = wbuf+1;
  
  if (pkg[0] != '{' || pkg[len-1] != '}') return;
  pkg[len-1] = 0;
  p = pkg+1; 
  do {
    ptag = p;
    p = strchr(p, '=');
    if (p != NULL) {
      *p++ = 0;
      pval = p;
      p = strchr(p, ',');
      if (p != NULL) *p++ = 0;
      if (process_command_call != NULL) {
        int ret;
        ret = process_command_call(ptag, pval, pwbuf);
        if (ret > 0) {
          pwbuf += ret;
          *pwbuf++ = ',';
        }
      }
    }
  } while (p != NULL);
  if (pwbuf - wbuf > 1) {
    wbuf[0] = '{';
    pwbuf[0] = 0;
    pwbuf[-1] = '}';
    uint16 cmd = 0;    
    zb_ReceiveDataIndication( 0, cmd, pwbuf-wbuf, (uint8 *)wbuf );
  }
}
/*********************************************************************************************
* ���ƣ�process_command_callback()
* ���ܣ�������յ���ָ��
* ������ 
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
int process_command_callback(char *ptag, char *pval, char *pout)
{ 
  int val;
  int ret = 0;
  
  val = atoi(pval);
  if (0 == strcmp("ECHO", ptag)) {
    ret = sprintf(pout, "ECHO=%s",pval);
  } else  
  if (0 == strcmp("PANID", ptag)) { 
    if (0 == strcmp("?", pval)) {
      uint16 tmp16;
      osal_nv_read( ZCD_NV_PANID, 0, sizeof( tmp16 ), &tmp16 );
      ret = sprintf(pout, "PANID=%u", tmp16);
    } else {
      uint8 startOptions = ZCD_STARTOPT_DEFAULT_NETWORK_STATE;
      uint16 tmp16;
      osal_nv_read( ZCD_NV_PANID, 0, sizeof( tmp16 ), &tmp16 );
      uint16 xval = atoi(pval);
      if (tmp16 != xval) {
        osal_nv_write(ZCD_NV_PANID, 0, osal_nv_item_len( ZCD_NV_PANID ), &xval);
        zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions ); //�������״̬�����ı�
      }
    } 
  }
  if (0 == strcmp("CHANNEL", ptag)) { 
    static uint32 chs[] = {0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000,
        0x00010000, 0x00020000, 0x00040000,0x00080000,0x00100000,0x00200000,
        0x00400000,0x00800000,0x01000000,0x02000000,0x04000000}; 
    if (0 == strcmp("?", pval)) {
      uint32 tmp32;
      uint8 i;
      osal_nv_read( ZCD_NV_CHANLIST, 0, sizeof( tmp32 ), &tmp32 );
     
      for (i=0; i<16; i++) {
        if (tmp32 == chs[i]) break;
      }
      i += 11;
      ret = sprintf(pout, "CHANNEL=%u", i);
    } else {
      uint32 tmp32, t32;
      uint8 startOptions = ZCD_STARTOPT_DEFAULT_NETWORK_STATE;
      tmp32 = val - 11;
      osal_nv_read( ZCD_NV_CHANLIST, 0, sizeof( tmp32 ), &t32 );
      if (tmp32 < 16) {
        if (t32 != chs[tmp32]) {
          osal_nv_write(ZCD_NV_CHANLIST, 0, osal_nv_item_len( ZCD_NV_CHANLIST ), &chs[tmp32]);
          zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions ); //�������״̬�����ı�
        }
      }
    }
  }
  /*if (0 == strcmp("RSSI", ptag)) {
    if (0 == strcmp("?", pval)) {
      ret = sprintf(pout, "RSSI=%d", rssi);
    }
  }*/
  if (0 == strcmp("TYPE", ptag)) {
    if (0 == strcmp("?", pval)) {
      ret = sprintf(pout, "TYPE=%d%d%s", NODE_CATEGORY, logicalType, NODE_NAME);
    }
  }
  if (0 == strcmp("TPN", ptag)) { 
    /*  ������ʽ x/y  ��ʾ��y�������ϱ�x������ 
     *  x = 0 ֹͣ�ϱ�,
     *  ����ÿ��������ϱ�6��,�����ϱ�1��
    */
    char *s = strchr(pval, '/');
    if (s != NULL) {
      int v1, v2;
      
      *s = 0;
      v1 = atoi(pval);
      v2 = atoi(s+1);
      
      if (v1 > 0 && v2 > 0) {
        _tm_delay = v2*60/v1;
        if (_tm_delay >= 10 && _tm_delay <= 65) {
          if (_tm_cnt == 0) {
            // start timer
            osal_start_timerEx( sapi_TaskID, __REPORT_EVT, (osal_rand()%_tm_delay) * 1000);
          } 
          _tm_cnt = v1;
        }
      }
    }
  } //TMAN
  return ret;
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
* ���ƣ�my_report_proc()
* ���ܣ��ڵ�������Ϣ�ϱ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void my_report_proc(void)
{
  sprintf(wbuf, "{PN=");
  //read_al(wbuf+strlen(wbuf), -1);
  read_nb(wbuf+strlen(wbuf), -1);
  if (strlen(wbuf) == 4) {
    sprintf(wbuf+4, "NULL");
  } 
  sprintf(wbuf+strlen(wbuf), ",TYPE=%d%d%s}", NODE_CATEGORY, logicalType, NODE_NAME);
  zb_ReceiveDataIndication(0/*source*/, 0/*cmd*/, strlen(wbuf), (uint8*)wbuf);
}