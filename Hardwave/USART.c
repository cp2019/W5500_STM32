/*************************************************************************************************************
 * 文件名:		usart.c
 * 功能:		STM32 USART驱动
 * 作者:		cp1300@139.com
 * 创建时间:	2011年6月11日
 * 最后修改时间:2013年5月29日
 * 详细:		已经修复已知bug
				2013-11-17:增加DMA收发模式
				2014-08-19:增加DMA,中断混合模式，主要是由于串口4，串口5不支持DMA
				2018-06-29:去掉中断接收模式下缓冲区满后重头覆盖，满了之后将不会进行存储
*************************************************************************************************************/		
#include "SYSTEM.H"
#include "GPIO_INIT.H"
#include "USART.H"

//UART外设结构指针
static const  USART_TypeDef * const USARTxN[5] = {USART1,USART2,USART3,UART4,UART5};
//DAM通道设置
#if UART_DMA_EN
#include "dma.h"
static const  DMA_Channel_TypeDef * const USARTxChannel[3] = {DMA1_Channel4, DMA1_Channel7, DMA1_Channel2};	//发送通道
static const  DMA_Channel_TypeDef * const USARRxChannel[3] = {DMA1_Channel5, DMA1_Channel6, DMA1_Channel3};	//接收通道
#endif	//UART_DMA_EN

//相关UART状态结构
typedef struct
{
	FlagStatus	NewDataFlag;//接收到新数据
	FlagStatus	BuffFull;	//接收Buff满
	FlagStatus	IntRx;		//是否开启中断接收
	u8 			*RxBuff;	//接收Buff指针
	u16			RxBuffSize;	//接收缓冲区大小,一帧数据大小
	u16 		UartRxCnt;	//接收数据计数器
	u8			TempData;	//用于接收溢出后读取数据寄存器，清除读取数据标志
} UartRx_TypeDef;


//UART1	接收状态结构
static UartRx_TypeDef UartRx[UART_ChMax + 1];

#ifdef _UCOS_II_
#include "ucos_ii.h"
#endif



/*************************************************************************************************************************
* 函数	:	bool UARTx_Init(UART_CH_Type ch,u32 Speed,u8 RX_Int)
* 功能	:	串口初始化
* 参数	:	ch:通道选择,0->usart1,Speed:串口速度,RX_Int:是否时能中断接受
* 返回	:	TRUE:成功,FALSE:失败
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 2013-11-17
* 说明	: 	USART1~UART5,对应通道UART_CH1-UART_CH5
			2013-11-17:添加DMA支持
*************************************************************************************************************************/
bool UARTx_Init(UART_CH_Type ch,u32 Speed,u8 RX_Int)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];//获取对应通道硬件基址指针
	u8 irq_n;

	if(ch > UART_ChMax - 1)
		return FALSE;	//端口号超出范围
	//初始化UART IO
	DeviceClockEnable(DEV_AFIO, ENABLE);					//复用功能AFIO时钟使能
	switch (ch)
	{
		case UART_CH1:		//通道1,USART1 ,TX:PA9;RX:PA10
		{
			DeviceClockEnable(DEV_GPIOA,ENABLE);		//GPIO A 时钟使能
			DeviceClockEnable(DEV_USART1,ENABLE);		//USART 1 时钟使能
			GPIOx_Init(GPIOA,BIT9,AF_PP, SPEED_50M);   	//PA09,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOA,BIT10,IN_IPU,IN_IPU);  	//上拉输入
			DeviceReset(DEV_USART1);					//复位串口1
			irq_n =  IRQ_USART1;						//串口1中断号
		}break;
		case UART_CH2:		//通道2,USART2 ,TX:PA2;RX:PA3
		{	
			DeviceClockEnable(DEV_GPIOA,ENABLE);		//GPIO A 时钟使能
			DeviceClockEnable(DEV_USART2,ENABLE);		//USART 2 时钟使能
			GPIOx_Init(GPIOA,BIT2,AF_PP, SPEED_50M);   	//PA2,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOA,BIT3,IN_IPU,IN_IPU);  		//上拉输入
			DeviceReset(DEV_USART2);					//复位串口2
			irq_n =  IRQ_USART2;						//串口2中断号		
		}break;
		case UART_CH3:		//通道3,USART3 ,TX:PB10;RX:PB11
		{
			DeviceClockEnable(DEV_GPIOB,ENABLE);		//GPIO B 时钟使能
			DeviceClockEnable(DEV_USART3,ENABLE);		//USART 3 时钟使能
			GPIOx_Init(GPIOB,BIT10,AF_PP, SPEED_50M);   	//PB10,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOB,BIT11,IN_IPU,IN_IPU);  	//上拉输入
			DeviceReset(DEV_USART3);					//复位串口3
			irq_n =  IRQ_USART3;						//串口3中断号		
		}break;

		case UART_CH4:		//通道4,UART4 ,TX:PC10;RX:PC11
		{
			DeviceClockEnable(DEV_GPIOC,ENABLE);		//GPIO C 时钟使能
			DeviceClockEnable(DEV_UART4,ENABLE);		//UART 4 时钟使能
			GPIOx_Init(GPIOC,BIT10,AF_PP, SPEED_50M);   //PC10,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOC,BIT11,IN_IPU,IN_IPU);  	//上拉输入
			DeviceReset(DEV_UART4);						//复位串口1
			irq_n =  IRQ_UART4;							//串口1中断号		
		}break;
		case UART_CH5:		//通道5,UART5 ,TX:PC12;RX:PD2
		{
			DeviceClockEnable(DEV_GPIOC,ENABLE);		//GPIO C 时钟使能
			DeviceClockEnable(DEV_GPIOD,ENABLE);		//GPIO D 时钟使能
			DeviceClockEnable(DEV_UART5,ENABLE);		//UART 5 时钟使能
			GPIOx_Init(GPIOC,BIT12,AF_PP, SPEED_50M);   //PC12,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOD,BIT2,IN_IPU,IN_IPU);  		//上拉输入
			DeviceReset(DEV_UART5);						//复位串口5
			irq_n =  IRQ_UART5;							//串口5中断号			
		}break;
		default : return FALSE;							//端口号超出范围,返回错误
	}
	//设置波特率分频系数
	UARTx_SetBaudRate(ch, Speed);						//设置波特率
	//配置UART
	UARTx->CR1 = 0x2000;								//使能USART,1个开始位,8位数据
	UARTx->CR1 |= 0x8;									//置TE = 1;发送使能;发送第一个空闲位
	UARTx->CR1 |= 0x04;									//RE = 1;接收使能
