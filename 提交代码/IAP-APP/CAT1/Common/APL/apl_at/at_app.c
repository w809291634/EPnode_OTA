#include <rtthread.h>
#include <rthw.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bsp_flash/flash.h"
#include "at_hw.h"
#include "at_app.h"
#include "at_thread.h"
#include "rf_thread.h"
#include "apl_rf/cat1/cat1_drv.h"

static uint8_t at_echo = 1;    //ATָ�����

#define ATOK()  at_uart_send("OK\r\n")
static uint16_t at_analyses(char *pat) {
  extern t_rf_info rf_info;
  uint16_t res = 0;
  char reply[50];
  if(strcmp(pat, "AT") == 0) {
    ATOK();
  } else if(strcmp(pat, "ATE0") == 0) {
    at_echo = 0;
    at_uart_send("ATE0\r\n");
    ATOK();
  } else if(strcmp(pat, "ATE1") == 0) {
    at_echo = 1;
    ATOK();
  } else if(strcmp(pat, "AT+HW?") == 0) {
    at_uart_send("+HW:CAT1\r\n");
    ATOK();
  } else if(strcmp(pat, "AT+LINK?") == 0) {
    extern uint8_t rf_Link_On;
    sprintf(reply, "+LINK:%u\r\n", rf_Link_On);
    at_uart_send(reply);
    ATOK();
  } else if(strcmp(pat, "AT+NODEID?") == 0) {
    sprintf(reply, "+NODEID:%s\r\n", rf_info.mac);
    at_uart_send(reply);
    ATOK();
  } else if(strncmp(pat, "AT+AID", 6) == 0) {
    if(pat[6] == '?') {
      sprintf(reply, "+AID:%s\r\n", rf_info.zhiyun_id);
      at_uart_send(reply);
    } else if(pat[6] == '=') {
      strncpy(rf_info.zhiyun_id, &pat[7], sizeof(rf_info.zhiyun_id));
    }
    ATOK();
  } else if(strncmp(pat, "AT+AKEY", 7) == 0) {
    if(pat[7] == '?') {
      sprintf(reply, "+AKEY:\"%s\"\r\n", rf_info.zhiyun_key);
      at_uart_send(reply);
    } else if(pat[7] == '=' && pat[8] == '"' && pat[strlen(pat)-1] == '"') {
      pat[strlen(pat)-1] = 0;
      strncpy(rf_info.zhiyun_key, &pat[9], sizeof(rf_info.zhiyun_key));
    }
    ATOK();
  } else if(strncmp(pat, "AT+SIP", 6) == 0) {
    if(pat[6] == '?') {
      if(rf_info.port != ZHIYUN_DEFAULT_PORT) {
        sprintf(reply, "+SIP:%s,%u\r\n", rf_info.ip, rf_info.port);
      } else {
        sprintf(reply, "+SIP:%s\r\n", rf_info.ip);
      }
      at_uart_send(reply);
    } else if(pat[6] == '=') {
      char *pcomma = strchr(pat, ',');
      if(pcomma != NULL) {
        *pcomma = 0;
        rf_info.port = atoi(pcomma+1);
      }
      strncpy(rf_info.ip, &pat[7], sizeof(rf_info.ip));
    }
    ATOK();
  } 
#ifdef CONFIG_AT_IAP
  else if (strncmp(pat, "AT+IAP=", 7)==0){
    ATOK();
    if(strncmp(pat+7, "IAP", 3)==0) {
      rf_info.boot_mode = BOOT_IAP;
    } else if(strncmp(pat+7, "APP", 3)==0) {
      rf_info.boot_mode = BOOT_APP;
    }
    rf_info.app_en = APP_OK;
    flash_write(FLASH_PARAM_START_ADDR, (uint16_t *)(&rf_info), sizeof(t_rf_info) / 2);
    rt_thread_mdelay(100);
    //�ر������ж�
    rt_base_t level;
    level=rt_hw_interrupt_disable();
    SysTick->CTRL = 0;
    SysTick->VAL  = 0;
    rt_hw_interrupt_enable(level);
    NVIC_DisableIRQ(EXTI3_IRQn);
    NVIC_DisableIRQ(EXTI4_IRQn);
    NVIC_DisableIRQ(EXTI9_5_IRQn);
    NVIC_DisableIRQ(USART1_IRQn);
    NVIC_DisableIRQ(USART2_IRQn);
    NVIC_DisableIRQ(USART3_IRQn);

    //��λ���г�ʼ����������
    EXTI_DeInit();
    USART_DeInit(USART1);
    USART_DeInit(USART2);
    USART_DeInit(USART3);
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_AFIODeInit();
    //��λʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
    RCC_DeInit();

    //ϵͳ��λ
    typedef  void (*pFunction)(void);
    pFunction Jump_To_Application; 
    unsigned JumpAddress;	
    uint32_t programe_addr = 0x08000000;

    JumpAddress = *(volatile unsigned*) (programe_addr + 4);
    Jump_To_Application = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_PSP(*(volatile unsigned*) programe_addr);
    __set_CONTROL(0);
    __set_MSP(*(volatile unsigned*) programe_addr);
    Jump_To_Application();
  }
#endif 
  else if(strcmp(pat, "AT+ENVSAVE") == 0) {
    flash_write(FLASH_PARAM_START_ADDR, (uint16_t *)(&rf_info), sizeof(t_rf_info) / 2);
    ATOK();
  } else if(strcmp(pat, "AT+RESET") == 0) {
    ATOK();
    cat1_reset();
    NVIC_SystemReset();
    while(1);
  } else if(strncmp(pat, "AT+SEND=", 8) == 0) {
    res = atoi(&pat[8]);
    if(res == 0 || res >= 256) {
      res = 0;
      at_uart_send("ERR: Bad command\r\n");
    }
  } else {
    at_uart_send("ERR: Bad command\r\n");
  }
  return res;
}

