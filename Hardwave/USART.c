/*************************************************************************************************************
 * �ļ���:		usart.c
 * ����:		STM32 USART����
 * ����:		cp1300@139.com
 * ����ʱ��:	2011��6��11��
 * ����޸�ʱ��:2013��5��29��
 * ��ϸ:		�Ѿ��޸���֪bug
				2013-11-17:����DMA�շ�ģʽ
				2014-08-19:����DMA,�жϻ��ģʽ����Ҫ�����ڴ���4������5��֧��DMA
				2018-06-29:ȥ���жϽ���ģʽ�»�����������ͷ���ǣ�����֮�󽫲�����д洢
*************************************************************************************************************/		
#include "SYSTEM.H"
#include "GPIO_INIT.H"
#include "USART.H"

//UART����ṹָ��
static const  USART_TypeDef * const USARTxN[5] = {USART1,USART2,USART3,UART4,UART5};
//DAMͨ������
#if UART_DMA_EN
#include "dma.h"
static const  DMA_Channel_TypeDef * const USARTxChannel[3] = {DMA1_Channel4, DMA1_Channel7, DMA1_Channel2};	//����ͨ��
static const  DMA_Channel_TypeDef * const USARRxChannel[3] = {DMA1_Channel5, DMA1_Channel6, DMA1_Channel3};	//����ͨ��
#endif	//UART_DMA_EN

//���UART״̬�ṹ
typedef struct
{
	FlagStatus	NewDataFlag;//���յ�������
	FlagStatus	BuffFull;	//����Buff��
	FlagStatus	IntRx;		//�Ƿ����жϽ���
	u8 			*RxBuff;	//����Buffָ��
	u16			RxBuffSize;	//���ջ�������С,һ֡���ݴ�С
	u16 		UartRxCnt;	//�������ݼ�����
	u8			TempData;	//���ڽ���������ȡ���ݼĴ����������ȡ���ݱ�־
} UartRx_TypeDef;


//UART1	����״̬�ṹ
static UartRx_TypeDef UartRx[UART_ChMax + 1];

#ifdef _UCOS_II_
#include "ucos_ii.h"
#endif



/*************************************************************************************************************************
* ����	:	bool UARTx_Init(UART_CH_Type ch,u32 Speed,u8 RX_Int)
* ����	:	���ڳ�ʼ��
* ����	:	ch:ͨ��ѡ��,0->usart1,Speed:�����ٶ�,RX_Int:�Ƿ�ʱ���жϽ���
* ����	:	TRUE:�ɹ�,FALSE:ʧ��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 2013-11-17
* ˵��	: 	USART1~UART5,��Ӧͨ��UART_CH1-UART_CH5
			2013-11-17:���DMA֧��
*************************************************************************************************************************/
bool UARTx_Init(UART_CH_Type ch,u32 Speed,u8 RX_Int)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];//��ȡ��Ӧͨ��Ӳ����ַָ��
	u8 irq_n;

	if(ch > UART_ChMax - 1)
		return FALSE;	//�˿ںų�����Χ
	//��ʼ��UART IO
	DeviceClockEnable(DEV_AFIO, ENABLE);					//���ù���AFIOʱ��ʹ��
	switch (ch)
	{
		case UART_CH1:		//ͨ��1,USART1 ,TX:PA9;RX:PA10
		{
			DeviceClockEnable(DEV_GPIOA,ENABLE);		//GPIO A ʱ��ʹ��
			DeviceClockEnable(DEV_USART1,ENABLE);		//USART 1 ʱ��ʹ��
			GPIOx_Init(GPIOA,BIT9,AF_PP, SPEED_50M);   	//PA09,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOA,BIT10,IN_IPU,IN_IPU);  	//��������
			DeviceReset(DEV_USART1);					//��λ����1
			irq_n =  IRQ_USART1;						//����1�жϺ�
		}break;
		case UART_CH2:		//ͨ��2,USART2 ,TX:PA2;RX:PA3
		{	
			DeviceClockEnable(DEV_GPIOA,ENABLE);		//GPIO A ʱ��ʹ��
			DeviceClockEnable(DEV_USART2,ENABLE);		//USART 2 ʱ��ʹ��
			GPIOx_Init(GPIOA,BIT2,AF_PP, SPEED_50M);   	//PA2,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOA,BIT3,IN_IPU,IN_IPU);  		//��������
			DeviceReset(DEV_USART2);					//��λ����2
			irq_n =  IRQ_USART2;						//����2�жϺ�		
		}break;
		case UART_CH3:		//ͨ��3,USART3 ,TX:PB10;RX:PB11
		{
			DeviceClockEnable(DEV_GPIOB,ENABLE);		//GPIO B ʱ��ʹ��
			DeviceClockEnable(DEV_USART3,ENABLE);		//USART 3 ʱ��ʹ��
			GPIOx_Init(GPIOB,BIT10,AF_PP, SPEED_50M);   	//PB10,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOB,BIT11,IN_IPU,IN_IPU);  	//��������
			DeviceReset(DEV_USART3);					//��λ����3
			irq_n =  IRQ_USART3;						//����3�жϺ�		
		}break;

		case UART_CH4:		//ͨ��4,UART4 ,TX:PC10;RX:PC11
		{
			DeviceClockEnable(DEV_GPIOC,ENABLE);		//GPIO C ʱ��ʹ��
			DeviceClockEnable(DEV_UART4,ENABLE);		//UART 4 ʱ��ʹ��
			GPIOx_Init(GPIOC,BIT10,AF_PP, SPEED_50M);   //PC10,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOC,BIT11,IN_IPU,IN_IPU);  	//��������
			DeviceReset(DEV_UART4);						//��λ����1
			irq_n =  IRQ_UART4;							//����1�жϺ�		
		}break;
		case UART_CH5:		//ͨ��5,UART5 ,TX:PC12;RX:PD2
		{
			DeviceClockEnable(DEV_GPIOC,ENABLE);		//GPIO C ʱ��ʹ��
			DeviceClockEnable(DEV_GPIOD,ENABLE);		//GPIO D ʱ��ʹ��
			DeviceClockEnable(DEV_UART5,ENABLE);		//UART 5 ʱ��ʹ��
			GPIOx_Init(GPIOC,BIT12,AF_PP, SPEED_50M);   //PC12,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOD,BIT2,IN_IPU,IN_IPU);  		//��������
			DeviceReset(DEV_UART5);						//��λ����5
			irq_n =  IRQ_UART5;							//����5�жϺ�			
		}break;
		default : return FALSE;							//�˿ںų�����Χ,���ش���
	}
	//���ò����ʷ�Ƶϵ��
	UARTx_SetBaudRate(ch, Speed);						//���ò�����
	//����UART
	UARTx->CR1 = 0x2000;								//ʹ��USART,1����ʼλ,8λ����
	UARTx->CR1 |= 0x8;									//��TE = 1;����ʹ��;���͵�һ������λ
	UARTx->CR1 |= 0x04;									//RE = 1;����ʹ��
