#include "sys.h"

// ���ܣ�ϵͳʱ�ӳ�ʼ��
void xtal_init(void)
{
  SLEEPCMD &= ~0x04;              //���ϵ�
  while(!(CLKCONSTA & 0x40));     //���������������ȶ�
  CLKCONCMD &= ~0x7F;             //ѡ��32MHz�������� ����TICKSPD����Ϊ000
  SLEEPCMD |= 0x04;
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

  largeWait = (largeWait >> CLKSPD);
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