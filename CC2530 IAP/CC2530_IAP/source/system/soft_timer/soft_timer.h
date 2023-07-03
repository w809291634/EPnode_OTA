#ifndef __SOFT_TIMER_H__
#define __SOFT_TIMER_H__
#include "sys.h"
#include "stm32f4xx_conf.h"
#include "stdio.h"

/*****************************************************
* ��ʹ�ö�ʱ��ID
******************************************************/
#define ESP32_TIMEOUT_TIMER_ID    0
#define LED_APP_TIMER_ID          1




#define TEST_TIMER_ID             9

typedef void TIMER_CALLBACK(void);
 
typedef struct softTimer
{
    uint8_t state;          //״̬
    uint8_t mode;           //ģʽ
    uint32_t match;         //����ʱ��
    uint32_t period;        //��ʱ����
    TIMER_CALLBACK *cb;     //�ص�����ָ��
    void *argv;             //����ָ��
    uint16_t argc;          //��������
}softTimer;
 
typedef enum tmrState
{
    SOFT_TIMER_STOPPED = 0,  //ֹͣ
    SOFT_TIMER_RUNNING,      //����
    SOFT_TIMER_TIMEOUT       //��ʱ
}tmrState;
 
typedef enum tmrMode
{
    MODE_ONE_SHOT = 0,       //����ģʽ
    MODE_PERIODIC,           //����ģʽ
}tmrMode;
 
void tickCnt_Update(void);
uint32_t tickCnt_Get(void);
void softTimer_Init(void);
void soft_timers_init(void);
uint8_t softTimer_GetState(uint16_t id);
void softTimer_create(uint16_t id, tmrMode mode, TIMER_CALLBACK *cb);
void softTimer_start(uint16_t id, uint32_t delay);
void softTimer_stop(uint16_t id);
void softTimer_Update(void);
#endif
