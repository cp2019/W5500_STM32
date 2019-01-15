/*************************************************************************************************************
 * �ļ���:			W5500.c
 * ����:			W5500����оƬ����
 * ����:			cp1300@139.com
 * ����ʱ��:		2015-01-25
 * ����޸�ʱ��:	2015-01-25
 * ��ϸ:			����оƬ����,֧��TCP,UDPЭ��
					���ʹ����UCOS,���в����������߳���ִ��
					����UDPģʽ��˵����UDP�ǲ������ӻ��ģ������ҵ�ǰUDP���˿�Ϊ123��Ŀ��IPΪ1.2.3.4���˿�Ϊ100�����Ŀ��IP�˿ڿ����˼�أ���ǰW5500�ǿ��Է������ݵ�Ŀ���������
						��������Է���UDP�����������ݵ�����IP��˿�123���򱾻�һ�������յ����ݣ�Ҳ�����Ǹ��˿�100���޷����������豸�������ݵ�������
					UDPģʽ�µĹ㲥������Ҫ���������ǲ�Ҫ�����㲥���ͣ������㲥���ͺ��޷����͵������ݰ��������Ҫ���͹㲥����ֻ��Ҫ��IP����Ϊ255����
*************************************************************************************************************/
#include "system.h"
#include "W5500.h"
#include "SPI.h"
#include "board.h"
#include "string.h"


//socket�Ĵ������ƼĴ���ѡ���б�
W5500_BLOCK_SELECT const SELECT_SOCKET_REG[8] = {SELECT_SECKET0_01, SELECT_SECKET1_05, SELECT_SECKET2_09, SELECT_SECKET3_13, SELECT_SECKET4_17, SELECT_SECKET5_21,
	SELECT_SECKET6_25, SELECT_SECKET7_29};
//socket���ͻ������Ĵ���ѡ���б�
W5500_BLOCK_SELECT const SELECT_SOCKET_TXBUFF[8] = {SELECT_SECKET0_TXREG_02, SELECT_SECKET1_TXREG_06, SELECT_SECKET2_TXREG_10, SELECT_SECKET3_TXREG_14, SELECT_SECKET4_TXREG_18, SELECT_SECKET5_TXREG_22,
	SELECT_SECKET6_TXREG_26, SELECT_SECKET7_TXREG_30};
//socket���ջ������Ĵ���ѡ���б�
W5500_BLOCK_SELECT const SELECT_SOCKET_RXBUFF[8] = {SELECT_SECKET0_RXREG_03, SELECT_SECKET1_RXREG_07, SELECT_SECKET2_RXREG_11, SELECT_SECKET3_RXREG_15, SELECT_SECKET4_RXREG_19, SELECT_SECKET5_RXREG_23,
	SELECT_SECKET6_RXREG_27, SELECT_SECKET7_RXREG_31};



//���ݰ�����ʱ��
#define SOCKET_TTL		        128
#define SOCKET_RETRY_COUNT		3			//ʧ������3+1��
#define SOCKET_TIME_OUT			(10*200)	//���η��ͳ�ʱʱ��200ms
#define SOCKET_SEND_TIME_OUT	((SOCKET_RETRY_COUNT+2)*(SOCKET_TIME_OUT/10))	//���ݰ����ͳ�ʱʱ��
	
	
//����䵥Ԫ
#define SOCKET_UDP_PACKSIZE		1472 		//1-1472
#define SOCKET_TCP_PACKSIZE		1460 		//1-1460
#define SOCKET_PPPOE_PACKSIZE	1464 		//1-1464
#define SOCKET_MACRAW_PACKSIZE	1514 		//1-1514

//��д����
#define W5500_READ_MODE		0	//��
#define W5500_WRTIE_MODE	1	//д


void W5500_OneSocketRxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);   //W5500 ��λһ��socket���ջ�����
void W5500_OneSocketTxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);  	//W5500 ��λһ��socket���ͻ�����  
    