#if UART_DMA_EN
	if(ch > UART_CH3)	//����4,5��֧��DMA
	{
		UARTx->CR3 = 0;	
	}
	else
	{
		UARTx->CR3 = BIT7;									//ȫ˫��,DMA����ģʽ,��ֹ�����ж�
		DMA_MemoryToPeripheralConfig((DMA_Channel_TypeDef *)USARTxChannel[ch], (u32)&UARTx->DR, DMA_SIZE_8BIT);		//�洢���������DMA��������
	}
	
#else
	UARTx->CR3 = 0;										//ȫ˫��,��ֹ�����ж�
#endif //UART_DMA_EN
	UARTx_SetRxBuff(ch,0,NULL);							//���ô��ڽ��ջ�����
	UARTx_ClearRxInt(ch);		   						//������ڽ����жϱ�־
	if(RX_Int)
	{
		
#if UART_DMA_EN
		if(ch > UART_CH3)	//����4,5��֧��DMA
		{
			UARTx->CR1 |= 0x20;								//RXNEIE = 1,��RXNE�ж�,�����������ж�
			NVIC_IntEnable(irq_n,1);						//����USART1ȫ���ж�
			UartRx[ch].IntRx = SET;							//�жϽ��ձ�־��Ч
		}
		else
		{
			UARTx->CR3 |= BIT6;							//DMA����ģʽ
		}
#else
		UARTx->CR1 |= 0x20;								//RXNEIE = 1,��RXNE�ж�,�����������ж�
	 	NVIC_IntEnable(irq_n,1);						//����USART1ȫ���ж�
		UartRx[ch].IntRx = SET;							//�жϽ��ձ�־��Ч
#endif //UART_DMA_EN	 	
		
	} 
	else
	{
		NVIC_IntEnable(irq_n,0); 						//�ر�USARTȫ���ж�
		UartRx[ch].IntRx = RESET;						//�жϽ��ձ�־��Ч
	}
	return TRUE;										//��ʼ���ɹ�,����0
}





