/*******************************************************************************
//�ļ���:	SPI.c
//����:		STM32Ӳ��SPI
//����:cp1300@139.com
//����ʱ��:2011-06-25 
//�޸�ʱ��:2012-09-29
//�޶�˵��:20120929���������ú���������ʹ��ʱ�ӻ�ȡ������ȡϵͳʱ��
	2014-08-07�����Ӵӻ�����ģʽ
//����:	 
********************************************************************************/

#include "SPI.H"
#include "SYSTEM.H"
#include "GPIO_INIT.H"


//����SPIͨ�������ֵ
#define SPI_ChMax	2

//SPI����ṹָ��
static const  SPI_TypeDef *SPIxN[3] = {SPI1,SPI2,SPI3};
//�����ʷ�Ƶ��
//512��Ƶ����SPI2,SPI3��,��ΪSPI2,3��ʱ����ϵͳʱ�ӵ�һ��,�൱���Ѿ�������2��Ƶ��,Ҳ����˵SPI2,3���ٽ���4��Ƶ 
static const u16 BAUD_RATE[9] = {2,4,8,16,32,64,128,256,512};


//����ģʽ1
//�������ڴ洢��,VS1003B��SPI�豸
//ȫ˫��ģʽ,SPI����,8������λ,����ʱ�Ӹߵ�ƽ,��λ��ǰ,�������SS,�������ж�,������DMA
const SPI_Config_Type SPI_DEFAULT_01 = {0,0,0,0,1,0,0,0,1,0,0,0,0,0,0};	



//��ģʽ
//ȫ˫��ģʽ,SPI����,8������λ,����ʱ�Ӹߵ�ƽ,��λ��ǰ,����SS,�����ж�,������DMA
const SPI_Config_Type SPI_DEFAULT_02 = {0,0,0,0,1,1,0,0,0,0,1,0,0,0,0};	



//���UART״̬�ṹ
typedef struct
{
	FlagStatus	NewDataFlag;//���յ�������
	FlagStatus	BuffFull;	//����Buff��
	FlagStatus	IntRx;		//�Ƿ����жϽ���
	u8 			*RxBuff;	//����Buffָ��
	u16			RxBuffSize;	//���ջ�������С,һ֡���ݴ�С
	u16 		RxCnt;		//�������ݼ�����
} SPIRx_TypeDef;


//SPI1	����״̬�ṹ
static SPIRx_TypeDef SPIRx[SPI_ChMax + 1];




/*************************************************************************************************************************
* ����	:	u8 SPIx_ReadWriteByte(SPI_CH_Type ch,u8 TxData)
* ����	:	SPI ��дһ���ֽ�
* ����	:	ch:ͨ��ѡ��;TxData:Ҫд����ֽ�
* ����	:	��ȡ�����ֽ�
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110625
* ����޸�ʱ�� : 20120404
* ˵��	: 	��
*************************************************************************************************************************/
u8 SPIx_ReadWriteByte(SPI_CH_Type ch,u8 TxData)
{		
	u8 retry=0;
	SPI_TypeDef *SPIx;

	if(ch > SPI_ChMax) return 0;			//ͨ���ų�����Χ,���ش���0
	SPIx = (SPI_TypeDef *)SPIxN[ch];		//��ȡSPI�ṹָ��				 
	while((SPIx->SR & 1 << 1)== 0)			//�ȴ���������	
	{
		retry ++;
		if(retry > 200) 					//�ȴ���ʱ
			return 0;
	}			  
	SPIx->DR = TxData;	 	  				//����һ��byte 
	retry = 0;
	while((SPIx->SR & 1 << 0) == 0) 		//�ȴ�������һ��byte  
	{
		retry ++;
		if(retry > 200) 					//�ȴ���ʱ
			return 0;
	}	  						    
	return SPIx->DR;  						//�����յ�������				    
}


