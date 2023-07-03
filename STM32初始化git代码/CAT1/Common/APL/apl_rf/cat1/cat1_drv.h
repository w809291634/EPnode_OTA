#ifndef __CAT1_DRV_H__
#define __CAT1_DRV_H__

//POWER引脚：PB15，PB15为高时，模块POWER引脚拉低
#define CAT1_POWER_CLK      RCC_APB2Periph_GPIOB
#define CAT1_POWER_GPIO     GPIOB
#define CAT1_POWER_PIN      GPIO_Pin_15
#define CAT1_POWER_H()      GPIO_ResetBits(CAT1_POWER_GPIO, CAT1_POWER_PIN)
#define CAT1_POWER_L()      GPIO_SetBits(CAT1_POWER_GPIO, CAT1_POWER_PIN)
//RESET引脚：PA1，PA1为高时，模块RESET引脚拉低
#define CAT1_RESET_CLK      RCC_APB2Periph_GPIOA
#define CAT1_RESET_GPIO     GPIOA
#define CAT1_RESET_PIN      GPIO_Pin_1
#define CAT1_RESET_H()      GPIO_ResetBits(CAT1_RESET_GPIO, CAT1_RESET_PIN)
#define CAT1_RESET_L()      GPIO_SetBits(CAT1_RESET_GPIO, CAT1_RESET_PIN)

void cat1_hw_init(void);
void cat1_power_ctrl(uint8_t state);
void cat1_reset(void);
void cat1_Get_IMEI(char *pimei);
void cat1_disconnect(void);
int8_t cat1_TCP_Connect(char *ip, uint16_t port);

#endif