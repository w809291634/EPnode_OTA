#ifndef __IAP_CONFIG_H__
#define __IAP_CONFIG_H__

#define BIT_0       (1<<0)
#define BIT_1       (1<<1)
#define BIT_2       (1<<2)
#define BIT_3       (1<<3)
#define BIT_4       (1<<4)
#define BIT_5       (1<<5)
#define BIT_6       (1<<6)
#define BIT_7       (1<<7)
#define ARRAY_SIZE(p) (sizeof(p)/sizeof(p[0]))

/* ������hal_board_cfg.h����������ο�,�ٷ�bootloader��ʹ������ */
#define HAL_SB_IMG_ADDR       0x2000
#define HAL_SB_CRC_ADDR       0x2090
// Size of internal flash less 4 pages for boot loader, 6 pages for NV, & 1 page for lock bits.
#define HAL_SB_IMG_SIZE      (0x40000 - 0x2000 - 0x3000 - 0x0800)   

/** ϵͳ�������� **/
typedef struct{
  // app����
  unsigned char current_part;             // ��ǰ���ڷ��� 1;0xff ������APP,����boot
  unsigned char app1_flag;                // �������
}sys_parameter_t;
extern sys_parameter_t sys_parameter;

/** FLASH��ַ **/
// ��ʼ��ַ�ʹ�С
#define CC2530_FLASH_BASE        (0x0000) 	        // cc2530 FLASH����ʼ��ַ
#define CC2530_FLASH_SIZE        (0x40000)          /* 256KB */
#define CC2530_FLASH_END         (CC2530_FLASH_BASE+CC2530_FLASH_SIZE-1)
#define CC2530_FLASH_MAX_PAGE    (128)

/** ������Ϣ **/
#define APP_OK      0x4D
#define APP_ERR     0xFF

#define BOOT_PARTITION_START_ADDR         CC2530_FLASH_BASE      
#define BOOT_PARTITION_END_ADDR           (PARA_PARTITION_START_ADDR-1)  // bootloader ������СΪ18KB       
#define PARA_PARTITION_START_ADDR         (0x4800)              // ����������ʼ��ַ
#define PARA_PARTITION_PAGE               (9)                   // ������������ҳ
#define PARA_PARTITION_SIZE               (0x800)               // ����������С2KB

#define APP1_PARTITION_START_ADDR         (long)(0x5000)       // APP������ʼ��ַ
// Size of internal flash less 4 pages for boot loader, 6 pages for NV(12KB), & 1 page for lock bits(2KB).
#define APP1_PARTITION_SIZE               (long)(CC2530_FLASH_SIZE - APP1_PARTITION_START_ADDR - 0x3000 - 0x0800)     // 222KB

// �����ϴ�����(û��ʹ��)
#define FLASH_IMAGE_SIZE                 (uint32) (CC2530_FLASH_SIZE - (APP1_PARTITION_START_ADDR - CC2530_FLASH_BASE)) // û������

/** �������� **/
#define SYS_PARAMETER_WORDSIZE                (sizeof(sys_parameter)/4+((sizeof(sys_parameter)%4)?1:0))     // ���ٸ���
#define SYS_PARAMETER_READ                {HalFlashRead(PARA_PARTITION_PAGE,0,\
                                            (uint8*)&sys_parameter,sizeof(sys_parameter));}
#define SYS_PARAMETER_WRITE               {HalFlashWrite(PARA_PARTITION_START_ADDR/4,\
                                            (uint8*)&sys_parameter,SYS_PARAMETER_WORDSIZE);}
/** debug ����� **/
// 0xff ��ʾ���в����Ϣ
// 0x00 ���в����Ϣ������ʾ
#define DEBUG           0x1E        // 0x01-��ʾ debug ��Ϣ 
                                    // 0x02-��ʾ error ��Ϣ 
                                    // 0x04-��ʾ warning ��Ϣ 
                                    // 0x08-��ʾ info ��Ϣ 
                                    // 0x10-��ʾ at ��Ϣ
#define ERR "ERROR:"
#define WARNING "WARNING:"
#define INFO "INFORMATION:"

#if (DEBUG & 0x01)
#define debug printf
#else
#define debug(...)
#endif

#if (DEBUG & 0x02)
#define debug_err printf
#else
#define debug_err(...)
#endif

#if (DEBUG & 0x04)
#define debug_war printf
#else
#define debug_war(...)
#endif

#if (DEBUG & 0x08)
#define debug_info printf
#else
#define debug_info(...)
#endif

#if (DEBUG & 0x10)
#define debug_at printf
#else
#define debug_at(...)
#endif
                                            
#endif
