/*************************************************************************************************************
 * 文件名:		system.c
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
#include "stm32f103_map.h"	 
#include "SYSTEM.H"
#include "DELAY.H"


#define EXT_CLOCK_INIT_TIME_OUT		250000

const char *const gc_ClockSourceString[] = {"内部时钟HSI","外部时钟HSE","外部时钟HSI+PLL","外部时钟HSE+PLL"};
const vu8 *g_CPU_ID = (const vu8 *)0x1FFFF7E8;	//CPU ID
volatile u64 g_OS_RunTime = 0;			//操作系统运行时间-单位ms

/*************************************************************************************************************************
* 函数	:	void SYSTEM_ClockInit(FunctionalState ExtClockEnable, FunctionalState PLLEnable, u8 PLL)
* 功能	:	系统时钟初始化函数
* 参数	:	ExtClockEnable:使能外部时钟;PLLEnable:PLL使能;PLL:PLL值
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2012-04-03
* 最后修改时间 : 2014-11-14
* 说明	: 	可以使用外部8MHZ时钟,内部8MHZ,以及内部8MHZ分频进入PLL
			外部时钟:8MHZ*PLL 内部时钟:4HMZ*PLL
			不要重复将RCC->CR清零，否则在使用外部晶振会引起flash操作失败
			flsh必须先设置，否则最高时钟下会导致flash操作失败
			2016-03-01：设置前先复位时钟，将PLL关闭，并使用内部时钟作为系统时钟
			2016-03-12：外部高速时钟旁路bug，旁路设置必须先关闭HSE
*************************************************************************************************************************/
bool SYSTEM_ClockInit(FunctionalState ExtClockEnable, FunctionalState PLLEnable, u8 PLL)
{
	u32 temp=0; 
	u32 TimeOut = 0;
	u32 MainClkSpeed;
	
	
	//先将时钟复位到内部,然后才进行时钟切换
	if((RCC->CFGR & (0x03<<2)) != 0)
	{
		RCC->CR |= BIT0;  					//内部高速时钟使能HSI
		while(!(RCC->CR & BIT1));			//等待内部时钟就绪
		RCC->CFGR &= ~(BIT0|BIT1);			//内部时钟HSI作为系统时钟
		while(((RCC->CFGR>>2)&0x03)!=0); 	//等待HSI作为系统时钟设置成功
	}
	RCC->CR &= ~BIT24;						//PLLOFF,一定要关闭，否则PLL重新初始化会失败
	
	
	PLL -= 2; 
	PLL &= 0xF;
	if(ExtClockEnable == ENABLE)	//使能外部高速时钟
	{
		RCC->CR &= ~BIT16;  		//外部高速时钟HSE关闭
		nop;
		RCC->CR &= ~BIT18;			//取消外部时钟旁路
		nop;
		RCC->CR |= BIT16;  			//外部高速时钟使能HSEON
		TimeOut = 0;
		while(!(RCC->CR & BIT17))	//等待外部时钟就绪
		{
			TimeOut ++;
			Delay_US(1);
			if(TimeOut > EXT_CLOCK_INIT_TIME_OUT) 
			{
				RCC->CR &= ~BIT16;  			//外部高速时钟使能关闭
				return FALSE;
			}
				
		}
	}
	else	//使能内部高速时钟
	{
		RCC->CR |= BIT0;  			//内部高速时钟使能HSI
		while(!(RCC->CR & BIT1));	//等待内部时钟就绪
	}
	RCC->CFGR = 0X00000400; 		//APB1=DIV/2;APB2=DIV/1;AHB=DIV/1;
	FLASH->ACR |= 0x32;				//flash先按照最大时钟设置延时，否则会出现高速时钟flash读写失败
	if(PLLEnable == ENABLE)			//使能PLL
	{
		RCC->CFGR |= PLL<<18;   	//设置PLL值2-7,分别对应4,5,6,7,8,9倍频
		if(ExtClockEnable == ENABLE)//使能外部高速时钟
		{
			RCC->CFGR |= BIT16;	  	//PLLSRC ON 
		}
		else	//内部高速时钟
		{	
			RCC->CFGR &= ~BIT16;	//HSI振荡器时钟经2分频后作为PLL输入时钟
		}
		RCC->CR |= BIT24;			//PLLON
		TimeOut = 0;
		while(!(RCC->CR & BIT25))	//等待PLL锁定
		{
			TimeOut ++;
			Delay_US(1);
			if(TimeOut > EXT_CLOCK_INIT_TIME_OUT) return FALSE;
		}		 
		temp = RCC->CFGR;
		temp &= ~(BIT0|BIT1);	//清除设置
		temp |= BIT1;
		RCC->CFGR = temp;		//PLL作为系统时钟	
		
		while(((RCC->CFGR>>2)&0x03)!=2); 	//等待PLL作为系统时钟设置成功
	}
	else	//没有使能PLL
	{
		if(ExtClockEnable == ENABLE)//使能外部高速时钟
		{
			temp = RCC->CFGR;
			temp &= ~(BIT0|BIT1);	//清除设置
			temp |= BIT0;
			RCC->CFGR = temp;		//外部高速时钟HSE作为系统时钟
			TimeOut = 0;
			while(((RCC->CFGR>>2)&0x03)!=1) 	//等待HSE作为系统时钟设置成功)     //等待HSE作为系统时钟设置成功
			{   
				TimeOut ++;
				Delay_US(1);
				if(TimeOut > EXT_CLOCK_INIT_TIME_OUT) return FALSE;
			}
		}
		else									//使能内部RC时钟
		{
			RCC->CFGR &= ~(BIT0|BIT1);			//内部时钟HSI作为系统时钟
			while(((RCC->CFGR>>2)&0x03)!=0); 	//等待HSI作为系统时钟设置成功
		}
	}
	
	if(ExtClockEnable==DISABLE)	//不使能外部时钟，则关闭外部时钟，降低功耗
	{
		RCC->CR &= ~BIT16;  	//外部高速时钟使能HSEON关闭，关闭外部时钟
	}
	if(PLLEnable == DISABLE)	//不使能PLL，则关闭PLL降低功耗
	{
		RCC->CR &= ~BIT24;		//PLLOFF
	}
	
	MainClkSpeed = SYSTEM_GetClkSpeed();		//获取系统时钟
	if(MainClkSpeed > 48000000)					//大于48M
	{
		temp = FLASH->ACR;
		temp &= ~7;	  
		temp |= 0x32;
		FLASH->ACR = temp;	  					//FLASH 2个延时周期
		
	}
	else if(MainClkSpeed > 24000000)			//大于24M
	{
		temp = FLASH->ACR;
		temp &= ~7;	  
		temp |= 0x31;
		FLASH->ACR = temp;	  					//FLASH 1个延时周期
	}
	else
	{
		FLASH->ACR &= ~7;	  					//FLASH 无需等待
	}
	
	return TRUE;
}




