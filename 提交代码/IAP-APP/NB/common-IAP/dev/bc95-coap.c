/*********************************************************************************************
* 文件：bc95-coap.c
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
//#include "bc95-udp.h"


static char sip[32];
static void (*on_data_call)(char *dat, int len);
/*
static char HEX2I(char x)
{
  if (x>='0' && x<='9') return x - '0';
  else if (x>='a' && x<='f') return 10+(x-'a');
  else return 10+(x-'A');
}

static void _on_nmgr_rsp(char *rsp)
{
  if (rsp == NULL) {
    return;
  }
  if (memcmp("rsp", "OK", 2) == 0) {
    return;
  }
  char *pdat = strchr(rsp, ',');
  if (pdat != NULL) {
    *pdat++ = '\0';
    int len = atoi(rsp);
    for (int i=0; i<len; i++) {
      pdat[i] = HEX2I(pdat[i*2])<<4 | HEX2I(pdat[i*2+1]);
    }
    if (on_data_call != NULL) {
      on_data_call(pdat, len);
    }
  }
}*/
void bc95_coap_on_data_notify(char *p, int len)
{
  if (on_data_call != NULL) {
      on_data_call(p, len);
  }
   //_request_at_3("at+nmgr\r\n", 1000, _on_nmgr_rsp);
}

int bc95_coap_send(char *dat, int len)
{
  static char buf[1024+64];

  int rlen = sprintf(buf, "at+nmgs=%d,",  len);
  
  for (int i=0; i<len; i++) {
    rlen += sprintf(&buf[rlen], "%02X", dat[i]);
    
  }
  sprintf(&buf[rlen], "\r\n");
  _request_at_3(buf, 5000, NULL);
  
  return len;
}

void bc95_coap_register_on_data_call(void (*fun)(char *dat, int len))
{
  on_data_call = fun;
}

static void _on_ncdp_rsp(char *rsp)
{
  //+NCDP:117.60.157.137,5683
  if (memcmp(rsp, "+NCDP:", 6) == 0 || memcmp(rsp, "ERROR", 5)==0) {
    char *ip = &rsp[6];
    char buf[64];
    if (memcmp(rsp, "ERROR", 5)==0 || strcmp(ip, sip) != 0) {
      sprintf(buf, "at+ncdp=%s\r\n", sip);
      _request_at_3(buf, 3000, NULL);
      
      _request_at_3("AT+S\r\n",3000, NULL);
      _request_at_3("AT+NRB\r\n",5000, NULL);
      _request_at_3("at\r\n", 3000, NULL);
      _request_at_3("at+nnmi=1\r\n", 3000, NULL);
      _request_at_3("at+cgatt=1\r\n", 3000, NULL);
    } 
  }
}
void bc95_coap_init(char *ip)
{
  strcpy(sip, ip);
  _request_at_3("at+nnmi=1\r\n", 3000, NULL);
  _request_at_3("at+ncdp?\r\n", 3000, _on_ncdp_rsp);
}