/*************************************************************************************************************************
* ����	:	void UARTx_PowerDown(UART_CH_Type ch)
* ����	:	UART����
* ����	:	ch:ͨ��ѡ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20130316
* ˵��	: 	����͹���ģʽ
*************************************************************************************************************************/
void UARTx_PowerDown(UART_CH_Type ch)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];	//��ȡ��Ӧͨ��Ӳ����ַָ��
	
	if(ch > UART_ChMax - 1)									//�ж϶˿��Ƿ񳬳���Χ
		return;
	//while(!(UARTx->SR & 0x80));								//�ȴ�������ɲſ����͹���ģʽ
	
	switch (ch)
	{
		case UART_CH1:		//ͨ��1,USART1 ,TX:PA9;RX:PA10
		{
			GPIOx_Init(GPIOA,BIT9,IN_IPT, IN_IN);   	
			GPIOx_Init(GPIOA,BIT10,IN_IPT,IN_IN);  
		}break;
		case UART_CH2:		//ͨ��2,USART2 ,TX:PA2;RX:PA3
		{	
			GPIOx_Init(GPIOA,BIT2,IN_IPT, IN_IN);   
			GPIOx_Init(GPIOA,BIT3,IN_IPT,IN_IN);  		
		}break;
		case UART_CH3:		//ͨ��3,USART3 ,TX:PB10;RX:PB11
		{
			GPIOx_Init(GPIOB,BIT10,IN_IPT, IN_IN);   
			GPIOx_Init(GPIOB,BIT11,IN_IPT,IN_IN);  
		}break;

		case UART_CH4:		//ͨ��4,UART4 ,TX:PC10;RX:PC11
		{
			GPIOx_Init(GPIOC,BIT10,IN_IPT, IN_IN);  
			GPIOx_Init(GPIOC,BIT11,IN_IPT,IN_IN);  	
		}break;
		case UART_CH5:		//ͨ��5,UART5 ,TX:PC12;RX:PD2
		{
			GPIOx_Init(GPIOC,BIT12,IN_IPT, IN_IN);  
			GPIOx_Init(GPIOD,BIT2,IN_IPT,IN_IN);  	
		}break;
		default : return ;							//�˿ںų�����Χ,����
	}
	UARTx->CR1 &= ~(1 << 13);								//UEλд0,�����͹���	
}


/*************************************************************************************************************************
* ����	:	void UARTx_PowerUp(UART_CH_Type ch)
* ����	:	UART�ϵ�
* ����	:	ch:ͨ��ѡ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20130316
* ˵��	: 	�˳��͹���ģʽ
*************************************************************************************************************************/
void UARTx_PowerUp(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)									//�ж϶˿��Ƿ񳬳���Χ
		return;
	
	switch (ch)
	{
		case UART_CH1:		//ͨ��1,USART1 ,TX:PA9;RX:PA10
		{
			GPIOx_Init(GPIOA,BIT9,AF_PP, SPEED_50M);   	//PA09,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOA,BIT10,IN_IPU,IN_IPU);  	//��������
		}break;
		case UART_CH2:		//ͨ��2,USART2 ,TX:PA2;RX:PA3
		{	
			GPIOx_Init(GPIOA,BIT2,AF_PP, SPEED_50M);   	//PA2,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOA,BIT3,IN_IPU,IN_IPU);  		//��������
		}break;
		case UART_CH3:		//ͨ��3,USART3 ,TX:PB10;RX:PB11
		{
			GPIOx_Init(GPIOB,BIT10,AF_PP, SPEED_50M);   	//PB10,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOB,BIT11,IN_IPU,IN_IPU);  	//��������
		}break;

		case UART_CH4:		//ͨ��4,UART4 ,TX:PC10;RX:PC11
		{
			GPIOx_Init(GPIOC,BIT10,AF_PP, SPEED_50M);   //PC10,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOC,BIT11,IN_IPU,IN_IPU);  	//��������
		}break;
		case UART_CH5:		//ͨ��5,UART5 ,TX:PC12;RX:PD2
		{
			GPIOx_Init(GPIOC,BIT12,AF_PP, SPEED_50M);   //PC12,TXDֻ�����óɸ����������
			GPIOx_Init(GPIOD,BIT2,IN_IPU,IN_IPU);  		//��������	
		}break;
		default : return ;							//�˿ںų�����Χ,����
	}
	((USART_TypeDef *)USARTxN[ch])->CR1 |= (1 << 13);		//UEλ1,�˳��͹���ģʽ	
}




