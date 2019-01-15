/*************************************************************************************************************
 * 文件名:			board.c
 * 功能:			板级支持
 * 作者:			cp1300@139.com
 * 创建时间:		2013-10-20
 * 最后修改时间:	2013-10-20
 * 详细:		
*************************************************************************************************************/
#include "system.h"
#include "usart.h"
#include "main.h"
#include "board.h"
#include "stm32flash.h"
#include "string.h"
#if SYS_WDG_EN_
#include "Wdg.h"
#endif
#include "STM32Flash_Lock.h"




//调试开关
#define BOARD_DBUG	1
#if BOARD_DBUG
	#include "system.h"
	#define BOARD_debug(format,...)	uart_printf(format,##__VA_ARGS__)
#else
	#define BOARD_debug(format,...)	/\
/
#endif	//BOARD_DBUG



/*************************************************************************************************************************
* 函数			:	void BOARD_HardwaveInit(void)
* 功能			:	模块底层硬件初始化
* 参数			:	无
* 返回			:	无	
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2014-08-18
* 最后修改时间	: 	2014-08-18
* 说明			: 	初始化底层IO，主要是电源使能引脚
*************************************************************************************************************************/ 
void BOARD_HardwaveInit(void)
{
	POWER_IO_INIT();							//初始化IO
	BOARD_AllPowerOFF();						//关闭所有电源
}




