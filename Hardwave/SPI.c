/*******************************************************************************
//文件名:	SPI.c
//功能:		STM32硬件SPI
//作者:cp1300@139.com
//创建时间:2011-06-25 
//修改时间:2012-09-29
//修订说明:20120929：增加配置函数，并且使用时钟获取函数获取系统时钟
	2014-08-07：增加从机接收模式
//声明:	 
********************************************************************************/

#include "SPI.H"
#include "SYSTEM.H"
#include "GPIO_INIT.H"


//定义SPI通道号最大值
#define SPI_ChMax	2

//SPI外设结构指针
static const  SPI_TypeDef *SPIxN[3] = {SPI1,SPI2,SPI3};
//波特率分频数
//512分频用于SPI2,SPI3等,因为SPI2,3的时钟是系统时钟的一半,相当于已经进行了2分频了,也就是说SPI2,3至少进行4分频 
static const u16 BAUD_RATE[9] = {2,4,8,16,32,64,128,256,512};


//常用模式1
//比如用于存储卡,VS1003B等SPI设备
//全双工模式,SPI主机,8个数据位,空闲时钟高电平,高位在前,软件控制SS,不开启中断,不开启DMA
const SPI_Config_Type SPI_DEFAULT_01 = {0,0,0,0,1,0,0,0,1,0,0,0,0,0,0};	



//从模式
//全双工模式,SPI主机,8个数据位,空闲时钟高电平,高位在前,控制SS,开启中断,不开启DMA
const SPI_Config_Type SPI_DEFAULT_02 = {0,0,0,0,1,1,0,0,0,0,1,0,0,0,0};	



//相关UART状态结构
typedef struct
{
	FlagStatus	NewDataFlag;//接收到新数据
	FlagStatus	BuffFull;	//接收Buff满
	FlagStatus	IntRx;		//是否开启中断接收
	u8 			*RxBuff;	//接收Buff指针
	u16			RxBuffSize;	//接收缓冲区大小,一帧数据大小
	u16 		RxCnt;		//接收数据计数器
} SPIRx_TypeDef;


//SPI1	接收状态结构
static SPIRx_TypeDef SPIRx[SPI_ChMax + 1];




/*************************************************************************************************************************
* 函数	:	u8 SPIx_ReadWriteByte(SPI_CH_Type ch,u8 TxData)
* 功能	:	SPI 读写一个字节
* 参数	:	ch:通道选择;TxData:要写入的字节
* 返回	:	读取到的字节
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110625
* 最后修改时间 : 20120404
* 说明	: 	无
*************************************************************************************************************************/
u8 SPIx_ReadWriteByte(SPI_CH_Type ch,u8 TxData)
{		
	u8 retry=0;
	SPI_TypeDef *SPIx;

	if(ch > SPI_ChMax) return 0;			//通道号超出范围,返回错误0
	SPIx = (SPI_TypeDef *)SPIxN[ch];		//获取SPI结构指针				 
	while((SPIx->SR & 1 << 1)== 0)			//等待发送区空	
	{
		retry ++;
		if(retry > 200) 					//等待超时
			return 0;
	}			  
	SPIx->DR = TxData;	 	  				//发送一个byte 
	retry = 0;
	while((SPIx->SR & 1 << 0) == 0) 		//等待接收完一个byte  
	{
		retry ++;
		if(retry > 200) 					//等待超时
			return 0;
	}	  						    
	return SPIx->DR;  						//返回收到的数据				    
}


/*************************************************************************************************************************
* 函数	:	u32 SPIx_SetSpeed(SPI_CH_Type ch,SPI_SPEED_Type Speed)
* 功能	:	SPI速度设置
* 参数	:	ch:通道选择,SYS_CLK:当前系统时钟,BaudRate:分频系数,2,4,8,16,32,64,128,256
* 返回	:	0:设置失败,其他:当前SPI波特率
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110625
* 最后修改时间 : 20120404
* 说明	: 	SPI2,SPI3最大速度是SPI1的一半
*************************************************************************************************************************/
u32 SPIx_SetSpeed(SPI_CH_Type ch,SPI_SPEED_Type Speed)
{
	SPI_TypeDef *SPIx;
	u32 SYS_CLK = SYSTEM_GetClkSpeed();		//获取系统时钟		

	if(ch > SPI_ChMax) return 0;			//通道号超出范围,返回错误0
	if(Speed > 7) return 0;					//波特率分频值设置错误,返回错误0
	SPIx = (SPI_TypeDef *)SPIxN[ch];		//获取SPI结构指针
	SPIx->CR1 &= ~(1<<6); 					//SPI设备失能
	SPIx->CR1 &= ~(7 << 3);					//清除波特率分频器
	SPIx->CR1 |= ((Speed - (ch ? 1 : 0)) << 3);	//设置波特率
	SPIx->CR1 |= 1<<6; 						//SPI设备使能	

	return (SYS_CLK / BAUD_RATE[Speed]) ;	//时钟分频,计算最终波特率时钟,并返回
} 



