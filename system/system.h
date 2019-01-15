/*************************************************************************************************************
 * �ļ���:		system.h
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

#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "stm32f103_map.h"
#include "typedef.h"
#include "delay.h"
#include "gpio_init.h"
#include "stdio.h"


//�꿪��
#define ENABLE_HSI			1			//ʹ���ڲ�����ʱ��
#define _UCOS_II_ 						//�Ƿ�ʹ��UCOS_II����ϵͳ
#define PRINTF_EN_ 			1		 	//printf����ض��� ,0:�ر�,1:����,2:LCD,3:ͬʱ�����ں�LCD
#define UART_PRINTF_CH		UART_CH1	//uart printf����ѡ��
#define SYS_CONFIG_EN_		0			//ϵͳ���ô洢���ع���,��Ҫg_SYS_Config.c֧��
#define SYS_MESSAGE_EN_		0			//ʹ��ϵͳ��Ϣ
#define SYS_CMD_EN_			0			//ʹ��ϵͳ������
#define SYS_WDG_EN_			1			//ʹ��ϵͳ���Ź�
#define BOARD_SUPPORT		1			//�弶֧�֣������ض�ϵ�п�����

extern const char *const gc_ClockSourceString[];	//ʱ����Դ����

#ifdef _UCOS_II_
#include "ucos_ii.h"

#endif 


//��ָ��,�궨��
#define nop __nop()  //��ָ��  


//�����ж�
void SYS_EnableIrq(void);        

//�ر����ж�
void SYS_DisableIrq(void);

//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr);

/*************************************************************************************************/
/*	��������	*/
bool SYSTEM_ClockInit(FunctionalState ExtClockEnable, FunctionalState PLLEnable, u8 PLL); //ʱ�ӳ�ʼ�� PLL = 2 - 9;��ӦƵ��Ϊ8 - 72MHz
void EXTI_IntConfig(u8 GPIOx,u8 BITx,u8 TRIM);	//ֻ���GPIOA~G,�ⲿ�ж����ú��� ��GPIO_A 0
void NVIC_IntEnable(u8 IRQ_,u8 Enable);			//NVICȫ���ж�ʹ�ܻ�ʧ��
void EXTI_ClearInt(u8 IRQ_n);					//����ⲿ�жϱ�־,���19���ж��ߵĶ�Ӧ�жϱ�־λ
void JTAG_Set(u8 mode);/*	JTAGģʽ����,��������JTAG��ģʽ		*///mode:jtag,swdģʽ����;00,ȫʹ��;01,ʹ��SWD;10,ȫ�ر�;	
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset);	//����ƫ�Ƶ�ַ
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);			//����NVIC����
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group);	//�����ж�
void SYSTEM_Standby(void);						//STM32�������ģʽ,��Ҫ�ⲿPA0���л���	
void SYSTEM_Sleep(void);						//����ȴ�ģʽ,��Ҫ�¼�����
void DeviceClockEnable(u8 DEV_,u8 Enable);		//����ʱ��ʹ����ȡ��
void DeviceReset(u8 DEV_);						//APB1,APB2���踴λ
void SYSTEM_SoftReset(void);					//ϵͳ��λ
u32 SYSTEM_GetClkSpeed(void);					//��ȡϵͳʱ���ٶ�
void SysTick_Configuration(u32 SYS_CLK);		//ϵͳʱ�����ã����1ms����һ���ж�
void EXTI_IntMask(u8 line, bool isMask);		//�ⲿ�ж�������
double user_atof(const char *pStr);				//�����ϵͳ��atof()��������󳤶�����16�ֽ�
u16 CRC16(const u8 *p, u16 datalen);					//CRC16
void SYSTEM_ReadCPUID(u8 id[12]);				//��ȡ12�ֽ�96bit�ĵ�Ƭ��Ψһid
u64 SYS_GetOSRunTime(void);						//��ȡ����ϵͳ����ʱ��
bool SYS_GetOsStartup(void);					//��ȡ����ϵͳ����״̬
void SYS_DelayMS(u32 ms);						//ͳһϵͳ��ʱ�ӿ�


extern const vu8 *g_CPU_ID;						//CPU ID,ֻ��
/******************************************************************************************************/
//Ex_NVIC_Configר�ö���
#define GPIO_A 0
#define GPIO_B 1
#define GPIO_C 2
#define GPIO_D 3
#define GPIO_E 4
#define GPIO_F 5
#define GPIO_G 6

