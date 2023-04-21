/*********************************************************************************************
* 文件：bc95-udp.c
* 作者：xuzhy 2018.1.17
* 说明：bc95 udp管理模块
*       
*       
*       
* 修改：
* 注释：
*********************************************************************************************/

#include <contiki.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stimer.h>
#include "gsm.h"
#include "bc95-udp.h"

#define DEBUG 0

#if DEBUG
#define debug(x...) printf(x)
#else
#define debug(x...) do{}while(0)
#endif
void gsm_uart_write(char *buf, int len);
int gsm_uart_read_ch(void);

#define SUPPORT_SOCKET_NUM 32

static on_data_call_t on_data_calls[SUPPORT_SOCKET_NUM];

static void (*on_create_socket_call)(int) = NULL;

static void _on_create_udp_socket(char *rsp)
{
  int _socket_id = -1;
  if (rsp == NULL) {
    _socket_id = -2;
  } else
  if (memcmp(rsp, "ERROR", 5) == 0) {
    _socket_id = -3;
  } else {
    int i = atoi(rsp);
    if (i >= 0) {
      _socket_id = i;
    }
  }
  if (on_create_socket_call != NULL) {
    on_create_socket_call(_socket_id);
    on_create_socket_call = NULL;
  }
}
int bc95_create_udp_socket(int port, void (*f)(int))
{
  char buf[32];
  if (on_create_socket_call != NULL) {
    return -1;
  }
  on_create_socket_call = f;
  if (port == 0) {
    port = rand()%65535;
    if (port < 0) port = -port;
  }
  sprintf(buf, "AT+NSOCR=DGRAM,17,%d,1\r\n", port);
  _request_at_3(buf, 3000, _on_create_udp_socket);
  return 0;
}
/*
static char send_buf[512];
static int send_len = 0;

static void _on_send_call(char *rsp)
{
  char buf[4];
  for (int i=0; i<send_len; i++) {
    sprintf(buf, "%02X", send_buf[i]);
    gsm_uart_write(buf, 2);
    debug(buf);
  }
  gsm_uart_write("\r\n", 2);
  debug("\r\n");
  send_len = 0;
}
*/
uint8_t bc95_udp_send_flag = 0;
int bc95_udp_send(int sid, char *sip, int sport, char *dat, int len)
{
  static char buf[1024+64];
  
  if(sip==0 && bc95_udp_send_flag==1 && strlen(buf) > 0) {
    _request_at_3(buf, 3000, NULL);
    return 0;
  }

  //at+NSOST=0,119.23.162.168,40000,6,313233343536
  int rlen = sprintf(buf, "at+nsost=%d,%s,%d,%d,", sid, sip, sport, len);
  
  //debug("BC95 UDP <<< %s", buf);
 
  for (int i=0; i<len; i++) {
    rlen += sprintf(&buf[rlen], "%02X", dat[i]);
    
  }
  sprintf(&buf[rlen], "\r\n");
  bc95_udp_send_flag = 1;
  _request_at_3(buf, 3000, NULL);
  
  return len;
}

void bc95_udp_close(int sid)
{
  char buf[32];
  sprintf(buf, "at+nsocl=%d\r\n", sid);
  _request_at_3(buf, 1000, NULL);
}

static char HEX2I(char x)
{
  if (x>='0' && x<='9') return x - '0';
  else if (x>='a' && x<='f') return 10+(x-'a');
  else return 10+(x-'A');
}

static void _on_udp_data(char *rsp)
{
  if (rsp == NULL) {
  } else 
  if (memcmp(rsp, "ERROR", 5) == 0) {
  } else
  if (memcmp(rsp, "OK", 2) == 0) {
  } else {
    char *p = rsp;
    int sid = atoi(p);
    p = strchr(p, ',') + 1;
    char *rip = p;
    p = strchr(p, ',');
    *p = '\0'; p+=1;
    int port = atoi(p);
    p = strchr(p, ',') + 1;
    int len = atoi(p);
    p = strchr(p, ',') + 1;
    char *pleft = strchr(p, ',');
    int leftlen = atoi(pleft);
    
    for (int i=0; i<len; i++) {
      p[i] = HEX2I(p[i*2])<<4 | HEX2I(p[i*2+1]);
    }
    if (sid < SUPPORT_SOCKET_NUM && on_data_calls[sid] != NULL) {
      on_data_calls[sid](sid, rip, port, p, len);
    }
    /* 
    if (leftlen > 0) {
      void bc95_on_udp_data_notify(int sid, int len);
      bc95_on_udp_data_notify(sid, leftlen);
    }*/
  }
}
void bc95_on_udp_data_notify(int sid, int len)
{
  char buf[32];
  sprintf(buf, "at+nsorf=%d,%d\r\n", sid, len);
  _request_at_3(buf, 3000, _on_udp_data);
}
void bc95_register_on_data_call(int sid, on_data_call_t call)
{
  if (sid < SUPPORT_SOCKET_NUM) {
    on_data_calls[sid] = call;
  }
}
/*
static void _on_create_udp_socket(char *rsp);

static struct pt _pt_create_udp;
static int _socket_id = -1;

void bc95_create_udp_socket_init(void)
{
  _socket_id = -1;
  PT_INIT(&_pt_create_udp);
}
int bc95_get_udp_socket(void)
{
  return _socket_id;
}

PT_THREAD(bc95_create_udp_socket(int port))
{
  char buf[32];
  PT_BEGIN(&_pt_create_udp);
  if (port < 0) {
    PT_EXIT(&_pt_create_udp);
  }
  if (port == 0) {
    port = rand()%65535;
    if (port < 0) port = -port;
  }
  sprintf(buf, "AT+NSOCR=DGRAM,17,%d,1\r\n", port);
  _request_at_3(buf, 3000, _on_create_udp_socket);
  PT_YIELD_UNTIL(&_pt_create_udp, _socket_id!=-1);
  PT_EXIT(&_pt_create_udp);
  PT_END(&_pt_create_udp);
}
static void _on_create_udp_socket(char *rsp)
{
  if (rsp == NULL) {
    _socket_id = -2;
  } else
  if (memcmp(rsp, "ERROR", 5) == 0) {
    _socket_id = -3;
  } else {
    int i = atoi(rsp);
    if (i >= 0) {
      _socket_id = i;
    }
  }
}*/
