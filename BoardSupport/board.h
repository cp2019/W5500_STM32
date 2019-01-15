/*************************************************************************************************************
 * �ļ���:			board.h
 * ����:			�弶���
 * ����:			cp1300@139.com
 * ����ʱ��:		2013-10-20
 * ����޸�ʱ��:	2019-01-13
 * ��ϸ:			RTU�弶֧��
					
*************************************************************************************************************/
#ifndef _BOARD_H_
#define _BOARD_H_
#include "system.h"
#include "DMA.h"
#include "spi.h"

//��Դʹ�ܽ�
#define W5500_POWER_EN		PGout(9)	//W5500��Դʹ��


//��Դ��ʼ��
__inline void POWER_IO_INIT(void)
{
	DeviceClockEnable(DEV_GPIOG, ENABLE);
	GPIOx_Init(GPIOG,BIT9, OUT_PP, SPEED_10M);
}
							


//��Դʹ��
#define W5500_PowerON()			(W5500_POWER_EN=0)	//W5500��Դ


//��Դ�ر�
#define W5500_PowerOFF()		(W5500_POWER_EN=1)	//W5500��Դ


//�ر����е�Դ
__inline void BOARD_AllPowerOFF(void)
{
	W5500_PowerOFF();		//W5500��̫����Դ
}



							
/////////////////////////////////////////////////////////////////////////////////////////////
//LED֧��
#define LED1_IO		PEout(3)	//LED1 
//LED���ؿ���
//LED1 ����ָʾ��
#define LED_ON() {LED1_IO=1;}
#define LED_OFF() {LED1_IO=0;}
#define LED_FLASH() {LED1_IO=~LED1_IO;}
//IO��ʼ��
#define LED_IO_INIT()	\
	DeviceClockEnable(DEV_GPIOE,ENABLE);/*ʹ��GPIOEʱ��*/\
	GPIOx_Init(GPIOE,BIT3, OUT_PP, SPEED_10M);
	


/////////////////////////////////////////////////////////////////////////////////////////////
//W5500��̫��֧��
//�ײ�IO����
#define W5500_SPI_CH					SPI_CH2			//SPI2

//W5500Ƭѡ����
__inline void W5500_SetCS(u8 level)
{
	PGout(6) = level;
}	

//��ȡW5500�ж�״̬
__inline u8 W5500_GetInt(void)		
{
	return PGin(11);
}

__inline u8 W5500_ReadWrtieByte(u8 data)
{
	return SPIx_ReadWriteByte(W5500_SPI_CH, data);
}

//W5500Ӳ���ӿڳ�ʼ��
__inline void W5500_HardwaveInit(void)		
{
	SPIx_Init(W5500_SPI_CH, &SPI_DEFAULT_01, SPI_1_4);//��ʼ��SPI
	DeviceClockEnable(DEV_GPIOG, ENABLE);
	GPIOx_Init(GPIOG,BIT6, OUT_PP,SPEED_50M);
	GPIOx_Init(GPIOG,BIT11,IN_IPT,IN_IN);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//ϵͳ��ʱ�ӿ�
__inline void OS_Sleep(u32 ms)		
{
	OSTimeDlyHMSM((ms/1000)/3600, ((ms/1000)%3600)/60, (ms/1000)%60, ms%1000);
}



void BOARD_HardwaveInit(void);												//��Ӳ����ʼ��

#endif /*_BOARD_H_*/