/*************************************************************************************************************************
* 函数	:	u8 SPIx_Init(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config,SPI_SPEED_Type Speed)
* 功能	:	SPI初始化函数
* 参数	:	ch:通道选择,SPI_Config:配置结构指针,Speed:速度,见宏定义
* 返回	:	0:初始化成功;1:初始化失败,2:波特率设置失败
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110625
* 最后修改时间 : 20120929
* 说明	: 	SPI2,SPI3最大速度是SPI1的一半
*************************************************************************************************************************/
u8 SPIx_Init(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config,SPI_SPEED_Type Speed)
{
	//初始化IO接口
	DeviceClockEnable(DEV_AFIO,ENABLE);				//复用功能AFIO时钟使能
	switch (ch)
	{
	 	case 0:	//SPI1,SCK:PA5,MISO:PA6,MOSI:PA7
		{
			DeviceClockEnable(DEV_SPI1,ENABLE);		//SPI1 时钟使能
			DeviceClockEnable(DEV_GPIOA,ENABLE);	//GPIOA时钟使能
			DeviceReset(DEV_SPI1);					//SPI1复位 	 
			if(SPI_Config->EnMSTR)					//使能主机模式
			{
				GPIOx_Init(GPIOA,BIT5,AF_PP,SPEED_50M); //SCK 设置成复用推挽输出
				GPIOx_Init(GPIOA,BIT6,IN_IPU,0);		//MISO设置为上拉输入
				GPIOx_Init(GPIOA,BIT7,AF_PP,SPEED_50M); //MOSI 设置成复用推挽输出
			}
			else		//从机模式
			{
				GPIOx_Init(GPIOA,BIT5,IN_IPU,0); 		//SCK 设置成上拉输入
				GPIOx_Init(GPIOA,BIT6,AF_PP,SPEED_50M);	//MISO设置为复用推挽输出
				GPIOx_Init(GPIOA,BIT7,IN_IPU,0); 		//MOSI 设置成上拉输入
				GPIOx_Init(GPIOA,BIT4,IN_IPU,0); 		//CS 设置成复上拉输入
				
				SPI1->CR1 = 0;							//清除设置,并停止SPI
				SPI1->CR2 = 0;	
				SPI1->CR1 |= BIT10 + BIT1;// + BIT0;		//只接收模式,空闲时钟高电平
				SPI1->CR2 |= BIT6;						//使能接收中断
				NVIC_IntEnable(IRQ_SPI1,1);				//开启SPI1全局中断
				SPI1->CR1 |= 1 << 6; 	//SPI设备使能
				SPIRx[ch].IntRx = SET;
			}
		}break;
		case 1:	//SPI2,SCK:PB13,MISO:PB14,MOSI:PB15
		{
			DeviceClockEnable(DEV_SPI2,ENABLE);		//SPI2 时钟使能
			DeviceClockEnable(DEV_GPIOB,ENABLE);	//GPIOB时钟使能
			DeviceReset(DEV_SPI2);					//SPI2复位 	 
			if(SPI_Config->EnMSTR)					//使能主机模式
			{
				GPIOx_Init(GPIOB,BIT13,AF_PP,SPEED_50M);//SCK 设置成复用推挽输出
				GPIOx_Init(GPIOB,BIT14,IN_IPU,0);		//MISO设置为上拉输入
				GPIOx_Init(GPIOB,BIT15,AF_PP,SPEED_50M);//MOSI 设置成复用推挽输出
			}
			else		//从机模式
			{
				GPIOx_Init(GPIOB,BIT13,IN_IPU,0); 		//SCK 设置成上拉输入
				GPIOx_Init(GPIOB,BIT14,AF_PP,SPEED_50M);	//MISO设置为复用推挽输出
				GPIOx_Init(GPIOB,BIT15,IN_IPU,0); 		//MOSI 设置成上拉输入
				GPIOx_Init(GPIOB,BIT12,IN_IPU,0); 		//CS 设置成复上拉输入
				
				SPI2->CR1 = 0;							//清除设置,并停止SPI
				SPI2->CR2 = 0;	
				SPI2->CR1 |= BIT10 + BIT1;// + BIT0;	//只接收模式,空闲时钟高电平
				SPI2->CR2 |= BIT6;						//使能接收中断
				NVIC_IntEnable(IRQ_SPI2,1);				//开启SPI2全局中断
				SPI2->CR1 |= 1 << 6; 	//SPI设备使能
				SPIRx[ch].IntRx = SET;
			}

		}break;
		case 2://SPI3,SCK:PB3,MISO:PB4,MOSI:PB5
		{
			DeviceClockEnable(DEV_SPI3,ENABLE);		//SPI3 时钟使能
			DeviceClockEnable(DEV_GPIOB,ENABLE);	//GPIOB时钟使能
			DeviceReset(DEV_SPI3);					//SPI3复位 	 
			GPIOx_Init(GPIOB,BIT3,AF_PP,SPEED_50M); //SCK 设置成复用推挽输出
			GPIOx_Init(GPIOB,BIT4,IN_IPU,0);		//MISO设置为上拉输入
			GPIOx_Init(GPIOB,BIT5,AF_PP,SPEED_50M); //MOSI 设置成复用推挽输出
		}break;
		default : return 1;							//通道选择错误,返回错误1
	}
	if(SPI_Config->EnMSTR)
	{
		if(SPIx_Config(ch,SPI_Config))					//配置
			return 1;									//配置失败
	}
	if(SPIx_SetSpeed(ch,Speed) == 0)				//设置波特率
		return 2;									//波特率设置失败
	//SPIx_ReadWriteByte(ch,0xff);					//启动传输
	return 0;										//初始化成功
}