/*************************************************************************************************************************
* 函数			:	CLOCK_SOURCE SYS_GetSystemClockSource(void)			
* 功能			:	STM32 获取系统主时钟来源
* 参数			:	无
* 返回			:	时钟来源，见CLOCK_SOURCE
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2016-03-13
* 最后修改时间 	: 	2016-03-28
* 说明			:	用于获取时钟来源，通过RCC寄存器获取
*************************************************************************************************************************/
CLOCK_SOURCE SYS_GetSystemClockSource(void)
{
	u32 temp = RCC->CFGR >> 2;
	
	temp &= 0x3;
	switch(temp)
	{
		case 0:	return SYS_CLOCK_HSI;	//HSI作为系统时钟
		case 1: return SYS_CLOCK_HSE;	//HSE作为系统时钟
		default:	//PLL
		{
			if(RCC->CFGR & BIT16)	//选择 HSE 振荡器时钟作为 PLL 时钟输入
				return SYS_CLOCK_HSE_PLL;
			else	//选择 HSI/2 时钟作为 PLL时钟输入
				return SYS_CLOCK_HSI_PLL;
		}
	}
}


/*************************************************************************************************************************
* 函数	:	u32 SYSTEM_GetClkSpeed(void)
* 功能	:	获取系统时钟速度
* 参数	:	无
* 返回	:	系统时钟速度,单位Hz
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com  
* 时间	:	2012-09-29
* 最后修改时间 : 2016-03-28
* 说明	: 	用于获取系统时钟速度
*************************************************************************************************************************/
u32 SYSTEM_GetClkSpeed(void)
{
	u8 i;
	u32	clk;

	switch(SYS_GetSystemClockSource())	//获取时钟来源
	{
		case SYS_CLOCK_HSI:	//HSI作为系统时钟
		{
			return 8*1000000;
		}
		case SYS_CLOCK_HSE: //HSE作为系统时钟
		{
			return 8*1000000;
		}
		case SYS_CLOCK_HSI_PLL:	//内部时钟，PLL
		{
			i =  (RCC->CFGR >> 18) & 0xf;
			clk = (i + 2) * 4000000;		//内部时钟2分频作为PLL输入
			return clk;
		}
		case SYS_CLOCK_HSE_PLL:	//外部时钟，PLL
		{
			i =  (RCC->CFGR >> 18) & 0xf;
			clk = (i + 2) * 8000000;
			return clk;
		}
		default:	return 8*1000000;
	}
}



///*************************************************************************************************************************
//* 函数	:	void SYSTEM_ClockInit(u8 PLL)
//* 功能	:	系统时钟初始化函数
//* 参数	:	PLL倍频数:从2开始，最大值为9
//* 返回	:	无
//* 依赖	:	底层宏定义
//* 作者	:	cp1300@139.com
//* 时间	:	20110608
//* 最后修改时间 : 20120403
//* 说明	: 	会修改全局系统时钟频率SYS_CLOCK
//*************************************************************************************************************************/
//#if(ENABLE_HSI==0)
//void SYSTEM_ClockInit(u8 PLL)
//{
//	u8 temp=0; 