/*************************************************************************************************************************
* ����	:	u32 SPIx_SetSpeed(SPI_CH_Type ch,SPI_SPEED_Type Speed)
* ����	:	SPI�ٶ�����
* ����	:	ch:ͨ��ѡ��,SYS_CLK:��ǰϵͳʱ��,BaudRate:��Ƶϵ��,2,4,8,16,32,64,128,256
* ����	:	0:����ʧ��,����:��ǰSPI������
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110625
* ����޸�ʱ�� : 20120404
* ˵��	: 	SPI2,SPI3����ٶ���SPI1��һ��
*************************************************************************************************************************/
u32 SPIx_SetSpeed(SPI_CH_Type ch,SPI_SPEED_Type Speed)
{
	SPI_TypeDef *SPIx;
	u32 SYS_CLK = SYSTEM_GetClkSpeed();		//��ȡϵͳʱ��		

	if(ch > SPI_ChMax) return 0;			//ͨ���ų�����Χ,���ش���0
	if(Speed > 7) return 0;					//�����ʷ�Ƶֵ���ô���,���ش���0
	SPIx = (SPI_TypeDef *)SPIxN[ch];		//��ȡSPI�ṹָ��
	SPIx->CR1 &= ~(1<<6); 					//SPI�豸ʧ��
	SPIx->CR1 &= ~(7 << 3);					//��������ʷ�Ƶ��
	SPIx->CR1 |= ((Speed - (ch ? 1 : 0)) << 3);	//���ò�����
	SPIx->CR1 |= 1<<6; 						//SPI�豸ʹ��	

	return (SYS_CLK / BAUD_RATE[Speed]) ;	//ʱ�ӷ�Ƶ,�������ղ�����ʱ��,������
} 



