/*************************************************************************************************************
 * 文件名:		STM32Flash.c
 * 功能:		STM32 内部FLASH编程驱动函数
 * 作者:		cp1300@139.com
 * 创建时间:		2013-10-20
 * 最后修改时间:	2013-10-20
 * 详细:		用于STM32内部flash读写驱动
*************************************************************************************************************/
#ifndef __STM32FLASH_H__
#define __STM32FLASH_H__
#include "system.h"  



//STM32 FLASH选择
#define STM32_FLASH_SIZE 	512 	 		//所选STM32的FLASH容量大小(单位为KB)
#define STM32_FLASH_WREN 	1              	//使能FLASH写入(0，不是能;1，使能)


//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 		//STM32 FLASH的起始地址


//FLASH解锁键值
#define FLASH_KEY1               0X45670123
#define FLASH_KEY2               0XCDEF89AB


//API
void STM32FLASH_Unlock(void);											//FLASH解锁
void STM32FLASH_Lock(void);					  							//FLASH上锁
u8 STM32FLASH_GetStatus(void);				  							//获得状态
u8 STM32FLASH_WaitDone(u16 time);				  						//等待操作结束
u8 STM32FLASH_ErasePage(u32 paddr);			  							//擦除页
u8 STM32FLASH_WriteHalfWord(u32 faddr, u16 dat);						//写入半字
u16 STM32FLASH_ReadHalfWord(u32 faddr);		 							//读出半字  
void STM32FLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//指定地址开始写入指定长度的数据
u32 STM32FLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//指定地址开始读取指定长度数据
u8 STM32FLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);			//从指定地址开始写入指定长度的数据
void STM32FLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据
u8 STM32FLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite); //不检查的写入 



#endif	//__STM32FLASH_H__

















