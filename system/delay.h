/**************************************************************************************************/
/*	us/ms延时函数,利用通用定时器2	*/
/* cp1300@139.com		20110609	*/

#ifndef _DELAY_H
#define _DELAY_H
#include "system.h"


/********************************************************************************************************/
/*	函数声明	*/

//SYSTICK的时钟固定为HCLK时钟的1/8
//void Delay_INIT(u8 SYSCLK);	  /*	初始化延迟函数 采用SysTick */
//void Delay_MS(u16 nms);
//void Delay_US(u16 nms);

void Delay_US(vu16 time);	//软件 延时时间1 - 65535us
void Delay_MS(vu16 time);	//软件 延时时间1 - 6553 MS

void delay_us(u32 nus);
#endif