/*************************************************************************************************************************
* 函数	:	u8  SPIx_Config(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config)	
* 功能	:	SPI配置
* 参数	:	ch：通道选择；SPI_Config：配置结构指针
* 返回	:	0:设置成功;1:超出编号范围,返回错误
* 依赖	:	底层寄存器操作
* 作者	:	cp1300@139.com
* 时间	:	20120929
* 最后修改时间 : 20120929
* 说明	: 	用于配置SPI模式，常用的是8bit，高位在前，空闲时钟高电平或者空闲时钟低电平
*************************************************************************************************************************/
u8  SPIx_Config(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config)
{
	SPI_TypeDef *SPIx;
	u32 CR1 = 0, CR2 = 0;		

	if(ch > SPI_ChMax) return 1;					//通道号超出范围,返回错误1
	SPIx = (SPI_TypeDef *)SPIxN[ch];				//获取SPI结构指针

	SPIx->CR1 = 0;			//清除设置,并停止SPI
	SPIx->CR2 = 0;
	//配置寄存器1
	if(SPI_Config->En16bit)	//使能16BIT模式
	{
	 	CR1 |= BIT11;
	}
	if(SPI_Config->EnLSB)	//使能低位在前
	{
		CR1 |= BIT7;
	}
	if(SPI_Config->EnCPOH)	//使能空闲时钟为高电平
	{
		CR1 |= BIT1;
	}
	if(SPI_Config->EnCPHA)	//使能数据采样从第二个边沿开始
	{
		CR1 |= BIT0;
	}
	if(SPI_Config->EnSSM)	//使能软件控制片选信号
	{
		CR1 |= BIT9;
		CR1 |= BIT8;
	}
	if(SPI_Config->EnRxOnly)//半双工使能
	{
		CR1 |= BIT10;	
	}
	if(SPI_Config->EnBIDI) 	//使能单线双向模式
	{
	 	CR1 |= BIT15;
	}
	if(SPI_Config->EnCRC)  //使能CRC校验
	{
		CR1 |= BIT13;
	}
	if(SPI_Config->EnMSTR)	//使能主设备模式	
	{
	 	CR1 |= BIT2;
	}
	//配置寄存器2
	if(SPI_Config->EnTxINT)	//使能发送缓冲区空中断
	{
		CR2 |= BIT7;
	}
	if(SPI_Config->EnRxINT)	//使能接收缓冲区空中断
	{
		CR2 |= BIT6;
	}
	if(SPI_Config->EnErrorINT)	//使能接收错误中断
	{
		CR2 |= BIT5;
	}
	if(SPI_Config->EnSSOE)	//设备开启时,开启主模式下SS输出,该设备不能工作在多主设备模式
	{
		CR2 |= BIT2;
	}
	if(SPI_Config->EnTxDMA)	//发送缓冲区DMA使能
	{
		CR2 |= BIT1;
	}
	if(SPI_Config->EnRxDMA)	//接收缓冲区DMA使能
	{
		CR2 |= BIT0;
	}
	SPIx->CR1 = CR1;		//写入配置数据到寄存器
	SPIx->CR2 = CR2;
	SPIx->CR1 |= 1 << 6; 	//SPI设备使能
	return 0;
}