/*************************************************************************************************************************
* ����			:	u8 W5500_ReadOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr)
* ����			:	�������Ĵ���
* ����			:	pHandle:W5500�ӿھ����RegBlockSelect:�Ĵ���ģ��ѡ��;RegAddr:�Ĵ�����ַ
* ����			:	��ȡ������
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
u8 W5500_ReadOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr)
{
	u8 temp;
	
	temp = RegBlockSelect << 3;					//BSB
	temp |= (W5500_READ_MODE&1)<<2;				//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);			//д��Ĵ�����ַ,��д���λ
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);				//д������
	//��ȡ
	temp = pHandle->ReadWrtieByte(0xff);		//��ȡ1�ֽ�
	pHandle->SetCS_IO(1);
	
	return temp;
}




/*************************************************************************************************************************
* ����			:	void W5500_WriteOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 data)
* ����			:	д�����Ĵ���
* ����			:	pHandle:W5500�ӿھ����RegBlockSelect:�Ĵ���ģ��ѡ��;RegAddr:�Ĵ�����ַ;data:��Ҫд�������
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
void W5500_WriteOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 data)
{
	u8 temp;
	
	temp = RegBlockSelect << 3;					//BSB
	temp |= (W5500_WRTIE_MODE&1)<<2;			//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);			//д��Ĵ�����ַ,��д���λ
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);				//д������
	//д��
	pHandle->ReadWrtieByte(data);				//д��1�ֽ�
	pHandle->SetCS_IO(1);
}



/*************************************************************************************************************************
* ����			:	void W5500_ReadMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr,  u8 DataBuff[], u16 DataLen)
* ����			:	������Ĵ���
* ����			:	pHandle:W5500�ӿھ����RegBlockSelect:�Ĵ���ģ��ѡ��;RegAddr:�Ĵ�����ַ;DataBuff:�Ĵ�����ȡ������;DataLen:��ȡ�ĳ���
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
void W5500_ReadMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr,  u8 DataBuff[], u16 DataLen)
{
	u8 temp;
	u16 i;
	
	temp = RegBlockSelect << 3;						//BSB
	temp |= (W5500_READ_MODE&1)<<2;					//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);				//д��Ĵ�����ַ,��д���λ
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);					//д������
	//ѭ����ȡ
	for(i = 0;i < DataLen;i ++)
	{
		DataBuff[i] = pHandle->ReadWrtieByte(0xff);	//��ȡ1�ֽ�
	}
	pHandle->SetCS_IO(1);
}



/*************************************************************************************************************************
* ����			:	void W5500_WriteMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 DataBuff[], u16 DataLen)
* ����			:	д����Ĵ���
* ����			:	pHandle:W5500�ӿھ����RegBlockSelect:�Ĵ���ģ��ѡ��;RegAddr:�Ĵ�����ַ;DataBuff:�Ĵ���д�뻺����;DataLen:д��ĳ���
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
void W5500_WriteMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 DataBuff[], u16 DataLen)
{
	u8 temp;
	u16 i;
	
	temp = RegBlockSelect << 3;					//BSB
	temp |= (W5500_WRTIE_MODE&1)<<2;			//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);			//д��Ĵ�����ַ,��д���λ
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);				//д������
	//ѭ��д��
	for(i = 0;i < DataLen;i ++)
	{
		pHandle->ReadWrtieByte(DataBuff[i]);	//д��1�ֽ�
	}
	pHandle->SetCS_IO(1);
}



/*************************************************************************************************************************
* ����			:	u16 W5500_ReadDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr)
* ����			:	��ȡ2�������ļĴ���(16λ�Ĵ���)
* ����			:	pHandle:W5500�ӿھ����RegBlockSelect:�Ĵ���ģ��ѡ��;RegAddr:�Ĵ�����ַ
* ����			:	16λ�Ĵ�������
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
u16 W5500_ReadDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr)
{
	u8 buff[2];
	u16 data;
	
	W5500_ReadMultiReg(pHandle, RegBlockSelect, RegAddr,  buff, 2);
	data = buff[0];
	data <<= 8;
	data |= buff[1];
	
	return data;
}	


/*************************************************************************************************************************
* ����			:	void W5500_WriteDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u16 data)
* ����			:	д��2�������ļĴ���(16λ�Ĵ���)
* ����			:	pHandle:W5500�ӿھ����RegBlockSelect:�Ĵ���ģ��ѡ��;RegAddr:�Ĵ�����ַ;data:��Ҫд���16λ����
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
void W5500_WriteDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u16 data)
{
	u8 buff[2];
	
	buff[0] = data >> 8;
	buff[1] = (u8)data;
	W5500_WriteMultiReg(pHandle, RegBlockSelect, RegAddr, buff, 2);
}















/*************************************************************************************************************************
* ����			:	void W5500_SetMode(W5500_HANDLE *pHandle, u8 W5500_MODE)
* ����			:	W5500ģʽ����
* ����			:	pHandle:W5500�ӿھ����W5500_MODE����ģʽ���壬�磺W5500_MODE_WOL_BIT��W5500_MODE_PING_BIT��W5500_MODE_PPPOE_BIT��W5500_MODE_FARP_BIT
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	W5500_MODE_WOL_BIT		(1<<5)		//���绽��ģʽ���أ�0���ر����绽��;1���������绽��
					W5500_MODE_PING_BIT		(1<<4)		//ping blockģʽ��0���ر�ping; 1:����ping
					W5500_MODE_PPPOE_BIT	(1<<3)		//PPPOEģʽ���أ�0���ر�PPPoE;1:����PPPoE
					W5500_MODE_FARP_BIT		(1<<1)		//ǿ��ARPģʽ��0���ر�ǿ�� ARP ģ��1������ǿ�� ARP ģʽ�� 
						��ǿ�� ARP ģʽ�£������Ƿ��� ���ݶ���ǿ��ARPARPARP��
*************************************************************************************************************************/
void W5500_SetMode(W5500_HANDLE *pHandle, u8 W5500_MODE)
{
	W5500_MODE &= 0x7F;	//���������λд1�����λд1��Ὣ�ڲ��Ĵ�����ʼ�������븴λ��ָ�
	
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_MODE_REG_0x00, W5500_MODE);		//д�Ĵ���
}


/*************************************************************************************************************************
* ����			:	u8 W5500_GetMode(W5500_HANDLE *pHandle)
* ����			:	��ȡW5500ģʽ
* ����			:	pHandle:W5500�ӿھ����
* ����			:	��ģʽ���壬�磺W5500_MODE_WOL_BIT��W5500_MODE_PING_BIT��W5500_MODE_PPPOE_BIT��W5500_MODE_FARP_BIT
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	W5500_MODE_WOL_BIT		(1<<5)		//���绽��ģʽ���أ�0���ر����绽��;1���������绽��
					W5500_MODE_PING_BIT		(1<<4)		//ping blockģʽ��0���ر�ping; 1:����ping
					W5500_MODE_PPPOE_BIT	(1<<3)		//PPPOEģʽ���أ�0���ر�PPPoE;1:����PPPoE
					W5500_MODE_FARP_BIT		(1<<1)		//ǿ��ARPģʽ��0���ر�ǿ�� ARP ģ��1������ǿ�� ARP ģʽ�� 
						��ǿ�� ARP ģʽ�£������Ƿ��� ���ݶ���ǿ��ARPARPARP��
*************************************************************************************************************************/
u8 W5500_GetMode(W5500_HANDLE *pHandle)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_MODE_REG_0x00);		//���Ĵ���
}



/*************************************************************************************************************************
* ����			:	void W5500_ClearSysInterrupt(W5500_HANDLE *pHandle, u8 W5500_INT)
* ����			:	W5500ϵͳ�ж����
* ����			:	pHandle:W5500�ӿھ����W5500_INT���ж�ѡ�񣻼�W5500_INT_CONFLICT��W5500_INT_UNREACH��W5500_INT_PPPOE��W5500_INT_MP
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	W5500_INT_CONFLICT		(1<<7)	//IP��ͻ�жϣ����յ� APR ����ʱ�������ͷ� IP �뱾�� IP �ظ�����λ������ ��1��
					W5500_INT_UNREACH		(1<<6)	//Ŀ�겻�ɵִ�,�����յ� ICMPICMPICMPICMP��Ŀ�Ķ˿ڲ��ɴ���󣬸�λ�á� 1��
					W5500_INT_PPPOE			(1<<5)	//PPPoE ���ӹرգ��� PPPoEģʽ�� ,PPPoE���ӶϿ�ʱ��Ч
					W5500_INT_MP			(1<<4)	//���绽��ģʽ���ܵ��������ݰ��Ǵ���
*************************************************************************************************************************/
void W5500_ClearSysInterrupt(W5500_HANDLE *pHandle, u8 W5500_INT)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_IR_REG_0x15, W5500_INT);		//д�Ĵ���
}

