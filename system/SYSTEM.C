/*************************************************************************************************************
 * �ļ���:		system.c
 * ����:		���STM32F4Ӳ��ϵͳ����
 * ����:		cp1300@139.com
 * ����:		cp1300@139.com
 * ����ʱ��:	2011-06-08
 * ����޸�ʱ��:2018-04-07
 * ��ϸ:		2016-10-23 ����ʱ����Դ����
				2017-11-22 ���Ӵ�ӡ��������
				2018-02-08 ���Ӳ���ϵͳ����ʱ���¼
				2018-04-07 ����ͳһ��ϵͳ��ʱ����
*************************************************************************************************************/
#include "stm32f103_map.h"	 
#include "SYSTEM.H"
#include "DELAY.H"


#define EXT_CLOCK_INIT_TIME_OUT		250000

const char *const gc_ClockSourceString[] = {"�ڲ�ʱ��HSI","�ⲿʱ��HSE","�ⲿʱ��HSI+PLL","�ⲿʱ��HSE+PLL"};
const vu8 *g_CPU_ID = (const vu8 *)0x1FFFF7E8;	//CPU ID
volatile u64 g_OS_RunTime = 0;			//����ϵͳ����ʱ��-��λms

/*************************************************************************************************************************
* ����	:	void SYSTEM_ClockInit(FunctionalState ExtClockEnable, FunctionalState PLLEnable, u8 PLL)
* ����	:	ϵͳʱ�ӳ�ʼ������
* ����	:	ExtClockEnable:ʹ���ⲿʱ��;PLLEnable:PLLʹ��;PLL:PLLֵ
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2012-04-03
* ����޸�ʱ�� : 2014-11-14
* ˵��	: 	����ʹ���ⲿ8MHZʱ��,�ڲ�8MHZ,�Լ��ڲ�8MHZ��Ƶ����PLL
			�ⲿʱ��:8MHZ*PLL �ڲ�ʱ��:4HMZ*PLL
			��Ҫ�ظ���RCC->CR���㣬������ʹ���ⲿ���������flash����ʧ��
			flsh���������ã��������ʱ���»ᵼ��flash����ʧ��
			2016-03-01������ǰ�ȸ�λʱ�ӣ���PLL�رգ���ʹ���ڲ�ʱ����Ϊϵͳʱ��
			2016-03-12���ⲿ����ʱ����·bug����·���ñ����ȹر�HSE
*************************************************************************************************************************/
bool SYSTEM_ClockInit(FunctionalState ExtClockEnable, FunctionalState PLLEnable, u8 PLL)
{
	u32 temp=0; 
	u32 TimeOut = 0;
	u32 MainClkSpeed;
	
	
	//�Ƚ�ʱ�Ӹ�λ���ڲ�,Ȼ��Ž���ʱ���л�
	if((RCC->CFGR & (0x03<<2)) != 0)
	{
		RCC->CR |= BIT0;  					//�ڲ�����ʱ��ʹ��HSI
		while(!(RCC->CR & BIT1));			//�ȴ��ڲ�ʱ�Ӿ���
		RCC->CFGR &= ~(BIT0|BIT1);			//�ڲ�ʱ��HSI��Ϊϵͳʱ��
		while(((RCC->CFGR>>2)&0x03)!=0); 	//�ȴ�HSI��Ϊϵͳʱ�����óɹ�
	}
	RCC->CR &= ~BIT24;						//PLLOFF,һ��Ҫ�رգ�����PLL���³�ʼ����ʧ��
	
	
	PLL -= 2; 
	PLL &= 0xF;
	if(ExtClockEnable == ENABLE)	//ʹ���ⲿ����ʱ��
	{
		RCC->CR &= ~BIT16;  		//�ⲿ����ʱ��HSE�ر�
		nop;
		RCC->CR &= ~BIT18;			//ȡ���ⲿʱ����·
		nop;
		RCC->CR |= BIT16;  			//�ⲿ����ʱ��ʹ��HSEON
		TimeOut = 0;
		while(!(RCC->CR & BIT17))	//�ȴ��ⲿʱ�Ӿ���
		{
			TimeOut ++;
			Delay_US(1);
			if(TimeOut > EXT_CLOCK_INIT_TIME_OUT) 
			{
				RCC->CR &= ~BIT16;  			//�ⲿ����ʱ��ʹ�ܹر�
				return FALSE;
			}
				
		}
	}
	else	//ʹ���ڲ�����ʱ��
	{
		RCC->CR |= BIT0;  			//�ڲ�����ʱ��ʹ��HSI
		while(!(RCC->CR & BIT1));	//�ȴ��ڲ�ʱ�Ӿ���
	}
	RCC->CFGR = 0X00000400; 		//APB1=DIV/2;APB2=DIV/1;AHB=DIV/1;
	FLASH->ACR |= 0x32;				//flash�Ȱ������ʱ��������ʱ���������ָ���ʱ��flash��дʧ��
	if(PLLEnable == ENABLE)			//ʹ��PLL
	{
		RCC->CFGR |= PLL<<18;   	//����PLLֵ2-7,�ֱ��Ӧ4,5,6,7,8,9��Ƶ
		if(ExtClockEnable == ENABLE)//ʹ���ⲿ����ʱ��
		{
			RCC->CFGR |= BIT16;	  	//PLLSRC ON 
		}
		else	//�ڲ�����ʱ��
		{	
			RCC->CFGR &= ~BIT16;	//HSI����ʱ�Ӿ�2��Ƶ����ΪPLL����ʱ��
		}
		RCC->CR |= BIT24;			//PLLON
		TimeOut = 0;
		while(!(RCC->CR & BIT25))	//�ȴ�PLL����
		{
			TimeOut ++;
			Delay_US(1);
			if(TimeOut > EXT_CLOCK_INIT_TIME_OUT) return FALSE;
		}		 
		temp = RCC->CFGR;
		temp &= ~(BIT0|BIT1);	//�������
		temp |= BIT1;
		RCC->CFGR = temp;		//PLL��Ϊϵͳʱ��	
		
		while(((RCC->CFGR>>2)&0x03)!=2); 	//�ȴ�PLL��Ϊϵͳʱ�����óɹ�
	}
	else	//û��ʹ��PLL
	{
		if(ExtClockEnable == ENABLE)//ʹ���ⲿ����ʱ��
		{
			temp = RCC->CFGR;
			temp &= ~(BIT0|BIT1);	//�������
			temp |= BIT0;
			RCC->CFGR = temp;		//�ⲿ����ʱ��HSE��Ϊϵͳʱ��
			TimeOut = 0;
			while(((RCC->CFGR>>2)&0x03)!=1) 	//�ȴ�HSE��Ϊϵͳʱ�����óɹ�)     //�ȴ�HSE��Ϊϵͳʱ�����óɹ�
			{   
				TimeOut ++;
				Delay_US(1);
				if(TimeOut > EXT_CLOCK_INIT_TIME_OUT) return FALSE;
			}
		}
		else									//ʹ���ڲ�RCʱ��
		{
			RCC->CFGR &= ~(BIT0|BIT1);			//�ڲ�ʱ��HSI��Ϊϵͳʱ��
			while(((RCC->CFGR>>2)&0x03)!=0); 	//�ȴ�HSI��Ϊϵͳʱ�����óɹ�
		}
	}
	
	if(ExtClockEnable==DISABLE)	//��ʹ���ⲿʱ�ӣ���ر��ⲿʱ�ӣ����͹���
	{
		RCC->CR &= ~BIT16;  	//�ⲿ����ʱ��ʹ��HSEON�رգ��ر��ⲿʱ��
	}
	if(PLLEnable == DISABLE)	//��ʹ��PLL����ر�PLL���͹���
	{
		RCC->CR &= ~BIT24;		//PLLOFF
	}
	
	MainClkSpeed = SYSTEM_GetClkSpeed();		//��ȡϵͳʱ��
	if(MainClkSpeed > 48000000)					//����48M
	{
		temp = FLASH->ACR;
		temp &= ~7;	  
		temp |= 0x32;
		FLASH->ACR = temp;	  					//FLASH 2����ʱ����
		
	}
	else if(MainClkSpeed > 24000000)			//����24M
	{
		temp = FLASH->ACR;
		temp &= ~7;	  
		temp |= 0x31;
		FLASH->ACR = temp;	  					//FLASH 1����ʱ����
	}
	else
	{
		FLASH->ACR &= ~7;	  					//FLASH ����ȴ�
	}
	
	return TRUE;
}




