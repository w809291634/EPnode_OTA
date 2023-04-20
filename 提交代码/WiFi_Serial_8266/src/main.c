#include <rtthread.h>
#include "finsh_ex.h"
#include "apl_rf/rf_thread.h"
#include "apl_at/at_thread.h"
#include "drv_led/fml_led.h"
#include "rf_thread.h"
#include "sensor_thread.h"
#include "sensor/sensor.h"

/* 8266ͨѶ���ڳ�ʼ�� */
#define RF_IF_TX_CLK       RCC_APB2Periph_GPIOA
#define RF_IF_TX_PORT      GPIOA
#define RF_IF_TX_PIN       GPIO_Pin_9
//#define RF_IF_TX_AF        GPIO_AF_UART5
//#define RF_IF_TX_SOURCE    GPIO_PinSource12
#define RF_IF_RX_CLK       RCC_APB2Periph_GPIOA
#define RF_IF_RX_PORT      GPIOA
#define RF_IF_RX_PIN       GPIO_Pin_10
//#define RF_IF_RX_AF        GPIO_AF_UART5
//#define RF_IF_RX_SOURCE    GPIO_PinSource2
//�����豸
#define RF_IF_UART         USART1
#define RF_IF_UART_CLK     RCC_APB2Periph_USART1
#define RF_IF_UART_CLKInit RCC_APB2PeriphClockCmd
#define RF_IF_UART_IRQn    USART1_IRQn
#define RF_IF_UART_ISR     USART1_IRQHandler
//����DMA����
#define RF_IF_DMA_CLK      RCC_AHBPeriph_DMA1
#define RF_IF_DMA_CHANNEL  DMA1_Channel5

/* ͨѶ���ڳ�ʼ�� */
#define WIFI_SERIAL
#ifdef WIFI_SERIAL      /* ʹ�ô���2 ��ΪAT�ӿ�*/
  //����IO
  #define AT_TX_CLK       RCC_APB2Periph_GPIOA
  #define AT_TX_PORT      GPIOA
  #define AT_TX_PIN       GPIO_Pin_2
  #define AT_RX_CLK       RCC_APB2Periph_GPIOA
  #define AT_RX_PORT      GPIOA
  #define AT_RX_PIN       GPIO_Pin_3
  //�����豸
  #define AT_UART         USART2
  #define AT_UART_CLK     RCC_APB1Periph_USART2
  #define AT_UART_CLKInit RCC_APB1PeriphClockCmd
  #define AT_UART_IRQn    USART2_IRQn
  #define AT_UART_ISR     USART2_IRQHandler
  //����DMA����
  #define AT_DMA_CLK      RCC_AHBPeriph_DMA1
  #define AT_DMA_CHANNEL  DMA1_Channel6
#else                   /* ʹ�ô���3��liteb���Դ��ڣ� ��ΪAT�ӿ� */
  //����IO
  #define AT_TX_CLK       RCC_APB2Periph_GPIOB
  #define AT_TX_PORT      GPIOB
  #define AT_TX_PIN       GPIO_Pin_10
  #define AT_RX_CLK       RCC_APB2Periph_GPIOB
  #define AT_RX_PORT      GPIOB
  #define AT_RX_PIN       GPIO_Pin_11
  //�����豸
  #define AT_UART         USART3
  #define AT_UART_CLK     RCC_APB1Periph_USART3
  #define AT_UART_CLKInit RCC_APB1PeriphClockCmd
  #define AT_UART_IRQn    USART3_IRQn
  #define AT_UART_ISR     USART3_IRQHandler
  //����DMA����
  #define AT_DMA_CLK      RCC_AHBPeriph_DMA1
  #define AT_DMA_CHANNEL  DMA1_Channel3
#endif

#define BUF_LEN           1024
static unsigned char RX1BUF[BUF_LEN];
static unsigned char RXNBUF[BUF_LEN];