/*************************************************************************************************************************
* ����	:	void UARTx_SetBaudRate(UART_CH_Type ch,u32 baud)
* ����	:	���ڲ���������
* ����	:	ch:ͨ��ѡ��,baud:������,��9600,115200�ȵ�
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013316
* ����޸�ʱ�� : 2013316
* ˵��	: 	USART1~UART5,��Ӧͨ��UART_CH1-UART_CH5
			����ǰ����رմ���
			���Զ���ȡϵͳ��ǰ��ʱ��,�����м���.
*************************************************************************************************************************/
void UARTx_SetBaudRate(UART_CH_Type ch,u32 baud)
{
	u32 SysClk = 0;
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   	//��ȡ��Ӧͨ��Ӳ����ַָ��
	float fclk;

	SysClk = SYSTEM_GetClkSpeed();							//��ȡϵͳʱ��
	if(ch > 0)
	{
		SysClk /= 2;		   								//USART2,3,4,5ʱ��
	}
	UARTx_PowerDown(ch);									//�������ģʽ,��������
	fclk = (float)SysClk / 16.0 / baud;						//���㲨���ʷ�Ƶϵ��
	SysClk = (u16)fclk;										//�õ������ʷ�Ƶϵ����������	
	UARTx->BRR =  SysClk << 4;								//���ò�������������
	fclk -= SysClk;											//�õ������ʷ�Ƶϵ��С������
	fclk *= 16;
	UARTx->BRR |= 0xf & (u16)fclk;							//���ò�����С������ 
	UARTx_PowerUp(ch);										//���������ϵ�
}




/*************************************************************************************************************************
* ����	:	bool UARTx_Config(UART_CH_Type ch,UART_Config_TypeDef * cfg)
* ����	:	��������
* ����	:	ch:ͨ��ѡ��,0->usart1;cfg:�������ýṹָ��
* ����	:	TRUE:�ɹ�,FALSE:ʧ��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120408
* ����޸�ʱ�� : 20120408
* ˵��	: 	USART1~UART5,��Ӧͨ��UART_CH1-UART_CH5
*************************************************************************************************************************/
bool UARTx_Config(UART_CH_Type ch,UART_Config_TypeDef * cfg)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   	//��ȡ��Ӧͨ��Ӳ����ַָ��

	if(ch > UART_ChMax - 1)									//�ж϶˿��Ƿ񳬳���Χ
		return FALSE;
	UARTx_PowerDown(ch);									//�������ģʽ,��������
	switch (cfg->OddEvenVerify)								//����У��λ
	{
	 	case UART_VERIFY_NULL:								//��У��
		{
			UARTx->CR1 &= ~BIT12;							//һ����ʼλ,8������λ
			UARTx->CR1 &= ~BIT10;							//��ֹУ�����
		}break;
		case UART_ODD:										//��У��
		{
			UARTx->CR1 |= BIT12;							//һ����ʼλ,9������λ
			UARTx->CR1 |= BIT10;							//ʹ��У�����
			UARTx->CR1 |= BIT9;								//��У��
		}break;
		case UART_EVEN:										//żУ��
		{
			UARTx->CR1 |= BIT12;							//һ����ʼλ,9������λ
			UARTx->CR1 |= BIT10;							//ʹ��У�����
			UARTx->CR1 &= ~BIT9;							//żУ��
		}break;
		default : 
		{
			UARTx_PowerUp(ch);								//���������ϵ�
			return FALSE;									//���ô���,����У�����ô���
		}
	}
	if(cfg->StopBitWidth == UART_STOP_1BIT) 				//����ֹͣλ
	{
		UARTx->CR2 &= ~(0x3 << 12);							//�������,Ĭ��һ��ֹͣλ
	}
	else if(cfg->StopBitWidth == UART_STOP_2BIT)
	{
		UARTx->CR2 &= ~(0x3 << 12);
		UARTx->CR2 |= (0x2 << 12);							//2��ֹͣλ
	} 
	else
	{
		UARTx_PowerUp(ch);									//���������ϵ�
		return FALSE;										//ֹͣλ���ô���,���ش���2
	}
	UARTx_PowerUp(ch);										//���������ϵ�	
	return TRUE;											//�������,����TRUE
}






/*************************************************************************************************************************
* ����	:	void UARTx_EnableRx(UART_CH_Type ch,FunctionalState Enable)
* ����	:	���ڽ���ʹ��
* ����	:	ch:ͨ��ѡ��,ENABLE:ʹ�ܽ���,DISABLE:�رս���
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2013316
* ����޸�ʱ�� : 2013316
* ˵��	: 	USART1~UART5,��Ӧͨ��UART_CH1-UART_CH5
*************************************************************************************************************************/
void UARTx_EnableRx(UART_CH_Type ch,FunctionalState Enable)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   //��ȡ��Ӧͨ��Ӳ����ַָ��

	if(Enable)
		UARTx->CR1 |= 0x04;									//RE = 1;����ʹ��
	else
		UARTx->CR1 &= ~0x04;								//RE = 0;���չر�
}






