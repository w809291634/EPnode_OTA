#include "time.h"
#include "soft_timer.h"

// 定时器1中断初始化
// 500 就是1ms
void time1Int_init(u16 timeout)
{
  T1CTL |= (1<<1);          //模计数，0--T1CC0
  T1CTL |= (1<<3);          //32分频
  
  T1CC0L = timeout&0xff;
  T1CC0H = (timeout>>8)&0xff;
  T1CCTL0 |= (1<<2);        //定时器设为比较模式

  //设置中断优先级最低
  IP0 &= ~(1<<1);
  IP1 &= ~(1<<1);
  
  IEN1 |= 0X02;             //定时器1中断使能
  EA=1;                     //开总中断 
}

// 定时器1中断服务程序
#pragma vector = T1_VECTOR      
__interrupt void T1_ISR(void)            
{  
  tickCnt_Update();
  T1IF=0;                                                       // 清除定时器中断标志位
}