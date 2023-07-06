#include "comdef.h"
#include "hal_board_cfg.h"
#include "hal_adc.h"
#include "hal_dma.h"
#include "hal_flash.h"
#include "hal_types.h"
#include "sb_exec.h"
#include "sb_main.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */

// The IAR C-Stack initializer value: 0xCD.
#if !defined SB_STACK_VALUE
#define SB_STACK_VALUE  0xCD
#endif
// Not zero and not SB_STACK_VALUE.
#if !defined SB_MAGIC_VALUE
#define SB_MAGIC_VALUE  0xF5
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

#define SB1_PRESS  (P1_2 == 0)
#define SB2_PRESS  (P1_3 == 0)

#if HAL_LED
#define SB_TURN_OFF_LED1()  HAL_TURN_OFF_LED1()
#define SB_TURN_ON_LED1()   HAL_TURN_ON_LED1()
#define SB_TOGGLE_LED1()    HAL_TOGGLE_LED1()
#define SB_TURN_OFF_LED2()  HAL_TURN_OFF_LED2()
#define SB_TURN_ON_LED2()   HAL_TURN_ON_LED2()
#define SB_TOGGLE_LED2()    HAL_TOGGLE_LED2()
#else
#define SB_TURN_OFF_LED1()
#define SB_TURN_ON_LED1()
#define SB_TOGGLE_LED1()
#define SB_TURN_OFF_LED2()
#define SB_TURN_ON_LED2()
#define SB_TOGGLE_LED2()
#endif

/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */

halDMADesc_t dmaCh0;

/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ISR's implemented in the boot loader must be able to quickly determine whether to jump to the
 * boot code handlers or the run code handlers. So mark the bottom of the C call stack space with
 * a special value. Since the boot code linker file starts the C call stack space one byte higher
 * than the run code, the IAR generated initialization code does not initialize this byte;
 * but the boot code does - marking it with the magic value.
 */
#pragma location="SB_MAGIC_SPACE"
volatile __no_init uint8 magicByte;

/* ------------------------------------------------------------------------------------------------
 *                                       Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void vddWait(uint8 vdd);
static void sblInit(void);
static void sblExec(void);
static uint8 sblWait(void);

// Saving code space by not using the _hal_uart.c API and directly including the low level drivers.
#include "_hal_uart_isr.c"

/**************************************************************************************************
 * @fn          main
 *
 * @brief       ISR for the reset vector.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
void main(void)
{
  sblInit();

//  if (sbImgValid())
//  {
    if (sblWait())
    {
      HalUARTUnInitISR();
      // Simulate a reset for the Application code by an absolute jump to location 0x2000.
      asm("LJMP 0x2000\n");
      HAL_SYSTEM_RESET();
    }
//  }

  sblExec();
  HAL_SYSTEM_RESET();
}

/**************************************************************************************************
 * @fn          vddWait
 *
 * @brief       Loop waiting for 16 reads of the Vdd over the requested limit.
 *
 * input parameters
 *
 * @param       vdd - Vdd level to wait for.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 **************************************************************************************************
 */
static void vddWait(uint8 vdd)
{
  uint8 cnt = 16;

  do {
    do {
      ADCCON3 = 0x0F;
      while (!(ADCCON1 & 0x80));
    } while (ADCH < vdd);
  } while (--cnt);
}

static void sblInit(void)
{
  halUARTCfg_t uartConfig;
  HAL_BOARD_INIT();
  vddWait(VDD_MIN_RUN);
  magicByte = SB_MAGIC_VALUE;

  /* This is in place of calling HalDmaInit() which would require init of the other 4 DMA
   * descriptors in addition to just Channel 0.
   */
  HAL_DMA_SET_ADDR_DESC0(&dmaCh0);

  HalUARTInitISR();
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = HAL_UART_BR_115200;
  uartConfig.flowControl          = FALSE;
  uartConfig.flowControlThreshold = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.rx.maxBufSize        = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.tx.maxBufSize        = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.idleTimeout          = 0;  // CC2530 by #define - see hal_board_cfg.h
  uartConfig.intEnable            = TRUE;
  uartConfig.callBackFunc         = NULL;
  HalUARTOpenISR(&uartConfig);
}

static void sblExec(void)
{
  uint32 dlyCnt = 0;
  vddWait(VDD_MIN_NV);
  HAL_ENABLE_INTERRUPTS();

  while (1)
  {
    if (dlyCnt++ & 0x4000)
    {
      SB_TOGGLE_LED1();
    }

    if (sbExec())
    {
      break;
    }
  }

  SB_TURN_ON_LED1();
  SB_TURN_ON_LED2();
  // Delay to allow the SB_ENABLE_CMD response to be flushed.
  for (dlyCnt = 0; dlyCnt < 0x40000; dlyCnt++)
  {
    asm("NOP");
  }
}

static uint8 sblWait(void)
{
//  uint32 dlyCnt = 0x260000;
  uint32 dlyCnt = 0x10000;
  uint8 rtrn = FALSE;
  HAL_ENABLE_INTERRUPTS();

  while (1)
  {
//    uint8 ch;
//
//    if (HalUARTReadISR(&ch, 1))
//    {
//      if (ch == SB_FORCE_BOOT)
//      {
//        break;
//      }
//      else if (ch == SB_FORCE_RUN)
//      {
//        dlyCnt = 0;
//      }
//    }

    if (SB2_PRESS)
    {
      rtrn = TRUE;
      break;
    }
    if (SB1_PRESS)
    {
      rtrn = TRUE;
      break;
    }
    if(dlyCnt-- == 0){
      break;
    }
    
//    if (SB2_PRESS || (dlyCnt-- == 0))
//    {
//      rtrn = TRUE;
//      break;
//    }

//    // RR-xing LED display while waiting.
//    if (dlyCnt & 0x2000)
//    {
//      SB_TURN_OFF_LED2();
//      SB_TURN_ON_LED1();
//    }
//    else
//    {
//      SB_TURN_OFF_LED1();
//      SB_TURN_ON_LED2();
//    }
  }

  HAL_DISABLE_INTERRUPTS();
  SB_TURN_OFF_LED1();
  SB_TURN_OFF_LED2();

  return rtrn;
}

HAL_ISR_FUNCTION( halUart0RxIsr, URX0_VECTOR )
{
  if (magicByte == SB_STACK_VALUE)
  {
    void (*rxIsr)(void);
    rxIsr = (void (*)(void))0x2013;
    rxIsr();
  }
  else if (magicByte == SB_MAGIC_VALUE)
  {
    halUartRxIsr();
  }
  else
  {
    asm("NOP");  // Not expected.
  }
}

HAL_ISR_FUNCTION( halUart0TxIsr, UTX0_VECTOR )
{
  if (magicByte == SB_STACK_VALUE)
  {
    void (*txIsr)(void);
    txIsr = (void (*)(void))0x203B;
    txIsr();
  }
  else if (magicByte == SB_MAGIC_VALUE)
  {
    halUartTxIsr();
  }
  else
  {
    asm("NOP");  // Not expected.
  }
}