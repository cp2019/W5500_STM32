/*************************************************************************************************************
 * 文件名:		system.h
 * 功能:		相关STM32F4硬件系统函数
 * 作者:		cp1300@139.com
 * 邮箱:		cp1300@139.com
 * 创建时间:	2011-06-08
 * 最后修改时间:2018-04-07
 * 详细:		2016-10-23 增加时钟来源定义
				2017-11-22 增加打印开关设置
				2018-02-08 增加操作系统运行时间记录
				2018-04-07 增加统一的系统延时定义
*************************************************************************************************************/

#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "stm32f103_map.h"
#include "typedef.h"
#include "delay.h"
#include "gpio_init.h"
#include "stdio.h"


//宏开关
#define ENABLE_HSI			1			//使能内部高速时钟
#define _UCOS_II_ 						//是否使能UCOS_II操作系统
#define PRINTF_EN_ 			1		 	//printf输出重定义 ,0:关闭,1:串口,2:LCD,3:同时到串口和LCD
#define UART_PRINTF_CH		UART_CH1	//uart printf串口选择
#define SYS_CONFIG_EN_		0			//系统配置存储加载功能,需要g_SYS_Config.c支持
#define SYS_MESSAGE_EN_		0			//使能系统消息
#define SYS_CMD_EN_			0			//使能系统命令行
#define SYS_WDG_EN_			1			//使能系统看门狗
#define BOARD_SUPPORT		1			//板级支持（用于特定系列开发）

extern const char *const gc_ClockSourceString[];	//时钟来源名称

#ifdef _UCOS_II_
#include "ucos_ii.h"

#endif 


//空指令,宏定义
#define nop __nop()  //空指令  


//开总中断
void SYS_EnableIrq(void);        

//关闭总中断
void SYS_DisableIrq(void);

//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr);

/*************************************************************************************************/
/*	函数声明	*/
bool SYSTEM_ClockInit(FunctionalState ExtClockEnable, FunctionalState PLLEnable, u8 PLL); //时钟初始化 PLL = 2 - 9;对应频率为8 - 72MHz
void EXTI_IntConfig(u8 GPIOx,u8 BITx,u8 TRIM);	//只针对GPIOA~G,外部中断配置函数 如GPIO_A 0
void NVIC_IntEnable(u8 IRQ_,u8 Enable);			//NVIC全局中断使能或失能
void EXTI_ClearInt(u8 IRQ_n);					//清除外部中断标志,清除19根中断线的对应中断标志位
void JTAG_Set(u8 mode);/*	JTAG模式设置,用于设置JTAG的模式		*///mode:jtag,swd模式设置;00,全使能;01,使能SWD;10,全关闭;	
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset);	//设置偏移地址
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);			//设置NVIC分组
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group);	//设置中断
void SYSTEM_Standby(void);						//STM32进入掉电模式,需要外部PA0进行唤醒	
void SYSTEM_Sleep(void);						//进入等待模式,需要事件唤醒
void DeviceClockEnable(u8 DEV_,u8 Enable);		//外设时钟使能与取消
void DeviceReset(u8 DEV_);						//APB1,APB2外设复位
void SYSTEM_SoftReset(void);					//系统软复位
u32 SYSTEM_GetClkSpeed(void);					//获取系统时钟速度
void SysTick_Configuration(u32 SYS_CLK);		//系统时钟配置，设计1ms产生一次中断
void EXTI_IntMask(u8 line, bool isMask);		//外部中断线屏蔽
double user_atof(const char *pStr);				//可替代系统的atof()函数，最大长度限制16字节
u16 CRC16(const u8 *p, u16 datalen);					//CRC16
void SYSTEM_ReadCPUID(u8 id[12]);				//读取12字节96bit的单片机唯一id
u64 SYS_GetOSRunTime(void);						//获取操作系统运行时间
bool SYS_GetOsStartup(void);					//获取操作系统启动状态
void SYS_DelayMS(u32 ms);						//统一系统延时接口


