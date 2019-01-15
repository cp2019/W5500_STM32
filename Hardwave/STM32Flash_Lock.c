/*************************************************************************************************************
 * �ļ���		:	STM32Flash_Lock.c
 * ����			:	STM32 �ڲ�FLASH��д����غ���
 * ����			:	cp1300@139.com
 * ����ʱ��		:	2017-11-17
 * ����޸�ʱ��	:	2017-11-17
 * ��ϸ			:	����STM32 ��д�������ã�ʹ�ùٷ����޸�
*************************************************************************************************************/
#include "STM32Flash.h"
#include "STM32Flash_Lock.h"
#include "delay.h"
#include "system.h"

/* FLASH MASK */
#define RDPRT_MASK              	((u32)0x00000002)
#define WRP0_MASK                	((u32)0x000000FF)
#define WRP1_MASK                	((u32)0x0000FF00)
#define WRP2_MASK               	((u32)0x00FF0000)
#define WRP3_MASK               	((u32)0xFF000000)
#define OB_USER_BFB2             	((u32)0x0008)

#define RDP_KEY                  	((u16)0x00A5)

#define FLASE_ERASE_TIME          	((u32)0x000B0000)
#define FLASH_PROGRAM_TIME        	((u32)0x00002000)

/* Flash Control Register bits */
#define CR_PG_SET                	((u32)0x00000001)
#define CR_PG_RESET              	((u32)0x00001FFE) 
#define CR_PER_SET               	((u32)0x00000002)
#define CR_PER_RESET            	((u32)0x00001FFD)
#define CR_MER_SET               	((u32)0x00000004)
#define CR_MER_RESET             	((u32)0x00001FFB)
#define CR_OPTPG_SET            	((u32)0x00000010)
#define CR_OPTPG_RESET          	((u32)0x00001FEF)
#define CR_OPTER_SET             	((u32)0x00000020)
#define CR_OPTER_RESET           	((u32)0x00001FDF)
#define CR_STRT_SET             	((u32)0x00000040)
#define CR_LOCK_SET              	((u32)0x00000080)







