#include "sys.h"
#include "iap_config.h"

// 初始化两个按键
void keyInit(void)
{
  P1SEL &= ~0x0C;       //设置P1端口的2、3为IO模式
  P1DIR &= ~0x0C;       //设置P1端口的2、3为输入模式
}

// 初始化LED
void led_init(void)
{
    P1SEL &= ~0x03;          //P1.0 P1.1为普通 I/O 口
    P1DIR |= 0x03;           //输出
    
    LED2 = 1;                //关LED
    LED1 = 1;
}

void key_led_uninit(void)
{
    P1SEL &= ~0x03;          
    P1DIR |= 0x03;        
}

// LED的APP
void led_app(void)
{
  static uint32 times=0;
  if(times>3000){
    times=0;
    LED1=!LED1;
  }
  times++;
}

// 等待电压
void vddWait(uint8 vdd)
{
  uint8 cnt = 16;

  do {
    do {
      ADCCON3 = 0x0F;
      while (!(ADCCON1 & 0x80));
    } while (ADCH < vdd);
  } while (--cnt);
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

  largeWait = (largeWait >> CLKSPEED);
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

// 写系统参数
void write_sys_parameter()
{
  if(sizeof(sys_parameter)<=PARA_PARTITION_SIZE ){
    HalFlashErase(PARA_PARTITION_PAGE);
    SYS_PARAMETER_WRITE;
    debug_info(INFO"System Parameter Write Success!\r\n");
  }else{
    debug_err(ERR"System Parameter Write Failed!");
  }
}
