/*********************************************************************************************
* �ļ���at.c
* ���ߣ�Xuzhy 2018.5.16
* ˵�����ڵ㴮��atָ��
* �޸ģ�fuyou ����͸������
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "ZComDef.h"
#include "sapi.h"
#include "hal_uart.h"
#include "hal_led.h"
#include "at.h"
#include "AppCommon.h"
#include "at-uart.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define AT_BUFF_SIZE    96                                      //AT���泤��
#define AT_BUFF_NUM     2                                       //AT�������
/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/
static uint8 __AT_EVT;                                          //AT�ӿ��¼�
static char at_echo = 0;
static int at_datalen = 0;                                     // ָʾ���ڽ��շ��͵����ݳ���
static int at_recvdata = 0;
static char atbuff[AT_BUFF_NUM][AT_BUFF_SIZE];                  //ATָ����ջ���
static char bufferbit = 0;
static char *pAtCommand = NULL;                                 // ��Ž��յ���atָ�������
/*********************************************************************************************
* ����ԭ��˵��
*********************************************************************************************/
static char* at_quebuffer_get(void);
static void at_quebuffer_put(char *buf);
static int _at_get_ch(char ch); 
uint8 _get_at_event(void);                                      // at �¼���ʶ
void at_recv_ch(char ch);
int8 getLastPkgRssi(void);
/*********************************************************************************************
* ���ƣ�at_quebuffer_put()
* ���ܣ�at�����ͷ�
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_quebuffer_put(char *buf)
{
  for (int i=0; i<AT_BUFF_NUM; i++) {
    if (atbuff[i] == buf){
      bufferbit &= ~(1<<i);
      return;
    }
  }
}
/*********************************************************************************************
* ���ƣ�at_quebuffer_get()
* ���ܣ������ȡ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
char* at_quebuffer_get(void)
{
  for (int i=0; i<AT_BUFF_NUM; i++) {
    if (((bufferbit>>i) & 1) == 0) {
      bufferbit |= 1<<i;
        return atbuff[i];
    }
  }
  return NULL;
}
/*********************************************************************************************
* ���ƣ�at_put_ch()
* ���ܣ�ͨ��at�˿ڷ���һ���ַ�
* ������ch: �����͵��ַ�
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static void _at_put_ch(uint8 ch)
{
  at_uart_write(ch);
}
/*********************************************************************************************
* ���ƣ�_at_get_ch()
* ���ܣ�����at�ӿڽ��յ�������
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
static int _at_get_ch(char ch)
{
  static char idx = 0;
  static char *pbuf = NULL;
  if (pbuf == NULL) {
    pbuf = at_quebuffer_get();
    if (pbuf == NULL) { 
      return -1;
    }
  }
  if (at_datalen == 0) {
    if (at_echo) {
      _at_put_ch(ch);
    }
    if (idx < AT_BUFF_SIZE-1) {
      pbuf[idx++] = ch;
      if (idx >= 2 && pbuf[idx-2]=='\r' && pbuf[idx-1]=='\n'){
        idx -= 2;
        pbuf[idx] = '\0';
        if (idx > 0) {
          /* atָ�������� */
          if (pAtCommand == NULL) {
            pAtCommand = pbuf;
            osal_set_event(sapi_TaskID, __AT_EVT);
          } else {
            at_quebuffer_put(pbuf); //��������
          }
           
          pbuf = NULL;
          idx = 0;
        } else {
           //��������
          idx = 0;
        }
      }
    } else {
      /*�������������������*/
      idx = 0;
    }
  } else {
    pbuf[at_recvdata++] = ch;
    if (at_recvdata == at_datalen) {
      /*���ݽ������*/
      if (pAtCommand == NULL) {
        pAtCommand = pbuf;
        osal_set_event(sapi_TaskID, __AT_EVT);
      }else {
         at_quebuffer_put(pbuf); //��������
         at_recvdata = 0;
      }
      at_datalen = 0;
      pbuf = NULL;
    }
  }
  return idx;
}
/*********************************************************************************************
* ���ƣ�at_response_buf()
* ���ܣ�ͨ��at�ӿڷ���һ������
* ������s:���ݿ�ʼ��ַ
*       len�����ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_response_buf(char *s, int len)
{
  for (int i=0; i<len; i++) {
    _at_put_ch(s[i]);
  }
}
/*********************************************************************************************
* ���ƣ�at_response()
* ���ܣ�ͨ��at�ӿڷ����ַ���
* ������s:�����͵��ַ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_response(char *s)
{
  at_response_buf(s, strlen(s));
}
/*********************************************************************************************
* ���ƣ�at_notify_data()
* ���ܣ�at�ӿڽ��յ�����֪ͨ
* ������buf:���յ�������
*       len:���յ����ݳ���
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_notify_data(char *buf, int len)
{
  char sc[16];
#ifdef CC2530_Serial
  sprintf(sc, "+RECV:%u,%d\r\n", len, getLastPkgRssi());
#else
  sprintf(sc, "+RECV:%u\r\n", len);
#endif
  at_response(sc);
  at_response_buf(buf, len);
}
/*********************************************************************************************
* ���ƣ�at_proc()
* ���ܣ�����atָ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_proc(void)
{
  char buf[32];
  if (pAtCommand == NULL) 
    return;
  char *p_msg = pAtCommand;
  
  if (at_recvdata != 0) {
    /*����������*/
    at_response(ATOK);
   
    zb_SendDataRequest( 0, 0/*cmd*/, at_recvdata, (uint8*)p_msg, 0xaa, AF_ACK_REQUEST, AF_DEFAULT_RADIUS );
    HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
    HalLedSet( HAL_LED_1, HAL_LED_MODE_BLINK );
    
    sprintf(buf, "+SEND:%u\r\n", at_recvdata);
    at_response(buf);
    at_recvdata = 0;
  } else {
    int msg_size = strlen(p_msg);
    for (int i=0; i<msg_size; i++){
      if (p_msg[i] == '?' || p_msg[i] == '=') break;
      p_msg[i] = toupper(p_msg[i]);
    }
    
    if (strcmp(p_msg, "AT") == 0) {
      at_response(ATOK);
    }
    else if (strcmp(p_msg, "ATE1") == 0) {
      at_echo = 1;
      at_response(ATOK);
    }
    else if (strcmp(p_msg, "ATE0") == 0) {
      at_echo = 0;
      at_response(ATOK);
    }  
    else if (memcmp(p_msg, "AT+HW?", 7) == 0) {
      at_response("+HW:CC2530\r\n");
      at_response(ATOK);
    }
    else if (memcmp(p_msg, "AT+MAC?", 7) == 0) {
      uint8 *mac = (uint8 *)(P_INFOPAGE+HAL_INFOP_IEEE_OSET);
      sprintf(buf, "+MAC:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
              mac[7],mac[6],mac[5],mac[4],mac[3],mac[2],mac[1],mac[0]); 
      at_response(buf);
      at_response(ATOK);
    } 
    else if (memcmp(p_msg, "AT+PANID", 8) == 0) {
      if (p_msg[8] == '?') {
        sprintf(buf, "+PANID:%u\r\n", GetPanId());
        at_response(buf);
        at_response(ATOK);
      } else if (p_msg[8] == '=') {
        uint16 id = atoi(&p_msg[9]);
        SetPanId(id);
        at_response(ATOK);
      } else {
        at_response(ATERROR);
      }
    }
    else if (memcmp(p_msg, "AT+CHANNEL", 10) == 0) {
      if (p_msg[10] == '?') {
        sprintf(buf, "+CHANNEL:%u\r\n", GetChannel());
        at_response(buf);
        at_response(ATOK);
      } else if (p_msg[10] == '=') {
        uint16 id = atoi(&p_msg[11]);
        SetChannel(id);
        at_response(ATOK);
      } else {
        at_response(ATERROR);
      }
    }
    else if (memcmp(p_msg, "AT+LOGICALTYPE", 14) == 0) {
      if (p_msg[14] == '?') {
        sprintf(buf, "+LOGICALTYPE:%u\r\n", GetLogicalType());
        at_response(buf);
        at_response(ATOK);
      } else if (p_msg[14] == '=') {
        uint16 id = atoi(&p_msg[15]);
        SetLogicalType(id);
        at_response(ATOK);
      } else {
        at_response(ATERROR);
      }
    }
    else if (memcmp(p_msg, "AT+LINK?", 8) == 0) {
      sprintf(buf, "+LINK:%u\r\n", GetLinkStatus());
      at_response(buf);
      at_response(ATOK);
    }
    /*else if (memcmp(p_msg, "AT+ENVSAVE", 10) == 0) {
      //ucfg_save();
      at_response(ATOK);
    } */
    else if (memcmp(p_msg, "AT+RSSI?", 8) == 0) {
      
      sprintf(buf, "+RSSI:%d\r\n", getLastPkgRssi());
      at_response(buf);
      at_response(ATOK);
    }
    else if (memcmp(p_msg, "AT+LQI?", 7) == 0) {
      uint8 getLastPkgLQI(void);
      sprintf(buf, "+LQI:%d\r\n", getLastPkgLQI());
      at_response(buf);
      at_response(ATOK);
    }
    
    else if (memcmp(p_msg, "AT+RESET", 8)==0) {
       at_response(ATOK);
       zb_SystemReset();
    }
    else if (memcmp(p_msg, "AT+SEND=", 8) == 0) {
      int len = atoi(&p_msg[8]);
      if (len > 0 && len<=AT_BUFF_SIZE) {
        at_response(">");
        at_datalen = len;
      } else {
        at_response(ATERROR);
      }
    }
    else {
      if (user_at_proc(p_msg)<0) {
        at_response(ATERROR);
      }
    }
    
  }
  at_quebuffer_put(p_msg);
  pAtCommand = NULL;
}

/* �����ϱ�link status */
void AT_reportedLinkStatus()
{
    char buf[16]={0};
    sprintf(buf, "+LINK:%u\r\n", GetLinkStatus());
    at_response(buf);
}


/*********************************************************************************************
* ���ƣ�at_init()
* ���ܣ�ATָ���ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_init(void)
{
    __AT_EVT = _get_at_event();

    at_uart_init();
    at_uart_set_input_call(at_recv_ch);

    at_response("+HW:CC2530\r\n");
    at_response("+RDY\r\n");
}
/*********************************************************************************************
* ���ƣ�at_recv_ch()
* ���ܣ�at���յ��ַ�����
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void at_recv_ch(char ch)
{
  _at_get_ch(ch);
}
