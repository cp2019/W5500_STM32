#ifndef __DMA_H
#define	__DMA_H	   
#include "system.h"



//传输数据位宽设置
#define DMA_SIZE_8BIT	0
#define DMA_SIZE_16BIT	1
#define DMA_SIZE_32BIT	2



void DMA_MemoryToPeripheralConfig(DMA_Channel_TypeDef *DMA_CHx,u32 ParADD,u8 SIZE_xbit);			//存储器到外设的DMA传输配置
void DMA_StartChannel(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize);					//启动DMA传输
void DMA_WaitComplete(DMA_Channel_TypeDef *DMA_CHx);												//等待传输完成
void DMA_PeripheralToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DataSize);//外设到存储器的DMA传输配置
void DMA_SetPeripheralToMemoryDataSize(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize);	//外设到存储器的DMA传输数据量设置
u16 DMA_GetCompleteResidualCnt(DMA_Channel_TypeDef *DMA_CHx);										//获取传输的剩余数据量
void DMA_SetPeripheralToMemoryDataSize(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize);	//外设到存储器的DMA传输数据量设置   


#endif






























