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
#include "apl_rf/wifi/wifi_drv.h"
#ifdef CONFIG_SENSOR_NODE
#include "sensor/sensor_node.h"
#include "at_common.h"
#else
#include "sensor/sensor.h"
#endif
#ifdef CONFIG_AT_RTT_DEBUG
#include "at_common.h"
#endif

static uint8_t at_echo = 1;    //AT指令回显

#define ATOK()  at_uart_send("OK\r\n")
static uint16_t at_analyses(char *pat) {
  extern t_rf_info rf_info;
  uint16_t res = 0;
  char reply[100];
#ifdef CONFIG_SENSOR_NODE  
  signed short temp;
#endif
  if(strcmp(pat, "AT") == 0) {
    ATOK();
  } 
  else if(strcmp(pat, "ATE0") == 0) {
    at_echo = 0;
    at_uart_send("ATE0\r\n");
    ATOK();
  } 
  else if(strcmp(pat, "ATE1") == 0) {
    at_echo = 1;
    ATOK();
  } 
  else if(strcmp(pat, "AT+HW?") == 0) {
    at_uart_send("+HW:ESP8266\r\n");
    ATOK();
  } 
  else if(strcmp(pat, "AT+LINK?") == 0) {
    extern uint8_t rf_Link_On;
    sprintf(reply, "+LINK:%u\r\n", rf_Link_On);
    at_uart_send(reply);
    ATOK();
  } 
  else if(strcmp(pat, "AT+MAC?") == 0) {
    sprintf(reply, "+MAC:%s\r\n", rf_info.mac);
    at_uart_send(reply);
    ATOK();
  } 
  else if(strncmp(pat, "AT+SSID", 7) == 0) {
    if(pat[7] == '?') {
      sprintf(reply, "+SSID:\"%s\"\r\n", rf_info.wifi_ssid);
      at_uart_send(reply);
    } else if(pat[7] == '=') {
      strncpy(rf_info.wifi_ssid, &pat[9], sizeof(rf_info.wifi_ssid));
      rf_info.wifi_ssid[strlen(rf_info.wifi_ssid)-1] = 0;
      wifi_param_error = 0;
    }
    ATOK();
  } 
  else if(strncmp(pat, "AT+KEY", 6) == 0) {
    if(pat[6] == '?') {
      sprintf(reply, "+KEY:\"%s\"\r\n", rf_info.wifi_key);
      at_uart_send(reply);
    } else if(pat[6] == '=') {
      strncpy(rf_info.wifi_key, &pat[8], sizeof(rf_info.wifi_key));
      rf_info.wifi_key[strlen(rf_info.wifi_key)-1] = 0;
      wifi_param_error = 0;
    }
    ATOK();
  } 
  else if(strncmp(pat, "AT+AID", 6) == 0) {
    if(pat[6] == '?') {
      sprintf(reply, "+AID:%s\r\n", rf_info.zhiyun_id);
      at_uart_send(reply);
    } else if(pat[6] == '=') {
      strncpy(rf_info.zhiyun_id, &pat[7], sizeof(rf_info.zhiyun_id));
      wifi_param_error = 0;
    }
    ATOK();
  } 
  else if(strncmp(pat, "AT+AKEY", 7) == 0) {
    if(pat[7] == '?') {
      sprintf(reply, "+AKEY:\"%s\"\r\n", rf_info.zhiyun_key);
      at_uart_send(reply);
    } else if(pat[7] == '=' && pat[8] == '"' && pat[strlen(pat)-1] == '"') {
      pat[strlen(pat)-1] = 0;
      strncpy(rf_info.zhiyun_key, &pat[9], sizeof(rf_info.zhiyun_key));
      wifi_param_error = 0;
    }
    ATOK();
  } 
  else if(strncmp(pat, "AT+SIP", 6) == 0) {
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
      wifi_param_error = 0;
    }
    ATOK();
  } 
  else if(strcmp(pat, "AT+ENVSAVE") == 0) {
    flash_write(FLASH_PARAM_START_ADDR, (uint16_t *)(&rf_info), sizeof(t_rf_info) / 2);
    ATOK();
  } 
  else if(strcmp(pat, "AT+RESET") == 0) {
    ATOK();
    wifi_reset();
    NVIC_SystemReset();
    while(1);
  }
  else if(strncmp(pat, "AT+SEND=", 8) == 0) {
    res = atoi(&pat[8]);
    if(res == 0 || res >= 256) {
      res = 0;
      at_uart_send("ERR: Bad command\r\n");
    }
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
    //关闭所有中断
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

    //复位所有初始化过的外设
    EXTI_DeInit();
    USART_DeInit(USART1);
    USART_DeInit(USART2);
    USART_DeInit(USART3);
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_AFIODeInit();
    //复位时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
    RCC_DeInit();

    //系统复位
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
#ifdef CONFIG_AT_RTT_DEBUG
  /*获取空闲RAM*/
  else if(strncmp(pat, "AT+FREEMEM", 10) == 0) {
    rt_uint32_t total;
    rt_uint32_t used;
    rt_uint32_t max_used;     
    rt_memory_info(&total,&used,&max_used);
    sprintf(reply, "+FREEMEM: total:%d used:%d max_used:%d\r\n", total,used,max_used); 
    at_uart_send(reply);
    ATOK();
  }
  /*获取线程状态*/
  else if(strncmp(pat, "AT+PS", 5) == 0) {
    list_thread(reply);
    ATOK();
  }
#endif
#ifdef CONFIG_SENSOR_NODE
  /*设置节点类型*/
  else if(strncmp(pat, "AT+NODE", 7) == 0) {
    if(pat[7] == '?') {
      switch(rf_info.sensor_type){
        case(SENSOR_A): temp=0; break;
        case(SENSOR_B): temp=1; break;
        case(SENSOR_C): temp=2; break;
        case(SENSOR_D): temp=3; break;
        case(SENSOR_EL): temp=4; break;
        case(SENSOR_EH): temp=5; break;
        case(SENSOR_F): temp=6; break;
        default: temp=-1; break;
      }
      if(temp >= 0 && temp < ARRAY(sensor_type_str))
      {
        sprintf(reply, "+NODE:%s\r\n", sensor_type_str[temp]);   
      }else{
        sprintf(reply, "ERR: +NODE:\"sensor:%d\"\r\n", temp);    
      }
      at_uart_send(reply);
    } 
    else if(pat[7] == '=') {
      if(strcmp(&pat[8], "A") == 0){
        rf_info.sensor_type=SENSOR_A;temp=0;
      }else if(strcmp(&pat[8], "B") == 0){
        rf_info.sensor_type=SENSOR_B;temp=1;
      }else if(strcmp(&pat[8], "C") == 0){
        rf_info.sensor_type=SENSOR_C;temp=2;
      }else if(strcmp(&pat[8], "D") == 0){
        rf_info.sensor_type=SENSOR_D;temp=3;
      }else if(strcmp(&pat[8], "EL") == 0){
        rf_info.sensor_type=SENSOR_EL;temp=4;
      }else if(strcmp(&pat[8], "EH") == 0){
        rf_info.sensor_type=SENSOR_EH;temp=5;
      }else if(strcmp(&pat[8], "F") == 0){
        rf_info.sensor_type=SENSOR_F;temp=6;
      }else temp=-1; 
      
      if(temp >= 0 && temp < ARRAY(sensor_type_str))
      {
        // 保存到flash
        flash_write(FLASH_PARAM_START_ADDR, (uint16_t *)(&rf_info), sizeof(t_rf_info) / 2);
        sprintf(reply, "+NODE:%s\r\n", sensor_type_str[temp]); 
        at_uart_send(reply);
        // 切换节点后进行复位
        ATOK();
        wifi_reset();
        NVIC_SystemReset();
        while(1);
      }else at_uart_send("ERR: Bad command\r\n");
    }
  } 
  /*QT接收到节点信息确认*/
  else if(strncmp(pat, "+Mode Recieved", 14) == 0) {
    if(node_report_mb!=RT_NULL){
      rt_mb_send(node_report_mb, 1);       //邮件内容：1--QT接收到节点信息
    }
  }
#endif
  else {
    at_uart_send("ERR: Bad command\r\n");
  }
  return res;
}

