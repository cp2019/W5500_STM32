/**************************************************************************************************/
/*	us/ms��ʱ����,����ͨ�ö�ʱ��2	*/
/* cp1300@139.com		20110609	*/

#ifndef _DELAY_H
#define _DELAY_H
#include "system.h"


/********************************************************************************************************/
/*	��������	*/

//SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
//void Delay_INIT(u8 SYSCLK);	  /*	��ʼ���ӳٺ��� ����SysTick */
//void Delay_MS(u16 nms);
//void Delay_US(u16 nms);

void Delay_US(vu16 time);	//��� ��ʱʱ��1 - 65535us
void Delay_MS(vu16 time);	//��� ��ʱʱ��1 - 6553 MS

void delay_us(u32 nus);
#endif
