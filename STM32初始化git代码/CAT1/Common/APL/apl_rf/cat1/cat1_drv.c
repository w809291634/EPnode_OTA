#include <rtthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rf_if_app.h"
#include "rf_if_hw.h"
#include "cat1_drv.h"

int cat1_rssi = 0;

//CAT1模组硬件控制引脚初始化
void cat1_hw_init(void) {
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(CAT1_POWER_CLK | CAT1_RESET_CLK, ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  //RESET引脚初始化
  GPIO_InitStructure.GPIO_Pin =  CAT1_RESET_PIN;
  GPIO_Init(CAT1_RESET_GPIO, &GPIO_InitStructure);
//  CAT1_RESET_H();
  cat1_reset();
  //POWER引脚初始化
  GPIO_InitStructure.GPIO_Pin =  CAT1_POWER_PIN;
  GPIO_Init(CAT1_POWER_GPIO, &GPIO_InitStructure);
  CAT1_POWER_H();
  //模组开机
  cat1_power_ctrl(1);
}

//CAT1模组电源控制：state为1时开机，state为0时关机
void cat1_power_ctrl(uint8_t state) {
  //现将模组POWER引脚下拉到地，上电时自动开机
//  //开机：拉低2秒以上；关机：拉低3.1秒以上
//  CAT1_POWER_L();
//  rt_thread_mdelay(2100);
//  if(state == 0) {
//    rt_thread_mdelay(1100);
//  }
//  CAT1_POWER_H();
}

//CAT1模组复位
void cat1_reset(void) {
  CAT1_RESET_L();
  rt_thread_mdelay(200);
  CAT1_RESET_H();
}

//获取CAT1的IMEI
void cat1_Get_IMEI(char *pimei) {
  uint16_t len;
  char recv[101];
  char *phead = NULL;
  
  rf_if_uart_send((uint8_t*)"AT+CGSN\r\n", strlen("AT+CGSN\r\n"));
  len = rf_if_uart_read((uint8_t*)recv, 100, 50);
  if(len > 15) {
    recv[len] = 0;
    phead = strstr((const char*)recv, "\r\n");
    if(phead != NULL) {
      strncpy(pimei, phead+2, 15);
      pimei[15] = 0;
    }
  }
  rf_if_uart_flush(50);
}

//CAT1等待串口就绪：成功返回1，失败返回0
static int8_t cat1_Wait4Uart(void) {
  uint8_t recv[51]; //串口接收数据
  uint16_t len;//读取到的串口数据长度
  rt_err_t err;
  uint32_t mb_recv;
  uint8_t wait_time = 30;//串口等待时间
  
  rf_if_uart_flush(1000);
  rf_quit_trans();  //退出透传模式（适用于EC20/CAT1，单片机复位而模组仍在透传模式下）
  while(wait_time>0) {
    rf_if_uart_flush(0);
    rf_if_uart_send((uint8_t*)"AT\r\n", strlen("AT\r\n"));
    err = rt_mb_recv(&rf_if_data_mb, (rt_ubase_t*)&mb_recv, 100);
    if(err == RT_EOK) {
      if(mb_recv == 1) {            //邮件来源：通讯串口中断，处理该数据
        len = rf_if_uart_read(recv, 50, 50);
        if(len>0) {
          recv[len] = 0;
          if(strstr((const char*)recv, "OK") != NULL) break;
        }
      }
    }
    rt_thread_mdelay(3000);
    wait_time -= 3;
  }
  
  if(wait_time == 0) {//未检测到设备
    return 0;
  }
  rt_thread_mdelay(1000);
  rf_if_uart_send((uint8_t*)"ATE0\r\n", strlen("ATE0\r\n"));
  len = rf_if_uart_read(recv, 50, 100);
  return 1;
}

//CAT1等待连接到网络：成功返回1，失败返回0
static int8_t cat1_Wait4Net(void) {
  uint16_t len;
  char recv[101];
  char *phead = NULL;
  uint8_t status = 0;
  rt_err_t err;
  uint32_t mb_recv;
  uint8_t wait_time = 60;//网络等待时间
  
  rf_if_uart_flush(1000);
  while(wait_time>0) {
    rf_if_uart_send((uint8_t*)"AT+CEREG?\r\n", strlen("AT+CEREG?\r\n"));
    err = rt_mb_recv(&rf_if_data_mb, (rt_ubase_t*)&mb_recv, 1000);
    if(err == RT_EOK) {
      if(mb_recv == 1) {            //邮件来源：通讯串口中断，处理该数据
        len = rf_if_uart_read((uint8_t *)recv, 100, 50);
        if(len > 6) {
          recv[len] = 0;
          phead = strstr((const char*)recv, "+CEREG:");
          if(phead != NULL) {
            phead = strchr(phead, ',');
            if(phead != NULL) {
              status = atoi(phead+1);
              if(status == 1) break;
            }
          }
        }
      }
    }
    rt_thread_mdelay(5000);
    wait_time -= 5;
  }
  
  if(wait_time == 0) {//无法连接到网络
    return 0;
  }
  return 1;
}

//CAT1获取CSQ
static void cat1_get_CSQ(void) {
  uint16_t len;
  char recv[51];
  char *phead = NULL;
  rt_err_t err;
  uint32_t mb_recv;
  
  while(1) {
    rf_if_uart_send((uint8_t*)"AT+CSQ\r\n", strlen("AT+CSQ\r\n"));
    err = rt_mb_recv(&rf_if_data_mb, (rt_ubase_t*)&mb_recv, 1000);
    if(err == RT_EOK) {
      if(mb_recv == 1) {            //邮件来源：通讯串口中断，处理该数据
        len = rf_if_uart_read((uint8_t *)recv, 50, 50);
        if(len > 5) {
          recv[len] = 0;
          phead = strstr((const char*)recv, "+CSQ:");
          if(phead != NULL) {
            cat1_rssi = atoi(phead+6);
            rf_if_uart_flush(0);
            break;
          }
        }
      }
    }
    rt_thread_mdelay(100);
  }
}

//CAT1从服务器断开
void cat1_disconnect(void) {
  uint8_t recv[51];
  
  rf_quit_trans();
  rf_if_uart_send((uint8_t*)"AT+MIPCLOSE=1\r\n", strlen("AT+MIPCLOSE=1\r\n"));
  rf_if_uart_read(recv, 50, 3000);
  rf_if_uart_flush(0);
}

//CAT1连接到指定服务器端口：成功返回1，失败返回0
static int8_t cat1_connect(char *ip, uint16_t port) {
  uint8_t cmd[120];     //串口发送命令
  char recv[101];    //串口接收数据
  uint16_t len;         //读取到的串口数据长度
  uint8_t try_times = 0;
  uint8_t status = 0;   //激活环境的状态
  rt_err_t err;
  uint32_t mb_recv;
  
  //设置收到服务器数据的格式：不转换直接输出，不带标识头
  rf_if_uart_send((uint8_t*)"AT+GTSET=\"IPRFMT\",1\r\n", strlen("AT+GTSET=\"IPRFMT\",1\r\n"));
  rf_if_uart_flush(1000);
  
  memset(cmd, 0, sizeof(cmd));
  sprintf((char *)cmd, "AT+MIPODM=1,,\"%s\",%d,0\r\n", ip, port);  //AT+MIPODM=1,,"api.zhiyun360.com",28082,0
  while(try_times++ < 3) {
    if(status == 0) {
      //请求运营商分配IP地址
      rf_if_uart_send((uint8_t*)"AT+MIPCALL=1\r\n", strlen("AT+MIPCALL=1\r\n"));
      err = rt_mb_recv(&rf_if_data_mb, (rt_ubase_t*)&mb_recv, 10000);
      if(err == RT_EOK) {
        if(mb_recv == 1) {            //邮件来源：通讯串口中断，处理该数据
          len = rf_if_uart_read((uint8_t *)recv, 50, 3000);
          if(len > 0) {
            recv[len] = 0;
            if(strstr(recv, "+MIPCALL") != NULL || strstr(recv, "ERROR") != NULL) {
              status = 1;
            }
          }
        }
      }
    }
    if(status == 1) {
      //连接到指定端口
      rf_if_uart_send(cmd, strlen((char *)cmd));
      err = rt_mb_recv(&rf_if_data_mb, (rt_ubase_t*)&mb_recv, 10000);
      if(err == RT_EOK) {
        if(mb_recv == 1) {            //邮件来源：通讯串口中断，处理该数据
          len = rf_if_uart_read((uint8_t *)recv, 100, 3000);
          if(len>0) {
            recv[len] = 0;
            if(strstr((const char*)recv, "+MIPODM") != NULL) {
              rf_if_uart_flush(0);
              return 1;
            }
          }
          //连接失败
          cat1_disconnect();
          status = 0;
        }
      }
    }
    rt_thread_mdelay(1000);
  }
  return 0;
}

//CAT1连接到服务器：成功返回1，失败返回0
int8_t cat1_TCP_Connect(char *ip, uint16_t port) {
  //等待串口就绪
  if(cat1_Wait4Uart() == 0) return 0;
  //等待连接到网络
  if(cat1_Wait4Net() == 0) return 0;
  //预防性断开
  cat1_disconnect();
  //更新CSQ
  cat1_get_CSQ();
  //连接到服务器指定端口
  if(cat1_connect(ip, port) == 0) {
    //连接失败：复位CAT1
    cat1_disconnect();
    cat1_reset();
    rf_if_uart_flush(5000);
    return 0;
  }
  return 1;
}