#if UART_DMA_EN
	if(ch > UART_CH3)	//串口4,5不支持DMA
	{
		UARTx->CR3 = 0;	
	}
	else
	{
		UARTx->CR3 = BIT7;									//全双工,DMA发送模式,禁止错误中断
		DMA_MemoryToPeripheralConfig((DMA_Channel_TypeDef *)USARTxChannel[ch], (u32)&UARTx->DR, DMA_SIZE_8BIT);		//存储器到外设的DMA传输配置
	}
	
#else
	UARTx->CR3 = 0;										//全双工,禁止错误中断
#endif //UART_DMA_EN
	UARTx_SetRxBuff(ch,0,NULL);							//设置串口接收缓冲区
	UARTx_ClearRxInt(ch);		   						//清除串口接收中断标志
	if(RX_Int)
	{
		
#if UART_DMA_EN
		if(ch > UART_CH3)	//串口4,5不支持DMA
		{
			UARTx->CR1 |= 0x20;								//RXNEIE = 1,开RXNE中断,即开启接收中断
			NVIC_IntEnable(irq_n,1);						//开启USART1全局中断
			UartRx[ch].IntRx = SET;							//中断接收标志有效
		}
		else
		{
			UARTx->CR3 |= BIT6;							//DMA接收模式
		}
#else
		UARTx->CR1 |= 0x20;								//RXNEIE = 1,开RXNE中断,即开启接收中断
	 	NVIC_IntEnable(irq_n,1);						//开启USART1全局中断
		UartRx[ch].IntRx = SET;							//中断接收标志有效
#endif //UART_DMA_EN	 	
		
	} 
	else
	{
		NVIC_IntEnable(irq_n,0); 						//关闭USART全局中断
		UartRx[ch].IntRx = RESET;						//中断接收标志无效
	}
	return TRUE;										//初始化成功,返回0
}





