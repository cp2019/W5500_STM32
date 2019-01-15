/*******************************************************************************
//�ļ���:	RTU_Config.h
//����:	�洢,����ϵͳ�������
//����:	cp1300@139.com
//����ʱ��:	2013-05-05 07:43
//�޸�ʱ��:2013-05-05
//�޶�˵��:
//����:	��Ҫ�洢֧��	
********************************************************************************/
#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_
#include "system.h"


#define TOUCH_CONFIG_	1	//TOUCHϵͳ����

//������ؽṹ
typedef __packed struct
{
#if TOUCH_CONFIG_
	//����������
	float 	xfac;
	float 	yfac;
	u16 	xoff;
	u16 	yoff;	
#endif //TOUCH_CONFIG_
}SYS_CONFIG;



extern SYS_CONFIG	*pSysConfig;	//ϵͳ���ýṹָ��




void SYS_SaveConfig(void);	//�洢��������
void SYS_LoadConfig(void);	//������������


#endif //_SYS_CONFIG_H_

