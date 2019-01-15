/*************************************************************************************************************
 * 文件名		:	STM32Flash_Lock.c
 * 功能			:	STM32 内部FLASH读写锁相关函数
 * 作者			:	cp1300@139.com
 * 创建时间		:	2017-11-17
 * 最后修改时间	:	2017-11-17
 * 详细			:	用于STM32 读写保护设置，使用官方库修改
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
* 函数			:	bool STM32FLASH_GetReadOutProtectionStatus(void)
* 功能			:	获取STM32闪存读保护状态
* 参数			:	无
* 返回			:	TRUE:开启了读保护；FALSE:开启了写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2017-11-17
* 最后修改时间 	: 	2017-11-17
* 说明			: 	
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
* 函数			:	u8 STM32FLASH_WaitDone(u16 time)
* 功能			:	等待操作完成
* 参数			:	time:要延时的长短,单位us
* 返回			:	0:完成;1:忙;2:错误;3:写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2017-11-17
* 最后修改时间 	: 	2017-11-17
* 说明			: 	
*************************************************************************************************************************/
FLASH_STATUS STM32FLASH_WaitOperation(u32 time)
{
	u8 res;
	
	do
	{
		res=STM32FLASH_GetStatus();
		if(res!=1)break;//非忙,无需等待了,直接退出.
		Delay_US(1);
		time--;
	}while(time);
	if(time ==0) return FLASH_TIMEOUT;	//超时
	switch(res)
	{
		case 0: return FLASH_COMPLETE;	//操作完成
		case 1:	return FLASH_BUSY;		//忙
		case 2:	return FLASH_ERROR_PG;	//编程错误
		case 3: return FLASH_ERROR_WRP;	//写保护
		default: return FLASH_TIMEOUT;
	}
}	


/*************************************************************************************************************************
* 函数			:	FLASH_STATUS FLASH_EraseOptionBytes(void)
* 功能			:	擦除选项字节
* 参数			:	无
* 返回			:	FLASH_STATUS
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2017-11-17
* 最后修改时间 	: 	2017-11-17
* 说明			: 	
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
* 函数			:	FLASH_STATUS STM32FLASH_ReadOutProtection(bool isEnableReadOutProtection)
* 功能			:	开启或关闭读保护
* 参数			:	无
* 返回			:	FLASH_STATUS
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2017-11-17
* 最后修改时间 	: 	2017-11-17
* 说明			: 	
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
		OB->RDP = 0x00;	//开启读保护
      }
      else	//关闭读保护
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
* 函数			:	u32 STM32FLASH_GetWriteProtectionOptionByte(void)
* 功能			:	获取写保护状态
* 参数			:	无
* 返回			:	写保护状态
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2017-11-17
* 最后修改时间 	: 	2017-11-17
* 说明			: 	
*************************************************************************************************************************/
u32 STM32FLASH_GetWriteProtectionOptionByte(void)
{
  /* Return the Flash write protection Register value */
  return (u32)(FLASH->WRPR);
}

