/*************************************************************************************************************************
* ����	:	u8 SPIx_Init(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config,SPI_SPEED_Type Speed)
* ����	:	SPI��ʼ������
* ����	:	ch:ͨ��ѡ��,SPI_Config:���ýṹָ��,Speed:�ٶ�,���궨��
* ����	:	0:��ʼ���ɹ�;1:��ʼ��ʧ��,2:����������ʧ��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110625
* ����޸�ʱ�� : 20120929
* ˵��	: 	SPI2,SPI3����ٶ���SPI1��һ��
*************************************************************************************************************************/
u8 SPIx_Init(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config,SPI_SPEED_Type Speed)
{
	//��ʼ��IO�ӿ�
	DeviceClockEnable(DEV_AFIO,ENABLE);				//���ù���AFIOʱ��ʹ��
	switch (ch)
	{
	 	case 0:	//SPI1,SCK:PA5,MISO:PA6,MOSI:PA7
		{
			DeviceClockEnable(DEV_SPI1,ENABLE);		//SPI1 ʱ��ʹ��
			DeviceClockEnable(DEV_GPIOA,ENABLE);	//GPIOAʱ��ʹ��
			DeviceReset(DEV_SPI1);					//SPI1��λ 	 
			if(SPI_Config->EnMSTR)					//ʹ������ģʽ
			{
				GPIOx_Init(GPIOA,BIT5,AF_PP,SPEED_50M); //SCK ���óɸ����������
				GPIOx_Init(GPIOA,BIT6,IN_IPU,0);		//MISO����Ϊ��������
				GPIOx_Init(GPIOA,BIT7,AF_PP,SPEED_50M); //MOSI ���óɸ����������
			}
			else		//�ӻ�ģʽ
			{
				GPIOx_Init(GPIOA,BIT5,IN_IPU,0); 		//SCK ���ó���������
				GPIOx_Init(GPIOA,BIT6,AF_PP,SPEED_50M);	//MISO����Ϊ�����������
				GPIOx_Init(GPIOA,BIT7,IN_IPU,0); 		//MOSI ���ó���������
				GPIOx_Init(GPIOA,BIT4,IN_IPU,0); 		//CS ���óɸ���������
				
				SPI1->CR1 = 0;							//�������,��ֹͣSPI
				SPI1->CR2 = 0;	
				SPI1->CR1 |= BIT10 + BIT1;// + BIT0;		//ֻ����ģʽ,����ʱ�Ӹߵ�ƽ
				SPI1->CR2 |= BIT6;						//ʹ�ܽ����ж�
				NVIC_IntEnable(IRQ_SPI1,1);				//����SPI1ȫ���ж�
				SPI1->CR1 |= 1 << 6; 	//SPI�豸ʹ��
				SPIRx[ch].IntRx = SET;
			}
		}break;
		case 1:	//SPI2,SCK:PB13,MISO:PB14,MOSI:PB15
		{
			DeviceClockEnable(DEV_SPI2,ENABLE);		//SPI2 ʱ��ʹ��
			DeviceClockEnable(DEV_GPIOB,ENABLE);	//GPIOBʱ��ʹ��
			DeviceReset(DEV_SPI2);					//SPI2��λ 	 
			if(SPI_Config->EnMSTR)					//ʹ������ģʽ
			{
				GPIOx_Init(GPIOB,BIT13,AF_PP,SPEED_50M);//SCK ���óɸ����������
				GPIOx_Init(GPIOB,BIT14,IN_IPU,0);		//MISO����Ϊ��������
				GPIOx_Init(GPIOB,BIT15,AF_PP,SPEED_50M);//MOSI ���óɸ����������
			}
			else		//�ӻ�ģʽ
			{
				GPIOx_Init(GPIOB,BIT13,IN_IPU,0); 		//SCK ���ó���������
				GPIOx_Init(GPIOB,BIT14,AF_PP,SPEED_50M);	//MISO����Ϊ�����������
				GPIOx_Init(GPIOB,BIT15,IN_IPU,0); 		//MOSI ���ó���������
				GPIOx_Init(GPIOB,BIT12,IN_IPU,0); 		//CS ���óɸ���������
				
				SPI2->CR1 = 0;							//�������,��ֹͣSPI
				SPI2->CR2 = 0;	
				SPI2->CR1 |= BIT10 + BIT1;// + BIT0;	//ֻ����ģʽ,����ʱ�Ӹߵ�ƽ
				SPI2->CR2 |= BIT6;						//ʹ�ܽ����ж�
				NVIC_IntEnable(IRQ_SPI2,1);				//����SPI2ȫ���ж�
				SPI2->CR1 |= 1 << 6; 	//SPI�豸ʹ��
				SPIRx[ch].IntRx = SET;
			}

		}break;
		case 2://SPI3,SCK:PB3,MISO:PB4,MOSI:PB5
		{
			DeviceClockEnable(DEV_SPI3,ENABLE);		//SPI3 ʱ��ʹ��
			DeviceClockEnable(DEV_GPIOB,ENABLE);	//GPIOBʱ��ʹ��
			DeviceReset(DEV_SPI3);					//SPI3��λ 	 
			GPIOx_Init(GPIOB,BIT3,AF_PP,SPEED_50M); //SCK ���óɸ����������
			GPIOx_Init(GPIOB,BIT4,IN_IPU,0);		//MISO����Ϊ��������
			GPIOx_Init(GPIOB,BIT5,AF_PP,SPEED_50M); //MOSI ���óɸ����������
		}break;
		default : return 1;							//ͨ��ѡ�����,���ش���1
	}
	if(SPI_Config->EnMSTR)
	{
		if(SPIx_Config(ch,SPI_Config))					//����
			return 1;									//����ʧ��
	}
	if(SPIx_SetSpeed(ch,Speed) == 0)				//���ò�����
		return 2;									//����������ʧ��
	//SPIx_ReadWriteByte(ch,0xff);					//��������
	return 0;										//��ʼ���ɹ�
}





