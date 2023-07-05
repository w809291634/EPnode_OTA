/*********************************************************************************************
* �ļ���zxbee-inf.c
* ���ߣ�Xuzhy 2018.5.16
* ˵����ZXBeeͨ��Э�����ݰ��շ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include<string.h>
#include "hal_types.h"
#include "AppCommon.h"
#include "hal_led.h"
#include "ZDApp.h"
#include "zxbee.h"
#include "sapi.h"
#include "stdio.h"
#include "zxbee-inf.h"
/*********************************************************************************************
* �궨��
*********************************************************************************************/
#define DEBUG 0
#if DEBUG
#define Debug   printf
#else
#define Debug(...)
#endif
/*********************************************************************************************
* ���ƣ�ZXBeeInfInit()
* ���ܣ�ZXBee�ӿڳ�ʼ��
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ZXBeeInfInit(void)
{

}

void ZXBeeSendConfirm(uint8 h, uint8 st)
{
  uint8 GetLinkStatus(void);
  static int8 txError = 0;
  if (h == 0xaa){
    if (GetLinkStatus()) {
      if (st == 0) {
        txError = 0;
      } else  txError += 1;
      
      if (txError >= 5) {
        void myReset(void);
        //zb_SystemReset();
        myReset();
      }
    }
  }
}
/*********************************************************************************************
* ���ƣ�ZXBeeInfSend()
* ���ܣ��ڵ㷢���������ݰ����㼯�ڵ�
* ������*p -- Ҫ���͵��������ݰ�
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ZXBeeInfSend(char *p, int len)
{
    HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
    HalLedSet( HAL_LED_1, HAL_LED_MODE_BLINK );
#if DEBUG
    Debug("Debug send:");
    for (int i=0; i<len; i++) {
      Debug("%c", p[i]);
    }
    Debug("\r\n");
#endif
    zb_SendDataRequest( 0, 0, len, (uint8*)p, 0xaa, AF_ACK_REQUEST, AF_DEFAULT_RADIUS );
}
/*********************************************************************************************
* ���ƣ�ZXBeeInfRecv()
* ���ܣ��ڵ��յ��������ݰ�
* ������*pkg -- �յ����������ݰ�
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void ZXBeeInfRecv(char *pkg, int len)
{
  char *p = ZXBeeDecodePackage(pkg, len);
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
}