/*************************************************************************************************************************
* ����			:	CLOCK_SOURCE SYS_GetSystemClockSource(void)			
* ����			:	STM32 ��ȡϵͳ��ʱ����Դ
* ����			:	��
* ����			:	ʱ����Դ����CLOCK_SOURCE
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2016-03-13
* ����޸�ʱ�� 	: 	2016-03-28
* ˵��			:	���ڻ�ȡʱ����Դ��ͨ��RCC�Ĵ�����ȡ
*************************************************************************************************************************/
CLOCK_SOURCE SYS_GetSystemClockSource(void)
{
	u32 temp = RCC->CFGR >> 2;
	
	temp &= 0x3;
	switch(temp)
	{
		case 0:	return SYS_CLOCK_HSI;	//HSI��Ϊϵͳʱ��
		case 1: return SYS_CLOCK_HSE;	//HSE��Ϊϵͳʱ��
		default:	//PLL
		{
			if(RCC->CFGR & BIT16)	//ѡ�� HSE ����ʱ����Ϊ PLL ʱ������
				return SYS_CLOCK_HSE_PLL;
			else	//ѡ�� HSI/2 ʱ����Ϊ PLLʱ������
				return SYS_CLOCK_HSI_PLL;
		}
	}
}


/*************************************************************************************************************************
* ����	:	u32 SYSTEM_GetClkSpeed(void)
* ����	:	��ȡϵͳʱ���ٶ�
* ����	:	��
* ����	:	ϵͳʱ���ٶ�,��λHz
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com  
* ʱ��	:	2012-09-29
* ����޸�ʱ�� : 2016-03-28
* ˵��	: 	���ڻ�ȡϵͳʱ���ٶ�
*************************************************************************************************************************/
u32 SYSTEM_GetClkSpeed(void)
{
	u8 i;
	u32	clk;

	switch(SYS_GetSystemClockSource())	//��ȡʱ����Դ
	{
		case SYS_CLOCK_HSI:	//HSI��Ϊϵͳʱ��
		{
			return 8*1000000;
		}
		case SYS_CLOCK_HSE: //HSE��Ϊϵͳʱ��
		{
			return 8*1000000;
		}
		case SYS_CLOCK_HSI_PLL:	//�ڲ�ʱ�ӣ�PLL
		{
			i =  (RCC->CFGR >> 18) & 0xf;
			clk = (i + 2) * 4000000;		//�ڲ�ʱ��2��Ƶ��ΪPLL����
			return clk;
		}
		case SYS_CLOCK_HSE_PLL:	//�ⲿʱ�ӣ�PLL
		{
			i =  (RCC->CFGR >> 18) & 0xf;
			clk = (i + 2) * 8000000;
			return clk;
		}
		default:	return 8*1000000;
	}
}