//	PLL -= 2;  
//	RCC->CR |= 0x00010000;  	//外部高速时钟使能HSEON
//	while(!(RCC->CR>>17));		//等待外部时钟就绪
//	RCC->CFGR = 0X00000400; 	//APB1=DIV2;APB2=DIV1;AHB=DIV1;
//	RCC->CFGR |= PLL<<18;   	//设置PLL值0-7
//	RCC->CFGR |= 1<<16;	  		//PLLSRC ON 
//	FLASH->ACR |= 0x32;	  		//FLASH 2个延时周期
//	RCC->CR |= 0x01000000;  	//PLLON
//	while(!(RCC->CR>>25));		//等待PLL锁定
//	RCC->CFGR |= 0x00000002;	//PLL作为系统时钟	 
//	while(temp != 0x02)     	//等待PLL作为系统时钟设置成功
//	{   
//		temp = RCC->CFGR>>2;
//		temp &= 0x03;
//	} 		    
//}
//#else
//void SYSTEM_ClockInit(u8 PLL)
//{
//	u8 temp=0; 

//	PLL -= 2;  
//	RCC->CR |= BIT0;  			//内部高速时钟使能HSI
//	while(!(RCC->CR&BIT1));		//等待内部时钟就绪
//	RCC->CFGR = 0X00000400; 	//APB1=DIV2;APB2=DIV1;AHB=DIV1;
//	RCC->CFGR |= PLL<<18;   	//设置PLL值0-7
//	//RCC->CFGR |= 1<<16;	  		//PLLSRC ON 
//	FLASH->ACR |= 0x32;	  		//FLASH 2个延时周期
//	RCC->CR |= 0x01000000;  	//PLLON
//	while(!(RCC->CR>>25));		//等待PLL锁定
//	RCC->CFGR |= 0x00000002;	//PLL作为系统时钟	 
//	while(temp != 0x02)     	//等待PLL作为系统时钟设置成功
//	{   
//		temp = RCC->CFGR>>2;
//		temp &= 0x03;
//	} 		    
//}
//#endif

/*************************************************************************************************************************
* 函数	:	void DeviceClockEnable(u8 DEV_n,u8 Enable)
* 功能	:	外设时钟使能与取消
* 参数	:	外设编号,EN=1使能时钟,EN=0取消时钟
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111113
* 最后修改时间 : 20111113
* 说明	: 	无
*************************************************************************************************************************/
void DeviceClockEnable(u8 DEV_n,u8 Enable)
{
	if(DEV_n & BIT5)	  //AHB外设
	{
		DEV_n -= BIT5;
		if(Enable) //使能
			RCC->AHBENR |= 1 << DEV_n;
		else   //失能
			RCC->AHBENR &= ~(1 << DEV_n);
	}
	else if(DEV_n & BIT6) //APB2外设
	{
		DEV_n -= BIT6;
		if(Enable) //使能
			RCC->APB2ENR |= 1 << DEV_n;
		else   //失能
			RCC->APB2ENR &= ~(1 << DEV_n);
	}
	else if(DEV_n & BIT7)  //APB1外设
	{
		DEV_n -= BIT7;
		if(Enable) //使能
			RCC->APB1ENR |= 1 << DEV_n;
		else   //失能
			RCC->APB1ENR &= ~(1 << DEV_n);
	}
}


