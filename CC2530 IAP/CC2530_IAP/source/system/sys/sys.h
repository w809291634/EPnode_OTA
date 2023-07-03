/*********************************************************************************************
* 文件：sys.h
* 作者：fuyou
* 说明：sys头文件
* 修改：
* 注释：
*********************************************************************************************/
#ifndef _sys_h_
#define _sys_h_
#include "ioCC2530.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "shell.h"

/*定义数据类型*/
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

#define CLKSPD  ( CLKCONCMD & 0x07 )    //getting the clock division factor

#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t* file, uint32_t line);
void xtal_init(void);
void delay_ms(u16 t);

#endif
