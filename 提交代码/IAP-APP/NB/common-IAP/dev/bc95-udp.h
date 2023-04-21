#ifndef __BC95_UDP_H__
#define __BC95_UDP_H__

int bc95_create_udp_socket(int port, void (*f)(int));
int bc95_udp_send(int sid, char *sip, int sport, char *dat, int len);
void bc95_udp_close(int sid);
typedef void (*on_data_call_t)(int sid, char *rip, int rport, char *dat, int len);

void bc95_register_on_data_call(int sid, on_data_call_t call);

/*
void bc95_create_udp_socket_init(void);
PT_THREAD(bc95_create_udp_socket(int port));
int bc95_get_udp_socket(void);
*/

#endif