///*************************************************************************************************************************
//* ����	:	void SYSTEM_ClockInit(u8 PLL)
//* ����	:	ϵͳʱ�ӳ�ʼ������
//* ����	:	PLL��Ƶ��:��2��ʼ�����ֵΪ9
//* ����	:	��
//* ����	:	�ײ�궨��
//* ����	:	cp1300@139.com
//* ʱ��	:	20110608
//* ����޸�ʱ�� : 20120403
//* ˵��	: 	���޸�ȫ��ϵͳʱ��Ƶ��SYS_CLOCK
//*************************************************************************************************************************/
//#if(ENABLE_HSI==0)
//void SYSTEM_ClockInit(u8 PLL)
//{
//	u8 temp=0; 

//	PLL -= 2;  
//	RCC->CR |= 0x00010000;  	//�ⲿ����ʱ��ʹ��HSEON
//	while(!(RCC->CR>>17));		//�ȴ��ⲿʱ�Ӿ���
//	RCC->CFGR = 0X00000400; 	//APB1=DIV2;APB2=DIV1;AHB=DIV1;
//	RCC->CFGR |= PLL<<18;   	//����PLLֵ0-7
//	RCC->CFGR |= 1<<16;	  		//PLLSRC ON 
//	FLASH->ACR |= 0x32;	  		//FLASH 2����ʱ����
//	RCC->CR |= 0x01000000;  	//PLLON
//	while(!(RCC->CR>>25));		//�ȴ�PLL����
//	RCC->CFGR |= 0x00000002;	//PLL��Ϊϵͳʱ��	 
//	while(temp != 0x02)     	//�ȴ�PLL��Ϊϵͳʱ�����óɹ�
//	{   
//		temp = RCC->CFGR>>2;
//		temp &= 0x03;
//	} 		    
//}
//#else
//void SYSTEM_ClockInit(u8 PLL)
//{
//	u8 temp=0; 

//	PLL -= 2;  
//	RCC->CR |= BIT0;  			//�ڲ�����ʱ��ʹ��HSI
//	while(!(RCC->CR&BIT1));		//�ȴ��ڲ�ʱ�Ӿ���
//	RCC->CFGR = 0X00000400; 	//APB1=DIV2;APB2=DIV1;AHB=DIV1;
//	RCC->CFGR |= PLL<<18;   	//����PLLֵ0-7
//	//RCC->CFGR |= 1<<16;	  		//PLLSRC ON 
//	FLASH->ACR |= 0x32;	  		//FLASH 2����ʱ����
//	RCC->CR |= 0x01000000;  	//PLLON
//	while(!(RCC->CR>>25));		//�ȴ�PLL����
//	RCC->CFGR |= 0x00000002;	//PLL��Ϊϵͳʱ��	 
//	while(temp != 0x02)     	//�ȴ�PLL��Ϊϵͳʱ�����óɹ�
//	{   
//		temp = RCC->CFGR>>2;
//		temp &= 0x03;
//	} 		    
//}
//#endif

/*************************************************************************************************************************
* ����	:	void DeviceClockEnable(u8 DEV_n,u8 Enable)
* ����	:	����ʱ��ʹ����ȡ��
* ����	:	������,EN=1ʹ��ʱ��,EN=0ȡ��ʱ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111113
* ����޸�ʱ�� : 20111113
* ˵��	: 	��
*************************************************************************************************************************/
void DeviceClockEnable(u8 DEV_n,u8 Enable)
{
	if(DEV_n & BIT5)	  //AHB����
	{
		DEV_n -= BIT5;
		if(Enable) //ʹ��
			RCC->AHBENR |= 1 << DEV_n;
		else   //ʧ��
			RCC->AHBENR &= ~(1 << DEV_n);
	}
	else if(DEV_n & BIT6) //APB2����
	{
		DEV_n -= BIT6;
		if(Enable) //ʹ��
			RCC->APB2ENR |= 1 << DEV_n;
		else   //ʧ��
			RCC->APB2ENR &= ~(1 << DEV_n);
	}
	else if(DEV_n & BIT7)  //APB1����
	{
		DEV_n -= BIT7;
		if(Enable) //ʹ��
			RCC->APB1ENR |= 1 << DEV_n;
		else   //ʧ��
			RCC->APB1ENR &= ~(1 << DEV_n);
	}
}


