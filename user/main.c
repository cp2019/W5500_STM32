#include "system.h"
#include "usart.h"
#include "led.h"
#include "main.h"
#include "string.h"
#include "board.h"
#include "WDG.h"
#include "USERTASK.h"

SYS_FLAG SysFlag;

//�����ջ����
__align(8) OS_STK	TASK_SYSTEM_STK[SYSTEM_STK_SIZE];	//ϵͳ������
OS_STK	TASK_W5500_STK[W5500_STK_SIZE];					//W5500����


//�����б�
void TaskSystem(void *pdata);							//����1:ϵͳ������


//������
int main(void)
{
	OS_CPU_SR  cpu_sr;
	
	IWDG_Init(7, 0xfff);											//��ʼ��ϵͳ�������Ź�,26S

	//��ʼ��ʼ��ģ��
	if(SYSTEM_ClockInit(ENABLE, ENABLE, 6) == FALSE)				//��ʼ��ϵͳʱ��48MHz
	{
		//�ⲿʱ�ӳ�ʼ��ʧ�ܣ�ʹ���ڲ�ʱ��
		SYSTEM_ClockInit(DISABLE, ENABLE, 12);
	}

	memset(&SysFlag,0,sizeof(SYS_FLAG));							//�������㣬����������ʱ��״̬ǰ����
	JTAG_Set(SWD_ENABLE);											//ֻ����SWD����ģʽ
	BOARD_HardwaveInit();											//��ʼ��ģ��ײ�IO
	UARTx_Init(UART_PRINTF_CH, 115200, ENABLE);						//��ʼ������,������115200,����


	uart_printf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n��������...\r\n");
	LED_Init();														//��ʼ��LED
	LED_ON();														//LED��

	uart_printf("\r\n\r\n\r\n\r\n\r\n\r\n");
	uart_printf("\r\n***********************�豸������Ϣ***********************\r\n");
	uart_printf("ʱ����Դ:%s\r\n", gc_ClockSourceString[SYS_GetSystemClockSource()]);
	uart_printf("ϵͳʱ��:%ldHz\r\n", SYSTEM_GetClkSpeed());
	{
		u8 i;
		
		uart_printf("CPUΨһID:");
		for(i = 0;i < 12;i ++)
		{
			uart_printf("%02X",g_CPU_ID[i]);
		}
		uart_printf("\r\n");
	}
	

	uart_printf("\r\n**********************************************************\r\n\r\n\r\n");
	
	OS_ENTER_CRITICAL();						//�ر�ϵͳ�ж�
	OSInit(); 									//����UCOS II ����ϵͳ
	OSTaskCreate(TaskSystem, (void *)0,&TASK_SYSTEM_STK[SYSTEM_STK_SIZE-1], SYSTEM_TASK_Prio);//ϵͳ������
	OS_EXIT_CRITICAL();	 						//����ϵͳ�ж�
	SysTick_Configuration(SYSTEM_GetClkSpeed());//ucOS_IIϵͳʱ�ӳ�ʼ��,������ں����ʼ��,�����ǰ���ʼ��������ظ��ж�,����OSIntNesting��Ϊ2
	OSStart();
}




//����1:
//ϵͳ����
void TaskSystem(void *pdata)
{
	int Status;

	//��ʼ������߳�
	OSStatInit();																									//ͳ�������ʼ��
	
	//��2������2ѡһ�Ľ��в���
	//Status = OSTaskCreate(TaskW5500, (void *)0,&TASK_W5500_STK[W5500_STK_SIZE-1], W5500_TASK_Prio);					//W5500�߳�
	//if(Status) DEBUG("���� %s ����ʧ�ܣ�����%d\r\n", "TaskW5500", Status);
	
	Status = OSTaskCreate(SOCKET_Task, (void *)0,&TASK_W5500_STK[W5500_STK_SIZE-1], W5500_TASK_Prio);					//SOCKET_Task
	if(Status) DEBUG("���� %s ����ʧ�ܣ�����%d\r\n", "SOCKET_Task", Status);
	

/////////////���Կ���/////////////////	
	uart_printf_enable();	
//////////////////////////////		
	
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,500);
		IWDG_Feed();
	}
}




//Ӳ���ж�
void HardFault_Handler (void)
{
	uart_printf_enable();											//�������ڵ�����Ϣ
	uart_printf("\r\n-----------------ERROR -----------------\r\nHardFault_Handler\r\n");
	SYSTEM_SoftReset();//��λ����	
}



