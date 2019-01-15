#include "dma.h"
#include "SYSTEM.H" 
#include "delay.h"

//DMA设置,主要用于存储器到存储器的数据传输,如将数据送入显存,或将显存数据送到显示器
//cp1300@139.com
//20111205 

	
/*************************************************************************************************************************
* 函数	:	void DMA_MemoryToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DATA_SIZE)
* 功能	:	存储器到存储器的DMA传输配置,用于更新显存或复制显存
* 参数	:	DMA1通道选择(DMA1_Channelx),存储器地址,外设存储器地址,传输位宽,传输数据个数
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111205
* 最后修改时间 : 20111205
* 说明	: 	无
*************************************************************************************************************************/	  
void DMA_MemoryToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DATA_SIZE)
{
	DeviceClockEnable(DEV_DMA1,1);//使能DMA1时钟
	DMA_CHx->CCR = 0;//失能DMA1并去除之前的设置
	DMA_CHx->CMAR = MemoryADD;//存储器地址
	DMA_CHx->CPAR = ParADD;//外设(存储器)地址
	DMA_CHx->CNDTR = DATA_SIZE;//传输的数据个数
	DMA_CHx->CCR |= BIT14;//使能存储器到存储器模式
	DMA_CHx->CCR |= BIT12;//通道优先级中等
	DMA_CHx->CCR |= SIZE_xbit << 10;//存储器数据大小
	DMA_CHx->CCR |= SIZE_xbit << 8;//外设数据大小(位宽)
	DMA_CHx->CCR |= BIT7;//使能存储器地址增量模式
	DMA_CHx->CCR |= BIT6;//使能外设(存储器)地址增量模式
	DMA_CHx->CCR |= BIT4;//传输方向:从存储器到外设
	DMA_CHx->CCR |=	BIT0;//开始传输
}


/*************************************************************************************************************************
* 函数	:			void DMA_MemoryToPeripheralConfig(DMA_Channel_TypeDef *DMA_CHx,u32 ParADD,u8 SIZE_xbit)
* 功能	:			存储器到外设的DMA传输配置
* 参数	:			DMA1通道选择(DMA1_Channelx),存储器地址,外设地址,传输位宽
* 返回	:			无
* 依赖	:			底层宏定义
* 作者	:			cp1300@139.com
* 时间	:			2011-12-05
* 最后修改时间 : 	2013-11-17
* 说明	: 			需要使能外设DMA触发
					需要启动发送
*************************************************************************************************************************/  
void DMA_MemoryToPeripheralConfig(DMA_Channel_TypeDef *DMA_CHx,u32 ParADD,u8 SIZE_xbit)
{
	DeviceClockEnable(DEV_DMA1,1);//使能DMA1时钟
	DMA_CHx->CCR = 0;//失能DMA1并去除之前的设置
	DMA_CHx->CPAR = ParADD;//外设(存储器)地址
//	DMA_CHx->CCR |= BIT14;//使能存储器到存储器模式
	DMA_CHx->CCR |= BIT12;//通道优先级中等
	DMA_CHx->CCR |= SIZE_xbit << 10;//存储器数据大小
	DMA_CHx->CCR |= SIZE_xbit << 8;//外设数据大小(位宽)
	DMA_CHx->CCR |= BIT7;//使能存储器地址增量模式
	//DMA_CHx->CCR |= BIT6;//使能外设(存储器)地址增量模式
	DMA_CHx->CCR |= BIT4;//传输方向:从存储器到外设
} 



/*************************************************************************************************************************
* 函数	:			void DMA_StartChannel(DMA_Channel_TypeDef *DMA_CHx,u16 DataSize)
* 功能	:			启动DMA	传输
* 参数	:			DMA1通道选择(DMA1_Channelx),MemoryADD:发送缓冲区地址,DataSize:传输数据大小
* 返回	:			无
* 依赖	:			底层宏定义
* 作者	:			cp1300@139.com
* 时间	:			2011-12-05
* 最后修改时间 : 	2013-11-17
* 说明	: 			需要使能外设DMA触发
					需要启动发送
					需要检查发送是否完成
*************************************************************************************************************************/  
void DMA_StartChannel(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize)
{
	DMA_CHx->CCR &=	~BIT0;			//停止传输
	DMA_CHx->CMAR = MemoryADD;		//存储器地址
	DMA_CHx->CNDTR = DataSize;		//传输的数据个数
	DMA_CHx->CCR |=	BIT0;			//开始传输
}


