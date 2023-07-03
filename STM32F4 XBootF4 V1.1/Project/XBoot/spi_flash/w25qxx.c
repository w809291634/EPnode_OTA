/*********************************************************************************************
* �ļ�: w25qxx.c
* ���ߣ�zonesion 2016.12.22
* ˵�����ⲿflash�����ļ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "w25qxx.h"
#include "spi.h"

/*********************************************************************************************
* ȫ�ֱ���
*********************************************************************************************/

u16 W25QXX_TYPE=W25Q64;	                                        //Ĭ����W25Q64


//spi��дһ���ֽ�
#define W25qxxSPI_ReadWriteByte(dat)  SPI3_ReadWriteByte(dat)

//����SPI����
#define W25qxxSPI_SetSpeed(SPI_BaudRatePrescaler)  SPI3_SetSpeed(SPI_BaudRatePrescaler)


/*********************************************************************************************
* ����:W25QXX_Init
* ����:��ʼ��SPI FLASH��IO��
* ����:��
* ����:��
* �޸�:
* ע��: 4KbytesΪһ��Sector 16������Ϊ1��Block 
*       W25Q64����Ϊ8M�ֽ�,����64��Block,2048��Sector
*********************************************************************************************/

void W25QXX_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(W25QXX_CS_RCC, ENABLE );               //PORTʱ��ʹ��
  GPIO_InitStructure.GPIO_Pin = W25QXX_CS_PIN;                  //PIN
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                 //�������
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;            //100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                  //����
  GPIO_Init(W25QXX_CS_GPIO, &GPIO_InitStructure);               //��ʼ��
  W25QXX_CS_H;				                                    //SPI FLASH��ѡ��
  
  SPI3_Init();		   	                                        //��ʼ��SPI
  W25qxxSPI_SetSpeed(SPI_BaudRatePrescaler_2);                  //����Ϊ18Mʱ��,����ģʽ
  
  W25QXX_TYPE=W25QXX_ReadID();                                  //��ȡFLASH ID. 
}

void W25qxx_CsEnable()
{
  W25qxxSPI_SetSpeed(SPI_BaudRatePrescaler_2);
  W25QXX_CS_L;
}

void W25qxx_CsDisable()
{
  W25QXX_CS_H;                                                  // ����������
}

/*********************************************************************************************
* ����:W25QXX_ReadSR
* ����:��ȡW25QXX��״̬�Ĵ���
* ����:��
* ����:byte
* �޸�:
* ע��: 
*       BIT7  6   5   4   3   2   1   0
*       SPR   RV  TB BP2 BP1 BP0 WEL BUSY
*       SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
*       TB,BP2,BP1,BP0:FLASH����д��������
*       WEL:дʹ������
*       BUSY:æ���λ(1,æ;0,����)
*       Ĭ��:0x004
*********************************************************************************************/
u8 W25QXX_ReadSR(void)
{
  u8 byte=0;
  W25qxx_CsEnable();                                                  //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_ReadStatusReg);                       //���Ͷ�ȡ״̬�Ĵ�������
  byte=W25qxxSPI_ReadWriteByte(0Xff);                                //��ȡһ���ֽ�
  W25qxx_CsDisable();                                                  //ȡ��Ƭѡ
  return byte;
}
/*********************************************************************************************
* ����:W25QXX_Write_SR
* ����:дW25QXX״̬�Ĵ���
* ����:��
* ����:��
* �޸�:
* ע��: ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
*********************************************************************************************/
void W25QXX_Write_SR(u8 sr)
{
  W25qxx_CsEnable();                                                  //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_WriteStatusReg);                      //����дȡ״̬�Ĵ�������
  W25qxxSPI_ReadWriteByte(sr);               	                //д��һ���ֽ�
  W25qxx_CsDisable();                                                  //ȡ��Ƭѡ
}
/*********************************************************************************************
* ����:W25QXX_Write_Enable
* ����:W25QXXдʹ�ܣ���WEL��λ
* ����:��
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/

void W25QXX_Write_Enable(void)
{
  W25qxx_CsEnable();                          	                //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_WriteEnable); 	                //����дʹ��
  W25qxx_CsDisable();                           	                //ȡ��Ƭѡ
}
/*********************************************************************************************
* ����:W25QXX_Write_Disable
* ����:W25QXXд��ֹ����WEL����
* ����:��
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/

void W25QXX_Write_Disable(void)
{
  W25qxx_CsEnable();                                                  //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_WriteDisable);                        //����д��ָֹ��
  W25qxx_CsDisable();                                                  //ȡ��Ƭѡ
}
//��ȡоƬID
//����ֵ����:

/*********************************************************************************************
* ����:W25QXX_ReadID
* ����:��ȡоƬID
* ����:��
* ����:Temp��
*       0XEF13,��ʾоƬ�ͺ�ΪW25Q80
*       0XEF14,��ʾоƬ�ͺ�ΪW25Q16
*       0XEF15,��ʾоƬ�ͺ�ΪW25Q32
*       0XEF16,��ʾоƬ�ͺ�ΪW25Q64
*       0XEF17,��ʾоƬ�ͺ�ΪW25Q128
* �޸�:
* ע��: 
*********************************************************************************************/