extern const vu8 *g_CPU_ID;						//CPU ID,只读
/******************************************************************************************************/
//Ex_NVIC_Config专用定义
#define GPIO_A 0
#define GPIO_B 1
#define GPIO_C 2
#define GPIO_D 3
#define GPIO_E 4
#define GPIO_F 5
#define GPIO_G 6

//系统主时钟来源
typedef enum
{
	SYS_CLOCK_HSI		=	0,	//内部时钟，无PLL
	SYS_CLOCK_HSE		=	1,	//外部时钟，无PLL
	SYS_CLOCK_HSI_PLL	=	2,	//内部时钟，PLL
	SYS_CLOCK_HSE_PLL	=	3,	//外部时钟，PLL
}CLOCK_SOURCE;
CLOCK_SOURCE SYS_GetSystemClockSource(void);	//获取系统时钟来源

//中断触发设置
#define OFF_INT		0	//关闭中断
#define NegEdge   	1  	//下降沿触发
#define PosEdge   	2  	//上升沿触发
#define Edge   		3  	//边沿触发

//JTAG模式设置定义
#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00

//外设编号,方便设置,用于外设时钟设置
//AHB
#define DEV_DMA1		(BIT5 | 0)	  
#define DEV_DMA2		(BIT5 | 1)  
#define DEV_SRAM		(BIT5 | 2)	  
#define DEV_FLITF		(BIT5 | 4) 	 //闪存接口时钟使能
#define DEV_CRC		 	(BIT5 | 6)
#define DEV_FSMC		(BIT5 | 8) 
#define DEV_SDIO		(BIT5 | 10)
//APB2		 			
#define DEV_AFIO		(BIT6 | 0) 
#define DEV_GPIOA		(BIT6 | 2)  
#define DEV_GPIOB		(BIT6 | 3) 
#define DEV_GPIOC		(BIT6 | 4)  
#define DEV_GPIOD		(BIT6 | 5)
#define DEV_GPIOE		(BIT6 | 6)  
#define DEV_GPIOF		(BIT6 | 7) 
#define DEV_GPIOG		(BIT6 | 8)  
#define DEV_ADC1		(BIT6 | 9) 
#define DEV_ADC2		(BIT6 | 10)  
#define DEV_TIM1		(BIT6 | 11)  
#define DEV_SPI1		(BIT6 | 12) 
#define DEV_TIM8		(BIT6 | 13) 
#define DEV_USART1		(BIT6 | 14) 
#define DEV_ADC3		(BIT6 | 15)  
//APB1
#define DEV_TIM2		(BIT7 | 0)  
#define DEV_TIM3		(BIT7 | 1) 
#define DEV_TIM4		(BIT7 | 2)  
#define DEV_TIM5		(BIT7 | 3)   
#define DEV_TIM6		(BIT7 | 4) 
#define DEV_TIM7		(BIT7 | 5)  
#define DEV_WWDG		(BIT7 | 11)  
#define DEV_SPI2		(BIT7 | 14)   
#define DEV_SPI3		(BIT7 | 15)  
#define DEV_USART2		(BIT7 | 17)  
#define DEV_USART3		(BIT7 | 18)   
#define DEV_UART4		(BIT7 | 19) 	  
#define DEV_UART5		(BIT7 | 20) 
#define DEV_I2C1		(BIT7 | 21) 	  
#define DEV_I2C2		(BIT7 | 22) 	 
#define DEV_USB			(BIT7 | 23)   
#define DEV_CAN		  	(BIT7 | 25) 
#define DEV_BKP		  	(BIT7 | 27)  //备份区
#define DEV_PWR		    (BIT7 | 28)  //电源接口
#define DEV_DAC		    (BIT7 | 29)

