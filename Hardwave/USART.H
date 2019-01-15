/*************************************************************************************************************
 * 文件名:		usart.h
 * 功能:		STM32 USART驱动
 * 作者:		cp1300@139.com
 * 创建时间:	2011年6月11日
 * 最后修改时间:2013年5月29日
 * 详细:		已经修复已知bug
				2013-11-17:增加DMA收发模式
				2014-08-19:增加DMA,中断混合模式，主要是由于串口4，串口5不支持DMA
*************************************************************************************************************/	

#ifndef _USART_H
#define _USART_H

#include "system.h"
#include "stdio.h"


/***********************配置相关************************/
#define UART_DMA_EN		1			//1:使能DAM收发模式;0:关闭DMA收发模式
#define UART_TX_TO_FIFI	1			//1:数据发送到发送FIFO则认为发送完成;0:数据从FIFO发送完成则认为发送完成
#define UART_ChMax		5			//串口通道数量


/*********************************************************/



//UART配置相关结构定义
typedef struct
{
	u8 OddEvenVerify;	//奇偶校验,奇,偶,无
	u8 StopBitWidth;	//停止位位宽1,2
} UART_Config_TypeDef;


//奇偶校验
#define UART_VERIFY_NULL	0	//无校验
#define UART_ODD			1	//奇校验
#define UART_EVEN			2	//偶校验
//停止位
#define UART_STOP_1BIT		0	//一个停止位
#define UART_STOP_2BIT		1	//2个停止位



//串口选择,串口1开始,到串口5
typedef enum
{
	UART_CH1	=		0,	//UART1
	UART_CH2	=		1,	//UART2
	UART_CH3	=		2,	//UART3
	UART_CH4	=		3,	//UART4
	UART_CH5	=		4,	//UART5
}UART_CH_Type;



//相关API
bool 	UARTx_Init(UART_CH_Type ch,u32 Speed,u8 RX_Int);				//串口初始化
bool 	UARTx_Config(UART_CH_Type ch,UART_Config_TypeDef * cfg);		//串口配置
void 	UARTx_SendByte(UART_CH_Type ch,u8 data);						//UART单字节发送
void 	UARTx_SendData(UART_CH_Type ch,u8 *tx_buff,u16 byte_number);	//UART数据发送函数
void 	UARTx_SendString(UART_CH_Type ch,char *pString);				//UART发送字符串
void 	UARTx_PowerDown(UART_CH_Type ch);								//UART掉电
void 	UARTx_PowerUp(UART_CH_Type ch);									//UART上电
bool 	UARTx_GetNewDataFlag(UART_CH_Type ch);							//获取串口新数据标志
bool 	UARTx_GetRxBuffFullFlag(UART_CH_Type ch);						//获取串口接收缓冲区满标志
u8 		UARTx_GetNewData(UART_CH_Type ch);								//获取串口新数据
void 	UARTx_SetRxBuff(UART_CH_Type ch,u8 *RxBuff,u16 RxBuffSize);		//设置串口接收缓冲区
void 	UARTx_ClearRxInt(UART_CH_Type ch);								//清除串口接收中断标志
u32 	UARTx_GetRxCnt(UART_CH_Type ch);								//获取串口接收数据计数器
void 	UARTx_ClearRxCnt(UART_CH_Type ch);								//清除串口接收数据计数器
void 	UARTx_EnableRx(UART_CH_Type ch,FunctionalState Enable);			//串口接收使能
void 	UARTx_SetBaudRate(UART_CH_Type ch,u32 baud);					//计算并设置串口波特率


#endif