/*************************************************************************************************************************
* 函数	:	void UARTx_PowerDown(UART_CH_Type ch)
* 功能	:	UART掉电
* 参数	:	ch:通道选择
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20130316
* 说明	: 	进入低功耗模式
*************************************************************************************************************************/
void UARTx_PowerDown(UART_CH_Type ch)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];	//获取对应通道硬件基址指针
	
	if(ch > UART_ChMax - 1)									//判断端口是否超出范围
		return;
	//while(!(UARTx->SR & 0x80));								//等待发送完成才开启低功耗模式
	
	switch (ch)
	{
		case UART_CH1:		//通道1,USART1 ,TX:PA9;RX:PA10
		{
			GPIOx_Init(GPIOA,BIT9,IN_IPT, IN_IN);   	
			GPIOx_Init(GPIOA,BIT10,IN_IPT,IN_IN);  
		}break;
		case UART_CH2:		//通道2,USART2 ,TX:PA2;RX:PA3
		{	
			GPIOx_Init(GPIOA,BIT2,IN_IPT, IN_IN);   
			GPIOx_Init(GPIOA,BIT3,IN_IPT,IN_IN);  		
		}break;
		case UART_CH3:		//通道3,USART3 ,TX:PB10;RX:PB11
		{
			GPIOx_Init(GPIOB,BIT10,IN_IPT, IN_IN);   
			GPIOx_Init(GPIOB,BIT11,IN_IPT,IN_IN);  
		}break;

		case UART_CH4:		//通道4,UART4 ,TX:PC10;RX:PC11
		{
			GPIOx_Init(GPIOC,BIT10,IN_IPT, IN_IN);  
			GPIOx_Init(GPIOC,BIT11,IN_IPT,IN_IN);  	
		}break;
		case UART_CH5:		//通道5,UART5 ,TX:PC12;RX:PD2
		{
			GPIOx_Init(GPIOC,BIT12,IN_IPT, IN_IN);  
			GPIOx_Init(GPIOD,BIT2,IN_IPT,IN_IN);  	
		}break;
		default : return ;							//端口号超出范围,返回
	}
	UARTx->CR1 &= ~(1 << 13);								//UE位写0,开启低功耗	
}


/*************************************************************************************************************************
* 函数	:	void UARTx_PowerUp(UART_CH_Type ch)
* 功能	:	UART上电
* 参数	:	ch:通道选择
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20130316
* 说明	: 	退出低功耗模式
*************************************************************************************************************************/
void UARTx_PowerUp(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)									//判断端口是否超出范围
		return;
	
	switch (ch)
	{
		case UART_CH1:		//通道1,USART1 ,TX:PA9;RX:PA10
		{
			GPIOx_Init(GPIOA,BIT9,AF_PP, SPEED_50M);   	//PA09,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOA,BIT10,IN_IPU,IN_IPU);  	//上拉输入
		}break;
		case UART_CH2:		//通道2,USART2 ,TX:PA2;RX:PA3
		{	
			GPIOx_Init(GPIOA,BIT2,AF_PP, SPEED_50M);   	//PA2,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOA,BIT3,IN_IPU,IN_IPU);  		//上拉输入
		}break;
		case UART_CH3:		//通道3,USART3 ,TX:PB10;RX:PB11
		{
			GPIOx_Init(GPIOB,BIT10,AF_PP, SPEED_50M);   	//PB10,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOB,BIT11,IN_IPU,IN_IPU);  	//上拉输入
		}break;

		case UART_CH4:		//通道4,UART4 ,TX:PC10;RX:PC11
		{
			GPIOx_Init(GPIOC,BIT10,AF_PP, SPEED_50M);   //PC10,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOC,BIT11,IN_IPU,IN_IPU);  	//上拉输入
		}break;
		case UART_CH5:		//通道5,UART5 ,TX:PC12;RX:PD2
		{
			GPIOx_Init(GPIOC,BIT12,AF_PP, SPEED_50M);   //PC12,TXD只能设置成复用推挽输出
			GPIOx_Init(GPIOD,BIT2,IN_IPU,IN_IPU);  		//上拉输入	
		}break;
		default : return ;							//端口号超出范围,返回
	}
	((USART_TypeDef *)USARTxN[ch])->CR1 |= (1 << 13);		//UE位1,退出低功耗模式	
}




/*************************************************************************************************************************
* 函数	:	void UARTx_SetBaudRate(UART_CH_Type ch,u32 baud)
* 功能	:	串口波特率设置
* 参数	:	ch:通道选择,baud:波特率,如9600,115200等等
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013316
* 最后修改时间 : 2013316
* 说明	: 	USART1~UART5,对应通道UART_CH1-UART_CH5
			设置前必须关闭串口
			会自动获取系统当前的时钟,并进行计算.
*************************************************************************************************************************/
void UARTx_SetBaudRate(UART_CH_Type ch,u32 baud)
{
	u32 SysClk = 0;
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   	//获取对应通道硬件基址指针
	float fclk;

	SysClk = SYSTEM_GetClkSpeed();							//获取系统时钟
	if(ch > 0)
	{
		SysClk /= 2;		   								//USART2,3,4,5时钟
	}
	UARTx_PowerDown(ch);									//进入掉电模式,进行配置
	fclk = (float)SysClk / 16.0 / baud;						//计算波特率分频系数
	SysClk = (u16)fclk;										//得到波特率分频系数整数部分	
	UARTx->BRR =  SysClk << 4;								//设置波特率整数部分
	fclk -= SysClk;											//得到波特率分频系数小数部分
	fclk *= 16;
	UARTx->BRR |= 0xf & (u16)fclk;							//设置波特率小数部分 
	UARTx_PowerUp(ch);										//串口重新上电
}