//中断编号宏定义
#define IRQ_WWDG 				  0//串口定时器中断
#define IRQ_PVD					  1//电源电压检测(PVD)中断
#define IRQ_TAMPER				  2//侵入检测中断
#define IRQ_RTC					  3//实时时钟(RTC)全局中断
#define IRQ_FLASH				  4//闪存全局中断
#define IRQ_RCC					  5//复位和时钟控制(RCC)中断
#define IRQ_EXTI0				  6//EXTI线0中断
#define IRQ_EXTI1				  7//EXTI线1中断
#define IRQ_EXTI2				  8//EXTI线2中断
#define IRQ_EXTI3				  9//EXTI线3中断
#define IRQ_EXTI4				  10//EXTI线4中断
#define IRQ_DMA1_1				  11//DMA1通道1
#define IRQ_DMA1_2				  12//DMA1通道1
#define IRQ_DMA1_3				  13//DMA1通道1
#define IRQ_DMA1_4				  14//DMA1通道1
#define IRQ_DMA1_5				  15//DMA1通道1
#define IRQ_DMA1_6				  16//DMA1通道1
#define IRQ_DMA1_7				  17//DMA1通道1
#define IRQ_ADC					  18//ADC全局中断
#define IRQ_USB_HP_CAN_TX		  19//USB高优先级或CAN发送中断
#define IRQ_USB_LP_CAN_RX0		  20//USB低优先级或CAM接收0中断
#define IRQ_CAN_RX1				  21//CAN接收1中断
#define IRQ_CAN_SCE				  22//CAN SCE中断
#define IRQ_EXTI9_5				  23//EXTI线[9:5]中断
#define IRQ_TIM1_BRK			  24//TIM1断开中断
#define IRQ_TIM1_UP				  25//TIM1更新中断
#define IRQ_TIM1_TRG_COM		  26//TIM1触发和通信中断
#define IRQ_TIM1_CC				  27//TIM1捕获比较中断
#define IRQ_TIM2				  28//TIM2全局中断
#define IRQ_TIM3				  29//TIM3全局中断
#define IRQ_TIM4				  30//TIM4全局中断
#define IRQ_I2C1_EV				  31//I2C1事件中断
#define IRQ_I2C1_ER				  32//I2C1错误中断
#define IRQ_I2C2_EV				  33//I2C2事件中断
#define IRQ_I2C2_ER				  34//I2C2错误中断
#define IRQ_SPI1				  35//SPI1全局中断
#define IRQ_SPI2				  36//SPI2全局中断
#define IRQ_USART1				  37//USART1全局中断
#define IRQ_USART2				  38//USART2全局中断
#define IRQ_USART3				  39//USART3全局中断
#define IRQ_EXTI15_10			  40//EXTI线10-15中断
#define IRQ_RTCAlarm			  41//EXTI的RTC闹钟中断
#define IRQ_USBRouse			  42//ECTI的从USB待机唤醒中断
#define IRQ_TIM8_BRK			  43//TIM8断开中断
#define IRQ_TIM8_UP				  44//TIM8更新中断
#define IRQ_TIM8_TRG_COM		  45//TIM8触发和通信中断
#define IRQ_TIM8_CC				  46//TIM8捕获比较中断
#define IRQ_ADC3				  47//ADC3全局中断
#define IRQ_FSMC				  48//FSMC全局中断
#define IRQ_SDIO				  49//SDIO全局中断
#define IRQ_TIM5				  50//TIM5全局中断
#define IRQ_SPI3				  51//SPI3全局中断
#define IRQ_UART4				  52//UART4全局中断
#define IRQ_UART5				  53//UART5全局中断
#define IRQ_TIM6				  54//TIM6全局中断
#define IRQ_TIM7				  55//TIM7全局中断
#define IRQ_DMA2_1				  56//DMA2通道1全局中断
#define IRQ_DMA2_2				  57//DMA2通道2全局中断
#define IRQ_DMA2_3				  58//DMA2通道3全局中断
#define IRQ_DMA2_4_5			  59//DMA2通道4,5全局中断


/********************************************************************************************************/
//GPIO 通用IO各寄存器偏移地址
#define GPIOx_CRL				0x00			  //端口配置低寄存器
#define GPIOx_CRH				0x04			  //端口配置高寄存器
#define GPIOx_IDR				0x08			  //端口输入数据寄存器
#define GPIOx_ODR				0x0c			  //端口输出数据寄存器
#define GPIOx_BSRR				0x10			  //端口位置位/复位寄存器
#define GPIOx_BRR				0x14			  //端口复位寄存器
#define GPIOx_LCKR				0x18			  //端口配置锁定寄存器

