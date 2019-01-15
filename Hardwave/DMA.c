#include "dma.h"
#include "SYSTEM.H" 
#include "delay.h"

//DMA����,��Ҫ���ڴ洢�����洢�������ݴ���,�罫���������Դ�,���Դ������͵���ʾ��
//cp1300@139.com
//20111205 

	
/*************************************************************************************************************************
* ����	:	void DMA_MemoryToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DATA_SIZE)
* ����	:	�洢�����洢����DMA��������,���ڸ����Դ�����Դ�
* ����	:	DMA1ͨ��ѡ��(DMA1_Channelx),�洢����ַ,����洢����ַ,����λ��,�������ݸ���
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111205
* ����޸�ʱ�� : 20111205
* ˵��	: 	��
*************************************************************************************************************************/	  
void DMA_MemoryToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DATA_SIZE)
{
	DeviceClockEnable(DEV_DMA1,1);//ʹ��DMA1ʱ��
	DMA_CHx->CCR = 0;//ʧ��DMA1��ȥ��֮ǰ������
	DMA_CHx->CMAR = MemoryADD;//�洢����ַ
	DMA_CHx->CPAR = ParADD;//����(�洢��)��ַ
	DMA_CHx->CNDTR = DATA_SIZE;//��������ݸ���
	DMA_CHx->CCR |= BIT14;//ʹ�ܴ洢�����洢��ģʽ
	DMA_CHx->CCR |= BIT12;//ͨ�����ȼ��е�
	DMA_CHx->CCR |= SIZE_xbit << 10;//�洢�����ݴ�С
	DMA_CHx->CCR |= SIZE_xbit << 8;//�������ݴ�С(λ��)
	DMA_CHx->CCR |= BIT7;//ʹ�ܴ洢����ַ����ģʽ
	DMA_CHx->CCR |= BIT6;//ʹ������(�洢��)��ַ����ģʽ
	DMA_CHx->CCR |= BIT4;//���䷽��:�Ӵ洢��������
	DMA_CHx->CCR |=	BIT0;//��ʼ����
}


/*************************************************************************************************************************
* ����	:			void DMA_MemoryToPeripheralConfig(DMA_Channel_TypeDef *DMA_CHx,u32 ParADD,u8 SIZE_xbit)
* ����	:			�洢���������DMA��������
* ����	:			DMA1ͨ��ѡ��(DMA1_Channelx),�洢����ַ,�����ַ,����λ��
* ����	:			��
* ����	:			�ײ�궨��
* ����	:			cp1300@139.com
* ʱ��	:			2011-12-05
* ����޸�ʱ�� : 	2013-11-17
* ˵��	: 			��Ҫʹ������DMA����
					��Ҫ��������
*************************************************************************************************************************/  
void DMA_MemoryToPeripheralConfig(DMA_Channel_TypeDef *DMA_CHx,u32 ParADD,u8 SIZE_xbit)
{
	DeviceClockEnable(DEV_DMA1,1);//ʹ��DMA1ʱ��
	DMA_CHx->CCR = 0;//ʧ��DMA1��ȥ��֮ǰ������
	DMA_CHx->CPAR = ParADD;//����(�洢��)��ַ
//	DMA_CHx->CCR |= BIT14;//ʹ�ܴ洢�����洢��ģʽ
	DMA_CHx->CCR |= BIT12;//ͨ�����ȼ��е�
	DMA_CHx->CCR |= SIZE_xbit << 10;//�洢�����ݴ�С
	DMA_CHx->CCR |= SIZE_xbit << 8;//�������ݴ�С(λ��)
	DMA_CHx->CCR |= BIT7;//ʹ�ܴ洢����ַ����ģʽ
	//DMA_CHx->CCR |= BIT6;//ʹ������(�洢��)��ַ����ģʽ
	DMA_CHx->CCR |= BIT4;//���䷽��:�Ӵ洢��������
} 



/*************************************************************************************************************************
* ����	:			void DMA_StartChannel(DMA_Channel_TypeDef *DMA_CHx,u16 DataSize)
* ����	:			����DMA	����
* ����	:			DMA1ͨ��ѡ��(DMA1_Channelx),MemoryADD:���ͻ�������ַ,DataSize:�������ݴ�С
* ����	:			��
* ����	:			�ײ�궨��
* ����	:			cp1300@139.com
* ʱ��	:			2011-12-05
* ����޸�ʱ�� : 	2013-11-17
* ˵��	: 			��Ҫʹ������DMA����
					��Ҫ��������
					��Ҫ��鷢���Ƿ����
*************************************************************************************************************************/  
void DMA_StartChannel(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize)
{
	DMA_CHx->CCR &=	~BIT0;			//ֹͣ����
	DMA_CHx->CMAR = MemoryADD;		//�洢����ַ
	DMA_CHx->CNDTR = DataSize;		//��������ݸ���
	DMA_CHx->CCR |=	BIT0;			//��ʼ����
}


