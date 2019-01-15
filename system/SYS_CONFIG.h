/*******************************************************************************
//文件名:	RTU_Config.h
//功能:	存储,加载系统相关配置
//作者:	cp1300@139.com
//创建时间:	2013-05-05 07:43
//修改时间:2013-05-05
//修订说明:
//声明:	需要存储支持	
********************************************************************************/
#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_
#include "system.h"


#define TOUCH_CONFIG_	1	//TOUCH系统配置

//配置相关结构
typedef __packed struct
{
#if TOUCH_CONFIG_
	//触摸屏配置
	float 	xfac;
	float 	yfac;
	u16 	xoff;
	u16 	yoff;	
#endif //TOUCH_CONFIG_
}SYS_CONFIG;



extern SYS_CONFIG	*pSysConfig;	//系统配置结构指针




void SYS_SaveConfig(void);	//存储配置数据
void SYS_LoadConfig(void);	//加载配置数据


#endif //_SYS_CONFIG_H_