/********************************************************************************************************/
/*		����ʱ��		*/
// 	RCC_AHBENR |= 0x1;				//DMAʱ��ʹ��
//	RCC->APB2ENR |= 0x1 << 14;		//USART 1 ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 12;		//SPI 1 ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 11;		//TIM 1 ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 10;		//ADC 2 ʱ�ӽӿ�ʹ��
//	RCC_APB2ENR |= 0x1 << 9;		//ADC 1 ʱ�ӽӿ�ʹ��
//	RCC->APB2ENR |= 0x1 << 8;		//GPIO G ʱ��ʹ��
//	RCC->APB2ENR |= 0x1 << 7;		//GPIO F ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 6;		//GPIO E ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 5;		//GPIO D ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 4;		//GPIO C ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 3;		//GPIO B ʱ��ʹ��
//	RCC_APB2ENR |= 0x1 << 2;		//GPIO A ʱ��ʹ��
//	RCC_APB2ENR |= 0x1;				//���ù���AFIOʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 29;		//DAC ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 28;		//��Դ�ӿ�ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 27;		//���ݽӿ�ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 26;		//CANʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 23;		//USBʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 22;		//I2C 2 ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 21;		//I2C 1 ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 18;		//USART 3 ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 17;		//USART 2 ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 14;		//SPI 2 ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 11;		//���ڿ��Ź�ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 3;		//TIM5 ��ʱ��5ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 2;		//TIM4 ��ʱ��4ʱ��ʹ��
//	RCC_APB1ENR |= 0x1 << 1;		//TIM3 ��ʱ��3ʱ��ʹ��
//	RCC_APB1ENR |= 0x1;				//TIM2 ��ʱ��2ʱ��ʹ��





/*************************************************************************************************************************
* ����	:	void NVIC_IntEnable(u16 IRQ_n,u8 ENABLE)
* ����	:	NVICȫ���ж�ʹ�ܻ�ʧ��
* ����	:	�жϱ��,ʹ�ܻ�ʧ��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111113
* ����޸�ʱ�� : 20120403
* ˵��	: 	��
*************************************************************************************************************************/
void NVIC_IntEnable(u8 IRQ_n,u8 ENABLE)
{	
	u8 n = IRQ_n / 32;	 //��������,��֪��Ϊʲô.��Ҫ����
	if(ENABLE)
	{
		NVIC->ISER[n] = (1 << (IRQ_n % 32));//������Ӧ���ж�
	}
	else
	{
		NVIC->ICER[n] = (1 << (IRQ_n % 32));//�رն�Ӧ���ж�  
	}
}


/*************************************************************************************************************************
* ����	:	void  DeviceReset(u8 DEV_n)
* ����	:	APB1,APB2���踴λ
* ����	:	������
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111113
* ����޸�ʱ�� : 20111113
* ˵��	: 	��
*************************************************************************************************************************/
void  DeviceReset(u8 DEV_n)
{
 	if(DEV_n & BIT6) //APB2����
	{
		DEV_n -= BIT6;
		RCC->APB2RSTR |=  (1 << DEV_n);	//��λ
		nop;nop;nop;nop;
		RCC->APB2RSTR &= ~(1 << DEV_n);	//ȡ����λ
	}
	else if(DEV_n & BIT7)  //APB1����
	{
		DEV_n -= BIT7;
		RCC->APB1RSTR |=  (1 << DEV_n);	//��λ
		nop;nop;nop;nop;
		RCC->APB1RSTR &= ~(1 << DEV_n);	//ȡ����λ
	}
}



/*************************************************************************************************************************
* ����	:	void  EXTI_IntConfig(u8 GPIOx,u8 BITx,u8 TRIM)
* ����	:	�ⲿ�ж����ú���
* ����	:	GPIOx:GPIOѡ����GPIO_A,BITx:IOλѡ��0-15,TRIM:����	  //GPIOx:0~6,����GPIOA~G;(��GPIO_A)BITx:��Ҫʹ�ܵ�λ;TRIM:����ģʽ,NegEdge:�½���;PosEdge:������,Edge:���ش���
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111113
* ����޸�ʱ�� : 20120403
* ˵��	: 	ֻ���GPIOA~G;������PVD,RTC��USB����������
			�ú���һ��ֻ������1��IO��,���IO��,���ε���
			�ú������Զ�������Ӧ�ж�,�Լ������� 
*************************************************************************************************************************/
void EXTI_IntConfig(u8 GPIOx,u8 BITx,u8 TRIM)
{
	u8 EXTADDR;
	u8 EXTOFFSET;

	EXTADDR = BITx / 4;//�õ��жϼĴ�����ı��
	EXTOFFSET = (BITx % 4) * 4;
	RCC->APB2ENR |= 0x01;//ʹ��io����ʱ��
	AFIO->EXTICR[EXTADDR] &= ~(0x000F<<EXTOFFSET);//���ԭ�����ã�����
	AFIO->EXTICR[EXTADDR] |= GPIOx<<EXTOFFSET;//EXTI.BITxӳ�䵽GPIOx.BITx
	//�Զ�����
	EXTI->IMR |= 1<<BITx;//  ����line BITx�ϵ��ж�
	//EXTI->EMR|=1<<BITx;//������line BITx�ϵ��¼� (������������,��Ӳ�����ǿ��Ե�,���������������ʱ���޷������ж�!)
	//�������
	EXTI->RTSR &= ~(1<<BITx);
	EXTI->FTSR &= ~(1<<BITx);
 	if(TRIM & 0x01)
	{
		EXTI->FTSR |= 1<<BITx;//line BITx���¼��½��ش���
	}
	else if(TRIM & 0x02)
	{
		EXTI->RTSR |= 1<<BITx;//line BITx���¼��������ش���
	}
}