/********************************************************************************************************/
/*		外设时钟		*/
// 	RCC_AHBENR |= 0x1;				//DMA时钟使能
//	RCC->APB2ENR |= 0x1 << 14;		//USART 1 时钟使能
//	RCC_APB2ENR |= 0x1 << 12;		//SPI 1 时钟使能
//	RCC_APB2ENR |= 0x1 << 11;		//TIM 1 时钟使能
//	RCC_APB2ENR |= 0x1 << 10;		//ADC 2 时钟接口使能
//	RCC_APB2ENR |= 0x1 << 9;		//ADC 1 时钟接口使能
//	RCC->APB2ENR |= 0x1 << 8;		//GPIO G 时钟使能
//	RCC->APB2ENR |= 0x1 << 7;		//GPIO F 时钟使能
//	RCC_APB2ENR |= 0x1 << 6;		//GPIO E 时钟使能
//	RCC_APB2ENR |= 0x1 << 5;		//GPIO D 时钟使能
//	RCC_APB2ENR |= 0x1 << 4;		//GPIO C 时钟使能
//	RCC_APB2ENR |= 0x1 << 3;		//GPIO B 时钟使能
//	RCC_APB2ENR |= 0x1 << 2;		//GPIO A 时钟使能
//	RCC_APB2ENR |= 0x1;				//复用功能AFIO时钟使能
//	RCC_APB1ENR |= 0x1 << 29;		//DAC 时钟使能
//	RCC_APB1ENR |= 0x1 << 28;		//电源接口时钟使能
//	RCC_APB1ENR |= 0x1 << 27;		//备份接口时钟使能
//	RCC_APB1ENR |= 0x1 << 26;		//CAN时钟使能
//	RCC_APB1ENR |= 0x1 << 23;		//USB时钟使能
//	RCC_APB1ENR |= 0x1 << 22;		//I2C 2 时钟使能
//	RCC_APB1ENR |= 0x1 << 21;		//I2C 1 时钟使能
//	RCC_APB1ENR |= 0x1 << 18;		//USART 3 时钟使能
//	RCC_APB1ENR |= 0x1 << 17;		//USART 2 时钟使能
//	RCC_APB1ENR |= 0x1 << 14;		//SPI 2 时钟使能
//	RCC_APB1ENR |= 0x1 << 11;		//窗口看门狗时钟使能
//	RCC_APB1ENR |= 0x1 << 3;		//TIM5 定时器5时钟使能
//	RCC_APB1ENR |= 0x1 << 2;		//TIM4 定时器4时钟使能
//	RCC_APB1ENR |= 0x1 << 1;		//TIM3 定时器3时钟使能
//	RCC_APB1ENR |= 0x1;				//TIM2 定时器2时钟使能





/*************************************************************************************************************************
* 函数	:	void NVIC_IntEnable(u16 IRQ_n,u8 ENABLE)
* 功能	:	NVIC全局中断使能或失能
* 参数	:	中断编号,使能或失能
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111113
* 最后修改时间 : 20120403
* 说明	: 	无
*************************************************************************************************************************/
void NVIC_IntEnable(u8 IRQ_n,u8 ENABLE)
{	
	u8 n = IRQ_n / 32;	 //做缓冲用,不知道为什么.非要不可
	if(ENABLE)
	{
		NVIC->ISER[n] = (1 << (IRQ_n % 32));//开启对应总中断
	}
	else
	{
		NVIC->ICER[n] = (1 << (IRQ_n % 32));//关闭对应总中断  
	}
}


/*************************************************************************************************************************
* 函数	:	void  DeviceReset(u8 DEV_n)
* 功能	:	APB1,APB2外设复位
* 参数	:	外设编号
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111113
* 最后修改时间 : 20111113
* 说明	: 	无
*************************************************************************************************************************/
void  DeviceReset(u8 DEV_n)
{
 	if(DEV_n & BIT6) //APB2外设
	{
		DEV_n -= BIT6;
		RCC->APB2RSTR |=  (1 << DEV_n);	//复位
		nop;nop;nop;nop;
		RCC->APB2RSTR &= ~(1 << DEV_n);	//取消复位
	}
	else if(DEV_n & BIT7)  //APB1外设
	{
		DEV_n -= BIT7;
		RCC->APB1RSTR |=  (1 << DEV_n);	//复位
		nop;nop;nop;nop;
		RCC->APB1RSTR &= ~(1 << DEV_n);	//取消复位
	}
}



/*************************************************************************************************************************
* 函数	:	void  EXTI_IntConfig(u8 GPIOx,u8 BITx,u8 TRIM)
* 功能	:	外部中断配置函数
* 参数	:	GPIOx:GPIO选择如GPIO_A,BITx:IO位选择0-15,TRIM:配置	  //GPIOx:0~6,代表GPIOA~G;(如GPIO_A)BITx:需要使能的位;TRIM:触发模式,NegEdge:下降沿;PosEdge:上升沿,Edge:边沿触发
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111113
* 最后修改时间 : 20120403
* 说明	: 	只针对GPIOA~G;不包括PVD,RTC和USB唤醒这三个
			该函数一次只能配置1个IO口,多个IO口,需多次调用
			该函数会自动开启对应中断,以及屏蔽线 
*************************************************************************************************************************/
void EXTI_IntConfig(u8 GPIOx,u8 BITx,u8 TRIM)
{
	u8 EXTADDR;
	u8 EXTOFFSET;

	EXTADDR = BITx / 4;//得到中断寄存器组的编号
	EXTOFFSET = (BITx % 4) * 4;
	RCC->APB2ENR |= 0x01;//使能io复用时钟
	AFIO->EXTICR[EXTADDR] &= ~(0x000F<<EXTOFFSET);//清除原来设置！！！
	AFIO->EXTICR[EXTADDR] |= GPIOx<<EXTOFFSET;//EXTI.BITx映射到GPIOx.BITx
	//自动设置
	EXTI->IMR |= 1<<BITx;//  开启line BITx上的中断
	//EXTI->EMR|=1<<BITx;//不屏蔽line BITx上的事件 (如果不屏蔽这句,在硬件上是可以的,但是在软件仿真的时候无法进入中断!)
	//清除设置
	EXTI->RTSR &= ~(1<<BITx);
	EXTI->FTSR &= ~(1<<BITx);
 	if(TRIM & 0x01)
	{
		EXTI->FTSR |= 1<<BITx;//line BITx上事件下降沿触发
	}
	else if(TRIM & 0x02)
	{
		EXTI->RTSR |= 1<<BITx;//line BITx上事件上升降沿触发
	}
}


