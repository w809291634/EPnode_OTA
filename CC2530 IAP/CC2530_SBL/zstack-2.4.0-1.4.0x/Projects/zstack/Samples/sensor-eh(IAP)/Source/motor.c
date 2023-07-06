#include "iocc2530.h"

void motorInit(void)
{
  P0SEL &= ~0x03;                                               //设置P0_0/P0_1为普通IO模式
  P0DIR |= 0x03;   
  P0_0 = 0;
  P0_1 = 0;
}

void motorSet(int st)
{
  if (st == 0) {
    P0_0 = 0;
    P0_1 = 0;
  }
  if (st == 1) {
    P0_1 = 0;
    P0_0 = 1;
  } 
  if (st == 2) {
    P0_0 = 0;
    P0_1 = 1;
  }
}