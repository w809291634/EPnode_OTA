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

/*********************************************************************************************
* 名称：xtal_init
* 功能：系统时钟初始化
* 参数：无
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void xtal_init(void);

/*********************************************************************************************
* 名称：delay_ms
* 功能：延时函数，ms
* 参数：t:延时时间
* 返回：无
* 修改：
* 注释：
*********************************************************************************************/
void delay_ms(u16 t);

#endif