/*************************************************************************************************************************
* ����			:	u8 W5500_GetSysInterrupt(W5500_HANDLE *pHandle)
* ����			:	��ȡW5500ϵͳ�ж�״̬
* ����			:	pHandle:W5500�ӿھ����
* ����			:	W5500_INT���ж�״̬����W5500_INT_CONFLICT��W5500_INT_UNREACH��W5500_INT_PPPOE��W5500_INT_MP
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	W5500_INT_CONFLICT		(1<<7)	//IP��ͻ�жϣ����յ� APR ����ʱ�������ͷ� IP �뱾�� IP �ظ�����λ������ ��1��
					W5500_INT_UNREACH		(1<<6)	//Ŀ�겻�ɵִ�,�����յ� ICMP��Ŀ�Ķ˿ڲ��ɴ���󣬸�λ�á� 1��
					W5500_INT_PPPOE			(1<<5)	//PPPoE ���ӹرգ��� PPPoEģʽ�� ,PPPoE���ӶϿ�ʱ��Ч
					W5500_INT_MP			(1<<4)	//���绽��ģʽ���ܵ��������ݰ��Ǵ���
*************************************************************************************************************************/
u8 W5500_GetSysInterrupt(W5500_HANDLE *pHandle)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_IR_REG_0x15);		//���Ĵ���
}



/*************************************************************************************************************************
* ����			:	void W5500_SetSysInterruptEnable(W5500_HANDLE *pHandle, u8 W5500_INT)
* ����			:	W5500ϵͳ�ж�ʹ������
* ����			:	pHandle:W5500�ӿھ����W5500_INT���ж�ʹ��ѡ�񣻼�W5500_INT_CONFLICT��W5500_INT_UNREACH��W5500_INT_PPPOE��W5500_INT_MP
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	W5500_INT_CONFLICT		(1<<7)	//IP��ͻ�жϣ����յ� APR ����ʱ�������ͷ� IP �뱾�� IP �ظ�����λ������ ��1��
					W5500_INT_UNREACH		(1<<6)	//Ŀ�겻�ɵִ�,�����յ� ICMP��Ŀ�Ķ˿ڲ��ɴ���󣬸�λ�á� 1��
					W5500_INT_PPPOE			(1<<5)	//PPPoE ���ӹرգ��� PPPoEģʽ�� ,PPPoE���ӶϿ�ʱ��Ч
					W5500_INT_MP			(1<<4)	//���绽��ģʽ���ܵ��������ݰ��Ǵ���
*************************************************************************************************************************/
void W5500_SetSysInterruptEnable(W5500_HANDLE *pHandle, u8 W5500_INT)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_IMR_REG_0x16, W5500_INT);		//д�Ĵ���
}


/*************************************************************************************************************************
* ����			:	u8 W5500_GetSysInterruptEnable(W5500_HANDLE *pHandle)
* ����			:	��ȡW5500ϵͳ�жϿ���״̬
* ����			:	pHandle:W5500�ӿھ����
* ����			:	W5500_INT���жϿ���״̬����W5500_INT_CONFLICT��W5500_INT_UNREACH��W5500_INT_PPPOE��W5500_INT_MP
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	W5500_INT_CONFLICT		(1<<7)	//IP��ͻ�жϣ����յ� APR ����ʱ�������ͷ� IP �뱾�� IP �ظ�����λ������ ��1��
					W5500_INT_UNREACH		(1<<6)	//Ŀ�겻�ɵִ�,�����յ� ICMPICMPICMPICMP��Ŀ�Ķ˿ڲ��ɴ���󣬸�λ�á� 1��
					W5500_INT_PPPOE			(1<<5)	//PPPoE ���ӹرգ��� PPPoEģʽ�� ,PPPoE���ӶϿ�ʱ��Ч
					W5500_INT_MP			(1<<4)	//���绽��ģʽ���ܵ��������ݰ��Ǵ���
*************************************************************************************************************************/
u8 W5500_GetSysInterruptEnable(W5500_HANDLE *pHandle)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_IMR_REG_0x16);		//���Ĵ���
}


/*************************************************************************************************************************
* ����			:	void W5500_SetPHY_OPMDC(W5500_HANDLE *pHandle, W5500_PHY_OPMDC OPMDC)
* ����			:	W5500 ����PHY�������˿����ã�
* ����			:	pHandle:W5500�ӿھ����OPMDC�����ü�W5500_PHY_OPMDC
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	
*************************************************************************************************************************/
void W5500_SetPHY_OPMDC(W5500_HANDLE *pHandle, W5500_PHY_OPMDC OPMDC)
{
	u8 temp = OPMDC;
	
	temp &= 0x07;
	temp <<=3;
	temp|=BIT7;	//���λ����Ϊ1������ᷢ����λ
	temp|=BIT6;//PHY����ģʽѡ��1��ͨ��OPMDC����
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E, temp);		//д�Ĵ���
}





/*************************************************************************************************************************
* ����			:	W5500_PHY_OPMDC W5500_GetPHY_OPMDC(W5500_HANDLE *pHandle)
* ����			:	W5500 ��ȡPHY���ã������˿����ã�
* ����			:	pHandle:W5500�ӿھ����
* ����			:	W5500_PHY_OPMDC
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	
*************************************************************************************************************************/
W5500_PHY_OPMDC W5500_GetPHY_OPMDC(W5500_HANDLE *pHandle)
{
	u8 temp = W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E);
	
	temp >>= 3;
	temp &= 0x07;
	
	return (W5500_PHY_OPMDC)temp;
}



/*************************************************************************************************************************
* ����			:	u8 W5500_GetPHY_Status(W5500_HANDLE *pHandle)
* ����			:	W5500 ��ȡPHY״̬
* ����			:	pHandle:W5500�ӿھ����
* ����			:	״̬��W5500_PHY_STATUS_DPX_BIT��W5500_PHY_STATUS_SPD_BIT��W5500_PHY_STATUS_LNK_BIT
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	W5500_PHY_STATUS_DPX_BIT		(1<<2)	//ȫ˫��״̬
					W5500_PHY_STATUS_SPD_BIT		(1<<1)	//�ٶ�״̬��1:100��0:10M
					W5500_PHY_STATUS_LNK_BIT		(1<<0)	//��������״̬��1���Ѿ����ӣ�0��δ����
*************************************************************************************************************************/
u8 W5500_GetPHY_Status(W5500_HANDLE *pHandle)
{
	u8 temp = W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E);
	temp &= 0x07;
	
	return temp;
}


/*************************************************************************************************************************
* ����			:	u8 W5500_GetOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket) 
* ����			:	W5500 ��ȡĳһ��socket�ľ����ж�״̬
* ����			:	pHandle:W5500�ӿھ����socket��socketѡ��0-7
* ����			:	״̬��SOCKET_IR_SEND_OK_BIT��SOCKET_IR_TIMEOUT_BIT��SOCKET_IR_RECV_BIT��SOCKET_IR_DISCON_BIT��SOCKET_IR_CON_BIT
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-12
* ����޸�ʱ��	:	2019-01-12
* ˵��			: 	SOCKET_IR_SEND_OK_BIT		(1<<4),	//�������
                    SOCKET_IR_TIMEOUT_BIT		(1<<3),	//APR��TCP���ͳ�ʱ
                    SOCKET_IR_RECV_BIT			(1<<2),	//�յ�������
                    SOCKET_IR_DISCON_BIT		(1<<1),	//�Է��Ͽ�����
                    SOCKET_IR_CON_BIT			(1<<0)	//���ӽ����ɹ�
*************************************************************************************************************************/
u8 W5500_GetOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket) 
{
    if(socket > 7) return 0;    //������Χ
	return W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_IR_0x02);
}