//NVIC 嵌套向量中断使能除能寄存器
//使能
#define NVIC_SETENA0 		*((volatile unsigned long *)(0xe000e100))	 //中断31-0使能寄存器	 	 共32个使能位
#define NVIC_SETENA1 		*((volatile unsigned long *)(0xe000e104))	 //中断63-32使能寄存器	 共32个使能位
#define NVIC_SETENA2 		*((volatile unsigned long *)(0xe000e108))	 //中断95-64使能寄存器	 共32个使能位
#define NVIC_SETENA3 		*((volatile unsigned long *)(0xe000e10c))	 //中断127-96使能寄存器	 共32个使能位
#define NVIC_SETENA4 		*((volatile unsigned long *)(0xe000e110))	 //中断159-128使能寄存器	 共32个使能位
#define NVIC_SETENA5 		*((volatile unsigned long *)(0xe000e114))	 //中断191-160使能寄存器	 共32个使能位
#define NVIC_SETENA6 		*((volatile unsigned long *)(0xe000e118))	 //中断223-192使能寄存器	 共32个使能位
#define NVIC_SETENA7 		*((volatile unsigned long *)(0xe000e11c))	 //中断239-224使能寄存器	 共16个使能位
//除能
#define NVIC_CLENA0 		*((volatile unsigned long *)(0xe000e180))	 //中断31-0除能寄存器	 	 共32个除能位
#define NVIC_CLENA1			*((volatile unsigned long *)(0xe000e184))	 //中断63-32除能寄存器	 共32个除能位
#define NVIC_CLENA2			*((volatile unsigned long *)(0xe000e188))	 //中断95-64除能寄存器	 共32个除能位
#define NVIC_CLENA3 		*((volatile unsigned long *)(0xe000e18c))	 //中断127-96除能寄存器	 共32个除能位
#define NVIC_CLENA4 		*((volatile unsigned long *)(0xe000e190))	 //中断159-128除能寄存器	 共32个除能位
#define NVIC_CLENA5 		*((volatile unsigned long *)(0xe000e194))	 //中断191-160除能寄存器	 共32个除能位
#define NVIC_CLENA6 		*((volatile unsigned long *)(0xe000e198))	 //中断223-192除能寄存器	 共32个除能位
#define NVIC_CLENA7 		*((volatile unsigned long *)(0xe000e19c))	 //中断239-224除能寄存器	 共16个除能位
/*************************************************************************************************/
/*	位带别名区	*/
//IO口操作宏定义 位带操作,实现51类似的GPIO控制功能
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR((GPIOA_BASE + GPIOx_ODR),n)  //输出 
#define PAin(n)    BIT_ADDR((GPIOA_BASE + GPIOx_IDR),n)  //输入 

#define PBout(n)   BIT_ADDR((GPIOB_BASE + GPIOx_ODR),n)  //输出 
#define PBin(n)    BIT_ADDR((GPIOB_BASE + GPIOx_IDR),n)  //输入 

#define PCout(n)   BIT_ADDR((GPIOC_BASE + GPIOx_ODR),n)  //输出 
#define PCin(n)    BIT_ADDR((GPIOC_BASE + GPIOx_IDR),n)  //输入 

#define PDout(n)   BIT_ADDR((GPIOD_BASE + GPIOx_ODR),n)  //输出 
#define PDin(n)    BIT_ADDR((GPIOD_BASE + GPIOx_IDR),n)  //输入 

#define PEout(n)   BIT_ADDR((GPIOE_BASE + GPIOx_ODR),n)  //输出 
#define PEin(n)    BIT_ADDR((GPIOE_BASE + GPIOx_IDR),n)  //输入

#define PFout(n)   BIT_ADDR((GPIOF_BASE + GPIOx_ODR),n)  //输出 
#define PFin(n)    BIT_ADDR((GPIOF_BASE + GPIOx_IDR),n)  //输入

#define PGout(n)   BIT_ADDR((GPIOG_BASE + GPIOx_ODR),n)  //输出 
#define PGin(n)    BIT_ADDR((GPIOG_BASE + GPIOx_IDR),n)  //输入