/*************************************************************************************************************************
* 函数			:	void EXTI_IntMask(u8 line, bool isMask)
* 功能			:	外部中断屏蔽
* 参数			:	line:中断线0-15， isMask:是否屏蔽中断
* 返回			:	无
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2016-01-11
* 最后修改时间	: 	2016-01-11
* 说明			: 	用于屏蔽中断，否则中断无法关闭
					一直以来都是用清除EXTI->RTSR，EXTI->FTSR这2个触发寄存器来关闭中断，发现并没有什么用
*************************************************************************************************************************/
void EXTI_IntMask(u8 line, bool isMask)
{
	if(line > 15) return;
	if(isMask == TRUE)	//屏蔽中断
	{
		EXTI->IMR &= ~(1<<line);
	}
	else
	{
		EXTI->IMR |= 1<<line;
	}
}

		    
/*************************************************************************************************************************
* 函数	:	void EXTI_ClearInt(u8 IRQ_n)
* 功能	:	清除外部中断标志
* 参数	:	中断号
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111113
* 最后修改时间 : 20120403
* 说明	: 	清除19根中断线的对应中断标志位
*************************************************************************************************************************/
void EXTI_ClearInt(u8 IRQ_n)
{
	EXTI->PR = 1 << IRQ_n;
}



/**********************************************************************************************************/
/*	JTAG模式设置,用于设置JTAG的模式*/		
//mode:jtag,swd模式设置;00,全使能;01,使能SWD;10,全关闭;		  
void JTAG_Set(u8 mode)
{
 	u32 temp;
	RCC->APB2ENR |= 1 << 0;    //开启辅助时钟
	temp = mode;
	temp <<= 25;
	//RCC_APB2ENR |= 1 << 0;     //开启辅助时钟	   
	AFIO->MAPR &= 0XF8FFFFFF; //清除MAPR的[26:24]
	AFIO->MAPR |= temp;       //设置jtag模式
}


/*************************************************************************************************************************
* 函数	:	void SYSTEM_ReadCPUID(u8 id[12])
* 功能	:	读取96位的唯一id
* 参数	:	id缓冲区
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2017-11-17
* 最后修改时间 : 2017-11-17
* 说明	: 	读取STM32的唯一id
*************************************************************************************************************************/
void SYSTEM_ReadCPUID(u8 id[12])
{
	u8 i;
	u8 *p =  (u8 *)0x1FFFF7E8;
	
	for(i = 0;i < 12;i ++)
	{
		id[i] = p[i];
	}
}


/**************************************************************************************************************/
/*设置向量表偏移地址*/	
//NVIC_VectTab:基址
//Offset:偏移量
//CHECK OK
//091207
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 
  	//检查参数合法性
//	assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
//	assert_param(IS_NVIC_OFFSET(Offset));  	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);//设置NVIC的向量表偏移寄存器
	//用于标识向量表是在CODE区还是在RAM区
}
//设置NVIC分组
//NVIC_Group:NVIC分组 0~4 总共5组 
//CHECK OK
//091209
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//取后三位
	temp1<<=8;
	temp=SCB->AIRCR;  //读取先前的设置
	temp&=0X0000F8FF; //清空先前分组
	temp|=0X05FA0000; //写入钥匙
	temp|=temp1;	   
	SCB->AIRCR=temp;  //设置分组	    	  				   
}
//设置NVIC 
//NVIC_PreemptionPriority:抢占优先级
//NVIC_SubPriority       :响应优先级
//NVIC_Channel           :中断编号
//NVIC_Group             :中断分组 0~4
//注意优先级不能超过设定的组的范围!否则会有意想不到的错误
//组划分:
//组0:0位抢占优先级,4位响应优先级
//组1:1位抢占优先级,3位响应优先级
//组2:2位抢占优先级,2位响应优先级
//组3:3位抢占优先级,1位响应优先级
//组4:4位抢占优先级,0位响应优先级
//NVIC_SubPriority和NVIC_PreemptionPriority的原则是,数值越小,越优先
//CHECK OK
//100329
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	u8 IPRADDR=NVIC_Channel/4;  //每组只能存4个,得到组地址 
	u8 IPROFFSET=NVIC_Channel%4;//在组内的偏移
	IPROFFSET=IPROFFSET*8+4;    //得到偏移的确切位置
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//设置分组
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;//取低四位

	if(NVIC_Channel<32)NVIC->ISER[0]|=1<<NVIC_Channel;//使能中断位(要清除的话,相反操作就OK)
	else NVIC->ISER[1]|=1<<(NVIC_Channel-32);    
	NVIC->IPR[IPRADDR]|=temp<<IPROFFSET;//设置响应优先级和抢断优先级   	    	  				   
}