/*************************************************************************************************************************
* 函数	:	void DMA_PeripheralToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DATA_SIZE)
* 功能	:	外设到存储器的DMA传输配置
* 参数	:	DMA1通道选择(DMA1_Channelx),存储器地址,外设地址,传输位宽,传输数据个数
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120604
* 最后修改时间 : 20120604
* 说明	: 	需要使能外设DMA触发
*************************************************************************************************************************/  
void DMA_PeripheralToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DataSize)
{
	DeviceClockEnable(DEV_DMA1,1);//使能DMA1时钟
	DMA_CHx->CCR = 0;//失能DMA1并去除之前的设置
	DMA_CHx->CMAR = MemoryADD;//存储器地址
	DMA_CHx->CPAR = ParADD;//外设(存储器)地址
	DMA_CHx->CNDTR = DataSize;//传输的数据个数
//	DMA_CHx->CCR |= BIT14;//使能存储器到存储器模式
	DMA_CHx->CCR |= BIT12;//通道优先级中等
	DMA_CHx->CCR |= SIZE_xbit << 10;//存储器数据大小
	DMA_CHx->CCR |= SIZE_xbit << 8;//外设数据大小(位宽)
	DMA_CHx->CCR |= BIT7;//使能存储器地址增量模式
	//DMA_CHx->CCR |= BIT6;//使能外设(存储器)地址增量模式
	//外设到存储器
//	DMA_CHx->CCR |= BIT4;//传输方向:从存储器到外设
	DMA_CHx->CCR |=	BIT0;//开始传输
} 


/*************************************************************************************************************************
* 函数	:	void DMA_SetPeripheralToMemoryDataSize(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize)
* 功能	:	外设到存储器的DMA传输数据量设置
* 参数	:	DMA1通道选择(DMA1_Channelx),存储器地址,存储器地址,传输数据个数
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120604
* 最后修改时间 : 20120604
* 说明	: 	需要使能外设DMA触发
*************************************************************************************************************************/  
void DMA_SetPeripheralToMemoryDataSize(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize)
{
	DMA_CHx->CCR &=	~BIT0;			//停止传输
	DMA_CHx->CMAR = MemoryADD;		//存储器地址
	DMA_CHx->CNDTR = DataSize;		//传输的数据个数
	DMA_CHx->CCR |=	BIT0;			//开始传输
}

/*************************************************************************************************************************
* 函数	:	u16 DMA_GetCompleteResidualCnt(DMA_Channel_TypeDef *DMA_CHx)
* 功能	:	获取传输的剩余数据量
* 参数	:	DMA1通道选择(DMA1_Channelx)
* 返回	:	剩余传输数据个数
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-11-17
* 最后修改时间 : 2013-11-17
* 说明	: 	无
*************************************************************************************************************************/  
u16 DMA_GetCompleteResidualCnt(DMA_Channel_TypeDef *DMA_CHx)	
{
	return (u16)(DMA_CHx->CNDTR);						//获取传输的剩余数据量
}


/*************************************************************************************************************************
* 函数	:	void Wait_DMA_Complete(DMA_Channel_TypeDef *DMA_CHx)
* 功能	:	等待DMA传输完成
* 参数	:	DMA1通道选择(DMA1_Channelx)
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111205
* 最后修改时间 : 20111205
* 说明	: 	通过判断剩余的数据量来判断DMA是否完成
*************************************************************************************************************************/	  
void DMA_WaitComplete(DMA_Channel_TypeDef *DMA_CHx)
{
	u32 i = 0xfffff;
	
	while((DMA_CHx->CNDTR)&&i)	 //数据没有发送完毕
	{
		i --;
	}
}






















