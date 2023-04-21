/*********************************************************************************************
* 文件：zhiyun.c
* 作者：xuzhy 
* 说明：
*       
*       
*      
* 修改：
* 注释：
*********************************************************************************************/
#include <contiki.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json/jsontree.h"
#include "json/jsonparse.h"
#include "gsm.h"
#include "bc95-coap.h"
#include "config.h"
#include "zxbee-inf.h"

#define DEBUG 0

#if DEBUG
#define Debug(x...) printf(x)
#else
#define Debug(x...) do{}while(0)
#endif

#define  COAP_SERVER_IP "119.23.162.168"   
#define  COAP_SERVER_PORT  5683

int _zxbee_send_call(char *pkg);

static char send_buf[512];
static struct etimer timer; //
extern nb_config_t  nbConfig;

void at_notify_data(char *buf, int len);


PROCESS(u_zhiyun_coap, "u_zhiyun_coap");


static int init = 0;
/*********************************************************************************************
* 名称：package_auth
* 功能：生成智云认证数据报
* 参数：
* 返回：认证数据报长度
* 修改：
* 注释：
*********************************************************************************************/
void zhiyun_auth_coap(void)
{
  int len = snprintf(send_buf, 512,
                "{\"method\":\"authenticate\",\"server\":\"%s:%d\",\"uid\":\"%s\",\"key\":\"%s\",\"addr\":\"NB:%s\",\"version\":\"0.1.0\", \"autodb\":true}",
       nbConfig.ip,nbConfig.port, nbConfig.id, nbConfig.key, gsm_info.imei);
       bc95_coap_send((char*)send_buf, len);
}
/*********************************************************************************************
* 名称：package_heartbeat
* 功能：生成心跳数据报
* 参数：
* 返回：心跳数据报长度
* 修改：
* 注释：
*********************************************************************************************/
void on_data_call_coap(char *buf, int len)
{
  buf[len] = 0;
  Debug("recv >>> %s\r\n", buf);
   
  if (strstr(buf, "method") && strstr(buf, "control")) {
    /*on zxbee data*/
    char *p = strstr(buf, "\"data\"");
    if (p != NULL) {
      p = strchr(p, ':');
      if (p != NULL) {
        p = strchr(p, '"');
        if (p != NULL) {
          p = p + 1;
          char *pe = strchr(p, '"');
          if (pe!=NULL) {
            *pe = '\0';
            at_notify_data(p, pe-p);
            ZXBeeInfRecv(p, pe-p);
          }
        }
      }
    }
    
  } else if (strstr(buf, "method") && strstr(buf, "authenticate_rsp")){
    if (strstr(buf, "error") && strstr(buf, "need authenticate")) {
       zhiyun_auth_coap();
    }
  }
}
/*********************************************************************************************
* 名称：zhiyun_send
* 功能：zxbee底层发送接口
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
int zhiyun_send_coap(char *pkg)
{
  if (init == 2) {
    
    int len = snprintf(send_buf, 512, "{\"method\":\"sensor\",\"data\":\"%s\", \"addr\":\"NB:%s\"}", pkg, gsm_info.imei);
    Debug("send <<< %s\r\n", send_buf);
    bc95_coap_send((char*)send_buf, len);
    return strlen(pkg);
  }
  return -1;
}


/*********************************************************************************************
* 名称：u_zhiyun
* 功能：nb模块与智云服务器交互线程
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
PROCESS_THREAD(u_zhiyun_coap, ev, data)
{
  
  PROCESS_BEGIN();
  
  etimer_set(&timer, CLOCK_SECOND);
  bc95_coap_register_on_data_call(on_data_call_coap);
  while (1) {
    PROCESS_YIELD();
    if (ev == PROCESS_EVENT_TIMER && etimer_expired(&timer)) {
      if (gsm_info.ppp_status != 2) {
        etimer_set(&timer, CLOCK_SECOND);
      } else {
        if (init == 0) {
          init = 1;
          char ip[32];
          sprintf(ip, "%s,%d", COAP_SERVER_IP,COAP_SERVER_PORT);
          bc95_coap_init(ip);          
        } else if (init == 1) {
          init = 2;
          zhiyun_auth_coap(); 
        }
        etimer_set(&timer, CLOCK_SECOND*10);
      }
    }
  }
  PROCESS_END();
}