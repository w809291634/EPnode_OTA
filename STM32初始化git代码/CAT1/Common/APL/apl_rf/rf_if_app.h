#ifndef __RF_IF_APP_H__
#define __RF_IF_APP_H__


#include <stdint.h>

#define ZHIYUN_DEFAULT_IP   "api.zhiyun360.com"
#define ZHIYUN_DEFAULT_PORT 28082

typedef struct {
  char mac[24];                 //ģ��mac
  char zhiyun_id[40];           //����id
  char zhiyun_key[120];         //����key
  char ip[38];                  //����IP
  uint16_t port;                //����Port
} t_rf_info;

typedef enum {
  RF_IF_NULL=0, RF_IF_CAT1
} e_rf_if_dev;

extern e_rf_if_dev rf_if_dev;
extern char rf_if_imei[];

void rf_info_init(void);
void rf_if_run(void);
void rf_if_init(void);
int16_t rf_if_send(uint8_t *pdata, uint16_t length, uint16_t timeout_ms);
void rf_if_Link(void);
void rf_quit_trans(void);

#endif  //__RF_IF_APP_H__
