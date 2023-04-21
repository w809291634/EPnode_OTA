/*********************************************************************************************
* 文件：at.c
* 作者：xuzhy 
* 说明：
*       
*       
*      
* 修改：
* 注释：
*********************************************************************************************/
#include <string.h>
#include <ctype.h>

#include "contiki.h"
#include "usart.h"
#include "config.h"
#include "gsm.h"
#include "at-uart.h"
#include "at.h"
#include "zxbee-inf.h"


PROCESS(at, "at");
static process_event_t  event_at;

extern void clock_delay_ms(unsigned int ms);
#define AT_BUFF_SIZE    256
#define AT_BUFF_NUM	 4
static char at_echo = 1;
static  int at_datalen = 0; //指示用于接收发送的数据长度
static  int at_recvdata = 0;
static char atbuff[AT_BUFF_NUM][AT_BUFF_SIZE];
static char bufferbit = 0;

/*********************************************************************************************
* 名称：at_quebuffer_get
* 功能：at缓存申请
* 参数：
* 返回：成功返回缓存地址，失败返回NULL
* 修改：
* 注释：
*********************************************************************************************/
char* at_quebuffer_get()
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (((bufferbit>>i) & 1) == 0) {
			bufferbit |= 1<<i;
			return atbuff[i];
		}
	}
	return NULL;
}
/*********************************************************************************************
* 名称：at_quebuffer_put
* 功能：at缓存释放
* 参数：buf：待释放缓存地址
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_quebuffer_put(char *buf)
{
	for (int i=0; i<AT_BUFF_NUM; i++) {
		if (atbuff[i] == buf){
			bufferbit &= ~(1<<i);
			return;
		}
	}
}
/*********************************************************************************************
* 名称：_at_get_ch
* 功能：at串口接收到1个字节处理函数
* 参数：ch：接收到的字节
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
static int _at_get_ch(char ch)
{
  static char idx = 0;
  static char *pbuf = NULL;
  if (pbuf == NULL) {
    pbuf = at_quebuffer_get();
    if (pbuf == NULL) { 
      return 0;
    }
  }
  if (at_datalen == 0) {
    if (at_echo) {
      at_uart_write(ch);
    }
    if (idx < AT_BUFF_SIZE-1) {
      pbuf[idx++] = ch;
      if (idx >= 2 && pbuf[idx-2]=='\r' && pbuf[idx-1]=='\n'){
        idx -= 2;
        pbuf[idx] = '\0';
        if (idx > 0) {
          /* 获取到at命令 */
          process_post(&at, event_at, (process_data_t)pbuf);
          pbuf = NULL;
          idx = 0;
        }
      }
    } else {
      /*丢弃*/
      idx = 0;
    }
  } else {
    pbuf[at_recvdata++] = ch;
    if (at_recvdata == at_datalen) {
      process_post(&at, event_at, (process_data_t)pbuf);
      //at_recvdata = 0;
      at_datalen = 0;
      pbuf = NULL;
    }
  }
  return 1;
}
/*********************************************************************************************
* 名称：at_response_buf
* 功能：at接口发送一段数据
* 参数：s：待发送数据地址
*      len：待发送数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_response_buf(char *s, int len)
{
  for (int i=0; i<len; i++) {
    at_uart_write(s[i]);
  }
}
/*********************************************************************************************
* 名称：at_response
* 功能：at接口发送一段字符串
* 参数：s：待发送字符串
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_response(char *s)
{
  at_response_buf(s, strlen(s));
}
/*********************************************************************************************
* 名称：LTEOnMessage
* 功能：从智云服务器接收到数据并输出到at串口，在zhiyun.c中被调用
* 参数：buf：接收到的zxbee数据
*       len： 数据长度
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void at_notify_data(char *buf, int len)
{
  char buff[32];
  
#ifdef NB_IoT_Serial
  extern int rssi;
  sprintf(buff, "+RECV:%d,%d\r\n", len,rssi);
#else
  sprintf(buff, "+RECV:%d\r\n", len);
#endif
  at_response(buff);
  at_response_buf(buf, len);
}
/*********************************************************************************************
* 名称：ATCommandInit
* 功能：at接口初始
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
void ATCommandInit(void)
{
 at_uart_init();
 at_uart_set_input_call(_at_get_ch);
 at_response("+HW:NB-IoT\r\n");
 at_response("+RDY\r\n");

}

uint8_t bc95_link_flag = 0;
/*********************************************************************************************
* 名称：at
* 功能：at命令处理线程
* 参数：
* 返回：
* 修改：
* 注释：
*********************************************************************************************/
PROCESS_THREAD(at, ev, data)
{

  char buf[120];
  
  PROCESS_BEGIN();
 
  event_at = process_alloc_event();
  ATCommandInit();
#if defined(NB_IoT_Serial)
  ZXBeeInfInit();
#endif
  
  while (1) {
    PROCESS_WAIT_EVENT();
    if (ev == event_at) {
      char *p_msg = (char *)data;
      if (at_recvdata != 0) { //got data
        int r;
        void  ZXBeeInfSend(char *p, int len);
        p_msg[at_recvdata] = '\0';
        r = strlen(p_msg);
        ZXBeeInfSend(p_msg, r);
        at_response(ATOK);
        
        at_recvdata = 0;
        if (r < 0) {
          at_response("+DATASEND:Error!\r\n");
        } else {
          sprintf(buf, "+DATASEND:%d\r\n", r);
          at_response(buf);
        }
      } else {
        int msg_size = strlen(p_msg);
        for (int i=0; i<msg_size; i++) {
          if (p_msg[i] == '?' || p_msg[i] == '=') break;
          p_msg[i] = toupper(p_msg[i]);
        }
        if (strcmp(p_msg, "AT") == 0) {
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "ATE1") == 0) {
          at_echo = 1;
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "ATE0") == 0) {
          at_echo = 0;
          at_response(ATOK);
        } 
        else if (strcmp(p_msg, "AT+HW?") == 0) {
          at_response("+HW:NB-IoT\r\n");
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "AT+LINK?") == 0) {
          if(bc95_link_flag) at_response("+LINK:1\r\n");
          else               at_response("+LINK:0\r\n");
          at_response(ATOK);
        }
        else if (strcmp(p_msg, "AT+NODEID?") == 0) {
          at_response("+NODEID:");
          at_response(gsm_info.imei);
          at_response("\r\n");
          at_response(ATOK);
        }
        else if (memcmp(p_msg, "AT+SEND=", 8)==0){
          int sz = atoi(&p_msg[8]);
          if (sz > 0 && sz<256) {
            at_response(">");
            at_datalen = sz;
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+ENVSAVE", 10)==0){
          config_save();
          at_response(ATOK);
        }
        
        else if (memcmp(p_msg, "AT+AID", 6)==0){
          if (p_msg[6] == '?') {
            at_response("+AID:");
            at_response(nbConfig.id);
            at_response("\r\n");
            at_response(ATOK);
            
          } else if (p_msg[6] == '=') {
            strncpy(nbConfig.id, &p_msg[7], sizeof nbConfig.id);
            at_response(ATOK);
          } else {
            at_response(ATERROR);
          }  
        } 
        else if (memcmp(p_msg, "AT+AKEY", 7)==0){
          if (p_msg[7] == '?') {
            at_response("+AKEY:\"");
            at_response(nbConfig.key);
            at_response("\"\r\n");
            at_response(ATOK);
            
          } else if (p_msg[7] == '=') {
            if (p_msg[8] == '"' && p_msg[msg_size-1] == '"') {
              p_msg[msg_size-1] = '\0';
              strncpy(nbConfig.key, &p_msg[9], sizeof nbConfig.key);
              at_response(ATOK);
            }
          } else {
            at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+SIP", 6)==0){
          if (p_msg[6] == '?') {
            at_response("+SIP:");
            at_response(nbConfig.ip);
            if (nbConfig.port != CONFIG_ZHIYUN_PORT) {
              sprintf(buf, ",%d", nbConfig.port);
              at_response(buf);
            }
            at_response("\r\n");
            at_response(ATOK); 
          } else if (p_msg[6] == '='){
            char *pport = strchr(&p_msg[7], ',');
            if (pport != NULL) {
              *pport = '\0';
              pport += 1;
              nbConfig.port = atoi(pport);
            }
            strncpy(nbConfig.ip, &p_msg[7], sizeof nbConfig.ip);
            at_response(ATOK);
          } else {
              at_response(ATERROR);
          }
        }
        else if (memcmp(p_msg, "AT+RESET", 8) == 0) {
          at_response(ATOK); 
          __set_FAULTMASK(1);
          NVIC_SystemReset();
        }
        else if (memcmp(p_msg, "AT+MODE", 7) == 0) {
          if (p_msg[7] == '?') {
            if (nbConfig.mode == 0){
              at_response("+MODE:COAP\r\n");
            } else {
              at_response("+MODE:UDP\r\n");
            }
            at_response(ATOK); 
          }else if (p_msg[7] == '='){
            int i = 8;
            for (i=8; p_msg[i] != '\0'; i++){
              p_msg[i] = toupper(p_msg[i]);
            }
            if (strcmp(&p_msg[8], "COAP") == 0){
              if (nbConfig.mode != 0) {
                nbConfig.mode = 0;
              }
              at_response(ATOK); 
            }else if (strcmp(&p_msg[8], "UDP") == 0){
              if (nbConfig.mode != 1) {
                nbConfig.mode = 1;
              }
              at_response(ATOK); 
            } else {
              at_response(ATERROR);
            }
          } else {
             at_response(ATERROR);
          }
          
        } else if (memcmp(p_msg, "AT+RSSI", 7) == 0) {
          extern int rssi;
          sprintf(buf, "+RSSI:%d\r\n", rssi);
          at_response(buf);
          at_response(ATOK); 
        }
        else if (memcmp(p_msg, "AT+IAP=", 7)==0){
          at_response(ATOK); 
          if(memcmp(p_msg+7, "IAP", 3)==0) {
            nbConfig.boot_mode = BOOT_IAP;
          } else if(memcmp(p_msg+7, "APP", 3)==0) {
            nbConfig.boot_mode = BOOT_APP;
          }
          nbConfig.app_en = APP_OK;
          config_save();
          clock_delay_ms(100);
          //关闭所有中断
          SysTick->CTRL = 0;
          SysTick->VAL  = 0;
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
        else {
          int8_t user_at_proc(char *msg);
          if (user_at_proc(p_msg) < 0) {
            at_response(ATERROR);
          }
        }
      }
      at_quebuffer_put(p_msg);
    }
  }
  PROCESS_END();
}