/*************************************************************************************************************************
* ����	:	u8  SPIx_Config(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config)	
* ����	:	SPI����
* ����	:	ch��ͨ��ѡ��SPI_Config�����ýṹָ��
* ����	:	0:���óɹ�;1:������ŷ�Χ,���ش���
* ����	:	�ײ�Ĵ�������
* ����	:	cp1300@139.com
* ʱ��	:	20120929
* ����޸�ʱ�� : 20120929
* ˵��	: 	��������SPIģʽ�����õ���8bit����λ��ǰ������ʱ�Ӹߵ�ƽ���߿���ʱ�ӵ͵�ƽ
*************************************************************************************************************************/
u8  SPIx_Config(SPI_CH_Type ch,const SPI_Config_Type *SPI_Config)
{
	SPI_TypeDef *SPIx;
	u32 CR1 = 0, CR2 = 0;		

	if(ch > SPI_ChMax) return 1;					//ͨ���ų�����Χ,���ش���1
	SPIx = (SPI_TypeDef *)SPIxN[ch];				//��ȡSPI�ṹָ��

	SPIx->CR1 = 0;			//�������,��ֹͣSPI
	SPIx->CR2 = 0;
	//���üĴ���1
	if(SPI_Config->En16bit)	//ʹ��16BITģʽ
	{
	 	CR1 |= BIT11;
	}
	if(SPI_Config->EnLSB)	//ʹ�ܵ�λ��ǰ
	{
		CR1 |= BIT7;
	}
	if(SPI_Config->EnCPOH)	//ʹ�ܿ���ʱ��Ϊ�ߵ�ƽ
	{
		CR1 |= BIT1;
	}
	if(SPI_Config->EnCPHA)	//ʹ�����ݲ����ӵڶ������ؿ�ʼ
	{
		CR1 |= BIT0;
	}
	if(SPI_Config->EnSSM)	//ʹ���������Ƭѡ�ź�
	{
		CR1 |= BIT9;
		CR1 |= BIT8;
	}
	if(SPI_Config->EnRxOnly)//��˫��ʹ��
	{
		CR1 |= BIT10;	
	}
	if(SPI_Config->EnBIDI) 	//ʹ�ܵ���˫��ģʽ
	{
	 	CR1 |= BIT15;
	}
	if(SPI_Config->EnCRC)  //ʹ��CRCУ��
	{
		CR1 |= BIT13;
	}
	if(SPI_Config->EnMSTR)	//ʹ�����豸ģʽ	
	{
	 	CR1 |= BIT2;
	}
	//���üĴ���2
	if(SPI_Config->EnTxINT)	//ʹ�ܷ��ͻ��������ж�
	{
		CR2 |= BIT7;
	}
	if(SPI_Config->EnRxINT)	//ʹ�ܽ��ջ��������ж�
	{
		CR2 |= BIT6;
	}
	if(SPI_Config->EnErrorINT)	//ʹ�ܽ��մ����ж�
	{
		CR2 |= BIT5;
	}
	if(SPI_Config->EnSSOE)	//�豸����ʱ,������ģʽ��SS���,���豸���ܹ����ڶ����豸ģʽ
	{
		CR2 |= BIT2;
	}
	if(SPI_Config->EnTxDMA)	//���ͻ�����DMAʹ��
	{
		CR2 |= BIT1;
	}
	if(SPI_Config->EnRxDMA)	//���ջ�����DMAʹ��
	{
		CR2 |= BIT0;
	}
	SPIx->CR1 = CR1;		//д���������ݵ��Ĵ���
	SPIx->CR2 = CR2;
	SPIx->CR1 |= 1 << 6; 	//SPI�豸ʹ��
	return 0;
}



//spi����
void SPIx_PowerDown(SPI_CH_Type ch)
{
	SPI_TypeDef *SPIx;	

	if(ch > SPI_ChMax) return;					//ͨ���ų�����Χ,���ش���1
	SPIx = (SPI_TypeDef *)SPIxN[ch];				//��ȡSPI�ṹָ��
	
	SPIx->CR1 &= ~(1 << 6); 	//SPI�豸����
}


