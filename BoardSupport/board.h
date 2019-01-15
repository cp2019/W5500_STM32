/*************************************************************************************************************
 * 文件名:			board.h
 * 功能:			板级相关
 * 作者:			cp1300@139.com
 * 创建时间:		2013-10-20
 * 最后修改时间:	2019-01-13
 * 详细:			RTU板级支持
					
*************************************************************************************************************/
#ifndef _BOARD_H_
#define _BOARD_H_
#include "system.h"
#include "DMA.h"
#include "spi.h"

//电源使能脚
#define W5500_POWER_EN		PGout(9)	//W5500电源使能


//电源初始化
__inline void POWER_IO_INIT(void)
{
	DeviceClockEnable(DEV_GPIOG, ENABLE);
	GPIOx_Init(GPIOG,BIT9, OUT_PP, SPEED_10M);
}
							


//电源使能
#define W5500_PowerON()			(W5500_POWER_EN=0)	//W5500电源


//电源关闭
#define W5500_PowerOFF()		(W5500_POWER_EN=1)	//W5500电源


//关闭所有电源
__inline void BOARD_AllPowerOFF(void)
{
	W5500_PowerOFF();		//W5500以太网电源
}



							
/////////////////////////////////////////////////////////////////////////////////////////////
//LED支持
#define LED1_IO		PEout(3)	//LED1 
//LED开关控制
//LED1 运行指示灯
#define LED_ON() {LED1_IO=1;}
#define LED_OFF() {LED1_IO=0;}
#define LED_FLASH() {LED1_IO=~LED1_IO;}
//IO初始化
#define LED_IO_INIT()	\
	DeviceClockEnable(DEV_GPIOE,ENABLE);/*使能GPIOE时钟*/\
	GPIOx_Init(GPIOE,BIT3, OUT_PP, SPEED_10M);
	


/////////////////////////////////////////////////////////////////////////////////////////////
//W5500以太网支持
//底层IO定义
#define W5500_SPI_CH					SPI_CH2			//SPI2

//W5500片选控制
__inline void W5500_SetCS(u8 level)
{
	PGout(6) = level;
}	

//获取W5500中断状态
__inline u8 W5500_GetInt(void)		
{
	return PGin(11);
}

__inline u8 W5500_ReadWrtieByte(u8 data)
{
	return SPIx_ReadWriteByte(W5500_SPI_CH, data);
}

//W5500硬件接口初始化
__inline void W5500_HardwaveInit(void)		
{
	SPIx_Init(W5500_SPI_CH, &SPI_DEFAULT_01, SPI_1_4);//初始化SPI
	DeviceClockEnable(DEV_GPIOG, ENABLE);
	GPIOx_Init(GPIOG,BIT6, OUT_PP,SPEED_50M);
	GPIOx_Init(GPIOG,BIT11,IN_IPT,IN_IN);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//系统延时接口
__inline void OS_Sleep(u32 ms)		
{
	OSTimeDlyHMSM((ms/1000)/3600, ((ms/1000)%3600)/60, (ms/1000)%60, ms%1000);
}



void BOARD_HardwaveInit(void);												//板硬件初始化

#endif /*_BOARD_H_*/