//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI
//CHECK OK
//091209
__asm void WFI_SET(void)
{
	WFI;    
}

__asm void WFE_SET(void)
{
	WFE;    
}


//开总中断
void SYS_EnableIrq(void)         
{ 
	__asm("cpsie f"); 
	//CPSIE F;
}

//关闭总中断
void SYS_DisableIrq()        
{ 
	__asm ("cpsid f"); 
}

//跳转函数
//单片机从睡眠模式唤醒后会自动跳转到此处，原因未知，跳转后自动返回
__asm void RETURN(void) 
{
    BX r14
}


//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


/*************************************************************************************************************************
* 函数	:	void SYSTEM_Standby(void)
* 功能	:	STM32进入掉电模式,需要外部PA0进行唤醒
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com  
* 时间	:	2013-11-14
* 最后修改时间 : 2013-11-14
* 说明	: 	最小功耗模式
*************************************************************************************************************************/
void SYSTEM_Standby(void)
{
	SCB->SCR|=1<<2;//使能SLEEPDEEP位 (SYS->CTRL)	 
  
	RCC->APB1ENR|=1<<28;     //使能电源时钟	    
 	PWR->CSR|=1<<8;          //设置WKUP用于唤醒
	PWR->CR|=1<<2;           //清除Wake-up 标志
	PWR->CR|=1<<1;           //PDDS置位		  
	WFI_SET();				 //执行WFI指令		 
}

/*
位段 名称 类型 复位值 描述
4 SEVONPEND RW ‐ 发生异常悬起时请发送事件，用于在一个新的中
断悬起时从WFE 指令处唤醒。不管这个中断的
优先级是否比当前的高，都唤醒。如果没有WFE
导致睡眠，则下次使用WFE 时将立即唤醒
3 保留 ‐ ‐ ‐
2 SLEEPDEEP R/W 0 当进入睡眠模式时，使能外部的SLEEPDEEP 信号，
以允许停止系统时钟
1 SLEEPONEXIT R/W ‐ 激活“SleepOnExit”功能
0 保留 ‐ ‐ ‐
通过执行WFI/WFE 指令，请求CM3 进入睡眠模式，它们在CM3 中*/
//进入SLEEP模式
/*************************************************************************************************************************
* 函数	:	void SYSTEM_Standby(void)
* 功能	:	STM32进入等待模式,需要事件唤醒
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com  
* 时间	:	2013-11-14
* 最后修改时间 : 2013-11-14
* 说明	: 	2016-01-06:睡眠模式关闭SRAM与FLASH时钟,不开启电源时钟
*************************************************************************************************************************/
void SYSTEM_Sleep(void)
{
	SCB->SCR &= ~BIT2;		//禁止关闭时钟
	SCB->SCR |= BIT4;		//使能WFE唤醒
	SCB->SCR &= ~BIT1;		//禁止SleepOnExit功能,时间来临之后唤醒单片机
	//RCC->APB1ENR|=1<<28;     //使能电源时钟	    
 	PWR->CSR|=1<<8;          //设置WKUP用于唤醒
	PWR->CR|=1<<2;           //清除Wake-up 标志
	PWR->CR|=1<<1;           //PDDS置位	

	RCC->AHBENR &= ~BIT2;	//睡眠模式SRAM时钟关闭
	RCC->AHBENR &= ~BIT4;	//睡眠模式flash时钟关闭
	
	WFE_SET();				 //执行WFI指令		 
}


//获取操作系统运行时间
u64 SYS_GetOSRunTime(void)
{
	return g_OS_RunTime;
}


//系统软复位
//CHECK OK
//091209
void SYSTEM_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 


//获取操作系统启动状态
bool SYS_GetOsStartup(void)
{
#if(UCOS_II_EN)	//使能了操作系统
	return (OSRunning==OS_FALSE)?FALSE:TRUE;//gs_isSystemStartup;
#else
	return FALSE;
#endif //UCOS_II_EN
}



/*************************************************************************************************************************
* 函数			:	void SYS_DelayMS(u32 ms)
* 功能			:	系统毫秒延时
* 参数			:	无
* 返回			:	无
* 依赖			:	底层宏定义，或者OS
* 作者			:	cp1300@139.com
* 时间			:	2018-03-10
* 最后修改时间 	: 	2018-03-10
* 说明			:	如果没有使用操作系统将使用Delay_MS()延时，如果启用了操作系统，并且操作系统运行了，则使用OSTimeDlyHMSM延时
*************************************************************************************************************************/
void SYS_DelayMS(u32 ms)
{
#ifdef _UCOS_II_	//使能了操作系统
	if(OSRunning==OS_FALSE) //操作系统还没有初始化
	{
		Delay_MS(ms);
	}
	else
	{
		OSTimeDlyHMSM((ms/1000)/3600, ((ms/1000)%3600)/60, (ms/1000)%60, ms%1000);
	}
#else //没有使能操作系统
	Delay_MS(ms);
#endif //UCOS_II_EN
}