//ϵͳ��ʱ����Դ
typedef enum
{
	SYS_CLOCK_HSI		=	0,	//�ڲ�ʱ�ӣ���PLL
	SYS_CLOCK_HSE		=	1,	//�ⲿʱ�ӣ���PLL
	SYS_CLOCK_HSI_PLL	=	2,	//�ڲ�ʱ�ӣ�PLL
	SYS_CLOCK_HSE_PLL	=	3,	//�ⲿʱ�ӣ�PLL
}CLOCK_SOURCE;
CLOCK_SOURCE SYS_GetSystemClockSource(void);	//��ȡϵͳʱ����Դ

//�жϴ�������
#define OFF_INT		0	//�ر��ж�
#define NegEdge   	1  	//�½��ش���
#define PosEdge   	2  	//�����ش���
#define Edge   		3  	//���ش���

//JTAGģʽ���ö���
#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00

//������,��������,��������ʱ������
//AHB
#define DEV_DMA1		(BIT5 | 0)	  
#define DEV_DMA2		(BIT5 | 1)  
#define DEV_SRAM		(BIT5 | 2)	  
#define DEV_FLITF		(BIT5 | 4) 	 //����ӿ�ʱ��ʹ��
#define DEV_CRC		 	(BIT5 | 6)
#define DEV_FSMC		(BIT5 | 8) 
#define DEV_SDIO		(BIT5 | 10)
//APB2		 			
#define DEV_AFIO		(BIT6 | 0) 
#define DEV_GPIOA		(BIT6 | 2)  
#define DEV_GPIOB		(BIT6 | 3) 
#define DEV_GPIOC		(BIT6 | 4)  
#define DEV_GPIOD		(BIT6 | 5)
#define DEV_GPIOE		(BIT6 | 6)  
#define DEV_GPIOF		(BIT6 | 7) 
#define DEV_GPIOG		(BIT6 | 8)  
#define DEV_ADC1		(BIT6 | 9) 
#define DEV_ADC2		(BIT6 | 10)  
#define DEV_TIM1		(BIT6 | 11)  
#define DEV_SPI1		(BIT6 | 12) 
#define DEV_TIM8		(BIT6 | 13) 
#define DEV_USART1		(BIT6 | 14) 
#define DEV_ADC3		(BIT6 | 15)  
//APB1
#define DEV_TIM2		(BIT7 | 0)  
#define DEV_TIM3		(BIT7 | 1) 
#define DEV_TIM4		(BIT7 | 2)  
#define DEV_TIM5		(BIT7 | 3)   
#define DEV_TIM6		(BIT7 | 4) 
#define DEV_TIM7		(BIT7 | 5)  
#define DEV_WWDG		(BIT7 | 11)  
#define DEV_SPI2		(BIT7 | 14)   
#define DEV_SPI3		(BIT7 | 15)  
#define DEV_USART2		(BIT7 | 17)  
#define DEV_USART3		(BIT7 | 18)   
#define DEV_UART4		(BIT7 | 19) 	  
#define DEV_UART5		(BIT7 | 20) 
#define DEV_I2C1		(BIT7 | 21) 	  
#define DEV_I2C2		(BIT7 | 22) 	 
#define DEV_USB			(BIT7 | 23)   
#define DEV_CAN		  	(BIT7 | 25) 
#define DEV_BKP		  	(BIT7 | 27)  //������
#define DEV_PWR		    (BIT7 | 28)  //��Դ�ӿ�
#define DEV_DAC		    (BIT7 | 29)

