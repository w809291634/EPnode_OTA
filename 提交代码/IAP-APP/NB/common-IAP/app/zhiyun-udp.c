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
#include "bc95-udp.h"
#include "config.h"
#include "zxbee-inf.h"

#define DEBUG 0

#if DEBUG
#define Debug(x...) printf(x)
#else
#define Debug(x...) do{}while(0)
#endif

#define UDP_SERVER_IP "119.23.162.168"
#define UDP_SERVER_PORT  40000

int _zxbee_send_call(char *pkg);

static char send_buf[512];
static struct etimer timer; //
extern nb_config_t  nbConfig;

void at_notify_data(char *buf, int len);
int zhiyun_send_udp(char *pkg);

PROCESS(u_zhiyun_udp, "u_zhiyun_udp");

static int socket_id = -1;

/*********************************************************************************************
* 名称：package_auth
* 功能：生成智云认证数据报
* 参数：
* 返回：认证数据报长度
* 修改：
* 注释：
*********************************************************************************************/
void zhiyun_auth_udp(void)
{
  char auth_send_buf[400];
  int len = snprintf(auth_send_buf, 400,
                     "{\"method\":\"authenticate\",\"server\":\"%s:%d\",\"uid\":\"%s\",\"key\":\"%s\",\"addr\":\"NB:%s\",\"version\":\"0.1.0\", \"autodb\":true}",
       nbConfig.ip,nbConfig.port, nbConfig.id, nbConfig.key, gsm_info.imei);
       bc95_udp_send(socket_id, UDP_SERVER_IP,
            UDP_SERVER_PORT,(char*)auth_send_buf, len);
}
/*********************************************************************************************
* 名称：package_heartbeat
* 功能：生成心跳数据报
* 参数：
* 返回：心跳数据报长度
* 修改：
* 注释：
*********************************************************************************************/
void on_data_call_udp(int sid, char *rip, int rport, char *buf, int len)
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
       zhiyun_auth_udp();
       zhiyun_send_udp(0); //
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
int zhiyun_send_udp(char *pkg)
{
  if(pkg == NULL) {
    bc95_udp_send(socket_id, UDP_SERVER_IP, UDP_SERVER_PORT,(char*)send_buf, strlen(send_buf));
    return 0;
  }
  
  if (socket_id >= 0) {
    
    int len = snprintf(send_buf, 512, "{\"method\":\"sensor\",\"data\":\"%s\", \"addr\":\"NB:%s\"}", pkg, gsm_info.imei);
    Debug("send <<< %s\r\n", send_buf);
    bc95_udp_send(socket_id, UDP_SERVER_IP,
                        UDP_SERVER_PORT,(char*)send_buf, len);
    return strlen(pkg);
  }
  return -1;
}

/*********************************************************************************************
* 名称：on_create_udp_socket
* 功能：udb套接口创建成功调用
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void on_create_udp_socket(int sk)
{
  extern uint8_t bc95_link_flag;
  if (sk >= 0) {
    socket_id = sk;
    //printf(" socket id %d \r\n", socket_id);
    bc95_register_on_data_call(socket_id, on_data_call_udp);
    
    zhiyun_auth_udp();
    bc95_link_flag = 1;
  }
}
/*********************************************************************************************
* 名称：u_zhiyun
* 功能：nb模块与智云服务器交互线程
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
PROCESS_THREAD(u_zhiyun_udp, ev, data)
{
  
  PROCESS_BEGIN();
  

  etimer_set(&timer, CLOCK_SECOND);
  while (1) {
    PROCESS_YIELD();
   
    if (ev == PROCESS_EVENT_TIMER && etimer_expired(&timer)) {
      if (gsm_info.ppp_status != 2) {
        etimer_set(&timer, CLOCK_SECOND);
      } else {
        /*
        bc95_create_udp_socket_init();
        while (PT_SCHEDULE(bc95_create_udp_socket(0))){
          process_poll(&u_zhiyun);
          PROCESS_YIELD();
        }
        int sid = bc95_get_udp_socket();
        */
        if (socket_id < 0) {
          bc95_create_udp_socket(0,on_create_udp_socket);
        } else {
          /*
          static int dat = 0;
          bc95_udp_send(socket_id, CONFIG_ZHIYUN_UDP_SERVER,
                        CONFIG_ZHIYUN_UDP_PORT,(char*)&dat, 4);
          dat += 1;
             */           
        }
        etimer_set(&timer, CLOCK_SECOND*10);
      }
    }
  }
  PROCESS_END();
}