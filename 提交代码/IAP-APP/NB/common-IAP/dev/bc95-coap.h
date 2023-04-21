#ifndef __BC95_H__
#define __BC95_H__

void bc95_coap_init(char *ip);
int bc95_coap_send(char *dat, int len);
void bc95_coap_register_on_data_call(void (*fun)(char *dat, int len));
void bc95_coap_on_data_notify(char *p, int len);
#endif