/*************************************************************************************************************************
* ����			:	void EXTI_IntMask(u8 line, bool isMask)
* ����			:	�ⲿ�ж�����
* ����			:	line:�ж���0-15�� isMask:�Ƿ������ж�
* ����			:	��
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2016-01-11
* ����޸�ʱ��	: 	2016-01-11
* ˵��			: 	���������жϣ������ж��޷��ر�
					һֱ�������������EXTI->RTSR��EXTI->FTSR��2�������Ĵ������ر��жϣ����ֲ�û��ʲô��
*************************************************************************************************************************/
void EXTI_IntMask(u8 line, bool isMask)
{
	if(line > 15) return;
	if(isMask == TRUE)	//�����ж�
	{
		EXTI->IMR &= ~(1<<line);
	}
	else
	{
		EXTI->IMR |= 1<<line;
	}
}

		    
/*************************************************************************************************************************
* ����	:	void EXTI_ClearInt(u8 IRQ_n)
* ����	:	����ⲿ�жϱ�־
* ����	:	�жϺ�
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	20111113
* ����޸�ʱ�� : 20120403
* ˵��	: 	���19���ж��ߵĶ�Ӧ�жϱ�־λ
*************************************************************************************************************************/
void EXTI_ClearInt(u8 IRQ_n)
{
	EXTI->PR = 1 << IRQ_n;
}



/**********************************************************************************************************/
/*	JTAGģʽ����,��������JTAG��ģʽ*/		
//mode:jtag,swdģʽ����;00,ȫʹ��;01,ʹ��SWD;10,ȫ�ر�;		  
void JTAG_Set(u8 mode)
{
 	u32 temp;
	RCC->APB2ENR |= 1 << 0;    //��������ʱ��
	temp = mode;
	temp <<= 25;
	//RCC_APB2ENR |= 1 << 0;     //��������ʱ��	   
	AFIO->MAPR &= 0XF8FFFFFF; //���MAPR��[26:24]
	AFIO->MAPR |= temp;       //����jtagģʽ
}


/*************************************************************************************************************************
* ����	:	void SYSTEM_ReadCPUID(u8 id[12])
* ����	:	��ȡ96λ��Ψһid
* ����	:	id������
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com
* ʱ��	:	2017-11-17
* ����޸�ʱ�� : 2017-11-17
* ˵��	: 	��ȡSTM32��Ψһid
*************************************************************************************************************************/
void SYSTEM_ReadCPUID(u8 id[12])
{
	u8 i;
	u8 *p =  (u8 *)0x1FFFF7E8;
	
	for(i = 0;i < 12;i ++)
	{
		id[i] = p[i];
	}
}


/**************************************************************************************************************/
/*����������ƫ�Ƶ�ַ*/	
//NVIC_VectTab:��ַ
//Offset:ƫ����
//CHECK OK
//091207
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 
  	//�������Ϸ���
//	assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
//	assert_param(IS_NVIC_OFFSET(Offset));  	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);//����NVIC��������ƫ�ƼĴ���
	//���ڱ�ʶ����������CODE��������RAM��
}
//����NVIC����
//NVIC_Group:NVIC���� 0~4 �ܹ�5�� 
//CHECK OK
//091209
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//ȡ����λ
	temp1<<=8;
	temp=SCB->AIRCR;  //��ȡ��ǰ������
	temp&=0X0000F8FF; //�����ǰ����
	temp|=0X05FA0000; //д��Կ��
	temp|=temp1;	   
	SCB->AIRCR=temp;  //���÷���	    	  				   
}
//����NVIC 
//NVIC_PreemptionPriority:��ռ���ȼ�
//NVIC_SubPriority       :��Ӧ���ȼ�
//NVIC_Channel           :�жϱ��
//NVIC_Group             :�жϷ��� 0~4
//ע�����ȼ����ܳ����趨����ķ�Χ!����������벻���Ĵ���
//�黮��:
//��0:0λ��ռ���ȼ�,4λ��Ӧ���ȼ�
//��1:1λ��ռ���ȼ�,3λ��Ӧ���ȼ�
//��2:2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
//��3:3λ��ռ���ȼ�,1λ��Ӧ���ȼ�
//��4:4λ��ռ���ȼ�,0λ��Ӧ���ȼ�
//NVIC_SubPriority��NVIC_PreemptionPriority��ԭ����,��ֵԽС,Խ����
//CHECK OK
//100329
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	u8 IPRADDR=NVIC_Channel/4;  //ÿ��ֻ�ܴ�4��,�õ����ַ 
	u8 IPROFFSET=NVIC_Channel%4;//�����ڵ�ƫ��
	IPROFFSET=IPROFFSET*8+4;    //�õ�ƫ�Ƶ�ȷ��λ��
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//���÷���
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;//ȡ����λ

	if(NVIC_Channel<32)NVIC->ISER[0]|=1<<NVIC_Channel;//ʹ���ж�λ(Ҫ����Ļ�,�෴������OK)
	else NVIC->ISER[1]|=1<<(NVIC_Channel-32);    
	NVIC->IPR[IPRADDR]|=temp<<IPROFFSET;//������Ӧ���ȼ����������ȼ�   	    	  				   
}

