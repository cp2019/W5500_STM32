#ifndef __MAIN_H__
#define __MAIN_H__

#include "system.h"
#include "ucos_ii.h"
#include "board.h"

//���������ջ��С
#define SYSTEM_STK_SIZE	 		(256)		 //ϵͳ������
#define W5500_STK_SIZE			(256)		 //W5500


//�����������ȼ�
#define SYSTEM_TASK_Prio		(11)	 	//ϵͳ������
#define W5500_TASK_Prio			(10)	 	//W5500



//ϵͳ״
typedef struct
{
	u8 up_sec;		//��
}SYS_FLAG;
extern SYS_FLAG SysFlag;



#endif

