#include "sys.h"
#include "iap_config.h"

// ��ʼ����������
void keyInit(void)
{
  P1SEL &= ~0x0C;       //����P1�˿ڵ�2��3ΪIOģʽ
  P1DIR &= ~0x0C;       //����P1�˿ڵ�2��3Ϊ����ģʽ
}

// ��ʼ��LED
void led_init(void)
{
    P1SEL &= ~0x03;          //P1.0 P1.1Ϊ��ͨ I/O ��
    P1DIR |= 0x03;           //���
    
    LED2 = 1;                //��LED
    LED1 = 1;
}

void key_led_uninit(void)
{
    P1SEL &= ~0x03;          
    P1DIR |= 0x03;        
}

// LED��APP
void led_app(void)
{
  static uint32 times=0;
  if(times>3000){
    times=0;
    LED1=!LED1;
  }
  times++;
}

// �ȴ���ѹ
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

//���ܣ���ʱ������ms
//������wait:��ʱʱ��,�����ʱ255ms
//���أ���
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

// ��ʱ����
void delay_ms(u16 t)
{
  while(t--)
  {
    halWait(1);
  }
}

// дϵͳ����
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