/*************************************************************************************************************************
* 函数	:	bool UARTx_Config(UART_CH_Type ch,UART_Config_TypeDef * cfg)
* 功能	:	串口配置
* 参数	:	ch:通道选择,0->usart1;cfg:串口配置结构指针
* 返回	:	TRUE:成功,FALSE:失败
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120408
* 最后修改时间 : 20120408
* 说明	: 	USART1~UART5,对应通道UART_CH1-UART_CH5
*************************************************************************************************************************/
bool UARTx_Config(UART_CH_Type ch,UART_Config_TypeDef * cfg)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   	//获取对应通道硬件基址指针

	if(ch > UART_ChMax - 1)									//判断端口是否超出范围
		return FALSE;
	UARTx_PowerDown(ch);									//进入掉电模式,进行配置
	switch (cfg->OddEvenVerify)								//设置校验位
	{
	 	case UART_VERIFY_NULL:								//无校验
		{
			UARTx->CR1 &= ~BIT12;							//一个起始位,8个数据位
			UARTx->CR1 &= ~BIT10;							//禁止校验控制
		}break;
		case UART_ODD:										//奇校验
		{
			UARTx->CR1 |= BIT12;							//一个起始位,9个数据位
			UARTx->CR1 |= BIT10;							//使能校验控制
			UARTx->CR1 |= BIT9;								//奇校验
		}break;
		case UART_EVEN:										//偶校验
		{
			UARTx->CR1 |= BIT12;							//一个起始位,9个数据位
			UARTx->CR1 |= BIT10;							//使能校验控制
			UARTx->CR1 &= ~BIT9;							//偶校验
		}break;
		default : 
		{
			UARTx_PowerUp(ch);								//串口重新上电
			return FALSE;									//设置错误,返回校验设置错误
		}
	}
	if(cfg->StopBitWidth == UART_STOP_1BIT) 				//设置停止位
	{
		UARTx->CR2 &= ~(0x3 << 12);							//清除设置,默认一个停止位
	}
	else if(cfg->StopBitWidth == UART_STOP_2BIT)
	{
		UARTx->CR2 &= ~(0x3 << 12);
		UARTx->CR2 |= (0x2 << 12);							//2个停止位
	} 
	else
	{
		UARTx_PowerUp(ch);									//串口重新上电
		return FALSE;										//停止位设置错误,返回错误2
	}
	UARTx_PowerUp(ch);										//串口重新上电	
	return TRUE;											//设置完成,返回TRUE
}






/*************************************************************************************************************************
* 函数	:	void UARTx_EnableRx(UART_CH_Type ch,FunctionalState Enable)
* 功能	:	串口接收使能
* 参数	:	ch:通道选择,ENABLE:使能接收,DISABLE:关闭接收
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2013316
* 最后修改时间 : 2013316
* 说明	: 	USART1~UART5,对应通道UART_CH1-UART_CH5
*************************************************************************************************************************/
void UARTx_EnableRx(UART_CH_Type ch,FunctionalState Enable)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   //获取对应通道硬件基址指针

	if(Enable)
		UARTx->CR1 |= 0x04;									//RE = 1;接收使能
	else
		UARTx->CR1 &= ~0x04;								//RE = 0;接收关闭
}






/*************************************************************************************************************************
* 函数	:	void UARTx_SendByte(UART_CH_Type ch,u8 data)
* 功能	:	UART单字节发送
* 参数	:	ch:通道号,dataL:要发送的数据
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 2012-04-29
* 说明	: 	USART1~UART5,对应通道UART_CH1-UART_CH5
			将等待发送完成注释掉了,可以稍微提高发送速度
*************************************************************************************************************************/
void UARTx_SendByte(UART_CH_Type ch,u8 data)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   	//获取对应通道硬件基址指针
	if(ch > UART_ChMax - 1)									//判断端口是否超出范围
		return;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//串口4,5不支持DMA
	{
		while(!(UARTx->SR & 0x80));													//等待发送寄存器为空,(否则连续发送时数据易丢失 )
		UARTx->DR = data;															//发送数据
#if !UART_TX_TO_FIFI	
		while(!(UARTx->SR & 0x40));													//等待TC = 1;也就是发送完成,数据从发送FIFO发送出去了才认为发送已经完成
		UARTx->SR &= ~(1 << 6);														//清除发送完成标志
#endif	//!UART_TX_TO_FIFI
	}
	else
	{
		DMA_StartChannel((DMA_Channel_TypeDef *)USARTxChannel[ch],(u32)&data, 1);	//申请DMA发送
		DMA_WaitComplete((DMA_Channel_TypeDef *)USARTxChannel[ch]);					//等待传输完成
	}