/*************************************************************************************************************************
* ����	:	void UARTx_SendByte(UART_CH_Type ch,u8 data)
* ����	:	UART���ֽڷ���
* ����	:	ch:ͨ����,dataL:Ҫ���͵�����
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 2012-04-29
* ˵��	: 	USART1~UART5,��Ӧͨ��UART_CH1-UART_CH5
			���ȴ��������ע�͵���,������΢��߷����ٶ�
*************************************************************************************************************************/
void UARTx_SendByte(UART_CH_Type ch,u8 data)
{
	USART_TypeDef *UARTx = (USART_TypeDef *)USARTxN[ch];   	//��ȡ��Ӧͨ��Ӳ����ַָ��
	if(ch > UART_ChMax - 1)									//�ж϶˿��Ƿ񳬳���Χ
		return;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//����4,5��֧��DMA
	{
		while(!(UARTx->SR & 0x80));													//�ȴ����ͼĴ���Ϊ��,(������������ʱ�����׶�ʧ )
		UARTx->DR = data;															//��������
#if !UART_TX_TO_FIFI	
		while(!(UARTx->SR & 0x40));													//�ȴ�TC = 1;Ҳ���Ƿ������,���ݴӷ���FIFO���ͳ�ȥ�˲���Ϊ�����Ѿ����
		UARTx->SR &= ~(1 << 6);														//���������ɱ�־
#endif	//!UART_TX_TO_FIFI
	}
	else
	{
		DMA_StartChannel((DMA_Channel_TypeDef *)USARTxChannel[ch],(u32)&data, 1);	//����DMA����
		DMA_WaitComplete((DMA_Channel_TypeDef *)USARTxChannel[ch]);					//�ȴ��������
	}
#else	
	while(!(UARTx->SR & 0x80));														//�ȴ����ͼĴ���Ϊ��,(������������ʱ�����׶�ʧ )
 	UARTx->DR = data;																//��������
#if !UART_TX_TO_FIFI	
	while(!(UARTx->SR & 0x40));														//�ȴ�TC = 1;Ҳ���Ƿ������,���ݴӷ���FIFO���ͳ�ȥ�˲���Ϊ�����Ѿ����
	UARTx->SR &= ~(1 << 6);															//���������ɱ�־
#endif	//!UART_TX_TO_FIFI
#endif //UART_DMA_EN
}



/*************************************************************************************************************************
* ����	:	void UARTx_SendData(UART_CH_Type ch,u8 *tx_buff,u16 byte_number)
* ����	:	UART���ݷ��ͺ���
* ����	:	ch:ͨ����,tx_buff:���ͻ�����,byte_number:��Ҫ���͵��ֽ�
* ����	:	��
* ����	:	void UART_SendByte(u8 ch,u8 data)
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20120403
* ˵��	: 	��DMA��ʽ,��FIFO��ʽ����
*************************************************************************************************************************/
void UARTx_SendData(UART_CH_Type ch,u8 *pTxBuff,u16 DataLen)
{
	u16 i;
	if(ch > UART_ChMax - 1)						//�ж϶˿��Ƿ񳬳���Χ
		return;
	
#if UART_DMA_EN
	
	if(ch > UART_CH3)	//����4,5��֧��DMA
	{
		for(i = 0;i < DataLen;i++)				//ѭ������,ֱ���������
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
	for(i = 0;i < DataLen;i++)				//ѭ������,ֱ���������
	{
	 	UARTx_SendByte(ch,pTxBuff[i]);
	}
#endif //UART_DMA_EN
}




/*************************************************************************************************************************
* ����	:	void UARTx_SendString(UART_CH_Type ch,char *pString)
* ����	:	UART�����ַ���
* ����	:	ch:ͨ����
			pString:�ַ���ָ��
* ����	:	��
* ����	:	void UART_SendByte(u8 ch,u8 data)
* ����	:	cp1300@139.com
* ʱ��	:	2013-04-18
* ����޸�ʱ�� : 2013-04-18
* ˵��	: 	��DMA��ʽ,��FIFO��ʽ����
*************************************************************************************************************************/
#include "string.h"
void UARTx_SendString(UART_CH_Type ch,char *pString)
{	
	if(ch > UART_ChMax - 1)						//�ж϶˿��Ƿ񳬳���Χ
		return;
	
	/*while(*pString != '\0')
	{
		UARTx_SendByte(ch, *pString ++);
	}*/
	UARTx_SendData(ch, (u8 *)pString, strlen(pString));
}





/*************************************************************************************************************************
* ����	:	bool UARTx_GetNewDataFlag(UART_CH_Type ch)
* ����	:	��ȡ���������ݱ�־
* ����	:	ch:ͨ��ѡ��
* ����	:	TRUE:�ɹ�,FALSE:ʧ��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20120403
* ˵��	: 	�����ж��Ƿ����µ�����,������������ݱ�־��
*************************************************************************************************************************/
bool UARTx_GetNewDataFlag(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)										//�ж϶˿��Ƿ񳬳���Χ
		return FALSE;

	if(UartRx[ch].IntRx == SET)									//�������жϽ���
	{
		if(UartRx[ch].NewDataFlag == SET) 						//��������
		{
		 	UartRx[ch].NewDataFlag = RESET;						//�����־
			return TRUE;										//������������
		}
	}
	else														//û�����жϽ���
	{
	 	if(((USART_TypeDef *)USARTxN[ch])->SR & BIT5)			//RXNE=1,���յ�������
		{
			((USART_TypeDef *)USARTxN[ch])->SR &= ~BIT5;		//�����־
			return TRUE;
		}
	}
	return FALSE;
}


