/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : SPI_LCD.h
* Author             : MCD Application Team
* Version            : V2.0.1
* Date               : 06/13/2008
* Description        : Header for SPI_LCD.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_LCD_H
#define __SPI_LCD_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
#define ROTATION  90

#if ROTATION == 90 || ROTATION == 270
#define LCDW  161
#define LCDH  130
#else
#define LCDW 128
#define LCDH 160
#endif
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */

/* Exported functions ------------------------------------------------------- */
/*----- High layer function -----*/
void SPI_LCD_Init(void);
void lcd_init(void);

void Display_Clear(unsigned int co);
void Display_Clear_Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int co);
void Output_Pixel(unsigned  int x,unsigned  int y, unsigned int co);
void Display_ASCII5X8(unsigned  int x,unsigned  int y, unsigned int co, char *s);
void Display_ASCII6X12(unsigned  int x0,unsigned  int y0,unsigned int co,  char *s);
void Display_GB_12(unsigned  int x0,unsigned  int y0,unsigned int co,  char *s);

extern void lcd_title_bar(char *title);
extern void lcd_status_bar(char *sta);
void lcd_off(void);
void lcd_on(void);
/**********************************************************************************///TT
#define TITLE_BAR_HEIGHT        9     
#define STATU_BAR_HEIGHT        9
#define CO_BG               0x001f 
#define CO_TEXT             0xf100

#define CO_TITLE_BG         0xffff
#define CO_TITLE_TEXT       0x0000

#define CO_STATU_BG         CO_TITLE_BG
#define CO_STATU_TEXT       CO_TITLE_TEXT

#define ALIGN_CENTER(s, w)   ((LCDW - (strlen(s)*w))/2)
#define ALIGN_RIGHT(s, w)    (LCDW-(strlen(s)*w))
    
#define LINE(x)     (TITLE_BAR_HEIGHT+1 + (x)*(8+1))
#define LINE_EX(x)  (TITLE_BAR_HEIGHT+1 + (x)*(12+1)) //
/*******************************************************************************/
#endif /* __SPI_LCD_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