#else	
	while(!(UARTx->SR & 0x80));														//等待发送寄存器为空,(否则连续发送时数据易丢失 )
 	UARTx->DR = data;																//发送数据
#if !UART_TX_TO_FIFI	
	while(!(UARTx->SR & 0x40));														//等待TC = 1;也就是发送完成,数据从发送FIFO发送出去了才认为发送已经完成
	UARTx->SR &= ~(1 << 6);															//清除发送完成标志
#endif	//!UART_TX_TO_FIFI
#endif //UART_DMA_EN
}



/*************************************************************************************************************************
* 函数	:	void UARTx_SendData(UART_CH_Type ch,u8 *tx_buff,u16 byte_number)
* 功能	:	UART数据发送函数
* 参数	:	ch:通道号,tx_buff:发送缓冲区,byte_number:需要发送的字节
* 返回	:	无
* 依赖	:	void UART_SendByte(u8 ch,u8 data)
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20120403
* 说明	: 	非DMA方式,非FIFO方式发送
*************************************************************************************************************************/
void UARTx_SendData(UART_CH_Type ch,u8 *pTxBuff,u16 DataLen)
{
	u16 i;
	if(ch > UART_ChMax - 1)						//判断端口是否超出范围
		return;
	
#if UART_DMA_EN
	
	if(ch > UART_CH3)	//串口4,5不支持DMA
	{
		for(i = 0;i < DataLen;i++)				//循环发送,直至发送完毕
		{
			UARTx_SendByte(ch,pTxBuff[i]);
		}
	}
	else
	{
		DMA_WaitComplete((DMA_Channel_TypeDef *)USARTxChannel[ch]);
		DMA_StartChannel((DMA_Channel_TypeDef *)USARTxChannel[ch],(u32) pTxBuff, DataLen);
	}
#else	
	for(i = 0;i < DataLen;i++)				//循环发送,直至发送完毕
	{
	 	UARTx_SendByte(ch,pTxBuff[i]);
	}
#endif //UART_DMA_EN
}




/*************************************************************************************************************************
* 函数	:	void UARTx_SendString(UART_CH_Type ch,char *pString)
* 功能	:	UART发送字符串
* 参数	:	ch:通道号
			pString:字符串指针
* 返回	:	无
* 依赖	:	void UART_SendByte(u8 ch,u8 data)
* 作者	:	cp1300@139.com
* 时间	:	2013-04-18
* 最后修改时间 : 2013-04-18
* 说明	: 	非DMA方式,非FIFO方式发送
*************************************************************************************************************************/
#include "string.h"
void UARTx_SendString(UART_CH_Type ch,char *pString)
{	
	if(ch > UART_ChMax - 1)						//判断端口是否超出范围
		return;
	
	/*while(*pString != '\0')
	{
		UARTx_SendByte(ch, *pString ++);
	}*/
	UARTx_SendData(ch, (u8 *)pString, strlen(pString));
}





/*************************************************************************************************************************
* 函数	:	bool UARTx_GetNewDataFlag(UART_CH_Type ch)
* 功能	:	获取串口新数据标志
* 参数	:	ch:通道选择
* 返回	:	TRUE:成功,FALSE:失败
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20120403
* 说明	: 	用于判断是否有新的数据,会清除掉新数据标志的
*************************************************************************************************************************/
bool UARTx_GetNewDataFlag(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)										//判断端口是否超出范围
		return FALSE;

	if(UartRx[ch].IntRx == SET)									//开启了中断接收
	{
		if(UartRx[ch].NewDataFlag == SET) 						//有新数据
		{
		 	UartRx[ch].NewDataFlag = RESET;						//清除标志
			return TRUE;										//返回有新数据
		}
	}
	else														//没开启中断接收
	{
	 	if(((USART_TypeDef *)USARTxN[ch])->SR & BIT5)			//RXNE=1,接收到新数据
		{
			((USART_TypeDef *)USARTxN[ch])->SR &= ~BIT5;		//清除标志
			return TRUE;
		}
	}
	return FALSE;
}


