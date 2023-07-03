/*********************************************************************************************
* �ļ���sys.h
* ���ߣ�fuyou
* ˵����sysͷ�ļ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
#ifndef _sys_h_
#define _sys_h_


#include "ioCC2530.h"
#include "stdio.h"
#include <string.h>
#include "math.h"

/*������������*/
typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned long 		u32;
typedef unsigned long long 	u64;

typedef signed char 			s8;
typedef signed short 			s16;
typedef signed long 			s32;
typedef signed long long 		s64;

#define CLKSPD  ( CLKCONCMD & 0x07 )    //getting the clock division factor

/*********************************************************************************************
* ���ƣ�xtal_init
* ���ܣ�ϵͳʱ�ӳ�ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void xtal_init(void);

/*********************************************************************************************
* ���ƣ�delay_ms
* ���ܣ���ʱ������ms
* ������t:��ʱʱ��
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void delay_ms(u16 t);

#endif