//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI
//CHECK OK
//091209
__asm void WFI_SET(void)
{
	WFI;    
}

__asm void WFE_SET(void)
{
	WFE;    
}


//�����ж�
void SYS_EnableIrq(void)         
{ 
	__asm("cpsie f"); 
	//CPSIE F;
}

//�ر����ж�
void SYS_DisableIrq()        
{ 
	__asm ("cpsid f"); 
}

//��ת����
//��Ƭ����˯��ģʽ���Ѻ���Զ���ת���˴���ԭ��δ֪����ת���Զ�����
__asm void RETURN(void) 
{
    BX r14
}


//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


/*************************************************************************************************************************
* ����	:	void SYSTEM_Standby(void)
* ����	:	STM32�������ģʽ,��Ҫ�ⲿPA0���л���
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com  
* ʱ��	:	2013-11-14
* ����޸�ʱ�� : 2013-11-14
* ˵��	: 	��С����ģʽ
*************************************************************************************************************************/
void SYSTEM_Standby(void)
{
	SCB->SCR|=1<<2;//ʹ��SLEEPDEEPλ (SYS->CTRL)	 
  
	RCC->APB1ENR|=1<<28;     //ʹ�ܵ�Դʱ��	    
 	PWR->CSR|=1<<8;          //����WKUP���ڻ���
	PWR->CR|=1<<2;           //���Wake-up ��־
	PWR->CR|=1<<1;           //PDDS��λ		  
	WFI_SET();				 //ִ��WFIָ��		 
}

/*
λ�� ���� ���� ��λֵ ����
4 SEVONPEND RW �\ �����쳣����ʱ�뷢���¼���������һ���µ���
������ʱ��WFE ָ����ѡ���������жϵ�
���ȼ��Ƿ�ȵ�ǰ�ĸߣ������ѡ����û��WFE
����˯�ߣ����´�ʹ��WFE ʱ����������
3 ���� �\ �\ �\
2 SLEEPDEEP R/W 0 ������˯��ģʽʱ��ʹ���ⲿ��SLEEPDEEP �źţ�
������ֹͣϵͳʱ��
1 SLEEPONEXIT R/W �\ ���SleepOnExit������
0 ���� �\ �\ �\
ͨ��ִ��WFI/WFE ָ�����CM3 ����˯��ģʽ��������CM3 ��*/
//����SLEEPģʽ
/*************************************************************************************************************************
* ����	:	void SYSTEM_Standby(void)
* ����	:	STM32����ȴ�ģʽ,��Ҫ�¼�����
* ����	:	��
* ����	:	��
* ����	:	�ײ�궨��
* ����	:	cp1300@139.com  
* ʱ��	:	2013-11-14
* ����޸�ʱ�� : 2013-11-14
* ˵��	: 	2016-01-06:˯��ģʽ�ر�SRAM��FLASHʱ��,��������Դʱ��
*************************************************************************************************************************/
void SYSTEM_Sleep(void)
{
	SCB->SCR &= ~BIT2;		//��ֹ�ر�ʱ��
	SCB->SCR |= BIT4;		//ʹ��WFE����
	SCB->SCR &= ~BIT1;		//��ֹSleepOnExit����,ʱ������֮���ѵ�Ƭ��
	//RCC->APB1ENR|=1<<28;     //ʹ�ܵ�Դʱ��	    
 	PWR->CSR|=1<<8;          //����WKUP���ڻ���
	PWR->CR|=1<<2;           //���Wake-up ��־
	PWR->CR|=1<<1;           //PDDS��λ	

	RCC->AHBENR &= ~BIT2;	//˯��ģʽSRAMʱ�ӹر�
	RCC->AHBENR &= ~BIT4;	//˯��ģʽflashʱ�ӹر�
	
	WFE_SET();				 //ִ��WFIָ��		 
}


//��ȡ����ϵͳ����ʱ��
u64 SYS_GetOSRunTime(void)
{
	return g_OS_RunTime;
}


//ϵͳ����λ
//CHECK OK
//091209
void SYSTEM_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 


//��ȡ����ϵͳ����״̬
bool SYS_GetOsStartup(void)
{
#if(UCOS_II_EN)	//ʹ���˲���ϵͳ
	return (OSRunning==OS_FALSE)?FALSE:TRUE;//gs_isSystemStartup;
#else
	return FALSE;
#endif //UCOS_II_EN
}