#ifdef _UCOS_II_
#include "ucos_ii.h"

//系统时钟中断服务函数
void SysTick_Handler(void)
{
	OS_CPU_SR  cpu_sr;

	OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
	g_OS_RunTime ++;		//操作系统运行时间增加 2018-02-08
    OS_EXIT_CRITICAL();

    OSTimeTick();        /* Call uC/OS-II's OSTimeTick()               */

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}



//系统时钟配置，设计1ms产生一次中断
void SysTick_Configuration(u32 SYS_CLK)
{
 	SysTick->CTRL&=~(1<<2);//SYSTICK使用外部时钟源
	SysTick->CTRL&=0xfffffffb;//bit2清空,选择外部时钟  HCLK/8
	SysTick->CTRL|=1<<1;   //开启SYSTICK中断
	SysTick->LOAD=SYS_CLK / 8 / 1000;    //产生1ms中断
	//MY_NVIC_Init(1,2,SystemHandler_SysTick,1);//组1，最低优先级 
	SysTick->CTRL|=1<<0;   //开启SYSTICK
}
#else
//系统时钟配置，产生1us计数脉冲
void SysTick_Configuration(u32 SYS_CLK)
{
 	SysTick->CTRL&=~(1<<2);		//SYSTICK使用外部时钟源
	SysTick->CTRL&=0xfffffffb;	//bit2清空,选择外部时钟  HCLK/8
	SysTick->CTRL&=~(1<<1);   	//关闭SYSTICK中断
	//SysTick->LOAD=SYS_CLK / 8 / 1000;    //产生1ms中断
	//MY_NVIC_Init(1,2,SystemHandler_SysTick,1);//组1，最低优先级 
	SysTick->CTRL|=1<<0;   //开启SYSTICK
}

#endif









//可替代系统的atof()函数，最大长度限制16字节
#include "string.h"
double user_atof(const char *pStr)
{
	double ftemp = 0;
	u32 Integer = 0;
	u32 Decimal = 0;			//小数
	u8 s=0;
	u8 i;
    u8 len = strlen(pStr);
	u8 num = 0;					//记录小数位数
    bool isDecimal = FALSE;
    bool isNegative = FALSE;	//是否为负数
    
    if(len > 16) len = 16;
    if(pStr[0]=='-') 
    {
		s = 1;	//负数，从第1位开始算
		isNegative = TRUE;	//负数
    }
    for(i = s;i < len;i ++)
    {
		if(((pStr[i] < '0') || (pStr[i] > '9')) && (pStr[i] != '.')) break;
        if(pStr[i] == '.')	//重复的小数点直接退出
        {
			if(isDecimal==FALSE)
            {
				isDecimal = TRUE;
                continue;
            }
				
            else break;
        }
        if(isDecimal == FALSE)	//整数部分
        {
			Integer *= 10;
            Integer += pStr[i]-'0';
        }
        else	//小数部分
        {
			Decimal *= 10;
			Decimal += pStr[i]-'0';
            num ++;	//小数位数增加
        }
    }
    //printf("decimal=%d num=%d\r\n",Decimal,num);
    //计算小数
    ftemp = Decimal;
    for(i = 0;i < num;i ++)
    {
		ftemp /= 10.0;
		//printf("ftemp=%f i=%d\r\n",ftemp,i);
		
    }
    //加上整数部分
    ftemp += Integer;
    //如果是负数
    if(isNegative == TRUE)
    {
		ftemp = 0-ftemp;
    }
    
    //printf("%s->%f\r\n",pStr,ftemp);
    return ftemp;
}






 /*******************************************************************************
 *函数名称：CRC16
 *函数功能：CRC16效验函数
 *参数说明：*p效验帧的指针   帧长 datalen ，除了校验位
 *返 回 值：效验字
 *注意事项：多项式码0xA001
 *******************************************************************************/
u16 CRC16(const u8 *p, u16 datalen)
{
    u8 CRC16Lo,CRC16Hi,CL,CH,SaveHi,SaveLo;
    u16 i,Flag;

    CRC16Lo = 0xFF;     CRC16Hi = 0xFF;
    CL = 0x01;          CH = 0xA0;
    for(i = 0;i < datalen; i++)
    {
        CRC16Lo ^= *(p+i);                  //每一个数据与CRC寄存器进行异或
        for(Flag = 0; Flag < 8; Flag++)
        {
            SaveHi = CRC16Hi;  SaveLo = CRC16Lo;
            CRC16Hi >>= 1; CRC16Lo >>= 1;   //高位右移一位，低位右移一位
            if((SaveHi & 0x01) == 0x01)     //如果高位字节最后一位为1
            CRC16Lo  |=0x80 ;     			//则低位字节右移后前面补1否则自动补0
            if((SaveLo & 0x01) == 0x01)     //如果LSB为1，则与多项式码进行异或
            { CRC16Hi ^= CH;  CRC16Lo ^= CL; }
        }
    }
    return (u16)(CRC16Hi<<8)|CRC16Lo;
}




