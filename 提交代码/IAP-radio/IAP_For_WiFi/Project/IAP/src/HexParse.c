/****************************************Copyright (c)**************************************************
**                               	����������ؼ������޹�˾
**                                     		ͨѶ������
**                                        
**
**                                		 http://www.dfmc.cc
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: HexParse.c
**��   ��   ��: ���ĳ�
**����޸�����: 2010��12��16��
**��        ��: ϵͳ������
**              
**--------------��ʷ�汾��Ϣ----------------------------------------------------------------------------
** ������: ���ĳ�
** ��  ��: v1.6
** �ա���: 2010��12��16��
** �衡��: �����汾
**
**--------------��ǰ�汾�޶�------------------------------------------------------------------------------
** �޸���: ���ĳ�
** �ա���: 2010��12��16��
** �衡��: 
**
**----------------------------------------------------------------------------------------------------
********************************************************************************************************/

//#include <includes.h>
#include "HexParse.h"
//#include "sst25vf016bdrv.h"
#include "ymodem.h"
#include "flash_if.h"
#include "stdio.h"

/*********************************************************************************************************
** ��������: void HexParse(char *buf,int len)
** ��������: ����Hex�ļ�
** ��ڲ���:  Hex�ļ����� ������
** ���ڲ���:  ��
** ˵����     ��Ϊascill��ʽ  ת��Ϊʮ�����ƴ���Flash����δУ��
*********************************************************************************************************/
static int PacketLenghth=0;
// /*********************************************************************************************************
//** ��������: command_reset
//** ��������: ϵͳ��λ��
//** �䡡��: cp		Ҫ�������ݵĻ�����ָ��
//** �䡡��: ��
//** ȫ�ֱ���: ��
//** ����ģ��: 
//**
//** ������: 
//** �ա���: 
//**-------------------------------------------------------------------------------------------------------
//** �޸���: 
//** �ա���: 
//**------------------------------------------------------------------------------------------------------
//********************************************************************************************************/
//__asm void SystemReset(void) 
//{ 
// MOV R0, #1           //;  
// MSR FAULTMASK, R0    //; ���FAULTMASK ��ֹһ���ж� 
// LDR R0, =0xE000ED0C  //; 
// LDR R1, =0x05FA0004  //; 
// STR R1, [R0]         //; ϵͳ�����λ  

//deadloop 
//    B deadloop        //; �ȴ���λ�ɹ�
//}
/*********************************************************************************************************
** ��������:  void IAPPrograming(unsigned CodeAddress,unsigned endaddress) 
** ��������:  IAP���
** ��ڲ���:  filelen  �����hex�ļ�ĩ��ַ,	CodeAddress  Hex�ļ���SPI Flash�е�ƫ�Ƶ�ַ
** ���ڲ���:  ��
*********************************************************************************************************/
unsigned char ram[516] __attribute__((at(0x2001C000))); //���ݻ�����;
void IAPPrograming(unsigned CodeAddress){
	extern int Image$$ER_IROM1$$Base; //��ɢ���ض�
  unsigned char data[20];
  unsigned char len=0,j=0;
  unsigned base=0,offset=0;
  unsigned i=0;  
  unsigned char *pdat=data;
	unsigned firstseg=0;
  pStrHex pst = (pStrHex)data;
//  EraseSector(FLASHSARTADDRESS);
start:   
  SSTF016B_RD(CodeAddress+i,1,&len);
	if(len!=0){
	  if(len<17){
		  i++;
		  SSTF016B_RD(CodeAddress+i,len+4,pdat);
		  i+=len+4;
		  switch(pst->type){		  
		    case  0x00:			//�����У�ִ��flash���
					offset =*pdat;
					offset=(offset<<8)+*(pdat+1);//�������ƫ�Ƶ�ַ			    	 
					offset = base + offset;
					if(offset>=APPLICATION_ADDRESS){	  //IAP����Σ����ܲ���
						  GPIO_ToggleBits(GPIOA,GPIO_Pin_15);
              if(firstseg==0){
								if(0!=FLASH_If_Erase(offset,offset)){
									DPRINTK("Seg:%d Erase faile\n",FLASH_GetSector(offset));
									SystemReset(); 
								}
								else
									DPRINTK( "\33[1A\33[KSeg:%d erased\n",FLASH_GetSector(offset));
								firstseg=1;
					    }						
							if(firstseg==1 && FLASH_GetSector(offset+len)!=FLASH_GetSector(offset)){									//�Ƿ���һ���¶Σ��¶εĻ���Ҫִ�в���
								FLASH_If_Erase(offset+len,offset+len);
								DPRINTK("\33[1A\33[KSeg:%d Erased\n",FLASH_GetSector(offset+len));
							}
							for(j=0;j<len;j++)
								ram[j]=*(pdat+3+j);
							if(0!=FLASH_If_Write(&offset,((unsigned *)(&ram[0])),len/4))
								DPRINTK("Flash Addr:%x Program Fail\n",offset);
					}else{
						printk("��̷Ƿ���ַ,ϵͳ��λ\r\n");
						SystemReset();
					}
			 break;
			case  0x01:
			 break;
			case  0x02:
			 break;
			case  0x03:
			 break;
			case  0x04:		   //��չ�����Լ�¼
			   base = *(pdat+3);
			   base =(base<<8)+ *(pdat+4);  //�����ַ 		   
			   base <<=16;
				break;
			case  0x05:
			default:
				break;
		  };
		  goto start;
		}	   //end of  if(len<17)
		else{
			  printk("��̴���,ϵͳ��λ\r\n");
				SystemReset();
				//ϵͳ��λ			    
		}
	} //end of if(len!=0)
	printk("\33[1A\33[K");
	printk("������³ɹ�,ϵͳ��λ\r\n");
	SystemReset();
}

