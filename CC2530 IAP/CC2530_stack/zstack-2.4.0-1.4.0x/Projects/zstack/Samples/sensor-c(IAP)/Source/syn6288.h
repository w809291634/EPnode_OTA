#ifndef __SYN6288_H__
#define __SYN6288_H__
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include <ioCC2530.h>

void syn6288_init();
int syn6288_busy(void);
void syn6288_play(char *s);
char *hex2unicode(char *str);
void syn6288_play_unicode(char *s,int len);

#endif