//spi�ϵ�
void SPIx_PowerUp(SPI_CH_Type ch)
{
	SPI_TypeDef *SPIx;	

	if(ch > SPI_ChMax) return;					//ͨ���ų�����Χ,���ش���1
	SPIx = (SPI_TypeDef *)SPIxN[ch];				//��ȡSPI�ṹָ��
	
	SPIx->CR1 |= 1 << 6; 	//SPI�豸ʹ��
}



/*************************************************************************************************************************
* ����	:	bool SPIx_GetNewDataFlag(SPI_CH_Type ch)
* ����	:	��ȡSPI�����ݱ�־
* ����	:	ch:ͨ��ѡ��
* ����	:	TRUE:�ɹ�,FALSE:ʧ��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-25
* ����޸�ʱ�� : 2013-09-25
* ˵��	: 	�����ж��Ƿ����µ�����,������������ݱ�־��
*************************************************************************************************************************/
bool SPIx_GetNewDataFlag(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)										//�ж϶˿��Ƿ񳬳���Χ
		return FALSE;

	if(SPIRx[ch].IntRx == SET)									//�������жϽ���
	{
		if(SPIRx[ch].NewDataFlag == SET) 						//��������
		{
		 	SPIRx[ch].NewDataFlag = RESET;						//�����־
			return TRUE;										//������������
		}
	}
	else														//û�����жϽ���
	{
	 	if(((SPI_TypeDef *)SPIxN[ch])->SR & BIT0)				//RXNE=1,���յ�������
		{
			((SPI_TypeDef *)SPIxN[ch])->SR &= ~BIT0;			//�����־
			return TRUE;
		}
	}
	return FALSE;
}


/*************************************************************************************************************************
* ����	:	bool SPIx_GetRxBuffFullFlag(SPI_CH_Type ch)
* ����	:	��ȡSPI���ջ���������־
* ����	:	ch:ͨ��ѡ��
* ����	:	TRUE:�ɹ�,FALSE:ʧ��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-25
* ����޸�ʱ�� : 2013-09-25
* ˵��	: 	�����жϽ��ջ������Ƿ���,�������־
*************************************************************************************************************************/
bool SPIx_GetRxBuffFullFlag(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)					//�ж϶˿��Ƿ񳬳���Χ
		return FALSE;
	if(SPIRx[ch].BuffFull == SET)			//����������
	{
	 	SPIRx[ch].BuffFull = RESET;			//�������־
		return TRUE;
	}
	return FALSE;
}





/*************************************************************************************************************************
* ����	:	u8 SPIx_GetNewData(SPI_CH_Type ch)
* ����	:	��ȡSPI������
* ����	:	ch:ͨ��ѡ��
* ����	:	�յ�������
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-25
* ����޸�ʱ�� : 2013-09-25
* ˵��	: 	���ڽ���һ���ֽ�����
*************************************************************************************************************************/
u8 SPIx_GetNewData(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)								//�ж϶˿��Ƿ񳬳���Χ
		return 0;

	return (((SPI_TypeDef *)SPIxN[ch])->DR);	//��������
}



/*************************************************************************************************************************
* ����	:	void SPIx_SetRxBuff(SPI_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
* ����	:	����SPI���ջ�����
* ����	:	ch:ͨ��ѡ��,RxBuffSize:��������С,RxBuff:������ָ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-25
* ����޸�ʱ�� : 2013-09-25
* ˵��	: 	һ��Ҫ����,�������жϽ���ʱ���ܻ��쳣
*************************************************************************************************************************/
void SPIx_SetRxBuff(SPI_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
{
	
	if(ch > SPI_ChMax - 1)					//�ж϶˿��Ƿ񳬳���Χ
		return;

	SPIRx[ch].RxBuffSize = RxBuffSize; 		//���û�������С
	SPIRx[ch].RxBuff = RxBuff;				//���û�����ָ��
	SPIRx[ch].RxCnt = 0;					//����������

}





/*************************************************************************************************************************
* ����	:	u32 SPIx_GetRxCnt(SPI_CH_Type ch)
* ����	:	��ȡSPI�������ݼ�����
* ����	:	ch:ͨ��ѡ��
* ����	:	���յ�����������
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-25
* ����޸�ʱ�� : 2013-09-25
* ˵��	: 	��
*************************************************************************************************************************/
u32 SPIx_GetRxCnt(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)						//�ж϶˿��Ƿ񳬳���Χ
		return 0;
	return SPIRx[ch].RxCnt;						//���ؼ���ֵ
}




