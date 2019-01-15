/*************************************************************************************************************
 * �ļ���		:	STM32Flash_Lock.c
 * ����			:	STM32 �ڲ�FLASH��д����غ���
 * ����			:	cp1300@139.com
 * ����ʱ��		:	2017-11-17
 * ����޸�ʱ��	:	2017-11-17
 * ��ϸ			:	����STM32 �ڲ�FLASH��д����غ���
*************************************************************************************************************/
#ifndef __STM32FLASH_LOCK_H__
#define __STM32FLASH_LOCK_H__
#include "system.h"  


//flash���״̬
typedef enum
{ 
  FLASH_BUSY = 1,		//æ
  FLASH_ERROR_PG,		//��̴���
  FLASH_ERROR_WRP,		//д����
  FLASH_COMPLETE,		//���
  FLASH_TIMEOUT			//��ʱ
}FLASH_STATUS;


bool STM32FLASH_GetReadOutProtectionStatus(void);							//��ȡSTM32���������״̬
FLASH_STATUS STM32FLASH_EraseOptionBytes(void);								//����ѡ���ֽ�
FLASH_STATUS STM32FLASH_ReadOutProtection(bool isEnableReadOutProtection);	//������رն�����

#endif	//__STM32FLASH_LOCK_H__

















