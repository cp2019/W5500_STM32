/*************************************************************************************************************
 * 文件名		:	STM32Flash_Lock.c
 * 功能			:	STM32 内部FLASH读写锁相关函数
 * 作者			:	cp1300@139.com
 * 创建时间		:	2017-11-17
 * 最后修改时间	:	2017-11-17
 * 详细			:	用于STM32 内部FLASH读写锁相关函数
*************************************************************************************************************/
#ifndef __STM32FLASH_LOCK_H__
#define __STM32FLASH_LOCK_H__
#include "system.h"  


//flash编程状态
typedef enum
{ 
  FLASH_BUSY = 1,		//忙
  FLASH_ERROR_PG,		//编程错误
  FLASH_ERROR_WRP,		//写保护
  FLASH_COMPLETE,		//完成
  FLASH_TIMEOUT			//超时
}FLASH_STATUS;


bool STM32FLASH_GetReadOutProtectionStatus(void);							//获取STM32闪存读保护状态
FLASH_STATUS STM32FLASH_EraseOptionBytes(void);								//擦除选项字节
FLASH_STATUS STM32FLASH_ReadOutProtection(bool isEnableReadOutProtection);	//开启或关闭读保护

#endif	//__STM32FLASH_LOCK_H__

