//�жϱ�ź궨��
#define IRQ_WWDG 				  0//���ڶ�ʱ���ж�
#define IRQ_PVD					  1//��Դ��ѹ���(PVD)�ж�
#define IRQ_TAMPER				  2//�������ж�
#define IRQ_RTC					  3//ʵʱʱ��(RTC)ȫ���ж�
#define IRQ_FLASH				  4//����ȫ���ж�
#define IRQ_RCC					  5//��λ��ʱ�ӿ���(RCC)�ж�
#define IRQ_EXTI0				  6//EXTI��0�ж�
#define IRQ_EXTI1				  7//EXTI��1�ж�
#define IRQ_EXTI2				  8//EXTI��2�ж�
#define IRQ_EXTI3				  9//EXTI��3�ж�
#define IRQ_EXTI4				  10//EXTI��4�ж�
#define IRQ_DMA1_1				  11//DMA1ͨ��1
#define IRQ_DMA1_2				  12//DMA1ͨ��1
#define IRQ_DMA1_3				  13//DMA1ͨ��1
#define IRQ_DMA1_4				  14//DMA1ͨ��1
#define IRQ_DMA1_5				  15//DMA1ͨ��1
#define IRQ_DMA1_6				  16//DMA1ͨ��1
#define IRQ_DMA1_7				  17//DMA1ͨ��1
#define IRQ_ADC					  18//ADCȫ���ж�
#define IRQ_USB_HP_CAN_TX		  19//USB�����ȼ���CAN�����ж�
#define IRQ_USB_LP_CAN_RX0		  20//USB�����ȼ���CAM����0�ж�
#define IRQ_CAN_RX1				  21//CAN����1�ж�
#define IRQ_CAN_SCE				  22//CAN SCE�ж�
#define IRQ_EXTI9_5				  23//EXTI��[9:5]�ж�
#define IRQ_TIM1_BRK			  24//TIM1�Ͽ��ж�
#define IRQ_TIM1_UP				  25//TIM1�����ж�
#define IRQ_TIM1_TRG_COM		  26//TIM1������ͨ���ж�
#define IRQ_TIM1_CC				  27//TIM1����Ƚ��ж�
#define IRQ_TIM2				  28//TIM2ȫ���ж�
#define IRQ_TIM3				  29//TIM3ȫ���ж�
#define IRQ_TIM4				  30//TIM4ȫ���ж�
#define IRQ_I2C1_EV				  31//I2C1�¼��ж�
#define IRQ_I2C1_ER				  32//I2C1�����ж�
#define IRQ_I2C2_EV				  33//I2C2�¼��ж�
#define IRQ_I2C2_ER				  34//I2C2�����ж�
#define IRQ_SPI1				  35//SPI1ȫ���ж�
#define IRQ_SPI2				  36//SPI2ȫ���ж�
#define IRQ_USART1				  37//USART1ȫ���ж�
#define IRQ_USART2				  38//USART2ȫ���ж�
#define IRQ_USART3				  39//USART3ȫ���ж�
#define IRQ_EXTI15_10			  40//EXTI��10-15�ж�
#define IRQ_RTCAlarm			  41//EXTI��RTC�����ж�
#define IRQ_USBRouse			  42//ECTI�Ĵ�USB���������ж�
#define IRQ_TIM8_BRK			  43//TIM8�Ͽ��ж�
#define IRQ_TIM8_UP				  44//TIM8�����ж�
#define IRQ_TIM8_TRG_COM		  45//TIM8������ͨ���ж�
#define IRQ_TIM8_CC				  46//TIM8����Ƚ��ж�
#define IRQ_ADC3				  47//ADC3ȫ���ж�
#define IRQ_FSMC				  48//FSMCȫ���ж�
#define IRQ_SDIO				  49//SDIOȫ���ж�
#define IRQ_TIM5				  50//TIM5ȫ���ж�
#define IRQ_SPI3				  51//SPI3ȫ���ж�
#define IRQ_UART4				  52//UART4ȫ���ж�
#define IRQ_UART5				  53//UART5ȫ���ж�
#define IRQ_TIM6				  54//TIM6ȫ���ж�
#define IRQ_TIM7				  55//TIM7ȫ���ж�
#define IRQ_DMA2_1				  56//DMA2ͨ��1ȫ���ж�
#define IRQ_DMA2_2				  57//DMA2ͨ��2ȫ���ж�
#define IRQ_DMA2_3				  58//DMA2ͨ��3ȫ���ж�
#define IRQ_DMA2_4_5			  59//DMA2ͨ��4,5ȫ���ж�


/********************************************************************************************************/
//GPIO ͨ��IO���Ĵ���ƫ�Ƶ�ַ
#define GPIOx_CRL				0x00			  //�˿����õͼĴ���
#define GPIOx_CRH				0x04			  //�˿����ø߼Ĵ���
#define GPIOx_IDR				0x08			  //�˿��������ݼĴ���
#define GPIOx_ODR				0x0c			  //�˿�������ݼĴ���
#define GPIOx_BSRR				0x10			  //�˿�λ��λ/��λ�Ĵ���
#define GPIOx_BRR				0x14			  //�˿ڸ�λ�Ĵ���
#define GPIOx_LCKR				0x18			  //�˿����������Ĵ���

