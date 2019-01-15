#include "system.h"
#include "usart.h"
#include "led.h"
#include "main.h"
#include "string.h"
#include "board.h"
#include "WDG.h"
#include "USERTASK.h"

SYS_FLAG SysFlag;

//任务堆栈声明
__align(8) OS_STK	TASK_SYSTEM_STK[SYSTEM_STK_SIZE];	//系统主进程
OS_STK	TASK_W5500_STK[W5500_STK_SIZE];					//W5500进程


//任务列表
void TaskSystem(void *pdata);							//任务1:系统主进程


//主函数
int main(void)
{
	OS_CPU_SR  cpu_sr;
	
	IWDG_Init(7, 0xfff);											//初始化系统独立看门狗,26S

	//开始初始化模块
	if(SYSTEM_ClockInit(ENABLE, ENABLE, 6) == FALSE)				//初始化系统时钟48MHz
	{
		//外部时钟初始化失败，使用内部时钟
		SYSTEM_ClockInit(DISABLE, ENABLE, 12);
	}

	memset(&SysFlag,0,sizeof(SYS_FLAG));							//数据清零，必须在设置时钟状态前清零
	JTAG_Set(SWD_ENABLE);											//只开启SWD调试模式
	BOARD_HardwaveInit();											//初始化模块底层IO
	UARTx_Init(UART_PRINTF_CH, 115200, ENABLE);						//初始化串口,波特率115200,调试


	uart_printf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n程序启动...\r\n");
	LED_Init();														//初始化LED
	LED_ON();														//LED亮

	uart_printf("\r\n\r\n\r\n\r\n\r\n\r\n");
	uart_printf("\r\n***********************设备程序信息***********************\r\n");
	uart_printf("时钟来源:%s\r\n", gc_ClockSourceString[SYS_GetSystemClockSource()]);
	uart_printf("系统时钟:%ldHz\r\n", SYSTEM_GetClkSpeed());
	{
		u8 i;
		
		uart_printf("CPU唯一ID:");
		for(i = 0;i < 12;i ++)
		{
			uart_printf("%02X",g_CPU_ID[i]);
		}
		uart_printf("\r\n");
	}
	

	uart_printf("\r\n**********************************************************\r\n\r\n\r\n");
	
	OS_ENTER_CRITICAL();						//关闭系统中断
	OSInit(); 									//启动UCOS II 操作系统
	OSTaskCreate(TaskSystem, (void *)0,&TASK_SYSTEM_STK[SYSTEM_STK_SIZE-1], SYSTEM_TASK_Prio);//系统主进程
	OS_EXIT_CRITICAL();	 						//开启系统中断
	SysTick_Configuration(SYSTEM_GetClkSpeed());//ucOS_II系统时钟初始化,必须放在后面初始化,如果在前面初始化会出现重复中断,导致OSIntNesting变为2
	OSStart();
}




//任务1:
//系统任务
void TaskSystem(void *pdata)
{
	int Status;

	//初始化相关线程
	OSStatInit();																									//统计任务初始化
	
	//这2个任务2选一的进行测试
	//Status = OSTaskCreate(TaskW5500, (void *)0,&TASK_W5500_STK[W5500_STK_SIZE-1], W5500_TASK_Prio);					//W5500线程
	//if(Status) DEBUG("任务 %s 建立失败，错误：%d\r\n", "TaskW5500", Status);
	
	Status = OSTaskCreate(SOCKET_Task, (void *)0,&TASK_W5500_STK[W5500_STK_SIZE-1], W5500_TASK_Prio);					//SOCKET_Task
	if(Status) DEBUG("任务 %s 建立失败，错误：%d\r\n", "SOCKET_Task", Status);
	

/////////////调试开启/////////////////	
	uart_printf_enable();	
//////////////////////////////		
	
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,500);
		IWDG_Feed();
	}
}




//硬件中断
void HardFault_Handler (void)
{
	uart_printf_enable();											//开启串口调试信息
	uart_printf("\r\n-----------------ERROR -----------------\r\nHardFault_Handler\r\n");
	SYSTEM_SoftReset();//复位重启	
}