/*************************************************************************************************************************
* ����			:	void SYS_DelayMS(u32 ms)
* ����			:	ϵͳ������ʱ
* ����			:	��
* ����			:	��
* ����			:	�ײ�궨�壬����OS
* ����			:	cp1300@139.com
* ʱ��			:	2018-03-10
* ����޸�ʱ�� 	: 	2018-03-10
* ˵��			:	���û��ʹ�ò���ϵͳ��ʹ��Delay_MS()��ʱ����������˲���ϵͳ�����Ҳ���ϵͳ�����ˣ���ʹ��OSTimeDlyHMSM��ʱ
*************************************************************************************************************************/
void SYS_DelayMS(u32 ms)
{
#ifdef _UCOS_II_	//ʹ���˲���ϵͳ
	if(OSRunning==OS_FALSE) //����ϵͳ��û�г�ʼ��
	{
		Delay_MS(ms);
	}
	else
	{
		OSTimeDlyHMSM((ms/1000)/3600, ((ms/1000)%3600)/60, (ms/1000)%60, ms%1000);
	}
#else //û��ʹ�ܲ���ϵͳ
	Delay_MS(ms);
#endif //UCOS_II_EN
}




#ifdef _UCOS_II_
#include "ucos_ii.h"

//ϵͳʱ���жϷ�����
void SysTick_Handler(void)
{
	OS_CPU_SR  cpu_sr;

	OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntNesting++;
	g_OS_RunTime ++;		//����ϵͳ����ʱ������ 2018-02-08
    OS_EXIT_CRITICAL();

    OSTimeTick();        /* Call uC/OS-II's OSTimeTick()               */

    OSIntExit();         /* Tell uC/OS-II that we are leaving the ISR  */
}



//ϵͳʱ�����ã����1ms����һ���ж�
void SysTick_Configuration(u32 SYS_CLK)
{
 	SysTick->CTRL&=~(1<<2);//SYSTICKʹ���ⲿʱ��Դ
	SysTick->CTRL&=0xfffffffb;//bit2���,ѡ���ⲿʱ��  HCLK/8
	SysTick->CTRL|=1<<1;   //����SYSTICK�ж�
	SysTick->LOAD=SYS_CLK / 8 / 1000;    //����1ms�ж�
	//MY_NVIC_Init(1,2,SystemHandler_SysTick,1);//��1��������ȼ� 
	SysTick->CTRL|=1<<0;   //����SYSTICK
}
#else
//ϵͳʱ�����ã�����1us��������
void SysTick_Configuration(u32 SYS_CLK)
{
 	SysTick->CTRL&=~(1<<2);		//SYSTICKʹ���ⲿʱ��Դ
	SysTick->CTRL&=0xfffffffb;	//bit2���,ѡ���ⲿʱ��  HCLK/8
	SysTick->CTRL&=~(1<<1);   	//�ر�SYSTICK�ж�
	//SysTick->LOAD=SYS_CLK / 8 / 1000;    //����1ms�ж�
	//MY_NVIC_Init(1,2,SystemHandler_SysTick,1);//��1��������ȼ� 
	SysTick->CTRL|=1<<0;   //����SYSTICK
}

#endif









//�����ϵͳ��atof()��������󳤶�����16�ֽ�
#include "string.h"
double user_atof(const char *pStr)
{
	double ftemp = 0;
	u32 Integer = 0;
	u32 Decimal = 0;			//С��
	u8 s=0;
	u8 i;
    u8 len = strlen(pStr);
	u8 num = 0;					//��¼С��λ��
    bool isDecimal = FALSE;
    bool isNegative = FALSE;	//�Ƿ�Ϊ����
    
    if(len > 16) len = 16;
    if(pStr[0]=='-') 
    {
		s = 1;	//�������ӵ�1λ��ʼ��
		isNegative = TRUE;	//����
    }
    for(i = s;i < len;i ++)
    {
		if(((pStr[i] < '0') || (pStr[i] > '9')) && (pStr[i] != '.')) break;
        if(pStr[i] == '.')	//�ظ���С����ֱ���˳�
        {
			if(isDecimal==FALSE)
            {
				isDecimal = TRUE;
                continue;
            }
				
            else break;
        }
        if(isDecimal == FALSE)	//��������
        {
			Integer *= 10;
            Integer += pStr[i]-'0';
        }
        else	//С������
        {
			Decimal *= 10;
			Decimal += pStr[i]-'0';
            num ++;	//С��λ������
        }
    }
    //printf("decimal=%d num=%d\r\n",Decimal,num);
    //����С��
    ftemp = Decimal;
    for(i = 0;i < num;i ++)
    {
		ftemp /= 10.0;
		//printf("ftemp=%f i=%d\r\n",ftemp,i);
		
    }
    //������������
    ftemp += Integer;
    //����Ǹ���
    if(isNegative == TRUE)
    {
		ftemp = 0-ftemp;
    }
    
    //printf("%s->%f\r\n",pStr,ftemp);
    return ftemp;
}






 /*******************************************************************************
 *�������ƣ�CRC16
 *�������ܣ�CRC16Ч�麯��
 *����˵����*pЧ��֡��ָ��   ֡�� datalen ������У��λ
 *�� �� ֵ��Ч����
 *ע���������ʽ��0xA001
 *******************************************************************************/