/*************************************************************************************************************************
* ����	:	void SPIx_ClearRxCnt(SPI_CH_Type ch)
* ����	:	���SPI�������ݼ�����
* ����	:	ch:ͨ��ѡ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-25
* ����޸�ʱ�� : 2013-09-25
* ˵��	: 	��
*************************************************************************************************************************/
void SPIx_ClearRxCnt(SPI_CH_Type ch)
{
	if(ch > SPI_ChMax - 1)					//�ж϶˿��Ƿ񳬳���Χ
		return;
	SPIRx[ch].RxCnt = 0;					//����������
}



/*************************************************************************************************************************
* ����	:	void SPI1_IRQHandler (void)
* ����	:	SPI1�жϽ��պ���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-24
* ����޸�ʱ�� : 2013-09-24
* ˵��	: 	��
*************************************************************************************************************************/
void SPI1_IRQHandler (void)
{
	if(SPIRx[SPI_CH1].RxBuffSize > 0)										//���ջ���������0
	{
	 	(SPIRx[SPI_CH1].RxBuff)[(SPIRx[SPI_CH1].RxCnt) ++] = SPI1->DR; 		//�����ݴ�ŵ�������
		//uart_printf("SPIRx[SPI_CH1].RxCnt=0x%X\r\n",SPIRx[SPI_CH1].RxCnt);
		//uart_printf("SPIRx[SPI_CH1].RxBuffSize=0x%X\r\n",SPIRx[SPI_CH1].RxBuffSize);
		if(SPIRx[SPI_CH1].RxCnt == SPIRx[SPI_CH1].RxBuffSize) 				//����������
		{
			  SPIRx[SPI_CH1].RxCnt = 0;										//���ռ���������
			  SPIRx[SPI_CH1].BuffFull = SET;								//������������־
		}	
	}
	else
	{
		SPI1->SR &= ~BIT0;
	}
	SPIRx[SPI_CH1].NewDataFlag = SET;										//�յ������ݱ�־
	//UARTx_ClearRxInt(UART_CH1);		   									//��������жϱ�־
}



/*************************************************************************************************************************
* ����	:	void SPI1_IRQHandler (void)
* ����	:	SPI1�жϽ��պ���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013-09-24
* ����޸�ʱ�� : 2013-09-24
* ˵��	: 	��
*************************************************************************************************************************/
void SPI2_IRQHandler (void)
{
	if(SPIRx[SPI_CH2].RxBuffSize > 0)										//���ջ���������0
	{
	 	(SPIRx[SPI_CH2].RxBuff)[(SPIRx[SPI_CH2].RxCnt) ++] = SPI2->DR; 		//�����ݴ�ŵ�������
		//uart_printf("SPIRx[SPI_CH1].RxCnt=0x%X\r\n",SPIRx[SPI_CH1].RxCnt);
		//uart_printf("SPIRx[SPI_CH1].RxBuffSize=0x%X\r\n",SPIRx[SPI_CH1].RxBuffSize);
		if(SPIRx[SPI_CH2].RxCnt == SPIRx[SPI_CH2].RxBuffSize) 				//����������
		{
			  SPIRx[SPI_CH2].RxCnt = 0;										//���ռ���������
			  SPIRx[SPI_CH2].BuffFull = SET;								//������������־
		}	
	}
	else
	{
		SPI2->SR &= ~BIT0;
	}
	SPIRx[SPI_CH2].NewDataFlag = SET;										//�յ������ݱ�־
	//UARTx_ClearRxInt(UART_CH1);		   									//��������жϱ�־
}