/*************************************************************************************************************************
* 函数	:	bool UARTx_GetRxBuffFullFlag(UART_CH_Type ch)
* 功能	:	获取串口接收缓冲区满标志
* 参数	:	ch:通道选择
* 返回	:	TRUE:成功,FALSE:失败
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20120403
* 说明	: 	用于判断接收缓冲区是否满,会清除标志
*************************************************************************************************************************/
bool UARTx_GetRxBuffFullFlag(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)					//判断端口是否超出范围
		return FALSE;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//串口4,5不支持DMA
	{
		if(UartRx[ch].BuffFull == SET)			//缓冲区已满
		{
			UartRx[ch].BuffFull = RESET;			//清除满标志
			return TRUE;
		}
		return FALSE;
		}
	else
	{
		if(DMA_GetCompleteResidualCnt((DMA_Channel_TypeDef *)USARRxChannel[ch]) == 0)
		{
			if(((USART_TypeDef *)USARTxN[ch])->SR & BIT3)	//清除过载标志
			{
				(USART_TypeDef *)USARTxN[ch]->DR;
			}
			return TRUE;
		}	
		else
			return FALSE;
	}
	
#else 
	if(UartRx[ch].BuffFull == SET)			//缓冲区已满
	{
	 	UartRx[ch].BuffFull = RESET;			//清除满标志
		return TRUE;
	}
	return FALSE;
#endif //UART_DMA_EN
}


/*************************************************************************************************************************
* 函数	:	void UART_ClearRxInt(UART_CH_Type ch)
* 功能	:	清除串口接收中断标志
* 参数	:	ch:通道选择
* 返回	:	物
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20120403
* 说明	: 	用于清除接收标志
*************************************************************************************************************************/
void UARTx_ClearRxInt(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)								//判断端口是否超出范围
		return;
	((USART_TypeDef *)USARTxN[ch])->SR &= ~BIT5;		//清除标志
}


/*************************************************************************************************************************
* 函数	:	u8 UARTx_GetNewData(UART_CH_Type ch)
* 功能	:	获取串口新数据
* 参数	:	ch:通道选择
* 返回	:	收到的数据
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20120403
* 说明	: 	用于接收一个字节数据
*************************************************************************************************************************/
u8 UARTx_GetNewData(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)								//判断端口是否超出范围
		return 0;

	return (((USART_TypeDef *)USARTxN[ch])->DR);	//返回数据
}



