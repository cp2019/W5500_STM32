/*************************************************************************************************************
 * �ļ���:		usart.h
 * ����:		STM32 USART����
 * ����:		cp1300@139.com
 * ����ʱ��:	2011��6��11��
 * ����޸�ʱ��:2013��5��29��
 * ��ϸ:		�Ѿ��޸���֪bug
				2013-11-17:����DMA�շ�ģʽ
				2014-08-19:����DMA,�жϻ��ģʽ����Ҫ�����ڴ���4������5��֧��DMA
*************************************************************************************************************/	

#ifndef _USART_H
#define _USART_H

#include "system.h"
#include "stdio.h"


/***********************�������************************/
#define UART_DMA_EN		1			//1:ʹ��DAM�շ�ģʽ;0:�ر�DMA�շ�ģʽ
#define UART_TX_TO_FIFI	1			//1:���ݷ��͵�����FIFO����Ϊ�������;0:���ݴ�FIFO�����������Ϊ�������
#define UART_ChMax		5			//����ͨ������


/*********************************************************/



//UART������ؽṹ����
typedef struct
{
	u8 OddEvenVerify;	//��żУ��,��,ż,��
	u8 StopBitWidth;	//ֹͣλλ��1,2
} UART_Config_TypeDef;


//��żУ��
#define UART_VERIFY_NULL	0	//��У��
#define UART_ODD			1	//��У��
#define UART_EVEN			2	//żУ��
//ֹͣλ
#define UART_STOP_1BIT		0	//һ��ֹͣλ
#define UART_STOP_2BIT		1	//2��ֹͣλ



//����ѡ��,����1��ʼ,������5
typedef enum
{
	UART_CH1	=		0,	//UART1
	UART_CH2	=		1,	//UART2
	UART_CH3	=		2,	//UART3
	UART_CH4	=		3,	//UART4
	UART_CH5	=		4,	//UART5
}UART_CH_Type;



//���API
bool 	UARTx_Init(UART_CH_Type ch,u32 Speed,u8 RX_Int);				//���ڳ�ʼ��
bool 	UARTx_Config(UART_CH_Type ch,UART_Config_TypeDef * cfg);		//��������
void 	UARTx_SendByte(UART_CH_Type ch,u8 data);						//UART���ֽڷ���
void 	UARTx_SendData(UART_CH_Type ch,u8 *tx_buff,u16 byte_number);	//UART���ݷ��ͺ���
void 	UARTx_SendString(UART_CH_Type ch,char *pString);				//UART�����ַ���
void 	UARTx_PowerDown(UART_CH_Type ch);								//UART����
void 	UARTx_PowerUp(UART_CH_Type ch);									//UART�ϵ�
bool 	UARTx_GetNewDataFlag(UART_CH_Type ch);							//��ȡ���������ݱ�־
bool 	UARTx_GetRxBuffFullFlag(UART_CH_Type ch);						//��ȡ���ڽ��ջ���������־
u8 		UARTx_GetNewData(UART_CH_Type ch);								//��ȡ����������
void 	UARTx_SetRxBuff(UART_CH_Type ch,u8 *RxBuff,u16 RxBuffSize);		//���ô��ڽ��ջ�����
void 	UARTx_ClearRxInt(UART_CH_Type ch);								//������ڽ����жϱ�־
u32 	UARTx_GetRxCnt(UART_CH_Type ch);								//��ȡ���ڽ������ݼ�����
void 	UARTx_ClearRxCnt(UART_CH_Type ch);								//������ڽ������ݼ�����
void 	UARTx_EnableRx(UART_CH_Type ch,FunctionalState Enable);			//���ڽ���ʹ��
void 	UARTx_SetBaudRate(UART_CH_Type ch,u32 baud);					//���㲢���ô��ڲ�����


#endif