/*************************************************************************************************************************
* ����	:	void DMA_PeripheralToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DATA_SIZE)
* ����	:	���赽�洢����DMA��������
* ����	:	DMA1ͨ��ѡ��(DMA1_Channelx),�洢����ַ,�����ַ,����λ��,�������ݸ���
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120604
* ����޸�ʱ�� : 20120604
* ˵��	: 	��Ҫʹ������DMA����
*************************************************************************************************************************/  
void DMA_PeripheralToMemory(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD,u32 ParADD,u8 SIZE_xbit,u16 DataSize)
{
	DeviceClockEnable(DEV_DMA1,1);//ʹ��DMA1ʱ��
	DMA_CHx->CCR = 0;//ʧ��DMA1��ȥ��֮ǰ������
	DMA_CHx->CMAR = MemoryADD;//�洢����ַ
	DMA_CHx->CPAR = ParADD;//����(�洢��)��ַ
	DMA_CHx->CNDTR = DataSize;//��������ݸ���
//	DMA_CHx->CCR |= BIT14;//ʹ�ܴ洢�����洢��ģʽ
	DMA_CHx->CCR |= BIT12;//ͨ�����ȼ��е�
	DMA_CHx->CCR |= SIZE_xbit << 10;//�洢�����ݴ�С
	DMA_CHx->CCR |= SIZE_xbit << 8;//�������ݴ�С(λ��)
	DMA_CHx->CCR |= BIT7;//ʹ�ܴ洢����ַ����ģʽ
	//DMA_CHx->CCR |= BIT6;//ʹ������(�洢��)��ַ����ģʽ
	//���赽�洢��
//	DMA_CHx->CCR |= BIT4;//���䷽��:�Ӵ洢��������
	DMA_CHx->CCR |=	BIT0;//��ʼ����
} 


/*************************************************************************************************************************
* ����	:	void DMA_SetPeripheralToMemoryDataSize(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize)
* ����	:	���赽�洢����DMA��������������
* ����	:	DMA1ͨ��ѡ��(DMA1_Channelx),�洢����ַ,�洢����ַ,�������ݸ���
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120604
* ����޸�ʱ�� : 20120604
* ˵��	: 	��Ҫʹ������DMA����
*************************************************************************************************************************/  
void DMA_SetPeripheralToMemoryDataSize(DMA_Channel_TypeDef *DMA_CHx,u32 MemoryADD, u16 DataSize)
{
	DMA_CHx->CCR &=	~BIT0;			//ֹͣ����
	DMA_CHx->CMAR = MemoryADD;		//�洢����ַ
	DMA_CHx->CNDTR = DataSize;		//��������ݸ���
	DMA_CHx->CCR |=	BIT0;			//��ʼ����
}

/*************************************************************************************************************************
* ����	:	u16 DMA_GetCompleteResidualCnt(DMA_Channel_TypeDef *DMA_CHx)
* ����	:	��ȡ�����ʣ��������
* ����	:	DMA1ͨ��ѡ��(DMA1_Channelx)
* ����	:	ʣ�ഫ�����ݸ���
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-11-17
* ����޸�ʱ�� : 2013-11-17
* ˵��	: 	��
*************************************************************************************************************************/  
u16 DMA_GetCompleteResidualCnt(DMA_Channel_TypeDef *DMA_CHx)	
{
	return (u16)(DMA_CHx->CNDTR);						//��ȡ�����ʣ��������
}


/*************************************************************************************************************************
* ����	:	void Wait_DMA_Complete(DMA_Channel_TypeDef *DMA_CHx)
* ����	:	�ȴ�DMA�������
* ����	:	DMA1ͨ��ѡ��(DMA1_Channelx)
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111205
* ����޸�ʱ�� : 20111205
* ˵��	: 	ͨ���ж�ʣ������������ж�DMA�Ƿ����
*************************************************************************************************************************/	  
void DMA_WaitComplete(DMA_Channel_TypeDef *DMA_CHx)
{
	u32 i = 0xfffff;
	
	while((DMA_CHx->CNDTR)&&i)	 //����û�з������
	{
		i --;
	}
}






