/*************************************************************************************************************************
* 函数	:	void UARTx_SetRxBuff(UART_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
* 功能	:	设置串口接收缓冲区
* 参数	:	ch:通道选择,RxBuffSize:缓冲区大小,RxBuff:缓冲区指针
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20120403
* 最后修改时间 : 20120403
* 说明	: 	一定要设置,否则开启中断接收时可能会异常
*************************************************************************************************************************/
void UARTx_SetRxBuff(UART_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
{
#ifdef _UCOS_II_
	OS_CPU_SR  cpu_sr;
#endif	//_UCOS_II_
	
	if(ch > UART_ChMax - 1)						//判断端口是否超出范围
		return;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//串口4,5不支持DMA
	{
		#ifdef _UCOS_II_
		OS_ENTER_CRITICAL();
		#endif	//_UCOS_II_
			UartRx[ch].RxBuffSize = RxBuffSize; 		//设置缓冲区大小
			UartRx[ch].RxBuff = RxBuff;					//设置缓冲区指针
			UartRx[ch].UartRxCnt = 0;					//计数器清零
		#ifdef _UCOS_II_
			OS_EXIT_CRITICAL();
		#endif	//_UCOS_II_
	}
	else
	{
		DMA_PeripheralToMemory((DMA_Channel_TypeDef *)USARRxChannel[ch], (u32)RxBuff, (u32)&((USART_TypeDef *)USARTxN[ch])->DR, DMA_SIZE_8BIT, RxBuffSize);//外设到存储器的DMA传输配置
	}
#endif //UART_DMA_EN
	
#ifdef _UCOS_II_
	OS_ENTER_CRITICAL();
#endif	//_UCOS_II_
	UartRx[ch].RxBuffSize = RxBuffSize; 		//设置缓冲区大小
	UartRx[ch].RxBuff = RxBuff;					//设置缓冲区指针
#if !UART_DMA_EN		
	UartRx[ch].UartRxCnt = 0;					//计数器清零
#endif //!UART_DMA_EN
#ifdef _UCOS_II_
	OS_EXIT_CRITICAL();
#endif	//_UCOS_II_
}





/*************************************************************************************************************************
* 函数	:	u32 UARTx_GetRxCnt(UART_CH_Type ch)
* 功能	:	获取串口接收数据计数器
* 参数	:	ch:通道选择
* 返回	:	接收到的数据数量
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20130307
* 最后修改时间 : 20130307
* 说明	: 	无
*************************************************************************************************************************/
u32 UARTx_GetRxCnt(UART_CH_Type ch)
{	
	if(ch > UART_ChMax - 1)						//判断端口是否超出范围
		return 0;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//串口4,5不支持DMA
	{
		return UartRx[ch].UartRxCnt;			//返回计数值
	}
	else
	{
		return  UartRx[ch].RxBuffSize - DMA_GetCompleteResidualCnt((DMA_Channel_TypeDef *)USARRxChannel[ch]);
	}
#else
	return UartRx[ch].UartRxCnt;			//返回计数值	
#endif //UART_DMA_EN	
}




/*************************************************************************************************************************
* 函数	:	void UARTx_ClearRxCnt(UART_CH_Type ch)
* 功能	:	清除串口接收数据计数器
* 参数	:	ch:通道选择
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20130307
* 最后修改时间 : 20130307
* 说明	: 	无
*************************************************************************************************************************/
void UARTx_ClearRxCnt(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)					//判断端口是否超出范围
		return;
#if UART_DMA_EN
	if(ch > UART_CH3)	//串口4,5不支持DMA
	{
		UartRx[ch].UartRxCnt = 0;				//计数器清零
	}
	else
	{
		DMA_SetPeripheralToMemoryDataSize((DMA_Channel_TypeDef *)USARRxChannel[ch], (u32)UartRx[ch].RxBuff, UartRx[ch].RxBuffSize);//外设到存储器的DMA传输数据量设置
		if(((USART_TypeDef *)USARTxN[ch])->SR & BIT3)	//清除过载标志
		{
			(USART_TypeDef *)USARTxN[ch]->DR;
		}
	}
#else
	UartRx[ch].UartRxCnt = 0;				//计数器清零
#endif //UART_DMA_EN
}






#if !UART_DMA_EN
/*************************************************************************************************************************
* 函数	:	void USART1_IRQHandler (void)
* 功能	:	UART1中断接收函数
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110611
* 最后修改时间 : 20120403
* 说明	: 	无
*************************************************************************************************************************/
void USART1_IRQHandler (void)
{
	if(UartRx[UART_CH1].RxBuffSize > 0 && UartRx[UART_CH1].UartRxCnt < UartRx[UART_CH1].RxBuffSize)												//接收缓冲区大于0
	{
	 	(UartRx[UART_CH1].RxBuff)[(UartRx[UART_CH1].UartRxCnt) ++] = USART1->DR; 	//将数据存放到缓冲区
		if(UartRx[UART_CH1].UartRxCnt == UartRx[UART_CH1].RxBuffSize) 				//缓冲区已满
		{
			 // UartRx[UART_CH1].UartRxCnt = 0;										//接收计数器清零
			  UartRx[UART_CH1].BuffFull = SET;										//缓冲区已满标志
		}	
	}
	else //缓冲区满了，清除接收到的数据
	{
		UartRx[UART_CH1].TempData = USART1->DR;
	}
	
	UartRx[UART_CH1].NewDataFlag = SET;												//收到新数据标志
	UARTx_ClearRxInt(UART_CH1);		   												//清除串口接收中断标志
}

#if UART_ChMax > 1
/*************************************************************************************************************************
* 函数	:	void USART2_IRQHandler (void)
* 功能	:	UART2中断接收函数
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110611
* 最后修改时间 : 20120403
* 说明	: 	无
*************************************************************************************************************************/
void USART2_IRQHandler (void)
{
	if(UartRx[UART_CH2].RxBuffSize > 0 && UartRx[UART_CH2].UartRxCnt < UartRx[UART_CH2].RxBuffSize)												//接收缓冲区大于0
	{
	 	(UartRx[UART_CH2].RxBuff)[(UartRx[UART_CH2].UartRxCnt) ++] = USART2->DR; 	//将数据存放到缓冲区
		if(UartRx[UART_CH2].UartRxCnt == UartRx[UART_CH2].RxBuffSize) 				//缓冲区已满
		{
			// UartRx[UART_CH2].UartRxCnt = 0;										//接收计数器清零
			  UartRx[UART_CH2].BuffFull = SET;										//缓冲区已满标志
		}	
	}
	else //缓冲区满了，清除接收到的数据
	{
		UartRx[UART_CH2].TempData = USART2->DR;
	}
	UartRx[UART_CH2].NewDataFlag = SET;												//收到新数据标志
	UARTx_ClearRxInt(UART_CH2);		   												//清除串口接收中断标志
}
#endif


