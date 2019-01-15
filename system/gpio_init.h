/**************************************************************************************************/
/*	GPIO 通用IO初始化	*/
/* 		20110608	*/
/*最后修改:20111105,采用指针方式,将函数简化到一个*/

#ifndef __GPIO_INIT_H
#define __GPIO_INIT_H
#include "SYSTEM.H"
/***************************************************************************************************/
/*	函数声明	*/

void GPIOx_Init(GPIO_TypeDef *GPIOx,vu16 BIT_N,vu8 IO_MODE,vu8 SPEED_MODE);
void GPIOx_PD01_Init(void);
void GPIOx_OneInit(GPIO_TypeDef *GPIOx,vu16 i,vu8 IO_MODE,vu8 SPEED_MODE);

/***************************************************************************************************/
/*IO工作模式配置宏定义*/
#define IN_AIN 			8//1000				//模拟输入 ,高位10为了区别输入或者输出
#define IN_FLOATING		9//1001				//浮空输入 ,高位10为了区别输入或者输出
#define IN_IPT			10//1010			//下拉输入 ,高位10为了区别输入或者输出
#define IN_IPU			14//1110			//上拉输入 ,高位11为了区别输入或者输出

#define OUT_OD			1//01				//开漏输出
#define OUT_PP			0//00				//推挽输出
#define AF_OD			3//11				//复用开漏输出
#define AF_PP			2//10				//复用推挽输出
#define SPEED_10M		1//01
#define SPEED_2M		2//10
#define SPEED_50M		3//11
#define IN_IN			0

#endif