//spi掉电
void SPIx_PowerDown(SPI_CH_Type ch)
{
	SPI_TypeDef *SPIx;	

	if(ch > SPI_ChMax) return;					//通道号超出范围,返回错误1
	SPIx = (SPI_TypeDef *)SPIxN[ch];				//获取SPI结构指针
	
	SPIx->CR1 &= ~(1 << 6); 	//SPI设备掉电
}


//spi上电
void SPIx_PowerUp(SPI_CH_Type ch)
{
	SPI_TypeDef *SPIx;	

	if(ch > SPI_ChMax) return;					//通道号超出范围,返回错误1
	SPIx = (SPI_TypeDef *)SPIxN[ch];				//获取SPI结构指针
	
	SPIx->CR1 |= 1 << 6; 	//SPI设备使能
}



/*************************************************************************************************************************
* 函数	:	bool SPIx_GetNewDataFlag(SPI_CH_Type ch)
* 功能	:	获取SPI新数据标志
* 参数	:	ch:通道选择
* 返回	:	TRUE:成功,FALSE:失败
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-25
* 最后修改时间 : 2013-09-25
* 说明	: 	用于判断是否有新的数据,会清除掉新数据标志的
*************************************************************************************************************************/
bool SPIx_GetNewDataFlag(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)										//判断端口是否超出范围
		return FALSE;

	if(SPIRx[ch].IntRx == SET)									//开启了中断接收
	{
		if(SPIRx[ch].NewDataFlag == SET) 						//有新数据
		{
		 	SPIRx[ch].NewDataFlag = RESET;						//清除标志
			return TRUE;										//返回有新数据
		}
	}
	else														//没开启中断接收
	{
	 	if(((SPI_TypeDef *)SPIxN[ch])->SR & BIT0)				//RXNE=1,接收到新数据
		{
			((SPI_TypeDef *)SPIxN[ch])->SR &= ~BIT0;			//清除标志
			return TRUE;
		}
	}
	return FALSE;
}


/*************************************************************************************************************************
* 函数	:	bool SPIx_GetRxBuffFullFlag(SPI_CH_Type ch)
* 功能	:	获取SPI接收缓冲区满标志
* 参数	:	ch:通道选择
* 返回	:	TRUE:成功,FALSE:失败
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-25
* 最后修改时间 : 2013-09-25
* 说明	: 	用于判断接收缓冲区是否满,会清除标志
*************************************************************************************************************************/
bool SPIx_GetRxBuffFullFlag(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)					//判断端口是否超出范围
		return FALSE;
	if(SPIRx[ch].BuffFull == SET)			//缓冲区已满
	{
	 	SPIRx[ch].BuffFull = RESET;			//清除满标志
		return TRUE;
	}
	return FALSE;
}





/*************************************************************************************************************************
* 函数	:	u8 SPIx_GetNewData(SPI_CH_Type ch)
* 功能	:	获取SPI新数据
* 参数	:	ch:通道选择
* 返回	:	收到的数据
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-25
* 最后修改时间 : 2013-09-25
* 说明	: 	用于接收一个字节数据
*************************************************************************************************************************/
u8 SPIx_GetNewData(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)								//判断端口是否超出范围
		return 0;

	return (((SPI_TypeDef *)SPIxN[ch])->DR);	//返回数据
}