//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if PRINTF_EN_
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数            
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
}
#endif




//////////////////////////////////////////////////////////////////
//???????′???,???printf????,
//PRINTF_EN == 1,?????printf??????
#if (PRINTF_EN_ == 1)
#include "usart.h" 
bool isPrintfOut = TRUE;	//printf输出使能
UART_CH_Type PrintfCh = UART_PRINTF_CH;	//printf通道设置

int fputc(int ch,FILE *f)
{     
	if(isPrintfOut == TRUE)	//使能输出
	{
		UARTx_SendByte(PrintfCh,	(u8)ch);  
	}
	return ch;
}


#endif

//PRINTF_EN == 2,?????printf?????
#if (PRINTF_EN_== 2)
#include "lcd.h"

int fputc(int ch, FILE *f)
{    
	static u16 X;
	static u16 Y;
	 	
	if(Y > LCD_Y_SIZE - 1)
	{
		LCD_ClearScreen(0xffff);
		X = 0;
		Y = 0;
	}
	if((u8)ch == '\n')
	{
	 	X = 0;
		Y += 12;
	}
	else if((u8)ch > 0x80) //汉字
	{
		return ch;
	}
	else
	{
		LCD_Char(X,Y,(u8 *)&ch,0x0000,0xffff,0x80 | 12);
		if(X > LCD_X_SIZE - 9)
		{
		 	X = 0;
			Y += 12;
		}
		else
			X += 8;
	}     

	return ch;
}
#endif


//PRINTF_EN == 3,???????printf??????????
#if (PRINTF_EN_ == 3)
#include "lcd.h"
#include "usart.h"
#include <locale.h>
u8 PrintfSet = 0;	//0:????printf??????;1:????printf?????
UART_CH_Type PrintfCh = UART_PRINTF_CH;	//printf通道设置

int fputc(int ch, FILE *f)
{    
	static u16 X;
	static u16 Y;
 	
	if(PrintfSet)
	{
		if(Y > LCD_Y_SIZE - 1)
		{
			LCD_ClearScreen(0xffff);
			X = 0;
			Y = 0;
		}
		if((u8)ch == '\n')
		{
		 	X = 0;
			Y += 12;
		}
		else if((u8)ch > 0x80) //汉字
		{
			return ch;
		}
		else
		{
			LCD_DispChar(X,Y,(char *)&ch,0x0000,0xffff,0x80 | 12);
			if(X > LCD_X_SIZE - 9)
			{
			 	X = 0;
				Y += 12;
			}
			else
				X += 8;
		}
	}
	else
		UARTx_SendByte(PrintfCh, (u8)ch);

	return ch;
}
#endif



#if PRINTF_EN_
int fgetc(FILE *f)
{
	u8 data;
	static u8 flag = 0;
	u8 gbk[2];
	
	while(UARTx_GetNewDataFlag(UART_PRINTF_CH) == FALSE)
	{
#ifdef _UCOS_II_
		OSTimeDly(2);
#else
		Delay_US(10);
#endif //_UCOS_II_
	}
	data = UARTx_GetNewData(UART_PRINTF_CH);
	
	if((data < 0x80) && (flag == 0))				//ASCII
	{
		UARTx_SendByte(UART_PRINTF_CH, data);		//回显
		if(data == '\r')
			UARTx_SendByte(UART_PRINTF_CH,'\n');	//换行
		else if(data == 8)			//退格
		{
			UARTx_SendByte(UART_PRINTF_CH,' ');
			UARTx_SendByte(UART_PRINTF_CH, 8);		
		}
	}
	else	//汉字
	{
		gbk[flag] = data;
		flag ++;
		if(flag == 2)
		{
			flag = 0;
			UARTx_SendByte(UART_PRINTF_CH,gbk[0]);
			UARTx_SendByte(UART_PRINTF_CH,gbk[1]);
		}
	}
			
	return (int)data;
}

#endif //PRINTF_EN_



//__use_no_semihosting was requested, but _ttywrch was 
//2018-02-07 当使用了cJSON库后会报错，需要添加此函数
_ttywrch(int ch)
{
ch = ch;
}


/*
#if	MALLOC
#pragma import(__use_realtime_heap)
//这个函数在rt_heap.h中声明,需要用户自己去实现,返回任意值即可
unsigned __rt_heap_extend(unsigned size, void **block)
{
     return 0;
}
#endif
*/
