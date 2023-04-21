/*********************************************************************************************
* 文件：zxbee-inf.c
* 作者：xuzhy 
* 说明：
*       
*       
*      
* 修改：
* 注释：
*********************************************************************************************/
#include "contiki.h"
#include <string.h>
#include "zxbee.h"
#include "zxbee-inf.h"
#include "dev/leds.h"
#include "config.h"

int zhiyun_send(char *pkg);
void clock_delay_ms(unsigned int ms);

int zhiyun_send_coap(char *pkg);
int zhiyun_send_udp(char *pkg);

#define COAP    0
#define UDP     1


PROCESS_NAME(u_zhiyun_coap);
PROCESS_NAME(u_zhiyun_udp);

/*********************************************************************************************
* 名称：ZXBeeInfInit
* 功能：ZXBee接口底层初始化
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ZXBeeInfInit(void)
{
  config_init();

  if (nbConfig.mode == 0) {
    process_start(&u_zhiyun_coap, NULL);
  } else {
     process_start(&u_zhiyun_udp, NULL);
  }
}

/*********************************************************************************************
* 名称：ZXBeeInfSend
* 功能：ZXBee底层发送接口
* 参数：p: ZXBee格式数据
       len：数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void  ZXBeeInfSend(char *p, int len)
{
  leds_on(1);
  clock_delay_ms(50);
  if (nbConfig.mode == COAP) {
    zhiyun_send_coap(p);
  } else {
    zhiyun_send_udp(p);
  }
  leds_off(1);
}
/*********************************************************************************************
* 名称：ZXBeeInfRecv
* 功能：接收到ZXbee数据报处理函数
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ZXBeeInfRecv(char *buf, int len)
{
  leds_on(1);
  clock_delay_ms(50);
  char *p = ZXBeeDecodePackage(buf, len);
  if (p != NULL) {
    ZXBeeInfSend(p, strlen(p));
  }
  leds_off(1);
}