/*************************************************************************************************************************
* ����			:	void W5500_ClearOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
* ����			:	W5500 ���ĳһ��socket�ľ����ж�״̬
* ����			:	pHandle:W5500�ӿھ����socket��socketѡ��0-7��SocketIntBit���ж�bit��
                        SOCKET_IR_SEND_OK_BIT��SOCKET_IR_TIMEOUT_BIT��SOCKET_IR_RECV_BIT��SOCKET_IR_DISCON_BIT��SOCKET_IR_CON_BIT
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-12
* ����޸�ʱ��	:	2019-01-12
* ˵��			: 	SOCKET_IR_SEND_OK_BIT		(1<<4),	//�������
                    SOCKET_IR_TIMEOUT_BIT		(1<<3),	//APR��TCP���ͳ�ʱ
                    SOCKET_IR_RECV_BIT			(1<<2),	//�յ�������
                    SOCKET_IR_DISCON_BIT		(1<<1),	//�Է��Ͽ�����
                    SOCKET_IR_CON_BIT			(1<<0)	//���ӽ����ɹ�
                    ��ӦλΪ1�������Ӧ�ж�״̬
*************************************************************************************************************************/
void W5500_ClearOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
{
    if(socket > 7) return ;    //������Χ
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_IR_0x02, SocketIntBit);
}




/*************************************************************************************************************************
* ����			:	void W5500_SetOneSocketIntEnable(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
* ����			:	W5500 ����ĳһ��socket��Ӧ��״̬���ݲ����ж�
* ����			:	pHandle:W5500�ӿھ����socket��socketѡ��0-7��SocketIntBit���ж�bit��
                        SOCKET_IR_SEND_OK_BIT��SOCKET_IR_TIMEOUT_BIT��SOCKET_IR_RECV_BIT��SOCKET_IR_DISCON_BIT��SOCKET_IR_CON_BIT
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-12
* ����޸�ʱ��	:	2019-01-12
* ˵��			: 	SOCKET_IR_SEND_OK_BIT		(1<<4),	//�������
                    SOCKET_IR_TIMEOUT_BIT		(1<<3),	//APR��TCP���ͳ�ʱ
                    SOCKET_IR_RECV_BIT			(1<<2),	//�յ�������
                    SOCKET_IR_DISCON_BIT		(1<<1),	//�Է��Ͽ�����
                    SOCKET_IR_CON_BIT			(1<<0)	//���ӽ����ɹ�
                    ��ӦλΪ1������Ӧ�жϣ����򽫲��Ὺ��
*************************************************************************************************************************/
void W5500_SetOneSocketIntEnable(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
{
    if(socket > 7) return ;    //������Χ
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_IMR_0x2C, SocketIntBit);
}



/*************************************************************************************************************************
* ����			:	void W5500_OneSocketRxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* ����			:	W5500 ��λһ��socket���ջ�����
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��
* ����			:	��
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	����������ջ�����
*************************************************************************************************************************/
void W5500_OneSocketRxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	u16 ptr;

	ptr = W5500_GetOneSocketRxBuffWR(pHandle, socket);               	//��ȡ���ջ�����дָ��λ��getSn_RX_WR(socket);
	W5500_SetOneSocketRxBuffRD(pHandle, socket, ptr);                	//����ָ��=дָ�룬��λ���ջ�����
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_RECV);			//ִ��RECV������¶�ȡָ��
	pHandle->Sleep(1);
}





/*************************************************************************************************************************
* ����			:	void W5500_OneSocketTxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* ����			:	W5500 ��λһ��socket���ͻ�����
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��
* ����			:	��
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	����������ͻ�����
*************************************************************************************************************************/
void W5500_OneSocketTxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	u16 ptr;

	ptr = W5500_GetOneSocketTxBuffRD(pHandle, socket);           	//��ȡ���͵Ķ�ָ��λ��
	W5500_SetOneSocketTxBuffWR(pHandle, socket, ptr);            	//����дָ��=��ָ�룬��λ���ͻ�����ָ��
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_SEND);		//ִ��send������·���ָ��λ��
	pHandle->Sleep(1);
}


/*************************************************************************************************************************
* ����			:	void W5500_ReadOneSocketTcpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen)
* ����			:	��ȡĳһ��socket TCP���ջ���������
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��,DataBuff:���ݻ�����,DataLen:Ҫ��ȡ���ݳ���
* ����			:	��
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	�����ȳ�ʼ��
*************************************************************************************************************************/
void W5500_ReadOneSocketTcpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen)
{
	u16 ptr;
	if(DataLen == 0) return;
	
	ptr = W5500_GetOneSocketRxBuffRD(pHandle, socket);                                   //��ȡ��ǰsocket�Ľ��ջ�������ָ��
	W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr,  DataBuff, DataLen);  //��ȡ����
	W5500_SetOneSocketRxBuffRD(pHandle, socket, ptr+DataLen);                            //��ȡ���ݺ���½��ջ�������ָ��λ��
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_RECV);		                    //����RECV������¶�ȡָ��
}


