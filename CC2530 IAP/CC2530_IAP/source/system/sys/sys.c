#include "sys.h"

// 功能：系统时钟初始化
void xtal_init(void)
{
  SLEEPCMD &= ~0x04;              //都上电
  while(!(CLKCONSTA & 0x40));     //晶体振荡器开启且稳定
  CLKCONCMD &= ~0x47;             //选择32MHz晶体振荡器
  SLEEPCMD |= 0x04;
}

//功能：延时函数，ms
//参数：wait:延时时间,最大延时255ms
//返回：无
void halWait(unsigned char wait)
{
  unsigned long largeWait;

  if(wait == 0)
  {return;}
  largeWait = ((unsigned short) (wait << 7));
  largeWait += 114*wait;

  largeWait = (largeWait >> CLKSPD);
  while(largeWait--);

  return;
}

// 延时函数
void delay_ms(u16 t)
{
  while(t--)
  {
    halWait(1);
  }
}