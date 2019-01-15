/*******************************************************************************
//文件名:	SYS_CONFIG.c
//功能:	存储,加载系统相关配置
//作者:	cp1300@139.com
//创建时间:	2013-05-05 07:43
//修改时间:2013-05-05
//修订说明:
//声明:	需要存储支持	
********************************************************************************/
#include "system.h"
#include "SYS_CONFIG.h"
#include "ff.h"
//#include "SetPage.h"


#if TOUCH_CONFIG_
#include "touch.h"
#endif //TOUCH_CONFIG_



//配置文件名
#define CONFIG_FILE_NAME	"0:/SysConf.sys"





#define SYS_CONFIG_BUFF_SIZE		sizeof(SYS_CONFIG)	//系统配置文件缓冲区大小
static u8	SysConfigBuff[SYS_CONFIG_BUFF_SIZE];		//系统配置缓冲区
SYS_CONFIG	*pSysConfig	 = (SYS_CONFIG *)SysConfigBuff;	//系统配置结构指针






static bool SYS_SaveConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize);	//存储配置数据文件到存储卡
static bool SYS_LoadConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize);	//从存储卡加载配置数据文件




/*************************************************************************************************************************
* 函数	:	bool SYS_SaveConfigFile(const char *ConfName, u8*pConfBuff, u32 ConfFileSize)
* 功能	:	存储配置数据到存储卡
* 参数	:	ConfName:配置文件名称
			pConfBuff:配置文件存储缓冲区指针
			ConfFileSize:配置文件大小
* 返回	:	TRUE:加载配置成功;FALSE:加载配置失败
* 依赖	:	FATFS
* 作者	:	cp1300@139.com
* 时间	:	2013-05-01
* 最后修改时间 : 2013-05-01
* 说明	: 	使用文件系统存储配置文件到SD卡
*************************************************************************************************************************/
static bool SYS_SaveConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize)
{
	FRESULT status;
	FIL file;
	UINT bw;
	bool sta;

	
	status = f_open(&file,(TCHAR *)ConfName, FA_WRITE | FA_CREATE_ALWAYS);		//创建一个配置文件,如果文件存在则进行覆盖
	if(status != FR_OK)															//打开文件错误
	{
		uart_printf("\r\nsys_config : create \"%s\" error!(%d)  ", ConfName, status);		//创建文件错误
		sta = FALSE;	
	}
	else
	{
		status = f_write(&file, pConfBuff, ConfFileSize, &bw);
		if(status == FR_OK && bw == ConfFileSize)
		{
			uart_printf("sys_config : write \"%s\" ok!\r\n", ConfName);
			sta = TRUE;
		}
		else
		{
			uart_printf("sys_config : write \"%s\" error!(%d)\r\n", ConfName, status);	
			sta = FALSE;
		}
	}
	
	f_close(&file);	//关闭文件
	
	return sta;
}




/*************************************************************************************************************************
* 函数	:	bool SYS_LoadConfigFile(const char *ConfName, u8	*pConfBuff, u32 ConfFileSize)
* 功能	:	从存储卡加载配置数据文件
* 参数	:	ConfName:配置文件名称
			pConfBuff:配置文件存储缓冲区指针
			ConfFileSize:配置文件大小
* 返回	:	TRUE:加载配置成功;FALSE:加载配置失败
* 依赖	:	FATFS
* 作者	:	cp1300@139.com
* 时间	:	2013-05-01
* 最后修改时间 : 2013-05-01
* 说明	: 	文件系统支持
*************************************************************************************************************************/
static bool SYS_LoadConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize)
{
	FRESULT status;
	FIL file;
	UINT bw;
	bool sta;

	
	status = f_open(&file,(TCHAR *)ConfName, FA_READ | FA__ERROR);				//打开配置文件,如果文件不存在则返回错误
	if(status != FR_OK)															//打开文件错误
	{
		uart_printf("\r\nopen \"%s\" error!(%d)  ", ConfName, status);			//打开文件错误
		sta = FALSE;	
	}
	else
	{
		ConfFileSize = (ConfFileSize > file.fsize) ? file.fsize : ConfFileSize;	//防止要读取的数据超过文件大小
		status = f_read(&file, pConfBuff, ConfFileSize, &bw);
		if(status == FR_OK && bw == ConfFileSize)
		{
			uart_printf("read \"%s\" ok!\r\n", ConfName);
			sta = TRUE;
		}
		else
		{
			uart_printf("read \"%s\" error!(%d)\r\n", ConfName, status);	
			sta = FALSE;
		}
	}
	f_close(&file);	//关闭文件
	
	return sta;
}





/*************************************************************************************************************************
* 函数	:	bool SYS_SaveConfig(const char *ConfName, u8*pConfBuff, u32 ConfFileSize)
* 功能	:	存储配置数据
* 参数	:	ConfName:配置文件名称
			pConfBuff:配置文件存储缓冲区指针
			ConfFileSize:配置文件大小
* 返回	:	TRUE:加载配置成功;FALSE:加载配置失败
* 依赖	:	FATFS
* 作者	:	cp1300@139.com
* 时间	:	2013-05-05
* 最后修改时间 : 2013-05-05
* 说明	: 	无
*************************************************************************************************************************/
void SYS_SaveConfig(void)
{
	uart_printf("配置文件大小:%dB", SYS_CONFIG_BUFF_SIZE);
	
		//存储触摸屏配置		
#if TOUCH_CONFIG_
		pSysConfig->xfac = Pen_Point.xfac; 
		pSysConfig->yfac = Pen_Point.yfac;
		pSysConfig->xoff = Pen_Point.xoff;
		pSysConfig->yoff = Pen_Point.yoff;
#endif //TOUCH_CONFIG_
	
	
	if(SYS_SaveConfigFile(CONFIG_FILE_NAME, SysConfigBuff, SYS_CONFIG_BUFF_SIZE) == TRUE)
	{
		uart_printf("存储配置成功!\r\n");
	}
	else
	{
		uart_printf("存储配置失败!\r\n");
	}
}



/*************************************************************************************************************************
* 函数	:	bool SYS_SaveConfig(const char *ConfName, u8*pConfBuff, u32 ConfFileSize)
* 功能	:	加载配置数据
* 参数	:	ConfName:配置文件名称
			pConfBuff:配置文件存储缓冲区指针
			ConfFileSize:配置文件大小
* 返回	:	TRUE:加载配置成功;FALSE:加载配置失败
* 依赖	:	FATFS
* 作者	:	cp1300@139.com
* 时间	:	2013-05-05
* 最后修改时间 : 2013-05-05
* 说明	: 	无
*************************************************************************************************************************/
void SYS_LoadConfig(void)
{
	u32 i;
	
	uart_printf("配置文件大小:%dB", SYS_CONFIG_BUFF_SIZE);
	if(SYS_LoadConfigFile(CONFIG_FILE_NAME, SysConfigBuff, SYS_CONFIG_BUFF_SIZE) == TRUE)
	{
		uart_printf("加载配置成功!\r\n");	
	}
	else
	{
		uart_printf("加载配置失败!\r\n");
		for(i = 0;i < SYS_CONFIG_BUFF_SIZE;i ++)
		{
			SysConfigBuff[i] = 0;				//加载配置失败,将全部配置清除
		}
	}
	
	//加载触摸屏配置		
#if TOUCH_CONFIG_
		Pen_Point.xfac = pSysConfig->xfac;
		Pen_Point.yfac = pSysConfig->yfac;
		Pen_Point.xoff = pSysConfig->xoff;
		Pen_Point.yoff = pSysConfig->yoff;
#endif //TOUCH_CONFIG_
		

}