u16 W25QXX_ReadID(void)
{
  u16 Temp = 0;
  W25qxx_CsEnable();
  W25qxxSPI_ReadWriteByte(0x90);                                     //���Ͷ�ȡID����
  W25qxxSPI_ReadWriteByte(0x00);
  W25qxxSPI_ReadWriteByte(0x00);
  W25qxxSPI_ReadWriteByte(0x00);
  Temp|=W25qxxSPI_ReadWriteByte(0xFF)<<8;
  Temp|=W25qxxSPI_ReadWriteByte(0xFF);
  W25qxx_CsDisable();
  return Temp;
}
/*********************************************************************************************
* ����:W25QXX_ReadID
* ����:��ȡSPI FLASH��ָ����ַ��ʼ��ȡָ�����ȵ�����
* ����:pBuffer-���ݴ洢��  ReadAddr-��ʼ��ȡ�ĵ�ַ(24bit)
*      NumByteToRead-Ҫ��ȡ���ֽ���(���65535)
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)
{
  u16 i;
  W25qxx_CsEnable();                            	                //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_ReadData);         	                //���Ͷ�ȡ����
  W25qxxSPI_ReadWriteByte((u8)((ReadAddr)>>16));  	                //����24bit��ַ
  W25qxxSPI_ReadWriteByte((u8)((ReadAddr)>>8));
  W25qxxSPI_ReadWriteByte((u8)ReadAddr);
  for(i=0;i<NumByteToRead;i++)
  {
    pBuffer[i]=W25qxxSPI_ReadWriteByte(0XFF);   	                //ѭ������
  }
  W25qxx_CsDisable();
}


/*********************************************************************************************
* ����:W25QXX_Write_Page
* ����:SPI��һҳ(0~65535)��д������256���ֽڵ����� ��ָ����ַ��ʼд�����256�ֽڵ�����
* ����:pBuffer-���ݴ洢�� 
*       NumByteToWrite-Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!
*      NumByteToRead-Ҫ��ȡ���ֽ���(���65535)
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
  u16 i;
  W25QXX_Write_Enable();                  	                //SET WEL
  W25qxx_CsEnable();                            	                //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_PageProgram);      	                //����дҳ����
  W25qxxSPI_ReadWriteByte((u8)((WriteAddr)>>16)); 	                //����24bit��ַ
  W25qxxSPI_ReadWriteByte((u8)((WriteAddr)>>8));
  W25qxxSPI_ReadWriteByte((u8)WriteAddr);
  for(i=0;i<NumByteToWrite;i++)W25qxxSPI_ReadWriteByte(pBuffer[i]);  //ѭ��д��
  W25qxx_CsDisable();                            	                //ȡ��Ƭѡ
  W25QXX_Wait_Busy();					        //�ȴ�д�����
}
/*********************************************************************************************
* ����:W25QXX_Write_NoCheck
* ����:�޼���дSPI FLASH 
*       ����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
* ����:pBuffer-���ݴ洢�� 
*       WriteAddr-��ʼд��ĵ�ַ(24bit)
*      NumByteToRead-Ҫ��ȡ���ֽ���(���65535)
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
  u16 pageremain;
  pageremain=256-WriteAddr%256;                                 //��ҳʣ����ֽ���
  if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;      //������256���ֽ�
  while(1)
  {
    W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
    if(NumByteToWrite==pageremain)break;                        //д�������
    else //NumByteToWrite>pageremain
    {
      pBuffer+=pageremain;
      WriteAddr+=pageremain;
      
      NumByteToWrite-=pageremain;			        //��ȥ�Ѿ�д���˵��ֽ���
      if(NumByteToWrite>256)pageremain=256;                     //һ�ο���д��256���ֽ�
      else pageremain=NumByteToWrite; 	                        //����256���ֽ���
    }
  };
}
/*********************************************************************************************
* ����:W25QXX_BUFFER
* ����:дSPI FLASH
*       ��ָ����ַ��ʼд��ָ�����ȵ����ݣ��ú�������������!
* ����:pBuffer-���ݴ洢�� 
*       WriteAddr-��ʼд��ĵ�ַ(24bit)
*      NumByteToRead-Ҫ��ȡ���ֽ���(���65535)
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
u32 num;
u8 W25QXX_BUFFER[4096];
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u32 NumByteToWrite)
{
  u32 secpos;
  u16 secoff;
  u32 secremain;
  u16 i;
  u8 * W25QXX_BUF;
  W25QXX_BUF=W25QXX_BUFFER;
  secpos=WriteAddr/4096;                                        //������ַ
  secoff=WriteAddr%4096;                                        //�������ڵ�ƫ��
  secremain=4096-secoff;                                        //����ʣ��ռ��С
  if(NumByteToWrite<=secremain)secremain=NumByteToWrite;        //������4096���ֽ�
  while(1)
  {
    W25QXX_Read(W25QXX_BUF,secpos*4096,4096);                   //������������������
    for(i=0;i<secremain;i++)                                    //У������
    {
      if(W25QXX_BUF[secoff+i]!=0XFF)break;                      //��Ҫ����
    }
    if(i<secremain)                                             //��Ҫ����
    {
      W25QXX_Erase_Sector(secpos);		                //�����������
      for(i=0;i<secremain;i++)	   		                //����
      {
        W25QXX_BUF[i+secoff]=pBuffer[i];
      }
      W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);        //д����������
      
    }else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);    //д�Ѿ������˵�,ֱ��д������ʣ������.
    if(NumByteToWrite==secremain)break;                         //д�������
    else                                                        //д��δ����
    {
      secpos++;                                                 //������ַ��1
      secoff=0;                                                 //ƫ��λ��Ϊ0
      
      pBuffer+=secremain;  				        //ָ��ƫ��
      WriteAddr+=secremain;				        //д��ַƫ��
      NumByteToWrite-=secremain;			        //�ֽ����ݼ�
      num=282744-NumByteToWrite;
      if(NumByteToWrite>4096)secremain=4096;                    //��һ����������д����
      else secremain=NumByteToWrite;		                //��һ����������д����
    }
  };
}
/*********************************************************************************************
* ����:W25QXX_Erase_Chip
* ����:��������оƬ
* ����:��
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_Erase_Chip(void)
{
  W25QXX_Write_Enable();                 	 	        //SET WEL
  W25QXX_Wait_Busy();
  W25qxx_CsEnable();                            	                //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_ChipErase);        	                //����Ƭ��������
  W25qxx_CsDisable();                            	                //ȡ��Ƭѡ
  W25QXX_Wait_Busy();   				        //�ȴ�оƬ��������
}
/*********************************************************************************************
* ����:W25QXX_Erase_Sector
* ����:����һ������
*       ����һ������������ʱ��:150ms
* ����:Dst_Addr-������ַ������ʵ���������ã�
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_Erase_Sector(u32 Dst_Addr)
{
  Dst_Addr*=4096;
  W25QXX_Write_Enable();                  	                //SET WEL
  W25QXX_Wait_Busy();
  W25qxx_CsEnable();                            	                //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_SectorErase);      	                //������������ָ��
  W25qxxSPI_ReadWriteByte((u8)((Dst_Addr)>>16));  	                //����24bit��ַ
  W25qxxSPI_ReadWriteByte((u8)((Dst_Addr)>>8));
  W25qxxSPI_ReadWriteByte((u8)Dst_Addr);
  W25qxx_CsDisable();                            	                //ȡ��Ƭѡ
  W25QXX_Wait_Busy();   				        //�ȴ��������
}
/*********************************************************************************************
* ����:W25QXX_Wait_Busy
* ����:�ȴ�����
* ����:
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_Wait_Busy(void)
{
  while((W25QXX_ReadSR()&0x01)==0x01);  		        // �ȴ�BUSYλ���
}
/*********************************************************************************************
* ����:W25QXX_PowerDown
* ����:�������ģʽ
* ����:
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_PowerDown(void)
{
  W25qxx_CsEnable();                           	 	        //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_PowerDown);                           //���͵�������
  W25qxx_CsDisable();                            	                //ȡ��Ƭѡ
}

/*********************************************************************************************
* ����:W25QXX_WAKEUP
* ����:����
* ����:
* ����:��
* �޸�:
* ע��: 
*********************************************************************************************/
void W25QXX_WAKEUP(void)
{
  W25qxx_CsEnable();                            	                //ʹ������
  W25qxxSPI_ReadWriteByte(W25X_ReleasePowerDown);	                //send W25X_PowerDown command 0xAB
  W25qxx_CsDisable();                            	                //ȡ��Ƭѡ
}