/*************************************************************************************************/
/*			对应位声明，方便位操作		*/
#define BIT0	(0x0001 << 0)
#define BIT1	(0x0001 << 1)
#define BIT2	(0x0001 << 2)
#define BIT3	(0x0001 << 3)
#define BIT4	(0x0001 << 4)
#define BIT5	(0x0001 << 5)
#define BIT6	(0x0001 << 6)
#define BIT7	(0x0001 << 7)
#define BIT8	(0x0001 << 8)
#define BIT9	(0x0001 << 9)
#define BIT10	(0x0001 << 10)
#define BIT11	(0x0001 << 11)
#define BIT12	(0x0001 << 12)
#define BIT13	(0x0001 << 13)
#define BIT14	(0x0001 << 14)
#define BIT15	(0x0001 << 15)
#define BIT16	(0x00000001 << 16)
#define BIT17	(0x00000001 << 17)
#define BIT18	(0x00000001 << 18)
#define BIT19	(0x00000001 << 19)
#define BIT20	(0x00000001 << 20)
#define BIT21	(0x00000001 << 21)
#define BIT22	(0x00000001 << 22)
#define BIT23	(0x00000001 << 23)
#define BIT24	(0x00000001 << 24)
#define BIT25	(0x00000001 << 25)
#define BIT26	(0x00000001 << 26)
#define BIT27	(0x00000001 << 27)
#define BIT28	(0x00000001 << 28)
#define BIT29	(0x00000001 << 29)
#define BIT30	(0x00000001 << 30)
#define BIT31	(0x80000000)




//中断向量表基址
#define SYS_NVIC_VectTab_RAM             ((u32)0x20000000)
#define SYS_NVIC_VectTab_FLASH           ((u32)0x08000000)












//printf输出定义
#if (PRINTF_EN_ == 1)	//使能到串口
#include "usart.h"
extern bool isPrintfOut;
extern UART_CH_Type PrintfCh;	//printf通道设置
#define get_uart_printf_status()	(isPrintfOut)
#define uart_printf_enable()	(isPrintfOut=TRUE)				//开启串口打印
#define uart_printf_disable()	(isPrintfOut=FALSE)				//关闭串口打印
#define uart_printf(format,...)	(printf(format, ##__VA_ARGS__))	//串口打印
#define DEBUG(format,...) 		(printf("<DebugFile: "__FILE__", Line: %d> "format, __LINE__, ##__VA_ARGS__))	//DEBUG输出
#define info_printf(format,...)	(printf("<info>:"format, ##__VA_ARGS__))	//系统打印信息
#define  PRINTF_SetUartCh(ch) (PrintfCh=ch)	//重定向printf输出通道
#endif


//printf输出定义
#if (PRINTF_EN_ == 2)	//使能到液晶
#define lcd_printf(format,...)	(printf(format, ##__VA_ARGS__))	//LCD打印
#define DEBUG(format,...) 		(printf("<DebugFile: "__FILE__", Line: %d> "format, __LINE__, ##__VA_ARGS__))	//DEBUG输出
#define info_printf(format,...)	(printf("<info>:"format, ##__VA_ARGS__))	//系统打印信息
#endif


//printf输出定义
#if (PRINTF_EN_ == 3)	//同时使能到液晶和串口
extern u8 PrintfSet;
#include "usart.h"
extern UART_CH_Type PrintfCh;	//printf通道设置
#define uart_printf(format,...)	PrintfSet=0;printf(format, ##__VA_ARGS__)	//串口打印
#define lcd_printf(format,...)	PrintfSet=1;printf(format, ##__VA_ARGS__)	//LCD打印
#define DEBUG(format,...)		PrintfSet=0;printf("<DebugFile: "__FILE__", Line: %d> "format, __LINE__, ##__VA_ARGS__)	//DEBUG输出
#define info_printf(format,...)	(printf("<info>:"format, ##__VA_ARGS__))	//系统打印信息
#define  PRINTF_SetUartCh(ch) (PrintfCh=ch)	//重定向printf输出通道
#endif



#if (SYS_MESSAGE_EN_)
#include "MessageQueue.h"
#include "SYSMalloc.h"
#endif //SYS_MESSAGE_EN_



#endif