/*************************************************************************************************************************
* ����	:	bool UARTx_GetRxBuffFullFlag(UART_CH_Type ch)
* ����	:	��ȡ���ڽ��ջ���������־
* ����	:	ch:ͨ��ѡ��
* ����	:	TRUE:�ɹ�,FALSE:ʧ��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20120403
* ˵��	: 	�����жϽ��ջ������Ƿ���,�������־
*************************************************************************************************************************/
bool UARTx_GetRxBuffFullFlag(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)					//�ж϶˿��Ƿ񳬳���Χ
		return FALSE;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//����4,5��֧��DMA
	{
		if(UartRx[ch].BuffFull == SET)			//����������
		{
			UartRx[ch].BuffFull = RESET;			//�������־
			return TRUE;
		}
		return FALSE;
		}
	else
	{
		if(DMA_GetCompleteResidualCnt((DMA_Channel_TypeDef *)USARRxChannel[ch]) == 0)
		{
			if(((USART_TypeDef *)USARTxN[ch])->SR & BIT3)	//������ر�־
			{
				(USART_TypeDef *)USARTxN[ch]->DR;
			}
			return TRUE;
		}	
		else
			return FALSE;
	}
	
#else 
	if(UartRx[ch].BuffFull == SET)			//����������
	{
	 	UartRx[ch].BuffFull = RESET;			//�������־
		return TRUE;
	}
	return FALSE;
#endif //UART_DMA_EN
}


/*************************************************************************************************************************
* ����	:	void UART_ClearRxInt(UART_CH_Type ch)
* ����	:	������ڽ����жϱ�־
* ����	:	ch:ͨ��ѡ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20120403
* ˵��	: 	����������ձ�־
*************************************************************************************************************************/
void UARTx_ClearRxInt(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)								//�ж϶˿��Ƿ񳬳���Χ
		return;
	((USART_TypeDef *)USARTxN[ch])->SR &= ~BIT5;		//�����־
}


/*************************************************************************************************************************
* ����	:	u8 UARTx_GetNewData(UART_CH_Type ch)
* ����	:	��ȡ����������
* ����	:	ch:ͨ��ѡ��
* ����	:	�յ�������
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20120403
* ˵��	: 	���ڽ���һ���ֽ�����
*************************************************************************************************************************/
u8 UARTx_GetNewData(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)								//�ж϶˿��Ƿ񳬳���Χ
		return 0;

	return (((USART_TypeDef *)USARTxN[ch])->DR);	//��������
}



/*************************************************************************************************************************
* ����	:	void UARTx_SetRxBuff(UART_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
* ����	:	���ô��ڽ��ջ�����
* ����	:	ch:ͨ��ѡ��,RxBuffSize:��������С,RxBuff:������ָ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20120403
* ����޸�ʱ�� : 20120403
* ˵��	: 	һ��Ҫ����,�������жϽ���ʱ���ܻ��쳣
*************************************************************************************************************************/
void UARTx_SetRxBuff(UART_CH_Type ch,u8 *RxBuff,u16 RxBuffSize)
{
#ifdef _UCOS_II_
	OS_CPU_SR  cpu_sr;
#endif	//_UCOS_II_
	
	if(ch > UART_ChMax - 1)						//�ж϶˿��Ƿ񳬳���Χ
		return;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//����4,5��֧��DMA
	{
		#ifdef _UCOS_II_
		OS_ENTER_CRITICAL();
		#endif	//_UCOS_II_
			UartRx[ch].RxBuffSize = RxBuffSize; 		//���û�������С
			UartRx[ch].RxBuff = RxBuff;					//���û�����ָ��
			UartRx[ch].UartRxCnt = 0;					//����������
		#ifdef _UCOS_II_
			OS_EXIT_CRITICAL();
		#endif	//_UCOS_II_
	}
	else
	{
		DMA_PeripheralToMemory((DMA_Channel_TypeDef *)USARRxChannel[ch], (u32)RxBuff, (u32)&((USART_TypeDef *)USARTxN[ch])->DR, DMA_SIZE_8BIT, RxBuffSize);//���赽�洢����DMA��������
	}
#endif //UART_DMA_EN
	
#ifdef _UCOS_II_
	OS_ENTER_CRITICAL();
#endif	//_UCOS_II_
	UartRx[ch].RxBuffSize = RxBuffSize; 		//���û�������С
	UartRx[ch].RxBuff = RxBuff;					//���û�����ָ��
#if !UART_DMA_EN		
	UartRx[ch].UartRxCnt = 0;					//����������
#endif //!UART_DMA_EN
#ifdef _UCOS_II_
	OS_EXIT_CRITICAL();
#endif	//_UCOS_II_
}