/* 8266ͨѶ���ڳ�ʼ�� */
static void rf_if_GPIO_Init(void) {
  GPIO_InitTypeDef	GPIO_InitStructure={0};
  
  RCC_APB2PeriphClockCmd(RF_IF_TX_CLK | RF_IF_RX_CLK, ENABLE);
  
  /* Configure USARTy Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = RF_IF_RX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(RF_IF_RX_PORT, &GPIO_InitStructure);
        
  /* Configure USARTx_Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = RF_IF_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(RF_IF_TX_PORT, &GPIO_InitStructure);
}

/* 8266ͨѶ���ڳ�ʼ�� */
static void rf_if_UART_Init(void) {
  USART_InitTypeDef USART_InitStructure={0};
//  NVIC_InitTypeDef NVIC_InitStructure;
    
  RF_IF_UART_CLKInit(RF_IF_UART_CLK, ENABLE);
  
  USART_DeInit(RF_IF_UART);
  USART_InitStructure.USART_BaudRate            = 115200;
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(RF_IF_UART, &USART_InitStructure);
  
//  NVIC_InitStructure.NVIC_IRQChannel = RF_IF_UART_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
  
//  USART_ClearFlag(RF_IF_UART, USART_FLAG_RXNE|USART_FLAG_IDLE);
//  USART_ITConfig(RF_IF_UART, USART_IT_IDLE, ENABLE);
  USART_DMACmd(RF_IF_UART, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(RF_IF_UART, ENABLE);
}

/* ���߽ӿ�1-IO��ʼ�� */
static void at_GPIO_Init(void) {
  GPIO_InitTypeDef	GPIO_InitStructure={0};
  
  RCC_APB2PeriphClockCmd(AT_TX_CLK | AT_RX_CLK, ENABLE);
  
  /* Configure USARTy Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = AT_RX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(AT_RX_PORT, &GPIO_InitStructure);
        
  /* Configure USARTx_Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = AT_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(AT_TX_PORT, &GPIO_InitStructure);
}

/* ���߽ӿ�1-���ڳ�ʼ�� */
static void at_UART_Init(void) {
  USART_InitTypeDef USART_InitStructure={0};
//  NVIC_InitTypeDef NVIC_InitStructure;
  AT_UART_CLKInit(AT_UART_CLK, ENABLE);
  
  USART_DeInit(AT_UART);
  USART_InitStructure.USART_BaudRate            = 115200;
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(AT_UART, &USART_InitStructure);
  
//  NVIC_InitStructure.NVIC_IRQChannel = AT_UART_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
  
//  USART_ClearFlag(AT_UART, USART_FLAG_RXNE|USART_FLAG_IDLE);
//  USART_ITConfig(AT_UART, USART_IT_IDLE, ENABLE);
  USART_DMACmd(AT_UART, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(AT_UART, ENABLE);
}

// ����1 PA9TX PA10  ��8266ͨѶ
// ����2 ͨѶ��͸������ PA2TX PA3
// ����3 ���Դ��� PB10 PB11

// ����1  --> RX F103 TX--> ����n 
// ����n  --> RX F103 TX--> ����1
static void app_main(void)
{
  DMA_InitTypeDef DMA_InitStructure={0};
  
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);                  //ʹ��DMA����

  DMA_DeInit(DMA1_Channel5);              // ����1RX
  DMA_DeInit(AT_DMA_CHANNEL);             // ����nRX
  
	DMA_InitStructure.DMA_BufferSize = BUF_LEN;                       //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;           //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;           
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;        
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                  
  // ����1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(USART1->DR);          //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&RX1BUF[0];                  //DMA�ڴ����ַ
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);
  // ����n
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(AT_UART->DR);           //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&RXNBUF[0];                  //DMA�ڴ����ַ
	DMA_Init(AT_DMA_CHANNEL, &DMA_InitStructure);
  
  DMA_Cmd(DMA1_Channel5 , ENABLE);
  DMA_Cmd(AT_DMA_CHANNEL , ENABLE);
  
  // 8266���ڳ�ʼ��
  rf_if_GPIO_Init();
  rf_if_UART_Init();
  // �ⲿͨ�Ŵ��ڳ�ʼ��
  at_GPIO_Init();
  at_UART_Init();
  
  unsigned int rf_r_index=0,rf_w_index=0,at_r_index=0,at_w_index=0;
  while(1) {
    rf_r_index = (BUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel5))%BUF_LEN;        // ��ȡ8266��������ǰ�洢����
    at_r_index = (BUF_LEN - DMA_GetCurrDataCounter(AT_DMA_CHANNEL))%BUF_LEN;
    
    // ��8266���͹��������� ת���� AT�˿ڣ����Դ��ڻ���͸��ͨѶ���ڣ�
    while(rf_w_index != rf_r_index) {
      USART_SendData(AT_UART, RX1BUF[rf_w_index]);
      while(USART_GetFlagStatus(AT_UART, USART_FLAG_TXE) == RESET);
      rf_w_index = (rf_w_index+1)%BUF_LEN;
    }
    // ��ͨѶ�������� ת���� 8266
    while(at_w_index != at_r_index) {
      USART_SendData(RF_IF_UART, RXNBUF[at_w_index]);
      while(USART_GetFlagStatus(RF_IF_UART, USART_FLAG_TXE) == RESET);
      at_w_index = (at_w_index+1)%BUF_LEN;
    }
  }
}

int main(void) {
//  sys_param_init();     //ϵͳ������ʼ��
//  led_Init();           //LED��ʼ��
////  sensor_thread_init(); //�������ɼ������߳�
//  rf_thread_init();     //ͨѶ�߳�
//  at_thread_init();     //ATָ����߳�
  app_main();
}

//static void sys_param_init(void) {
////  //��ʼ������ģ�����
////  rf_info_init();
//  //��ʼ�����в���
////  sensor_para_init();
//}