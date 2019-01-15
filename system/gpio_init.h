/**************************************************************************************************/
/*	GPIO ͨ��IO��ʼ��	*/
/* 		20110608	*/
/*����޸�:20111105,����ָ�뷽ʽ,�������򻯵�һ��*/

#ifndef __GPIO_INIT_H
#define __GPIO_INIT_H
#include "SYSTEM.H"
/***************************************************************************************************/
/*	��������	*/

void GPIOx_Init(GPIO_TypeDef *GPIOx,vu16 BIT_N,vu8 IO_MODE,vu8 SPEED_MODE);
void GPIOx_PD01_Init(void);
void GPIOx_OneInit(GPIO_TypeDef *GPIOx,vu16 i,vu8 IO_MODE,vu8 SPEED_MODE);

/***************************************************************************************************/
/*IO����ģʽ���ú궨��*/
#define IN_AIN 			8//1000				//ģ������ ,��λ10Ϊ����������������
#define IN_FLOATING		9//1001				//�������� ,��λ10Ϊ����������������
#define IN_IPT			10//1010			//�������� ,��λ10Ϊ����������������
#define IN_IPU			14//1110			//�������� ,��λ11Ϊ����������������

#define OUT_OD			1//01				//��©���
#define OUT_PP			0//00				//�������
#define AF_OD			3//11				//���ÿ�©���
#define AF_PP			2//10				//�����������
#define SPEED_10M		1//01
#define SPEED_2M		2//10
#define SPEED_50M		3//11
#define IN_IN			0

#endif