/*************************************************************************************************************************
* 函数	:	void SPIx_SetRxBuff(SPI_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
* 功能	:	设置SPI接收缓冲区
* 参数	:	ch:通道选择,RxBuffSize:缓冲区大小,RxBuff:缓冲区指针
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-25
* 最后修改时间 : 2013-09-25
* 说明	: 	一定要设置,否则开启中断接收时可能会异常
*************************************************************************************************************************/
void SPIx_SetRxBuff(SPI_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
{
	
	if(ch > SPI_ChMax - 1)					//判断端口是否超出范围
		return;

	SPIRx[ch].RxBuffSize = RxBuffSize; 		//设置缓冲区大小
	SPIRx[ch].RxBuff = RxBuff;				//设置缓冲区指针
	SPIRx[ch].RxCnt = 0;					//计数器清零

}





/*************************************************************************************************************************
* 函数	:	u32 SPIx_GetRxCnt(SPI_CH_Type ch)
* 功能	:	获取SPI接收数据计数器
* 参数	:	ch:通道选择
* 返回	:	接收到的数据数量
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-25
* 最后修改时间 : 2013-09-25
* 说明	: 	无
*************************************************************************************************************************/
u32 SPIx_GetRxCnt(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)						//判断端口是否超出范围
		return 0;
	return SPIRx[ch].RxCnt;						//返回计数值
}




/*************************************************************************************************************************
* 函数	:	void SPIx_ClearRxCnt(SPI_CH_Type ch)
* 功能	:	清除SPI接收数据计数器
* 参数	:	ch:通道选择
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-25
* 最后修改时间 : 2013-09-25
* 说明	: 	无
*************************************************************************************************************************/
void SPIx_ClearRxCnt(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)					//判断端口是否超出范围
		return;
	SPIRx[ch].RxCnt = 0;					//计数器清零
}



/*************************************************************************************************************************
* 函数	:	void SPI1_IRQHandler (void)
* 功能	:	SPI1中断接收函数
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-24
* 最后修改时间 : 2013-09-24
* 说明	: 	无
*************************************************************************************************************************/
void SPI1_IRQHandler (void)
{
	if(SPIRx[SPI_CH1].RxBuffSize > 0)										//接收缓冲区大于0
	{
	 	(SPIRx[SPI_CH1].RxBuff)[(SPIRx[SPI_CH1].RxCnt) ++] = SPI1->DR; 		//将数据存放到缓冲区
		//uart_printf("SPIRx[SPI_CH1].RxCnt=0x%X\r\n",SPIRx[SPI_CH1].RxCnt);
		//uart_printf("SPIRx[SPI_CH1].RxBuffSize=0x%X\r\n",SPIRx[SPI_CH1].RxBuffSize);
		if(SPIRx[SPI_CH1].RxCnt == SPIRx[SPI_CH1].RxBuffSize) 				//缓冲区已满
		{
			  SPIRx[SPI_CH1].RxCnt = 0;										//接收计数器清零
			  SPIRx[SPI_CH1].BuffFull = SET;								//缓冲区已满标志
		}	
	}
	else
	{
		SPI1->SR &= ~BIT0;
	}
	SPIRx[SPI_CH1].NewDataFlag = SET;										//收到新数据标志
	//UARTx_ClearRxInt(UART_CH1);		   									//清除接收中断标志
}



/*************************************************************************************************************************
* 函数	:	void SPI1_IRQHandler (void)
* 功能	:	SPI1中断接收函数
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013-09-24
* 最后修改时间 : 2013-09-24
* 说明	: 	无
*************************************************************************************************************************/
void SPI2_IRQHandler (void)
{
	if(SPIRx[SPI_CH2].RxBuffSize > 0)										//接收缓冲区大于0
	{
	 	(SPIRx[SPI_CH2].RxBuff)[(SPIRx[SPI_CH2].RxCnt) ++] = SPI2->DR; 		//将数据存放到缓冲区
		//uart_printf("SPIRx[SPI_CH1].RxCnt=0x%X\r\n",SPIRx[SPI_CH1].RxCnt);
		//uart_printf("SPIRx[SPI_CH1].RxBuffSize=0x%X\r\n",SPIRx[SPI_CH1].RxBuffSize);
		if(SPIRx[SPI_CH2].RxCnt == SPIRx[SPI_CH2].RxBuffSize) 				//缓冲区已满
		{
			  SPIRx[SPI_CH2].RxCnt = 0;										//接收计数器清零
			  SPIRx[SPI_CH2].BuffFull = SET;								//缓冲区已满标志
		}	
	}
	else
	{
		SPI2->SR &= ~BIT0;
	}
	SPIRx[SPI_CH2].NewDataFlag = SET;										//收到新数据标志
	//UARTx_ClearRxInt(UART_CH1);		   									//清除接收中断标志
}