/* Ӧ������ */
void at_run(void) {
  static uint8_t send_mode = 0;     //����ģʽ��ƽʱΪ0�����յ�AT+SEND�����Ϊ1
  static uint16_t send_length = 0;  //���ͳ���
  static char send_buf[256];        //���ͻ���
  rt_err_t err;
  char recv[201];
  int16_t length = 0;
  uint32_t mb_recv;
  
  err = rt_mb_recv(&at_data_mb, (rt_ubase_t*)&mb_recv, RT_WAITING_FOREVER);
  if(err == RT_EOK) {
    if(mb_recv == 1) {
      length = at_uart_read((uint8_t *)recv, 200, 0);
      recv[length] = 0;
      if(send_mode == 0) {          //����ATģʽ
        if(length <= 2 || recv[length-2] != '\r' || recv[length-1] != '\n') return;
        if(at_echo == 1) at_uart_send(recv);    //����
        //AT��������봦��
        recv[length - 2] = 0;   //��ȡ\r\n֮ǰ������
        send_length = at_analyses(recv);
        if(send_length > 0) {
          send_buf[0] = 0;
          send_mode = 1;
          at_uart_send(">");
        }
      } else if(send_mode == 1) {   //���ݷ���
        strcat(send_buf, recv);
        if(strlen(send_buf) >= send_length) {
          length = rf_send((uint8_t *)send_buf, send_length, 0);
          ATOK();
          if(length < 0) {
            at_uart_send("+DATASEND:Error!\r\n");
          } else {
            sprintf(send_buf, "+DATASEND:%d\r\n", length);
            at_uart_send(send_buf);
          }
          send_mode = 0;
        }
      }
    }
  }
}

/* Ӧ�ó�ʼ�� */
void at_init(void) {
  at_HW_Init();
}

/* ���յ�ͨѶģ�����ݺ󣬶�AT���ڷ���+RECV���� */
void at_recv(char *pdata) {
  extern int cat1_rssi;
  char cmd[40];
  sprintf(cmd, "+RECV:%u,%d\r\n", strlen(pdata), cat1_rssi);
  at_uart_send(cmd);
  at_uart_send(pdata);
}
