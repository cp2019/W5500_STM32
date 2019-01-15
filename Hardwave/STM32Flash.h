/*************************************************************************************************************
 * �ļ���:		STM32Flash.c
 * ����:		STM32 �ڲ�FLASH�����������
 * ����:		cp1300@139.com
 * ����ʱ��:		2013-10-20
 * ����޸�ʱ��:	2013-10-20
 * ��ϸ:		����STM32�ڲ�flash��д����
*************************************************************************************************************/
#ifndef __STM32FLASH_H__
#define __STM32FLASH_H__
#include "system.h"  



//STM32 FLASHѡ��
#define STM32_FLASH_SIZE 	512 	 		//��ѡSTM32��FLASH������С(��λΪKB)
#define STM32_FLASH_WREN 	1              	//ʹ��FLASHд��(0��������;1��ʹ��)


//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 		//STM32 FLASH����ʼ��ַ


//FLASH������ֵ
#define FLASH_KEY1               0X45670123
#define FLASH_KEY2               0XCDEF89AB


//API
void STM32FLASH_Unlock(void);											//FLASH����
void STM32FLASH_Lock(void);					  							//FLASH����
u8 STM32FLASH_GetStatus(void);				  							//���״̬
u8 STM32FLASH_WaitDone(u16 time);				  						//�ȴ���������
u8 STM32FLASH_ErasePage(u32 paddr);			  							//����ҳ
u8 STM32FLASH_WriteHalfWord(u32 faddr, u16 dat);						//д�����
u16 STM32FLASH_ReadHalfWord(u32 faddr);		 							//��������  
void STM32FLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
u32 STM32FLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//ָ����ַ��ʼ��ȡָ����������
u8 STM32FLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);			//��ָ����ַ��ʼд��ָ�����ȵ�����
void STM32FLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
u8 STM32FLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite); //������д�� 



#endif	//__STM32FLASH_H__

















