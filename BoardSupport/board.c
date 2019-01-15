/*************************************************************************************************************
 * �ļ���:			board.c
 * ����:			�弶֧��
 * ����:			cp1300@139.com
 * ����ʱ��:		2013-10-20
 * ����޸�ʱ��:	2013-10-20
 * ��ϸ:		
*************************************************************************************************************/
#include "system.h"
#include "usart.h"
#include "main.h"
#include "board.h"
#include "stm32flash.h"
#include "string.h"
#if SYS_WDG_EN_
#include "Wdg.h"
#endif
#include "STM32Flash_Lock.h"




//���Կ���
#define BOARD_DBUG	1
#if BOARD_DBUG
	#include "system.h"
	#define BOARD_debug(format,...)	uart_printf(format,##__VA_ARGS__)
#else
	#define BOARD_debug(format,...)	/\
/
#endif	//BOARD_DBUG



/*************************************************************************************************************************
* ����			:	void BOARD_HardwaveInit(void)
* ����			:	ģ��ײ�Ӳ����ʼ��
* ����			:	��
* ����			:	��	
* ����			:	�ײ�
* ����			:	cp1300@139.com
* ʱ��			:	2014-08-18
* ����޸�ʱ��	: 	2014-08-18
* ˵��			: 	��ʼ���ײ�IO����Ҫ�ǵ�Դʹ������
*************************************************************************************************************************/ 
void BOARD_HardwaveInit(void)
{
	POWER_IO_INIT();							//��ʼ��IO
	BOARD_AllPowerOFF();						//�ر����е�Դ
}




