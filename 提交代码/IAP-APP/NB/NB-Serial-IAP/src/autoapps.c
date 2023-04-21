#include <contiki.h>

PROCESS_NAME(gsm);
//PROCESS_NAME(u_zhiyun);
PROCESS_NAME(at);
struct process * const autostart_processes[] = {
  &gsm,
  //&u_zhiyun,
  &at,
  NULL
};
