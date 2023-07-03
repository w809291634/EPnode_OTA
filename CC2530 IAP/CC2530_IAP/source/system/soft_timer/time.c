#include "time.h"
#include "soft_timer.h"

// ��ʱ��1�жϳ�ʼ��
// 500 ����1ms
void time1Int_init(u16 timeout)
{
  T1CTL |= (1<<1);          //ģ������0--T1CC0
  T1CTL |= (1<<3);          //32��Ƶ
  
  T1CC0L = timeout&0xff;
  T1CC0H = (timeout>>8)&0xff;
  T1CCTL0 |= (1<<2);        //��ʱ����Ϊ�Ƚ�ģʽ

  //�����ж����ȼ����
  IP0 &= ~(1<<1);
  IP1 &= ~(1<<1);
  
  IEN1 |= 0X02;             //��ʱ��1�ж�ʹ��
  EA=1;                     //�����ж� 
}

// ��ʱ��1�жϷ������
#pragma vector = T1_VECTOR      
__interrupt void T1_ISR(void)            
{  
  tickCnt_Update();
  T1IF=0;                                                       // �����ʱ���жϱ�־λ
}