/*************************************************************************************************************************
* ����	:	u32 UARTx_GetRxCnt(UART_CH_Type ch)
* ����	:	��ȡ���ڽ������ݼ�����
* ����	:	ch:ͨ��ѡ��
* ����	:	���յ�����������
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20130307
* ����޸�ʱ�� : 20130307
* ˵��	: 	��
*************************************************************************************************************************/
u32 UARTx_GetRxCnt(UART_CH_Type ch)
{	
	if(ch > UART_ChMax - 1)						//�ж϶˿��Ƿ񳬳���Χ
		return 0;
	
#if UART_DMA_EN
	if(ch > UART_CH3)	//����4,5��֧��DMA
	{
		return UartRx[ch].UartRxCnt;			//���ؼ���ֵ
	}
	else
	{
		return  UartRx[ch].RxBuffSize - DMA_GetCompleteResidualCnt((DMA_Channel_TypeDef *)USARRxChannel[ch]);
	}
#else
	return UartRx[ch].UartRxCnt;			//���ؼ���ֵ	
#endif //UART_DMA_EN	
}




/*************************************************************************************************************************
* ����	:	void UARTx_ClearRxCnt(UART_CH_Type ch)
* ����	:	������ڽ������ݼ�����
* ����	:	ch:ͨ��ѡ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20130307
* ����޸�ʱ�� : 20130307
* ˵��	: 	��
*************************************************************************************************************************/
void UARTx_ClearRxCnt(UART_CH_Type ch)
{
	if(ch > UART_ChMax - 1)					//�ж϶˿��Ƿ񳬳���Χ
		return;
#if UART_DMA_EN
	if(ch > UART_CH3)	//����4,5��֧��DMA
	{
		UartRx[ch].UartRxCnt = 0;				//����������
	}
	else
	{
		DMA_SetPeripheralToMemoryDataSize((DMA_Channel_TypeDef *)USARRxChannel[ch], (u32)UartRx[ch].RxBuff, UartRx[ch].RxBuffSize);//���赽�洢����DMA��������������
		if(((USART_TypeDef *)USARTxN[ch])->SR & BIT3)	//������ر�־
		{
			(USART_TypeDef *)USARTxN[ch]->DR;
		}
	}
#else
	UartRx[ch].UartRxCnt = 0;				//����������
#endif //UART_DMA_EN
}






#if !UART_DMA_EN
/*************************************************************************************************************************
* ����	:	void USART1_IRQHandler (void)
* ����	:	UART1�жϽ��պ���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110611
* ����޸�ʱ�� : 20120403
* ˵��	: 	��
*************************************************************************************************************************/
void USART1_IRQHandler (void)
{
	if(UartRx[UART_CH1].RxBuffSize > 0 && UartRx[UART_CH1].UartRxCnt < UartRx[UART_CH1].RxBuffSize)												//���ջ���������0
	{
	 	(UartRx[UART_CH1].RxBuff)[(UartRx[UART_CH1].UartRxCnt) ++] = USART1->DR; 	//�����ݴ�ŵ�������
		if(UartRx[UART_CH1].UartRxCnt == UartRx[UART_CH1].RxBuffSize) 				//����������
		{
			 // UartRx[UART_CH1].UartRxCnt = 0;										//���ռ���������
			  UartRx[UART_CH1].BuffFull = SET;										//������������־
		}	
	}
	else //���������ˣ�������յ�������
	{
		UartRx[UART_CH1].TempData = USART1->DR;
	}
	
	UartRx[UART_CH1].NewDataFlag = SET;												//�յ������ݱ�־
	UARTx_ClearRxInt(UART_CH1);		   												//������ڽ����жϱ�־
}

#if UART_ChMax > 1
/*************************************************************************************************************************
* ����	:	void USART2_IRQHandler (void)
* ����	:	UART2�жϽ��պ���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110611
* ����޸�ʱ�� : 20120403
* ˵��	: 	��
*************************************************************************************************************************/
void USART2_IRQHandler (void)
{
	if(UartRx[UART_CH2].RxBuffSize > 0 && UartRx[UART_CH2].UartRxCnt < UartRx[UART_CH2].RxBuffSize)												//���ջ���������0
	{
	 	(UartRx[UART_CH2].RxBuff)[(UartRx[UART_CH2].UartRxCnt) ++] = USART2->DR; 	//�����ݴ�ŵ�������
		if(UartRx[UART_CH2].UartRxCnt == UartRx[UART_CH2].RxBuffSize) 				//����������
		{
			// UartRx[UART_CH2].UartRxCnt = 0;										//���ռ���������
			  UartRx[UART_CH2].BuffFull = SET;										//������������־
		}	
	}
	else //���������ˣ�������յ�������
	{
		UartRx[UART_CH2].TempData = USART2->DR;
	}
	UartRx[UART_CH2].NewDataFlag = SET;												//�յ������ݱ�־
	UARTx_ClearRxInt(UART_CH2);		   												//������ڽ����жϱ�־
}
#endif


