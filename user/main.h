#ifndef __MAIN_H__
#define __MAIN_H__

#include "system.h"
#include "ucos_ii.h"
#include "board.h"

//设置任务堆栈大小
#define SYSTEM_STK_SIZE	 		(256)		 //系统主进程
#define W5500_STK_SIZE			(256)		 //W5500


//设置任务优先级
#define SYSTEM_TASK_Prio		(11)	 	//系统主进程
#define W5500_TASK_Prio			(10)	 	//W5500



//系统状
typedef struct
{
	u8 up_sec;		//秒
}SYS_FLAG;
extern SYS_FLAG SysFlag;



#endif

