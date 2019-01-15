#ifndef _LED_H_
#define _LED_H_
#include "SYSTEM.H"
#include "DELAY.H"
#include "GPIO_INIT.H"


#if(BOARD_SUPPORT)	//��Ҫ�弶֧��
#include "board.h" 
#else	//Ĭ��֧��

#define LED_ON() (PBout(7)=1)
#define LED_OFF() (PBout(7)=0)
#define LED_FLASH() (PBout(7)=~PBout(7))
#define LED_IO_INIT()	\
	DeviceClockEnable(DEV_GPIOB,ENABLE);/*ʹ��GPIOBʱ��*/\
	GPIOx_Init(GPIOB,BIT7, OUT_PP, SPEED_10M);\

#endif





void LED_Init(void);


#endif