/* 应用运行 */
void at_run(void) {
  static uint8_t send_mode = 0;     //发送模式：平时为0，接收到AT+SEND命令后为1
  static uint16_t send_length = 0;  //发送长度
  static char send_buf[256];        //发送缓冲
  rt_err_t err;
  char recv[201];
  int16_t length = 0;
  uint32_t mb_recv;
  
  err = rt_mb_recv(&at_data_mb, (rt_ubase_t*)&mb_recv, RT_WAITING_FOREVER);
  if(err == RT_EOK) {
    if(mb_recv == 1) {
      length = at_uart_read((uint8_t *)recv, 200, 0);
      recv[length] = 0;
      if(send_mode == 0) {          //正常AT模式
        if(length <= 2 || recv[length-2] != '\r' || recv[length-1] != '\n') return;
        if(at_echo == 1) at_uart_send(recv);    //回显
        //AT命令解析与处理
        recv[length - 2] = 0;   //截取\r\n之前的内容
        send_length = at_analyses(recv);
        if(send_length > 0) {
          send_buf[0] = 0;
          send_mode = 1;
          at_uart_send(">");
        }
      } else if(send_mode == 1) {   //数据发送
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

/* 应用初始化 */
void at_init(void) {
  at_HW_Init();
}

/* 接收到通讯模块数据后，对AT串口发送+RECV命令 */
void at_recv(char *pdata) {
  extern int wifi_rssi;
  char cmd[40];
  sprintf(cmd, "+RECV:%u,%d\r\n", strlen(pdata), wifi_rssi);
  at_uart_send(cmd);
  at_uart_send(pdata);
}