/*************************************************************************************************************************
* ����			:	bool STM32FLASH_GetReadOutProtectionStatus(void)
* ����			:	��ȡSTM32���������״̬
* ����			:	��
* ����			:	TRUE:�����˶�������FALSE:������д����
* ����			:	�ײ�
* ����			:	cp1300@139.com
* ʱ��			:	2017-11-17
* ����޸�ʱ�� 	: 	2017-11-17
* ˵��			: 	
*************************************************************************************************************************/
bool STM32FLASH_GetReadOutProtectionStatus(void)
{
	//uart_printf("FLASH->OBR:0x%X\r\nOB->RDP=0x%02X\r\n",FLASH->OBR, OB->RDP);
	if ((FLASH->OBR & RDPRT_MASK) != 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*************************************************************************************************************************
* ����			:	u8 STM32FLASH_WaitDone(u16 time)
* ����			:	�ȴ��������
* ����			:	time:Ҫ��ʱ�ĳ���,��λus
* ����			:	0:���;1:æ;2:����;3:д����
* ����			:	�ײ�
* ����			:	cp1300@139.com
* ʱ��			:	2017-11-17
* ����޸�ʱ�� 	: 	2017-11-17
* ˵��			: 	
*************************************************************************************************************************/
FLASH_STATUS STM32FLASH_WaitOperation(u32 time)
{
	u8 res;
	
	do
	{
		res=STM32FLASH_GetStatus();
		if(res!=1)break;//��æ,����ȴ���,ֱ���˳�.
		Delay_US(1);
		time--;
	}while(time);
	if(time ==0) return FLASH_TIMEOUT;	//��ʱ
	switch(res)
	{
		case 0: return FLASH_COMPLETE;	//�������
		case 1:	return FLASH_BUSY;		//æ
		case 2:	return FLASH_ERROR_PG;	//��̴���
		case 3: return FLASH_ERROR_WRP;	//д����
		default: return FLASH_TIMEOUT;
	}
}	


/*************************************************************************************************************************
* ����			:	FLASH_STATUS FLASH_EraseOptionBytes(void)
* ����			:	����ѡ���ֽ�
* ����			:	��
* ����			:	FLASH_STATUS
* ����			:	�ײ�
* ����			:	cp1300@139.com
* ʱ��			:	2017-11-17
* ����޸�ʱ�� 	: 	2017-11-17
* ˵��			: 	
*************************************************************************************************************************/
FLASH_STATUS STM32FLASH_EraseOptionBytes(void)
{
  u16 rdptmp = RDP_KEY;

  FLASH_STATUS status = FLASH_COMPLETE;

  /* Get the actual read protection Option Byte value */ 
  if(STM32FLASH_GetReadOutProtectionStatus() != FALSE)
  {
    rdptmp = 0x00;  
  }

  status = STM32FLASH_WaitOperation(FLASE_ERASE_TIME);
  if(status == FLASH_COMPLETE)
  {
    /* Authorize the small information block programming */
    FLASH->OPTKEYR = FLASH_KEY1;
    FLASH->OPTKEYR = FLASH_KEY2;
    
    /* if the previous operation is completed, proceed to erase the option bytes */
    FLASH->CR |= CR_OPTER_SET;
    FLASH->CR |= CR_STRT_SET;
    /* Wait for last operation to be completed */
    status = STM32FLASH_WaitOperation(FLASE_ERASE_TIME);
    
    if(status == FLASH_COMPLETE)
    {
      /* if the erase operation is completed, disable the OPTER Bit */
      FLASH->CR &= CR_OPTER_RESET;
       
      /* Enable the Option Bytes Programming operation */
      FLASH->CR |= CR_OPTPG_SET;
      /* Restore the last read protection Option Byte value */
      OB->RDP = (u16)rdptmp; 
      /* Wait for last operation to be completed */
      status = STM32FLASH_WaitOperation(FLASH_PROGRAM_TIME);
 
      if(status != FLASH_TIMEOUT)
      {
        /* if the program operation is completed, disable the OPTPG Bit */
        FLASH->CR &= CR_OPTPG_RESET;
      }
    }
    else
    {
      if (status != FLASH_TIMEOUT)
      {
        /* Disable the OPTPG Bit */
        FLASH->CR &= CR_OPTPG_RESET;
      }
    }  
  }
  /* Return the erase status */
  return status;
}


/*************************************************************************************************************************
* ����			:	FLASH_STATUS STM32FLASH_ReadOutProtection(bool isEnableReadOutProtection)
* ����			:	������رն�����
* ����			:	��
* ����			:	FLASH_STATUS
* ����			:	�ײ�
* ����			:	cp1300@139.com
* ʱ��			:	2017-11-17
* ����޸�ʱ�� 	: 	2017-11-17
* ˵��			: 	
*************************************************************************************************************************/
FLASH_STATUS STM32FLASH_ReadOutProtection(bool isEnableReadOutProtection)
{
  FLASH_STATUS status = FLASH_COMPLETE;

  status = STM32FLASH_WaitOperation(FLASE_ERASE_TIME);
  if(status == FLASH_COMPLETE)
  {
    /* Authorizes the small information block programming */
    FLASH->OPTKEYR = FLASH_KEY1;
    FLASH->OPTKEYR = FLASH_KEY2;
    FLASH->CR |= CR_OPTER_SET;
    FLASH->CR |= CR_STRT_SET;
    /* Wait for last operation to be completed */
    status = STM32FLASH_WaitOperation(FLASE_ERASE_TIME);
    if(status == FLASH_COMPLETE)
    {
      /* if the erase operation is completed, disable the OPTER Bit */
      FLASH->CR &= CR_OPTER_RESET;
      /* Enable the Option Bytes Programming operation */
      FLASH->CR |= CR_OPTPG_SET; 
      if(isEnableReadOutProtection != FALSE)
      {
		OB->RDP = 0x00;	//����������
      }
      else	//�رն�����
      {
        OB->RDP = RDP_KEY;  
      }
      /* Wait for last operation to be completed */
      status = STM32FLASH_WaitOperation(FLASE_ERASE_TIME);
    
      if(status != FLASH_TIMEOUT)
      {
        /* if the program operation is completed, disable the OPTPG Bit */
        FLASH->CR &= CR_OPTPG_RESET;
      }
    }
    else 
    {
      if(status != FLASH_TIMEOUT)
      {
        /* Disable the OPTER Bit */
        FLASH->CR &= CR_OPTER_RESET;
      }
    }
  }
  /* Return the protection operation Status */
  return status;       
}


/*************************************************************************************************************************
* ����			:	u32 STM32FLASH_GetWriteProtectionOptionByte(void)
* ����			:	��ȡд����״̬
* ����			:	��
* ����			:	д����״̬
* ����			:	�ײ�
* ����			:	cp1300@139.com
* ʱ��			:	2017-11-17
* ����޸�ʱ�� 	: 	2017-11-17
* ˵��			: 	
*************************************************************************************************************************/
u32 STM32FLASH_GetWriteProtectionOptionByte(void)
{
  /* Return the Flash write protection Register value */
  return (u32)(FLASH->WRPR);
}

