/*********************************************************************************************************
** ��������: __inline void HexPacketReset(void)
** ��������: ��λ��������
** ��ڲ���:  ��
** ���ڲ���:  ��
** ˵����     
*********************************************************************************************************/

 void HexPacketReset(void){	

   PacketLenghth=0;
}
void HexParse(unsigned char buf[],int len){

	 int i;
	 static char dat=0,flag=0;
	 static unsigned char tmptarr[22];
	 static unsigned char index=0;
	 static unsigned int  addroffset=OFFSET;

	 for(i=0;i<len;i++){
	 
	    switch(buf[i]){
		
		  case 0x3A: 		      
			  break;
     	  case 0x0D:
		      break;
		  case 0x0A:
		      SSTF016B_WR(addroffset,tmptarr,tmptarr[0]+5);  //дFlash	 			  
					addroffset=OFFSET+PacketLenghth;
					index=0;
		      break;
		  case 0x30:   //0
		  case 0x31:   //1
		  case 0x32:   //2
		  case 0x33:   //3
		  case 0x34:   //4
		  case 0x35:   //5
		  case 0x36:   //6
		  case 0x37:   //7
		  case 0x38:   //8
		  case 0x39:   //9
		   if(25>index){
		     if(flag==FALSE){
			   dat=buf[i]-'0';
			   flag=TRUE;
			 }
			 else{
			  dat= (dat<<4)+(buf[i]-'0');
			  tmptarr[index++]=dat;
			  flag=FALSE;
			  PacketLenghth++;
			 }
		   }
		   break;
		  case 0x41:   //A
		  case 0x42:   //B
		  case 0x43:   //C
		  case 0x44:   //D
		  case 0x45:   //E
		  case 0x46:   //F
		     if(25>index){
			   if(flag==FALSE){
			     dat=buf[i]-'7';
				 flag=TRUE;
			   }
			   else{
			     dat= (dat<<4)+(buf[i]-'7');
			     tmptarr[index++]=dat;
			     flag=FALSE;
				 PacketLenghth++;
			   }
			 }
		    break;
		  default:
		    break;
		}
	 }   
}
/*********************************************************************************************************
** ��������: unsigned char  HexFileVerify(void)
** ��������: Hex�ļ�У��
** ��ڲ���: ��
** ���ڲ���: У����ȷ TRUE   У�����FALSE
** ˵����    ת�����ʮ�������ļ�У�� 
*********************************************************************************************************/
unsigned   HexFileVerify(unsigned *ret){

  unsigned index=0;
	unsigned dat=0,dat1=0,dat2=0;
	unsigned char i=0,j=0,len=0;
	unsigned char temarr[20];
	unsigned filelen=0;

start:
	SSTF016B_RD(index+OFFSET,1,&len);  //��ȡ�г���
	if(len==0){				           //�ж��Ƿ񵽴��ļ�ĩβ
	 SSTF016B_RD(index+OFFSET,5,temarr);
	 filelen=index;
	 if(temarr[1]==0x00&&temarr[2]==0x00&&temarr[3]==0x01&&temarr[4]==0xFF){	//�ļ��� 00000001FF	��β 
	   i=0xAA;
	   SSTF016B_WR(STOPADDR-8,&i,1);
	   return filelen;
	 }
	 else
	   return FALSE ;
	 }
	 else if(len>16){
	   return FALSE;
	 }
	 index++;
	 SSTF016B_RD(index+OFFSET,len+4,temarr);
     index+=len+4;
	 dat=len;
	 if(j%2)
	   dat1=(temarr[0]<<8)+temarr[1];
	 else
	   dat2=(temarr[0]<<8)+temarr[1];
	 
	 if(temarr[2]==0x05){   
	   if(j%2)
		*ret=dat2;		
	   else 
	    *ret=dat1;
	 }
	 j++;

	 for(i=0;i<(len+4);i++){ 	 
	   dat+=temarr[i];
	 }
	 if((dat%0x100)==0){		  
	   
	    goto start;		   
	  }
	 else
	  return FALSE;	
	  	 
}