/*************************************************************************************************************************
* ����			:	void W5500_ReadOneSocketUdpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen, u8 ip[4], u16 *pPort)
* ����			:	��ȡĳһ��socket UDP���ջ���������
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��,DataBuff:���ݻ�����,DataLen:���յ������ݳ��ȣ�����IP�˿�����ռ�õ�8�ֽ�����;ip:�Է�IP��pPort���Է��˿�
* ����			:	ʵ�����ݳ���
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-14
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	UDP���ݸ�ʽ IP1 IP2 IP3 IP4 PORT_H PORTL LEN_H LEN_L ����
*************************************************************************************************************************/
u16 W5500_ReadOneSocketUdpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen, u8 ip[4], u16 *pPort)
{
	u16 ptr;
	u8 buff[8];			//ǰ��8�ֽڴ�ŶԷ�IP��ַ���˿ڣ����ݳ�����Ϣ
	u16 temp;
	
	if(DataLen == 0) return 0;
	if(DataLen < 9)		//����9�ֽ�
	{
		ptr = W5500_GetOneSocketRxBuffRD(pHandle, socket);                                   	//��ȡ��ǰsocket�Ľ��ջ�������ָ��
		W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr,  DataBuff, DataLen);  	//��ȡ����
		*pPort = 0;																				//��Ч�Ķ˿�
		
		return 0;
	}
	
	ptr = W5500_GetOneSocketRxBuffRD(pHandle, socket);                                   //��ȡ��ǰsocket�Ľ��ջ�������ָ��
	W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr,  buff, 8);  			 //��ȡ����-�ȶ�ȡ8�ֽ�
	//����֡ͷ����
	memcpy(ip, buff, 4);	//IP
	temp = buff[4];	
	temp <<= 8;
	temp |= buff[5];
	*pPort = temp;			//�˿ں�
	temp = buff[6];
	temp <<= 8;
	temp |= buff[7];		//���ݳ���
	if(temp > (DataLen-8)) temp = DataLen-8;
	
	W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr+8,  DataBuff, temp);  //��ȡ����
	W5500_SetOneSocketRxBuffRD(pHandle, socket, ptr+DataLen);                            //��ȡ���ݺ���½��ջ�������ָ��λ��
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_RECV);		                     //����RECV������¶�ȡָ��
	
	return temp;			//���س���
}


/*************************************************************************************************************************
* ����			:	W5500_SOCKET_ERROR W5500_WriteOneSocketTcpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen)
* ����			:	TCPģʽд�뷢�����ݵ�һ��socket���ͻ�����
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��,DataBuff:���ݻ�����,DataLen:���ݳ���
* ����			:	W5500_SOCKET_ERROR
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	�����ȳ�ʼ��������TCP�ͻ��˷�������
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_WriteOneSocketTcpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen)
{
	u16 ptr;
	u16 SurSize;
    
	if(DataLen == 0) return SOCKET_ERROR;
	SurSize = W5500_GetOneSocketFreeTxBuffSzie(pHandle, socket);				//��ȡ���ͻ�����ʣ��ռ�
	if(DataLen > SurSize) return SOCKET_TXBUFF_SIZE;                			//�����������ռ䲻��
 
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_SEND_OK_BIT|SOCKET_IR_TIMEOUT_BIT);	//�����������ж�����ճ�ʱ�ж�
    ptr = W5500_GetOneSocketTxBuffRD(pHandle, socket);                       	//��ȡ���͵Ķ�ָ��λ��
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_TXBUFF[socket], ptr,  (u8 *)DataBuff, DataLen);//д�����ݵ����ͻ�������  
	W5500_SetOneSocketTxBuffWR(pHandle, socket, ptr+DataLen);                	//���ͺ����дָ��λ��
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_SEND);	            	//ִ��send������·���ָ��λ��
	
	return SOCKET_OK;
}



/*************************************************************************************************************************
* ����			:	W5500_SOCKET_ERROR W5500_WriteOneSocketUdpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen, u8 ip[4], u16 port)
* ����			:	UDPд�뷢��ָ��IP�˿ڵ����ݵ�һ��socket���ͻ�����
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��,DataBuff:���ݻ�����,DataLen:���ݳ���;ip:Ŀ��IP��ַ(���ΪNULL�򲻽�������)��port��Ŀ��˿�(���Ϊ0�򲻽�������)
* ����			:	W5500_SOCKET_ERROR
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	�����ȳ�ʼ��������UDP��������
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_WriteOneSocketUdpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen, u8 ip[4], u16 port)
{
	u16 ptr;
	u16 SurSize;
    
	if(DataLen == 0) return SOCKET_ERROR;
	SurSize = W5500_GetOneSocketFreeTxBuffSzie(pHandle, socket);				//��ȡ���ͻ�����ʣ��ռ�
	if(DataLen > SurSize) return SOCKET_TXBUFF_SIZE;                			//�����������ռ䲻��
 
	if(port != 0 && ip != NULL)	//��Ч�Ķ˿ڣ�����������Ŀ��˿���IP��ַ
	{
		W5500_SetOneSocketDestIP(pHandle, socket, ip);							//д��Ŀ��IP��ַ
		W5500_SetOneSocketDestPort(pHandle, socket, port);						//д��Ŀ��˿�
	}
	
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_SEND_OK_BIT|SOCKET_IR_TIMEOUT_BIT);	//�����������ж�����ճ�ʱ�ж�
    ptr = W5500_GetOneSocketTxBuffRD(pHandle, socket);                       	//��ȡ���͵Ķ�ָ��λ��
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_TXBUFF[socket], ptr,  (u8 *)DataBuff, DataLen);//д�����ݵ����ͻ�������  
	W5500_SetOneSocketTxBuffWR(pHandle, socket, ptr+DataLen);                	//���ͺ����дָ��λ��
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_SEND);	            	//ִ��send������·���ָ��λ��
	
	return SOCKET_OK;
}

/*************************************************************************************************************************
* ����			:	W5500_PROTOCOL W5500_GetOneSocketProtocol(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* ����			:	��ȡһ��socket��Ӧ��Э������
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��
* ����			:	W5500_PROTOCOL
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	�����ȳ�ʼ��
*************************************************************************************************************************/
W5500_PROTOCOL W5500_GetOneSocketProtocol(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return SOCKET_CLOSED;    															//������Χ
	return (W5500_PROTOCOL)(W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00)&0xf);		//��ȡsocketЭ��ģʽ
}



/*************************************************************************************************************************
* ����			:	void W5500_CloseOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* ����			:	ǿ�ƹر�һ��socket
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��
* ����			:	W5500_PROTOCOL
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-13
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	�����ȳ�ʼ��
*************************************************************************************************************************/
void W5500_CloseOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	W5500_SOCKET_STATUS SocketStatus;
    if((u8)socket > 7) return;    //������Χ

	SocketStatus = W5500_GetOneSocketStatus(pHandle, socket);					//��ȡsocket״̬ 
	if(SocketStatus == SOCK_LISTEN || SocketStatus == SOCK_ESTABLISHED)			//TCP����״̬������״̬���ͶϿ�����
	{
		W5500_DisconnectOneSocket(pHandle, socket);								//����һ��SOCKET����رյ�ǰsocket-��֪ͨ�Է��Ͽ�����
	}
	
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00, 0x00);	//ǿ�ƹر�һ��socket
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);				//�����ǰsocket�����ж�״̬
	W5500_SetOneSocketIntEnable(pHandle, socket, 0);							//�ر������ж�
	
	uart_printf("ǿ�ƹر�socket %d\r\n", socket);
}


