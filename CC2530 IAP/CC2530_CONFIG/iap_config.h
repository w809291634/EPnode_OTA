#ifndef __CONFIG_H__
#define __CONFIG_H__

#define BIT_0       (1<<0)
#define BIT_1       (1<<1)
#define BIT_2       (1<<2)
#define BIT_3       (1<<3)
#define BIT_4       (1<<4)
#define BIT_5       (1<<5)
#define BIT_6       (1<<6)
#define BIT_7       (1<<7)
#define ARRAY_SIZE(p) (sizeof(p)/sizeof(p[0]))

/** 系统参数类型 **/
typedef struct{
  // app分区
  unsigned char current_part;             // 当前所在分区 1;0xff 不启动APP,进入boot
  unsigned char app1_flag;                // 分区标记
}sys_parameter_t;
extern sys_parameter_t sys_parameter;

typedef  void (*pFunction)(void); 

/** 分区信息 **/
#define APP_OK      0x4D
#define APP_ERR     0xFF

#define BOOT_PARTITION_START_ADDR         CC2530_FLASH_BASE     // bootloader 分区大小为36KB        
#define BOOT_PARTITION_END_ADDR           (PARA_PARTITION_START_ADDR-1)  
#define PARA_PARTITION_START_ADDR         (0xA000)              // 参数分区起始地址,这里页为20,21
#define PARA_PARTITION_SIZE               (0x1000)              // 参数分区大小4KB
#define APP1_PARTITION_START_ADDR         (PARA_PARTITION_START_ADDR+PARA_PARTITION_SIZE)                 // APP分区起始地址
#define APP1_PARTITION_END_ADDR           CC2530_FLASH_END
#define APP1_PARTITION_SIZE               (CC2530_FLASH_SIZE-CC2530_FLASH_BASE-APP1_PARTITION_START_ADDR) 

// 计算上传镜像(没有使用)
#define FLASH_IMAGE_SIZE                 (uint32_t) (CC2530_FLASH_SIZE - (APP1_PARTITION_START_ADDR - CC2530_FLASH_BASE)) // 没有适配

/** 参数定义 **/
#define SYS_PARAMETER_SIZE                (sizeof(sys_parameter)/4+((sizeof(sys_parameter)%4)?1:0))     // 多少个字
#define SYS_PARAMETER_READ                {FlashRead(PARA_PARTITION_START_ADDR,\
                                            (uint8_t*)&sys_parameter,sizeof(sys_parameter));}
#define SYS_PARAMETER_WRITE               FlashWrite(PARA_PARTITION_START_ADDR,\
                                            (uint8_t*)&sys_parameter,SYS_PARAMETER_SIZE)

/** FLASH地址 **/
// 起始地址和大小
#define CC2530_FLASH_BASE        (0x0000) 	        // cc2530 FLASH的起始地址
#define CC2530_FLASH_SIZE        (0x40000)          /* 256KB */
#define CC2530_FLASH_END         (CC2530_FLASH_BASE+CC2530_FLASH_SIZE-1)

/** debug 层控制 **/
// 0xff 显示所有层的信息
// 0x00 所有层的信息都不显示
#define DEBUG           0x1E        // 0x01-显示 debug 信息 
                                    // 0x02-显示 error 信息 
                                    // 0x04-显示 warning 信息 
                                    // 0x08-显示 info 信息 
                                    // 0x10-显示 at 信息
#define ERR "ERROR:"
#define WARNING "WARNING:"
#define INFO "INFORMATION:"

#if (DEBUG & 0x01)
#define debug printk
#else
#define debug(...)
#endif

#if (DEBUG & 0x02)
#define debug_err printk
#else
#define debug_err(...)
#endif

#if (DEBUG & 0x04)
#define debug_war printk
#else
#define debug_war(...)
#endif

#if (DEBUG & 0x08)
#define debug_info printk
#else
#define debug_info(...)
#endif

#if (DEBUG & 0x10)
#define debug_at printk
#else
#define debug_at(...)
#endif
                                            
#endif