u16 CRC16(const u8 *p, u16 datalen)
{
    u8 CRC16Lo,CRC16Hi,CL,CH,SaveHi,SaveLo;
    u16 i,Flag;

    CRC16Lo = 0xFF;     CRC16Hi = 0xFF;
    CL = 0x01;          CH = 0xA0;
    for(i = 0;i < datalen; i++)
    {
        CRC16Lo ^= *(p+i);                  //ÿһ��������CRC�Ĵ����������
        for(Flag = 0; Flag < 8; Flag++)
        {
            SaveHi = CRC16Hi;  SaveLo = CRC16Lo;
            CRC16Hi >>= 1; CRC16Lo >>= 1;   //��λ����һλ����λ����һλ
            if((SaveHi & 0x01) == 0x01)     //�����λ�ֽ����һλΪ1
            CRC16Lo  |=0x80 ;     			//���λ�ֽ����ƺ�ǰ�油1�����Զ���0
            if((SaveLo & 0x01) == 0x01)     //���LSBΪ1���������ʽ��������
            { CRC16Hi ^= CH;  CRC16Lo ^= CL; }
        }
    }
    return (u16)(CRC16Hi<<8)|CRC16Lo;
}




//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if PRINTF_EN_
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
#endif




//////////////////////////////////////////////////////////////////
//???????��???,???printf????,
//PRINTF_EN == 1,?????printf??????
#if (PRINTF_EN_ == 1)
#include "usart.h" 
bool isPrintfOut = TRUE;	//printf���ʹ��
UART_CH_Type PrintfCh = UART_PRINTF_CH;	//printfͨ������

int fputc(int ch,FILE *f)
{     
	if(isPrintfOut == TRUE)	//ʹ�����
	{
		UARTx_SendByte(PrintfCh,	(u8)ch);  
	}
	return ch;
}


#endif

//PRINTF_EN == 2,?????printf?????
#if (PRINTF_EN_== 2)
#include "lcd.h"

int fputc(int ch, FILE *f)
{    
	static u16 X;
	static u16 Y;
	 	
	if(Y > LCD_Y_SIZE - 1)
	{
		LCD_ClearScreen(0xffff);
		X = 0;
		Y = 0;
	}
	if((u8)ch == '\n')
	{
	 	X = 0;
		Y += 12;
	}
	else if((u8)ch > 0x80) //����
	{
		return ch;
	}
	else
	{
		LCD_Char(X,Y,(u8 *)&ch,0x0000,0xffff,0x80 | 12);
		if(X > LCD_X_SIZE - 9)
		{
		 	X = 0;
			Y += 12;
		}
		else
			X += 8;
	}     

	return ch;
}
#endif


//PRINTF_EN == 3,???????printf??????????
#if (PRINTF_EN_ == 3)
#include "lcd.h"
#include "usart.h"
#include <locale.h>
u8 PrintfSet = 0;	//0:????printf??????;1:????printf?????
UART_CH_Type PrintfCh = UART_PRINTF_CH;	//printfͨ������

int fputc(int ch, FILE *f)
{    
	static u16 X;
	static u16 Y;
 	
	if(PrintfSet)
	{
		if(Y > LCD_Y_SIZE - 1)
		{
			LCD_ClearScreen(0xffff);
			X = 0;
			Y = 0;
		}
		if((u8)ch == '\n')
		{
		 	X = 0;
			Y += 12;
		}
		else if((u8)ch > 0x80) //����
		{
			return ch;
		}
		else
		{
			LCD_DispChar(X,Y,(char *)&ch,0x0000,0xffff,0x80 | 12);
			if(X > LCD_X_SIZE - 9)
			{
			 	X = 0;
				Y += 12;
			}
			else
				X += 8;
		}
	}
	else
		UARTx_SendByte(PrintfCh, (u8)ch);

	return ch;
}
#endif



#if PRINTF_EN_
int fgetc(FILE *f)
{
	u8 data;
	static u8 flag = 0;
	u8 gbk[2];
	
	while(UARTx_GetNewDataFlag(UART_PRINTF_CH) == FALSE)
	{
#ifdef _UCOS_II_
		OSTimeDly(2);
#else
		Delay_US(10);
#endif //_UCOS_II_
	}
	data = UARTx_GetNewData(UART_PRINTF_CH);
	
	if((data < 0x80) && (flag == 0))				//ASCII
	{
		UARTx_SendByte(UART_PRINTF_CH, data);		//����
		if(data == '\r')
			UARTx_SendByte(UART_PRINTF_CH,'\n');	//����
		else if(data == 8)			//�˸�
		{
			UARTx_SendByte(UART_PRINTF_CH,' ');
			UARTx_SendByte(UART_PRINTF_CH, 8);		
		}
	}
	else	//����
	{
		gbk[flag] = data;
		flag ++;
		if(flag == 2)
		{
			flag = 0;
			UARTx_SendByte(UART_PRINTF_CH,gbk[0]);
			UARTx_SendByte(UART_PRINTF_CH,gbk[1]);
		}
	}
			
	return (int)data;
}

#endif //PRINTF_EN_



//__use_no_semihosting was requested, but _ttywrch was 
//2018-02-07 ��ʹ����cJSON���ᱨ������Ҫ���Ӵ˺���
_ttywrch(int ch)
{
ch = ch;
}


/*
#if	MALLOC
#pragma import(__use_realtime_heap)
//���������rt_heap.h������,��Ҫ�û��Լ�ȥʵ��,��������ֵ����
unsigned __rt_heap_extend(unsigned size, void **block)
{
     return 0;
}
#endif
*/