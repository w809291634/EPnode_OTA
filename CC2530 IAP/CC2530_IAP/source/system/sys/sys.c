#include "sys.h"
#include "soft_timer.h"
#include "time.h"

// 功能：系统时钟初始化
void xtal_init(void)
{
  SLEEPCMD &= ~0x04;              //都上电
  while(!(CLKCONSTA & 0x40));     //晶体振荡器开启且稳定
  CLKCONCMD &= ~0x7F;             //选择32MHz晶体振荡器 并将TICKSPD设置为000
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

//获取系统的运行时间，单位ms
uint32_t millis(void)
{
  return (tickCnt_Get()*(1000/TICK_PER_SECOND));
}

// 毫秒延时
void hw_ms_delay(unsigned int ms)
{
  uint32_t start_ms= tickCnt_Get();
  while(tickCnt_Get()- start_ms <ms);
}

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  printk("\r\nWrong parameters value: file %s on line %d\r\n", file, line);
  
  /* Infinite loop */
  while (1)
  {
  }
}