/*************************************************************************************************************************
* ����			:	W5500_PROTOCOL W5500_CreateUdpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort,const  u8 DestIp[4], u16 SestPort, bool isEnableBroadcast)
* ����			:	����һ��UDP�ͻ���
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��LoaclPort�����ض˿ڣ�DestIp��Ŀ��IP��SestPort��ģ��˿ڣ�isEnableBroadcast���Ƿ�ʹ�ܹ㲥
* ����			:	W5500_PROTOCOL
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-13
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	��������˹㲥��ֻ��Ҫ��Ŀ��ip����Ϊ255����ʵ�ֹ㲥���鲥
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_CreateUdpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort, const u8 DestIp[4], u16 DestPort, bool isEnableBroadcast)
{
	W5500_SOCKET_STATUS status;
	u8 cnt = 0;
	
    if((u8)socket > 7) return SOCKET_ERROR;    //������Χ
	//��ȡ��ǰsocket״̬������ǿ���״̬�����ȹر�
	status = W5500_GetOneSocketStatus(pHandle, socket);
	if(status != SOCK_CLOSED)					//û�йرգ��ȹر�
	{
		W5500_CloseOneSocket(pHandle, socket);								//ǿ�ƹر�
		pHandle->Sleep(1);		
	}
	W5500_SetOneSocketUdpMode(pHandle, socket, isEnableBroadcast);			//���õ�ǰsocketΪUDPģʽ
	W5500_SetOneSocketLoaclPort(pHandle, socket, LoaclPort);				//���ñ��ض˿�
	W5500_SetOneSocketDestIP(pHandle, socket, DestIp);						//����Ŀ�������IP
	W5500_SetOneSocketDestPort(pHandle, socket, DestPort);					//����Ŀ��������˿�	 
	//���������socket
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_OPEN);				//����һ��SOCKET���� open
	//�ȴ�socket�����ɹ�
	for(cnt = 0;cnt < 40;cnt ++)
	{
		status = W5500_GetOneSocketStatus(pHandle, socket);					//��ȡ��ǰsocket״̬
		if(status == SOCK_UDP) break;
		pHandle->Sleep(100);
	}
	if(status != SOCK_UDP)
	{
		uart_printf("����socket %dΪUDPģʽ��ʱ����ǰ״̬��%d\r\n",socket,status );
		W5500_CloseOneSocket(pHandle, socket);								//ǿ�ƹر�
		return SOCKET_OPEN_ERROR;											//����socket��ʱ
	}

	W5500_SetOneSocketMaxTransUnit(pHandle, socket, SOCKET_UDP_PACKSIZE);	//����һ��socket������䵥Ԫ
	W5500_OneSocketRxBuffReset(pHandle, socket);   							//W5500 ��λһ��socket���ջ�����
	W5500_OneSocketTxBuffReset(pHandle, socket);  							//W5500 ��λһ��socket���ͻ����� 	
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);			//�����ǰsocket�����ж�״̬
	W5500_SetOneSocketIntEnable(pHandle, socket, SOCKET_IR_RECV_BIT);		//������ǰsocket�����ж�
	//��֪Ϊ��UDP��һ���ᶪʧ��������ʱ���ٸ�700ms,�˴���800MS��ʱ
	pHandle->Sleep(800);
	
	uart_printf("����socket %dΪUDPģʽ�ɹ�!\r\n", socket);
	return SOCKET_OK;	//���ӳɹ�
}


/*************************************************************************************************************************
* ����			:	W5500_SOCKET_ERROR W5500_CreateTcpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort, const u8 DestIp[4], u16 DestPort, u16 TimeOutSecond)
* ����			:	����һ��TCP�ͻ���
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��LoaclPort�����ض˿ڣ�DestIp��Ŀ��IP��SestPort��ģ��˿ڣ�TimeOutSecond�����ӳ�ʱʱ�䣬��λ��
* ����			:	W5500_PROTOCOL
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-13
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	�����ȳ�ʼ��
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_CreateTcpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort, const u8 DestIp[4], u16 DestPort, u16 TimeOutSecond)
{
	W5500_SOCKET_STATUS status;
	u8 cnt = 0;
	u16 TimeOutCnt = 0;
	
    if(socket > 7) return SOCKET_ERROR;    							//������Χ
	if(TimeOutSecond < 1) TimeOutSecond = 1;
	//��ȡ��ǰsocket״̬������ǿ���״̬�����ȹر�
	status = W5500_GetOneSocketStatus(pHandle, socket);
	if(status != SOCK_CLOSED)										//û�йرգ��ȹر�
	{
		W5500_CloseOneSocket(pHandle, socket);						//ǿ�ƹر�
		pHandle->Sleep(1);	
	}
	W5500_SetOneSocketTcpMode(pHandle, socket);						//���õ�ǰsocketΪTCPģʽ
	W5500_SetOneSocketLoaclPort(pHandle, socket, LoaclPort);		//���ñ��ض˿�
	W5500_SetOneSocketDestIP(pHandle, socket, DestIp);				//����Ŀ�������IP
	W5500_SetOneSocketDestPort(pHandle, socket, DestPort);			//����ģ��������˿�	
	//���������socket
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_OPEN);		//����һ��SOCKET���� open
	//�ȴ�socket�����ɹ�-��Ϊinitģʽ
	for(cnt = 0;cnt < 40;cnt ++)
	{
		status = W5500_GetOneSocketStatus(pHandle, socket);			//��ȡ��ǰsocket״̬
		if(status == SOCK_INIT) break;
		pHandle->Sleep(100);
	}
	if(status != SOCK_INIT)
	{
		uart_printf("����socket %dΪTCPģʽ��ʱ����ǰ״̬��%d\r\n",socket,status );
		W5500_CloseOneSocket(pHandle, socket);						//ǿ�ƹر�
		return SOCKET_OPEN_ERROR;									//����socket��ʱ
	}
	//ִ��CONHECT ���ӷ�����
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_CONHECT);	//����һ��SOCKET���� CONHECT
	//�ȴ����ӷ������ɹ�
	for(TimeOutCnt = 0; TimeOutCnt < TimeOutSecond; TimeOutCnt ++)
	{
		for(cnt = 0;cnt < 10;cnt ++)
		{
			pHandle->Sleep(100);
			status = W5500_GetOneSocketStatus(pHandle, socket);		//��ȡ��ǰsocket״̬
			if(status == SOCK_ESTABLISHED) break;
			pHandle->Sleep(100);
		}
	}
	if(status == SOCK_ESTABLISHED) //���ӷ������ɹ�
	{
		W5500_SetOneSocketMaxTransUnit(pHandle, socket, SOCKET_TCP_PACKSIZE);	//����һ��socket������䵥Ԫ
		W5500_SetOneSocketTcpHeartPackTime(pHandle, socket, 2);					//����һ��socket TCP������������ʱ�䣨��λ��5��,�������Ϊ0���رգ�-10��
		W5500_OneSocketRxBuffReset(pHandle, socket);   							//W5500 ��λһ��socket���ջ�����
		W5500_OneSocketTxBuffReset(pHandle, socket);  							//W5500 ��λһ��socket���ͻ�����  
		W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);			//�����ǰsocket�����ж�״̬
		W5500_SetOneSocketIntEnable(pHandle, socket, SOCKET_IR_RECV_BIT|SOCKET_IR_DISCON_BIT);//���������ж�,�Ͽ������ж�

		uart_printf("����socket %dΪTCPģʽ�ɹ�!\r\n", socket);
		return SOCKET_OK;	//���ӳɹ�
	}
	else
	{
		uart_printf("����socket %dΪTCPģʽ��ʱ����ǰ״̬��%d\r\n",socket,status);
		W5500_CloseOneSocket(pHandle, socket);						//ǿ�ƹر�
		return SOCKET_TIMEOUT;								//���ӳ�ʱ
	}
}



/*************************************************************************************************************************
* ����			:	W5500_SOCKET_ERROR W5500_CreateTcpServer(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LoaclPort)
* ����			:	W5500����һ��socket�����
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��;LoaclPort:���ؼ�ض˿�
* ����			:	��
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2019-01-13
* ˵��			: 	һ��socket�����ֻ������һ���ͻ���
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_CreateTcpServer(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LoaclPort)
{
	W5500_SOCKET_STATUS status;
	u8 cnt = 0;
	
    if(socket > 7) return SOCKET_ERROR;    //������Χ
	//��ȡ��ǰsocket״̬������ǿ���״̬�����ȹر�
	status = W5500_GetOneSocketStatus(pHandle, socket);
	if(status != SOCK_CLOSED)								//û�йرգ��ȹر�
	{
		W5500_CloseOneSocket(pHandle, socket);						//ǿ�ƹر�
		pHandle->Sleep(1);	
	}
	W5500_SetOneSocketTcpMode(pHandle, socket);						//���õ�ǰsocketΪTCPģʽ
	W5500_SetOneSocketLoaclPort(pHandle, socket, LoaclPort);			//���ñ��ض˿�	
	//���������socket
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_OPEN);		//����һ��SOCKET���� open
	pHandle->Sleep(2);
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_LISTEN);		//����һ��SOCKET���� SOCKET_CMD_LISTEN
	
	//�ȴ�socket��ΪSOCK_LISTENģʽ
	for(cnt = 0;cnt < 40;cnt ++)
	{
		status = W5500_GetOneSocketStatus(pHandle, socket);				//��ȡ��ǰsocket״̬
		if(status == SOCK_LISTEN) break;
		pHandle->Sleep(100);
	}
	if(status != SOCK_LISTEN) //��������ʧ��
	{
		uart_printf("����socket %dΪTCP����LISTEN��ʱ����ǰ״̬��%d\r\n",socket,status );
		W5500_CloseOneSocket(pHandle, socket);							//ǿ�ƹر�
		return SOCKET_OPEN_ERROR;								//����socket��ʱ
	}
	else //���������ɹ�
	{
		W5500_SetOneSocketMaxTransUnit(pHandle, socket, SOCKET_TCP_PACKSIZE);	//����һ��socket������䵥Ԫ
		W5500_SetOneSocketTcpHeartPackTime(pHandle, socket, 2);					//����һ��socket TCP������������ʱ�䣨��λ��5��,�������Ϊ0���رգ�-10��
		W5500_OneSocketRxBuffReset(pHandle, socket);   							//W5500 ��λһ��socket���ջ�����
		W5500_OneSocketTxBuffReset(pHandle, socket);  							//W5500 ��λһ��socket���ͻ�����  
		W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);			//�����ǰsocket�����ж�״̬
		W5500_SetOneSocketIntEnable(pHandle, socket, SOCKET_IR_RECV_BIT|SOCKET_IR_DISCON_BIT|SOCKET_IR_CON_BIT);//�������������ж�,�Ͽ������ж�,�½������ж�
		
		uart_printf("����socket %dΪTCP����LISTEN�ɹ�!\r\n", socket);
		return SOCKET_OK;	//���ӳɹ�
	}
}


 
/*************************************************************************************************************************
* ����			:	void W5500_DisconnectOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* ����			:	�Ͽ�һ��socket����
* ����			:	pHandle:W5500�ӿھ����socket:socketѡ��
* ����			:	W5500_PROTOCOL
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-14
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	��֪ͨ�Է��Ͽ�����-�Ƚ����ƵĶϿ�����
*************************************************************************************************************************/
void W5500_DisconnectOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	W5500_SOCKET_STATUS status;
	u8 retry;
	
	if((u8)socket > 7) return;    //������Χ
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_CLOSE);											//����һ��SOCKET����رյ�ǰsocket
	for(retry = 0;retry < 20;retry ++)
	{
		pHandle->Sleep(5);	
		status = W5500_GetOneSocketStatus(pHandle, socket);												//��ȡsocket״̬
		if(status == SOCK_CLOSED) break;
	}
	if(status != SOCK_CLOSED)
	{
		uart_printf("socket %d �Ͽ����ӳ�ʱ��״̬��%d\r\n",socket, status);
	}
}
   
    
/*************************************************************************************************************************
* ����			:	void W5500_Init(W5500_HANDLE *pHandle, u8 MAC[6])
* ����			:	��ʼ��W5500
* ����			:	pHandle:W5500�ӿھ����pHandle�������MAC:MAC��ַ,��6B
					void (*SetCS_IO)(u8 Level);             //CS IO���ýӿڣ�1���ߵ�ƽ��0���͵�ƽ��
					u8  (*GetInt_IO)(void);                 //��ȡ�ж�IO״̬���ߵ�ƽ����1���͵�ƽ����0��
					u8 (*ReadWrtieByte)(u8 data);           //SPI��д�ӿڣ�SPI��дһ�ֽڽӿڣ�
					void (*Sleep)(u32 ms);               	//������ʱ�ӿ�
					MAC[6];                              	//MAC��ַ
					HostName[16+1];                      	//��ǰ�������ƣ�����DHCP����ʾ���������ƣ�
					isEnablePing;                      		//�Ƿ�ʹ��ping
					isEnableWOL;                       		//�Ƿ������绽��
* ����			:	��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
bool W5500_Init(W5500_HANDLE *pHandle, u8 MAC[6], void (*SetCS_IO)(u8 Level),  u8  (*GetInt_IO)(void), u8 (*ReadWrtieByte)(u8 data), void (*Sleep)(u32 ms), char HostName[16], 
	bool isEnablePing, bool isEnableWOL)
{
	u8 len;
	u8 i;
	
	if(pHandle == NULL)
	{
		DEBUG("[W5500]:��Ч�ľ������ʼ��ʧ�ܣ�\r\n");
		return FALSE;
	}
	if(SetCS_IO == NULL || GetInt_IO == NULL || ReadWrtieByte == NULL || Sleep == NULL)
	{
		DEBUG("[W5500]:��Ч��ͨѶ�ӿڣ���ʼ��ʧ�ܣ�\r\n");
		return FALSE;
	}
	
	//��ʼ�������Ӳ���ӿ�
	pHandle->SetCS_IO = SetCS_IO;					//CS IO���ýӿڣ�1���ߵ�ƽ��0���͵�ƽ��
	pHandle->GetInt_IO = GetInt_IO;					//��ȡ�ж�IO״̬���ߵ�ƽ����1���͵�ƽ����0��
	pHandle->ReadWrtieByte = ReadWrtieByte;			//SPI��д�ӿڣ�SPI��дһ�ֽڽӿڣ�
	pHandle->Sleep = Sleep;							//������ʱ�ӿ�
	len = strlen(HostName);							//���Ƴ���
	if(len > 16) len = 16;
	memcpy(pHandle->HostName, HostName, len);		//��������
	pHandle->HostName[len] = 0;
	pHandle->isEnablePing = isEnablePing;			//�Ƿ�ʹ��ping
	pHandle->isEnableWOL = isEnableWOL;				//�Ƿ�ʹ�����绽��
	memcpy(pHandle->MAC, MAC, 6);					//����MAC��ַ
	
	//��λ
	pHandle->SetCS_IO(1);							//����ƬѡĬ�ϸߵ�ƽ
	pHandle->Sleep(2);
	W5500_SoftwareReset(pHandle);					//�����λ
	pHandle->Sleep(2);
	
	for(i = 0;i < 8;i ++)
	{
		pHandle->SocketStatus[i] = SOCK_CLOSED;		//socket״̬
		pHandle->SocketProtocol[i] = SOCKET_CLOSED;	//socketЭ��
	}
	
	return W5500_InitConfig(pHandle);				//�����豸
}



/*************************************************************************************************************************
* ����			:	bool W5500_ConfigInit(W5500_HANDLE *pHandle, u8 MAC[6])
* ����			:	����W5500
* ����			:	pHandle:W5500�ӿھ����MAC:MAC��ַ,��6B
* ����			:	TRUE:��ʼ���ɹ���FALSE:��ʼ��ʧ��
* ����			:	SPI
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-14
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	���жϰ汾���Ƿ���ȷ���������ȷ��ζ��оƬû��������������ͨѶ�쳣
*************************************************************************************************************************/
bool W5500_InitConfig(W5500_HANDLE *pHandle)
{
	u8 i;
	u8 retry;
	u8 Version;
	u8 temp;
	
	
	for(retry = 0; retry < 5; retry ++)
	{
		Version = W5500_GetVersion(pHandle);
		if(Version!=0x04)
		{
			uart_printf("���W5500ʧ��,����İ汾��Ϊ��0x%02X,��������...\r\n",Version);
		}
		else break;
		pHandle->Sleep(100);
	}
	if(Version!=0x04)
	{
		uart_printf("���W5500ʧ��,����İ汾��Ϊ��0x%02X\r\n",Version);
		return FALSE;
	}
	
	W5500_SetPHY_OPMDC(pHandle, W5500_OPMDC_FULL_FUNC_AUTO);		//W5500 ����PHY-ȫ���ܣ��Զ�Э��	
	W5500_SetMAC(pHandle, pHandle->MAC);							//����MAC��ַ
	temp = 0;
	if(pHandle->isEnablePing)	//������ping
	{
		temp |= W5500_MODE_PING_BIT;
	}
	if(pHandle->isEnableWOL)	//���������绽��
	{
		temp |= W5500_MODE_WOL_BIT;
	}
	W5500_SetMode(pHandle, temp);									//����ping�����绽��
	
	W5500_SetIntLevel(pHandle, 1);									//�����жϼ��
	W5500_SetRetryTime(pHandle, SOCKET_TIME_OUT);					//�����ش���ʱʱ��-200MS
	W5500_SetRetryCount(pHandle, SOCKET_RETRY_COUNT);				//W5500���÷������Դ���,RetryCount+1�Σ�������ᴥ����ʱ�ж�
	//��ʼ��8��socket
	for(i = 0;i < 8;i ++)
	{
		W5500_SetOneSocketTOS(pHandle, (W5500_SOCKET_NUM) i,  0);							//����һ��socket�ķ��������ֶ�TOS(ͨ������Ϊ0����)
		W5500_SetOneSocketTTL(pHandle, (W5500_SOCKET_NUM) i,  SOCKET_TTL);					//����һ��socket��IP����ʱ��TTL
		W5500_SetOneSocketNotFrag(pHandle, (W5500_SOCKET_NUM) i);							//�ر�һ��socket�ֶΣ�һ��ʹ�ö����鲻Ҫ�ֶΣ�
		W5500_SetOneSocketRxBuffSize(pHandle, (W5500_SOCKET_NUM) i,  SOCKET_BUFF_SIZE_2KB);	//����һ��socket�Ľ��ջ�������С-2KB
		W5500_SetOneSocketTxBuffSize(pHandle, (W5500_SOCKET_NUM) i,  SOCKET_BUFF_SIZE_2KB);	//����һ��socket�ķ��ͻ�������С-2KB
	}
	W5500_SetSocketTotalInterruptEnable(pHandle, 0xFF);										//����socket���ж�״̬-ÿһ��bit����һ��socket�Ƿ������ж�-��������socket���ж�
	//uart_printf("socket���жϿ���=%02X\r\n",W5500_GetSocketTotalInterruptEnable());
	
	return TRUE;
}



/*************************************************************************************************************************
* ����			:	void W5500_DHCP_ip_assign(void)
* ����			:	����DHCP����Ĭ��IP�Ľӿ�
* ����			:	��
* ����			:	��
* ����			:	�ײ�궨��
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-11
* ����޸�ʱ��	:	2019-01-11
* ˵��			: 	����DHCPʱ����ʼ��ΪĬ��ip��ȫ������Ϊ0.0.0.0
*************************************************************************************************************************/
void W5500_DHCP_ip_assign(void)
{
	/*uart_printf("W5500_DHCP_ip_assign\r\n");
	W5500_SetLocalIP(0, 0, 0, 0);					//���ñ���IP
	W5500_SetGatewayAddr(0, 0, 0, 0);				//��������IP
	W5500_SetMaskAddr(0, 0, 0, 0);					//������������*/
}