#if UART_ChMax > 2
/*************************************************************************************************************************
* 函数	:	void USART3_IRQHandler (void)
* 功能	:	UART3中断接收函数
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110611
* 最后修改时间 : 20120403
* 说明	: 	无
*************************************************************************************************************************/
void USART3_IRQHandler (void)
{
	if(UartRx[UART_CH3].RxBuffSize > 0  && UartRx[UART_CH3].UartRxCnt < UartRx[UART_CH3].RxBuffSize)												//接收缓冲区大于0
	{
	 	(UartRx[UART_CH3].RxBuff)[(UartRx[UART_CH3].UartRxCnt) ++] = USART3->DR; 	//将数据存放到缓冲区
		if(UartRx[UART_CH3].UartRxCnt == UartRx[UART_CH3].RxBuffSize) 				//缓冲区已满
		{
			  //UartRx[UART_CH3].UartRxCnt = 0;										//接收计数器清零
			  UartRx[UART_CH3].BuffFull = SET;										//缓冲区已满标志
		}	
	}
	else //缓冲区满了，清除接收到的数据
	{
		UartRx[UART_CH3].TempData = USART3->DR;
	}
	UartRx[UART_CH3].NewDataFlag = SET;												//收到新数据标志
	UARTx_ClearRxInt(UART_CH3);		   												//清除串口接收中断标志
}
#endif
#endif //!UART_DMA_EN

#if UART_ChMax > 3
/*************************************************************************************************************************
* 函数	:	void UART4_IRQHandler (void)
* 功能	:	UART4中断接收函数
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110611
* 最后修改时间 : 20120403
* 说明	: 	无
*************************************************************************************************************************/
void UART4_IRQHandler (void)
{
	if(UartRx[UART_CH4].RxBuffSize > 0 && UartRx[UART_CH4].UartRxCnt < UartRx[UART_CH4].RxBuffSize)												//接收缓冲区大于0
	{
	 	(UartRx[UART_CH4].RxBuff)[(UartRx[UART_CH4].UartRxCnt) ++] = UART4->DR; 	//将数据存放到缓冲区
		if(UartRx[UART_CH4].UartRxCnt == UartRx[UART_CH4].RxBuffSize) 				//缓冲区已满
		{
			  //UartRx[UART_CH4].UartRxCnt = 0;										//接收计数器清零
			  UartRx[UART_CH4].BuffFull = SET;										//缓冲区已满标志
		}	
	}
	else //缓冲区满了，清除接收到的数据
	{
		UartRx[UART_CH4].TempData = UART4->DR;
	}
	UartRx[UART_CH4].NewDataFlag = SET;												//收到新数据标志
	UARTx_ClearRxInt(UART_CH4);		   												//清除串口接收中断标志
}
#endif



#if UART_ChMax > 4
/*************************************************************************************************************************
* 函数	:	void UART5_IRQHandler (void)
* 功能	:	UART5中断接收函数
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20110611
* 最后修改时间 : 20120403
* 说明	: 	无
*************************************************************************************************************************/
void UART5_IRQHandler (void)
{
	if(UartRx[UART_CH5].RxBuffSize > 0 && UartRx[UART_CH5].UartRxCnt < UartRx[UART_CH5].RxBuffSize)												//接收缓冲区大于0
	{
	 	(UartRx[UART_CH5].RxBuff)[(UartRx[UART_CH5].UartRxCnt) ++] = UART5->DR; 	//将数据存放到缓冲区
		if(UartRx[UART_CH5].UartRxCnt == UartRx[UART_CH5].RxBuffSize) 				//缓冲区已满
		{
			 // UartRx[UART_CH5].UartRxCnt = 0;										//接收计数器清零
			  UartRx[UART_CH5].BuffFull = SET;										//缓冲区已满标志
		}	
	}
	else //缓冲区满了，清除接收到的数据
	{
		UartRx[UART_CH5].TempData = UART5->DR;
	}
	UartRx[UART_CH5].NewDataFlag = SET;												//收到新数据标志
	UARTx_ClearRxInt(UART_CH5);		   												//清除串口接收中断标志
}
#endif






//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 0
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
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	UARTx_SendByte(0,(u8)ch);      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////






#undef UART_ChMax
