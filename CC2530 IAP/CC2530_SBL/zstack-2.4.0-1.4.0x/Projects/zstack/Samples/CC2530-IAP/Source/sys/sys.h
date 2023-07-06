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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hal_board_cfg.h"
#include "hal_adc.h"
#include "hal_dma.h"
#include "hal_flash.h"
#include "hal_types.h"

/*������������*/
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed          long int32_t;
typedef   signed     long long int64_t;

typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           long uint32_t;
typedef unsigned      long long uint64_t;

typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned long 		u32;
typedef unsigned long long 	u64;

typedef signed char 			s8;
typedef signed short 			s16;
typedef signed long 			s32;
typedef signed long long 		s64;

#define SB1_PRESS  (P1_2 == 0)
#define SB2_PRESS  (P1_3 == 0)

#define LED2    P1_0                      //����LED2ΪP1_0�ڿ���
#define LED1    P1_1                      //����LED1ΪP1_1�ڿ���
#define CLKSPEED  ( CLKCONCMD & 0x07 )    //getting the clock division factor

void vddWait(uint8 vdd);
void delay_ms(u16 t);
void keyInit(void);
void led_init(void);
void led_app(void);
void write_sys_parameter();
#endif
