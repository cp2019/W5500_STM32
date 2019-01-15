#include "GPIO_INIT.H"
#include "SYSTEM.H"


/*************************************************************************************************************************
* ����	:	void GPIOx_Init(GPIO_TypeDef *GPIOx,vu16 BIT_N,vu8 IO_MODE,vu8 SPEED_MODE)
* ����	:	IO��ʼ������
* ����	:	*GPIOx : ��ӦIO�Ļ�ַָ��(��GPIOA),BIT_N : ��Ӧ��IOλ,IO_MODE : IOģʽ,SPEED_MODE : IO�ٶ�
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111105
* ����޸�ʱ�� : 20111105
* ˵��	: 	һ�γ�ʼ�����IO
*************************************************************************************************************************/
void GPIOx_Init(GPIO_TypeDef *GPIOx,vu16 BIT_N,vu8 IO_MODE,vu8 SPEED_MODE)
{
 	u8 i;

	for(i = 0;i < 16;i ++)
	{
	 	if(BIT_N & 0x1)
		{
		 	if(i < 8)
			{
				GPIOx->CRL &= ~(0xf << 4 * i);
				GPIOx->CRL |= (IO_MODE & 0x3) << 4 * i + 2;
				if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //���������ģʽ,MODE = 00
					GPIOx->CRL |= SPEED_MODE << i * 4;
			}
			else
			{
				GPIOx->CRH &= ~(0xf << 4 * (i - 8));
				GPIOx->CRH |= (IO_MODE & 0x3) << 4 * (i - 8) + 2;
				if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //���������ģʽ,MODE = 00
					GPIOx->CRH |= SPEED_MODE << (i - 8) * 4;
			}
			if(IO_MODE == IN_IPT)
				GPIOx->ODR &= ~(1 << i);  //��������
			else if(IO_MODE == IN_IPU)
				GPIOx->ODR |= (1 << i);  //��������
		}
		BIT_N >>= 1;
	}	
}




/*************************************************************************************************************************
* ����	:	void GPIOx_OneInit(GPIO_TypeDef *GPIOx,vu16 i,vu8 IO_MODE,vu8 SPEED_MODE)
* ����	:	��IO��ʼ������
* ����	:	*GPIOx : ��ӦIO�Ļ�ַָ��(��GPIOA),i : ��Ӧ��IOλ,IO_MODE : IOģʽ,SPEED_MODE : IO�ٶ�
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111105
* ����޸�ʱ�� : 20111105
* ˵��	: 	һ�γ�ʼ�����IO
*************************************************************************************************************************/
void GPIOx_OneInit(GPIO_TypeDef *GPIOx,vu16 i,vu8 IO_MODE,vu8 SPEED_MODE)
{
	if(i > 15) return;

	if(i < 8)
	{
		GPIOx->CRL &= ~(0xf << 4 * i);
		GPIOx->CRL |= (IO_MODE & 0x3) << 4 * i + 2;
		if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //���������ģʽ,MODE = 00
			GPIOx->CRL |= SPEED_MODE << i * 4;
	}
	else
	{
		GPIOx->CRH &= ~(0xf << 4 * (i - 8));
		GPIOx->CRH |= (IO_MODE & 0x3) << 4 * (i - 8) + 2;
		if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //���������ģʽ,MODE = 00
			GPIOx->CRH |= SPEED_MODE << (i - 8) * 4;
	}
	if(IO_MODE == IN_IPT)
		GPIOx->ODR &= ~(1 << i);  //��������
	else if(IO_MODE == IN_IPU)
		GPIOx->ODR |= (1 << i);  //��������
}





/*************************************************************************************************************************
* ����	:	void GPIOx_PD01_Init(void)
* ����	:	��ʼ��PD0,PD1Ϊ��ͨIO
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2014-08-19
* ����޸�ʱ�� : 2014-08-19
* ˵��	: 	��Ҫ����PD0 PD1��ӳ��
*************************************************************************************************************************/
void GPIOx_PD01_Init(void)
{
	RCC->APB2ENR |= 1 << 0;    //��������ʱ��
	
	AFIO->MAPR &= ~BIT15;
	AFIO->MAPR |= BIT15;	//PD0 PD1ӳ��Ϊ��ͨIO
	DeviceClockEnable(DEV_AFIO, ENABLE);
	DeviceClockEnable(DEV_GPIOD, ENABLE);
}


