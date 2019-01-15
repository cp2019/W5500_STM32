/*	SPI驱动函数		*/
//20110625
//修改时间 20110625


#ifndef __SPI_H_
#define __SPI_H_
#include "system.h"


//SPI配置结构体
typedef struct
{
  u8	En16bit;	//使能16bit格式，否则为8bit模式
  u8	EnLSB;		//使能低位在前，否则高位在前
  u8	EnCPOH;		//使能空闲时钟为高电平，否则为低电平
  u8	EnCPHA;		//使能数据采样从第二个边沿开始
  u8	EnSSM;		//使能软件控制CS信号
  u8	EnRxOnly;	//禁止输出,只接收,否则为全双工模式
  u8	EnBIDI;		//使能单线双向模式
  u8	EnCRC;		//使能CRC校验
  u8	EnMSTR;		//使能主设备模式
  u8	EnTxINT;	//开启发送中断
  u8	EnRxINT;	//开启接收中断
  u8	EnErrorINT;	//开启错误中断
  u8	EnSSOE;		//使能主模式下SS输出
  u8	EnTxDMA;	//使能发送DMA
  u8	EnRxDMA;	//使能接收DMA
} SPI_Config_Type;




//SPI通道配置
typedef enum
{
	SPI_CH1		=	0,	//SPI1
	SPI_CH2		=	1,	//SPI2
	SPI_CH3		=	2,	//SPI3
} SPI_CH_Type;




//SPI通道配置
typedef enum
{
	SPI_1_2  	=			0,   		//SPI 时钟2分频,(只适用于SPI1)
	SPI_1_4    	=			1,   		//SPI 时钟4分频
	SPI_1_8   	=			2, 			//SPI 时钟8分频
	SPI_1_16  	=			3, 			//SPI 时钟16分频
	SPI_1_32   	=			4,   		//SPI 时钟32分频
	SPI_1_64  	=			5,   		//SPI 时钟64分频
	SPI_1_128  	=			6,  		//SPI 时钟128分频
	SPI_1_256 	=			7,			//SPI 时钟256分频
	SPI_1_512 	=			8,			//SPI 时钟512分频,(只适用于SPI2,3)
} SPI_SPEED_Type;





extern const SPI_Config_Type SPI_DEFAULT_01;	//常用模式1

extern const SPI_Config_Type SPI_DEFAULT_02;	//常用模式2

//相关API
u8 SPIx_Init(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config,SPI_SPEED_Type Speed);//SPI初始化函数
u32 SPIx_SetSpeed(SPI_CH_Type ch,SPI_SPEED_Type Speed);							//SPI速度设置
u8 SPIx_ReadWriteByte(SPI_CH_Type ch,u8 TxData);						//SPI 读写一个字节
u8 SPIx_Config(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config);		//SPI配置

bool SPIx_GetNewDataFlag(SPI_CH_Type ch);//获取SPI新数据标志
bool SPIx_GetRxBuffFullFlag(SPI_CH_Type ch);//获取SPI接收缓冲区满标志
u8 SPIx_GetNewData(SPI_CH_Type ch);//获取SPI新数据
void SPIx_SetRxBuff(SPI_CH_Type ch,u8 *RxBuff,u16 RxBuffSize);//设置SPI接收缓冲区
u32 SPIx_GetRxCnt(SPI_CH_Type ch);//获取SPI接收数据计数器
void SPIx_ClearRxCnt(SPI_CH_Type ch);//清除SPI接收数据计数器
	

//spi掉电
void SPIx_PowerDown(SPI_CH_Type ch);
//spi上电
void SPIx_PowerUp(SPI_CH_Type ch);
#endif