#if UART_ChMax > 2
/*************************************************************************************************************************
* ����	:	void USART3_IRQHandler (void)
* ����	:	UART3�жϽ��պ���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110611
* ����޸�ʱ�� : 20120403
* ˵��	: 	��
*************************************************************************************************************************/
void USART3_IRQHandler (void)
{
	if(UartRx[UART_CH3].RxBuffSize > 0  && UartRx[UART_CH3].UartRxCnt < UartRx[UART_CH3].RxBuffSize)												//���ջ���������0
	{
	 	(UartRx[UART_CH3].RxBuff)[(UartRx[UART_CH3].UartRxCnt) ++] = USART3->DR; 	//�����ݴ�ŵ�������
		if(UartRx[UART_CH3].UartRxCnt == UartRx[UART_CH3].RxBuffSize) 				//����������
		{
			  //UartRx[UART_CH3].UartRxCnt = 0;										//���ռ���������
			  UartRx[UART_CH3].BuffFull = SET;										//������������־
		}	
	}
	else //���������ˣ�������յ�������
	{
		UartRx[UART_CH3].TempData = USART3->DR;
	}
	UartRx[UART_CH3].NewDataFlag = SET;												//�յ������ݱ�־
	UARTx_ClearRxInt(UART_CH3);		   												//������ڽ����жϱ�־
}
#endif
#endif //!UART_DMA_EN

#if UART_ChMax > 3
/*************************************************************************************************************************
* ����	:	void UART4_IRQHandler (void)
* ����	:	UART4�жϽ��պ���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110611
* ����޸�ʱ�� : 20120403
* ˵��	: 	��
*************************************************************************************************************************/
void UART4_IRQHandler (void)
{
	if(UartRx[UART_CH4].RxBuffSize > 0 && UartRx[UART_CH4].UartRxCnt < UartRx[UART_CH4].RxBuffSize)												//���ջ���������0
	{
	 	(UartRx[UART_CH4].RxBuff)[(UartRx[UART_CH4].UartRxCnt) ++] = UART4->DR; 	//�����ݴ�ŵ�������
		if(UartRx[UART_CH4].UartRxCnt == UartRx[UART_CH4].RxBuffSize) 				//����������
		{
			  //UartRx[UART_CH4].UartRxCnt = 0;										//���ռ���������
			  UartRx[UART_CH4].BuffFull = SET;										//������������־
		}	
	}
	else //���������ˣ�������յ�������
	{
		UartRx[UART_CH4].TempData = UART4->DR;
	}
	UartRx[UART_CH4].NewDataFlag = SET;												//�յ������ݱ�־
	UARTx_ClearRxInt(UART_CH4);		   												//������ڽ����жϱ�־
}
#endif



#if UART_ChMax > 4
/*************************************************************************************************************************
* ����	:	void UART5_IRQHandler (void)
* ����	:	UART5�жϽ��պ���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20110611
* ����޸�ʱ�� : 20120403
* ˵��	: 	��
*************************************************************************************************************************/
void UART5_IRQHandler (void)
{
	if(UartRx[UART_CH5].RxBuffSize > 0 && UartRx[UART_CH5].UartRxCnt < UartRx[UART_CH5].RxBuffSize)												//���ջ���������0
	{
	 	(UartRx[UART_CH5].RxBuff)[(UartRx[UART_CH5].UartRxCnt) ++] = UART5->DR; 	//�����ݴ�ŵ�������
		if(UartRx[UART_CH5].UartRxCnt == UartRx[UART_CH5].RxBuffSize) 				//����������
		{
			 // UartRx[UART_CH5].UartRxCnt = 0;										//���ռ���������
			  UartRx[UART_CH5].BuffFull = SET;										//������������־
		}	
	}
	else //���������ˣ�������յ�������
	{
		UartRx[UART_CH5].TempData = UART5->DR;
	}
	UartRx[UART_CH5].NewDataFlag = SET;												//�յ������ݱ�־
	UARTx_ClearRxInt(UART_CH5);		   												//������ڽ����жϱ�־
}
#endif






//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	UARTx_SendByte(0,(u8)ch);      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////






#undef UART_ChMax
