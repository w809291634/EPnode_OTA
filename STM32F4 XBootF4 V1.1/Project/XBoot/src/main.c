/**
  ******************************************************************************
  * @file    IAP/src/main.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    10/15/2010
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/** @addtogroup IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "common.h"
#include "flash_if.h"
#include "command.h"
#include "flash.h"
#include "term.h"
#include "w25qxx.h"
#include "saveData_manage.h"

//#include "command.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define USER_APP_BEGIN  (unsigned)(&Image$$ER_IROM1$$Base)
/* Private variables ---------------------------------------------------------*/
//tUSER UserConfig;
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;
extern int Image$$ER_IROM1$$Base; //?????

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
	

/* Private function prototypes -----------------------------------------------*/
static void Terminal_Init (void) ;

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : static void LED_Init(void)
* Description    : LED 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void LED_Init(void){
  GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(LED_RUN_CLK, ENABLE);
  /* Configure Button pin as input */
	GPIO_InitStructure.GPIO_Pin=LED_RUN_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(LED_RUN_PORT,&GPIO_InitStructure);
	LED_OFF(); 
}
/*******************************************************************************
* Function Name  : void LED_Ctl(uint8_t en)
* Description    : LED 控制
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LED_Ctl(uint8_t en){
	if(en) {
		LED_ON();
	} else {
		LED_OFF();
  }
}
/*******************************************************************************
* Function Name  : void LED_Toogle(void)
* Description    : LED 控制
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LED_Toogle(void){
	static uint8_t state = 0;
	if(state) {
		LED_ON(); 
  } else {
		LED_OFF();
  }
	state = (state+1)&0x01;
}
/*******************************************************************************
* Function Name  : Terminal_Init
* Description    : Terminal 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//static void WatchDog_Init(void){
//	#define LSI_FREQ  40000
//	/* Enable write access to IWDG_PR and IWDG_RLR registers */
//  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

//  /* IWDG counter clock: LSI/32 */
//  IWDG_SetPrescaler(IWDG_Prescaler_32);

//  /* Set counter reload value to obtain 250ms IWDG TimeOut.
//     Counter Reload Value = 250ms/IWDG counter clock period
//                          = 250ms / (LSI/32)
//                          = 0.25s / (LSI_FREQ/32)
//                          = LSI_FREQ/(32 * 4)
//                          = LSI_FREQ/128
//   */
//  IWDG_SetReload(LSI_FREQ/32);

//  /* Reload IWDG counter */
//  IWDG_ReloadCounter();

//  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
//  IWDG_Enable();
//}

//模式获取函数-读取模式引脚，获取要进入的模式
static uint8_t _mode_Get(void) {
  //模式引脚初始化：PA10-为低时进入DFU，否则进入APP
  GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(REC_DETECT_CLK, ENABLE);
  /* Configure Button pin as input */
	GPIO_InitStructure.GPIO_Pin=REC_DETECT_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(REC_DETECT_PORT,&GPIO_InitStructure);
  
  uint32_t count = 0;
  LED_ON();
  while(count++ < REC_WAIT_CNT) {
    if(count % 666666 == 0) {
      LED_Toogle();
    }
    
    if(!REC_DETECT_ACTIVE()) {
      LED_OFF();
      return BOOT_IAP;
    }
  }
  LED_OFF();
  return BOOT_REC;
}

/*******************************************************************************
* Function Name  : Terminal_Init
* Description    : Terminal 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
		extern unsigned char userdata[];
	  user_data_t *p = (user_data_t*)userdata;

	  LED_Init();
		flash_read(FLASH_PARAM_START_ADDR, (unsigned char *)(userdata), sizeof(user_data_t));//读取配置信息

    if(_mode_Get() == BOOT_REC) {   //开机时长按REC按键：恢复出厂设置
      if(p->rec_sta == REC_DONE) {  //恢复区有代码，执行恢复
        //即将开始恢复出厂设置，为防止中途失败，预先将模式设为IAP
        p->app_en = APP_ERR;
        p->boot_mode = BOOT_IAP;
        flash_write(FLASH_PARAM_START_ADDR, (unsigned char *)(userdata), sizeof(user_data_t));
        //初始化SPIFlash
        W25QXX_Init();
        //重置传感器参数
        saveData_Clear();
        //将恢复区代码拷贝到APP区
        LED_ON();
        if(rec_Code_Recovery() == 0) { //拷贝成功
          LED_OFF();
          //恢复出厂已完成，将模式设定为APP模式，重启后直接进入APP
          p->app_en = APP_OK;
          p->boot_mode = BOOT_APP;
          flash_write(FLASH_PARAM_START_ADDR, (unsigned char *)(userdata), sizeof(user_data_t));
        }
        //重启节点
        NVIC_SystemReset();
        while(1);
      } else {  //恢复区无代码
        printk("ERROR: REC CODE NOT EXIST!\n");
      }
    } else if(p->boot_mode == BOOT_REC) { //REC模式：跳转到出厂固件运行--恢复区放的是可以拷贝到APP区的代码，无法在恢复区直接运行
      //RunUserProgram(RECOVERY_ADDRESS);
    } else if(p->boot_mode == BOOT_APP && p->app_en == APP_OK) { //APP模式：跳转到APP运行
      RunUserProgram(APPLICATION_ADDRESS);
    }
    
    //既不是APP模式，也不是REC模式：运行IAP程序
    Terminal_Init();
		init_builtin_cmds();
    printk("\tEnter IAP mode, input \"IAP update\" to update firmware!\n");
		while (1)
		{
			serial_terminal();
		}
}
/*******************************************************************************
* Function Name  : Terminal_Init
* Description    : Terminal 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TERM_UART_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
/*******************************************************************************
* Function Name  : Terminal_Init
* Description    : Terminal 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Terminal_Init (void)  
{
	extern unsigned char userdata[];
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
  
	RCC_AHB1PeriphClockCmd(TERM_GPIO_CLK,ENABLE);
	TERM_UART_CLK_INIT(TERM_UART_CLK, ENABLE);

	GPIO_PinAFConfig(TERM_GPIO_PORT,TERM_GPIO_TX_PINSOURCE,TERM_GPIO_AF);
	GPIO_PinAFConfig(TERM_GPIO_PORT,TERM_GPIO_RX_PINSOURCE,TERM_GPIO_AF);

	GPIO_InitStructure.GPIO_Pin=TERM_GPIO_TX_PIN;  //PA9 tx 
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(TERM_GPIO_PORT,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=TERM_GPIO_RX_PIN;
	GPIO_Init(TERM_GPIO_PORT,&GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(TERM_UART, &USART_InitStructure);
	USART_Cmd(TERM_UART, ENABLE);
	NVIC_Configuration();
	USART_ITConfig(TERM_UART,USART_IT_RXNE,ENABLE);
}
#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : Terminal_Init
* Description    : Terminal 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(TERM_UART, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(TERM_UART, USART_FLAG_TC) == RESET)
  {}

  return ch;
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
