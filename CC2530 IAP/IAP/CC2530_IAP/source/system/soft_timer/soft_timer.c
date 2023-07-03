#include "soft_timer.h"

/*****************************************************
* �û�����
* tickCnt_Update ������systick�ж���
* softTimer_Update �����ڴ�ѭ����
* softTimer_Init ������Ӳ����ʼ��
******************************************************/
/* �����ʱ������ */
#define TIMER_NUM           10

static volatile uint32_t tickCnt = 0;   //�����ʱ��ʱ�ӽ���
static softTimer timer[TIMER_NUM];      //�����ʱ������
 
/*****************************************************
* function: ����ʱ�ӽ���
* param:
* return:
* note:     ���ڶ�ʱ���ж���ִ��
******************************************************/
void tickCnt_Update(void)
{
    tickCnt++;
}
 
/*****************************************************
* function: ��ȡʱ�ӽ���
* param:
* return:   ʱ�ӽ���
* note:
******************************************************/
uint32_t tickCnt_Get(void)
{
  return tickCnt;
}
 
/*****************************************************
* function: �����ʱ����ʼ��
* param:
* return:
* note:
******************************************************/
void softTimer_Init(void)
{
    uint16_t i;
 
    for(i=0; i<TIMER_NUM; i++)
    {
        timer[i].state = SOFT_TIMER_STOPPED;
        timer[i].mode = MODE_ONE_SHOT;
        timer[i].match = 0;
        timer[i].period = 0;
        timer[i].cb = NULL;
        timer[i].argv = NULL;
        timer[i].argc = 0;
    }
}
 
/*****************************************************
* function: ��ȡ�����ʱ��״̬
* param:    �����ʱ��ID
* return:   ��ʱ��״̬
* note:
******************************************************/
uint8_t softTimer_GetState(uint16_t id)
{
    return timer[id].state;
}
 
void softTimer_create(uint16_t id, tmrMode mode, TIMER_CALLBACK *cb)
{
    assert_param(id < TIMER_NUM);
    assert_param(mode == MODE_ONE_SHOT || mode == MODE_PERIODIC);
    timer[id].state = SOFT_TIMER_STOPPED;
    timer[id].mode = mode;
    timer[id].cb = cb;
}
 
/*****************************************************
* function: ���������ʱ��
* param1:   �����ʱ��ID
* param2:   ��ʱ��ģʽ
* param3:   ��ʱʱ��(�����ڶ�ʱ������������ʱ��)����λΪ���ٸ�tick
* param4:   �ص�����ָ��
* param5:   �ص���������������ָ��
* param6:   �ص�������������������
* return:   
* note:     
******************************************************/
void softTimer_start(uint16_t id, uint32_t delay)
{
    assert_param(id < TIMER_NUM);
 
    timer[id].match = tickCnt_Get() + delay;
    timer[id].period = delay;
    timer[id].state = SOFT_TIMER_RUNNING;
}
 
/*****************************************************
* function: ֹͣ�����ʱ��
* param:    �����ʱ��ID
* return:   
* note:     
******************************************************/
void softTimer_stop(uint16_t id)
{
    assert_param(id < TIMER_NUM);
    timer[id].state = SOFT_TIMER_STOPPED;
}
 
/*****************************************************
* function: ���������ʱ��״̬
* param:    
* return:   
* note:     
******************************************************/
void softTimer_Update(void)
{
    uint16_t i;
 
    for(i=0; i<TIMER_NUM; i++)
    {
        switch (timer[i].state)
        {
        case SOFT_TIMER_STOPPED:
            break;
 
        case SOFT_TIMER_RUNNING:
            if(timer[i].match <= tickCnt_Get())
            {
                timer[i].state = SOFT_TIMER_TIMEOUT;
                timer[i].cb();       //ִ�лص�����
            }
            break;
 
        case SOFT_TIMER_TIMEOUT:
            if(timer[i].mode == MODE_ONE_SHOT)
            {
                timer[i].state = SOFT_TIMER_STOPPED;
            }
            else
            {
                timer[i].match = tickCnt_Get() + timer[i].period;
                timer[i].state = SOFT_TIMER_RUNNING;
            }
            break;
 
        default:
            //printf("timer[%d] state error!\r\n", i);
            break;
        }
    }
}