//NVIC Ƕ�������ж�ʹ�ܳ��ܼĴ���
//ʹ��
#define NVIC_SETENA0 		*((volatile unsigned long *)(0xe000e100))	 //�ж�31-0ʹ�ܼĴ���	 	 ��32��ʹ��λ
#define NVIC_SETENA1 		*((volatile unsigned long *)(0xe000e104))	 //�ж�63-32ʹ�ܼĴ���	 ��32��ʹ��λ
#define NVIC_SETENA2 		*((volatile unsigned long *)(0xe000e108))	 //�ж�95-64ʹ�ܼĴ���	 ��32��ʹ��λ
#define NVIC_SETENA3 		*((volatile unsigned long *)(0xe000e10c))	 //�ж�127-96ʹ�ܼĴ���	 ��32��ʹ��λ
#define NVIC_SETENA4 		*((volatile unsigned long *)(0xe000e110))	 //�ж�159-128ʹ�ܼĴ���	 ��32��ʹ��λ
#define NVIC_SETENA5 		*((volatile unsigned long *)(0xe000e114))	 //�ж�191-160ʹ�ܼĴ���	 ��32��ʹ��λ
#define NVIC_SETENA6 		*((volatile unsigned long *)(0xe000e118))	 //�ж�223-192ʹ�ܼĴ���	 ��32��ʹ��λ
#define NVIC_SETENA7 		*((volatile unsigned long *)(0xe000e11c))	 //�ж�239-224ʹ�ܼĴ���	 ��16��ʹ��λ
//����
#define NVIC_CLENA0 		*((volatile unsigned long *)(0xe000e180))	 //�ж�31-0���ܼĴ���	 	 ��32������λ
#define NVIC_CLENA1			*((volatile unsigned long *)(0xe000e184))	 //�ж�63-32���ܼĴ���	 ��32������λ
#define NVIC_CLENA2			*((volatile unsigned long *)(0xe000e188))	 //�ж�95-64���ܼĴ���	 ��32������λ
#define NVIC_CLENA3 		*((volatile unsigned long *)(0xe000e18c))	 //�ж�127-96���ܼĴ���	 ��32������λ
#define NVIC_CLENA4 		*((volatile unsigned long *)(0xe000e190))	 //�ж�159-128���ܼĴ���	 ��32������λ
#define NVIC_CLENA5 		*((volatile unsigned long *)(0xe000e194))	 //�ж�191-160���ܼĴ���	 ��32������λ
#define NVIC_CLENA6 		*((volatile unsigned long *)(0xe000e198))	 //�ж�223-192���ܼĴ���	 ��32������λ
#define NVIC_CLENA7 		*((volatile unsigned long *)(0xe000e19c))	 //�ж�239-224���ܼĴ���	 ��16������λ
/*************************************************************************************************/
/*	λ��������	*/
//IO�ڲ����궨�� λ������,ʵ��51���Ƶ�GPIO���ƹ���
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

//IO�ڲ���,ֻ�Ե�һ��IO��!
//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR((GPIOA_BASE + GPIOx_ODR),n)  //��� 
#define PAin(n)    BIT_ADDR((GPIOA_BASE + GPIOx_IDR),n)  //���� 

#define PBout(n)   BIT_ADDR((GPIOB_BASE + GPIOx_ODR),n)  //��� 
#define PBin(n)    BIT_ADDR((GPIOB_BASE + GPIOx_IDR),n)  //���� 

#define PCout(n)   BIT_ADDR((GPIOC_BASE + GPIOx_ODR),n)  //��� 
#define PCin(n)    BIT_ADDR((GPIOC_BASE + GPIOx_IDR),n)  //���� 

#define PDout(n)   BIT_ADDR((GPIOD_BASE + GPIOx_ODR),n)  //��� 
#define PDin(n)    BIT_ADDR((GPIOD_BASE + GPIOx_IDR),n)  //���� 

#define PEout(n)   BIT_ADDR((GPIOE_BASE + GPIOx_ODR),n)  //��� 
#define PEin(n)    BIT_ADDR((GPIOE_BASE + GPIOx_IDR),n)  //����

#define PFout(n)   BIT_ADDR((GPIOF_BASE + GPIOx_ODR),n)  //��� 
#define PFin(n)    BIT_ADDR((GPIOF_BASE + GPIOx_IDR),n)  //����

#define PGout(n)   BIT_ADDR((GPIOG_BASE + GPIOx_ODR),n)  //��� 
#define PGin(n)    BIT_ADDR((GPIOG_BASE + GPIOx_IDR),n)  //����

/*************************************************************************************************/
/*			��Ӧλ����������λ����		*/
#define BIT0	(0x0001 << 0)
#define BIT1	(0x0001 << 1)
#define BIT2	(0x0001 << 2)
#define BIT3	(0x0001 << 3)
#define BIT4	(0x0001 << 4)
#define BIT5	(0x0001 << 5)
#define BIT6	(0x0001 << 6)
#define BIT7	(0x0001 << 7)
#define BIT8	(0x0001 << 8)
#define BIT9	(0x0001 << 9)
#define BIT10	(0x0001 << 10)
#define BIT11	(0x0001 << 11)
#define BIT12	(0x0001 << 12)
#define BIT13	(0x0001 << 13)
#define BIT14	(0x0001 << 14)
#define BIT15	(0x0001 << 15)
#define BIT16	(0x00000001 << 16)
#define BIT17	(0x00000001 << 17)
#define BIT18	(0x00000001 << 18)
#define BIT19	(0x00000001 << 19)
#define BIT20	(0x00000001 << 20)
#define BIT21	(0x00000001 << 21)
#define BIT22	(0x00000001 << 22)
#define BIT23	(0x00000001 << 23)
#define BIT24	(0x00000001 << 24)
#define BIT25	(0x00000001 << 25)
#define BIT26	(0x00000001 << 26)
#define BIT27	(0x00000001 << 27)
#define BIT28	(0x00000001 << 28)
#define BIT29	(0x00000001 << 29)
#define BIT30	(0x00000001 << 30)
#define BIT31	(0x80000000)




//�ж��������ַ
#define SYS_NVIC_VectTab_RAM             ((u32)0x20000000)
#define SYS_NVIC_VectTab_FLASH           ((u32)0x08000000)












//printf�������
#if (PRINTF_EN_ == 1)	//ʹ�ܵ�����
#include "usart.h"
extern bool isPrintfOut;
extern UART_CH_Type PrintfCh;	//printfͨ������
#define get_uart_printf_status()	(isPrintfOut)
#define uart_printf_enable()	(isPrintfOut=TRUE)				//�������ڴ�ӡ
#define uart_printf_disable()	(isPrintfOut=FALSE)				//�رմ��ڴ�ӡ
#define uart_printf(format,...)	(printf(format, ##__VA_ARGS__))	//���ڴ�ӡ
#define DEBUG(format,...) 		(printf("<DebugFile: "__FILE__", Line: %d> "format, __LINE__, ##__VA_ARGS__))	//DEBUG���
#define info_printf(format,...)	(printf("<info>:"format, ##__VA_ARGS__))	//ϵͳ��ӡ��Ϣ
#define  PRINTF_SetUartCh(ch) (PrintfCh=ch)	//�ض���printf���ͨ��
#endif


//printf�������
#if (PRINTF_EN_ == 2)	//ʹ�ܵ�Һ��
#define lcd_printf(format,...)	(printf(format, ##__VA_ARGS__))	//LCD��ӡ
#define DEBUG(format,...) 		(printf("<DebugFile: "__FILE__", Line: %d> "format, __LINE__, ##__VA_ARGS__))	//DEBUG���
#define info_printf(format,...)	(printf("<info>:"format, ##__VA_ARGS__))	//ϵͳ��ӡ��Ϣ
#endif


//printf�������
#if (PRINTF_EN_ == 3)	//ͬʱʹ�ܵ�Һ���ʹ���
extern u8 PrintfSet;
#include "usart.h"
extern UART_CH_Type PrintfCh;	//printfͨ������
#define uart_printf(format,...)	PrintfSet=0;printf(format, ##__VA_ARGS__)	//���ڴ�ӡ
#define lcd_printf(format,...)	PrintfSet=1;printf(format, ##__VA_ARGS__)	//LCD��ӡ
#define DEBUG(format,...)		PrintfSet=0;printf("<DebugFile: "__FILE__", Line: %d> "format, __LINE__, ##__VA_ARGS__)	//DEBUG���
#define info_printf(format,...)	(printf("<info>:"format, ##__VA_ARGS__))	//ϵͳ��ӡ��Ϣ
#define  PRINTF_SetUartCh(ch) (PrintfCh=ch)	//�ض���printf���ͨ��
#endif



#if (SYS_MESSAGE_EN_)
#include "MessageQueue.h"
#include "SYSMalloc.h"
#endif //SYS_MESSAGE_EN_



#endif
