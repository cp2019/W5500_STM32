/*************************************************************************************************************
 * �ļ���:			W5500.c
 * ����:			W5500����оƬ����
 * ����:			cp1300@139.com
 * ����ʱ��:		2015-01-25
 * ����޸�ʱ��:	2015-01-25
 * ��ϸ:			����оƬ����,֧��TCP,UDPЭ��
					���ʹ����UCOS,���в����������߳���ִ��
*************************************************************************************************************/
#ifndef _W5500_H_
#define _W5500_H_
#include "system.h"


//16λ�������ߵͶԵ�
#ifndef SWAP16
#define SWAP16(x)   (((x & 0xff00) >> 8) | ((x & 0xff) << 8))
#endif //SWAP16


//�Ĵ���ѡ����
typedef enum
{
	SELECT_GREG_00		    	=	0,	//ͨ�üĴ���
	SELECT_SECKET0_01	        =	1,	//socket0�Ĵ���
	SELECT_SECKET1_05	        =	5,	//socket1�Ĵ���
	SELECT_SECKET2_09   	    =	9,	//socket2�Ĵ���
	SELECT_SECKET3_13   	    =	13,	//socket3�Ĵ���
	SELECT_SECKET4_17   	    =	17,	//socket4�Ĵ���
	SELECT_SECKET5_21       	=	21,	//socket5�Ĵ���
	SELECT_SECKET6_25       	=	25,	//socket6�Ĵ���
	SELECT_SECKET7_29       	=	29,	//socket7�Ĵ���
	SELECT_SECKET0_TXREG_02 	=	2,	//socket0���ͻ�����
	SELECT_SECKET1_TXREG_06	    =	6,	//socket0���ͻ�����
	SELECT_SECKET2_TXREG_10 	=	10,	//socket0���ͻ�����
	SELECT_SECKET3_TXREG_14	    =	14,	//socket0���ͻ�����
	SELECT_SECKET4_TXREG_18	    =	18,	//socket0���ͻ�����
	SELECT_SECKET5_TXREG_22 	=	22,	//socket0���ͻ�����
	SELECT_SECKET6_TXREG_26 	=	26,	//socket0���ͻ�����
	SELECT_SECKET7_TXREG_30 	=	30,	//socket0���ͻ�����
	SELECT_SECKET0_RXREG_03 	=	3,	//socket0���ջ�����
	SELECT_SECKET1_RXREG_07	    =	7,	//socket0���ջ�����
	SELECT_SECKET2_RXREG_11	    =	11,	//socket0���ջ�����
	SELECT_SECKET3_RXREG_15	    =	15,	//socket0���ջ�����
	SELECT_SECKET4_RXREG_19	    =	19,	//socket0���ջ�����
	SELECT_SECKET5_RXREG_23	    =	23,	//socket0���ջ�����
	SELECT_SECKET6_RXREG_27	    =	27,	//socket0���ջ�����
	SELECT_SECKET7_RXREG_31 	=	31,	//socket0���ջ�����
}W5500_BLOCK_SELECT;


//ͨ�üĴ�����ַ����
typedef enum 
{
	W5500_MODE_REG_0x00		    =	0x0000,	//ģʽ�Ĵ���
	W5500_GAR0_REG_0x01 		=	0x0001,	//���ص�ַ�ֽ�1
	W5500_GAR1_REG_0x02 		=	0x0002,	//���ص�ַ�ֽ�2
	W5500_GAR2_REG_0x03 		=	0x0003,	//���ص�ַ�ֽ�3
	W5500_GAR3_REG_0x04 		=	0x0004,	//���ص�ַ�ֽ�4
	W5500_SUBR0_REG_0x05		=	0x0005,	//���������ֽ�1
	W5500_SUBR1_REG_0x06		=	0x0006,	//���������ֽ�2
	W5500_SUBR2_REG_0x07		=	0x0007,	//���������ֽ�3
	W5500_SUBR3_REG_0x08		=	0x0008,	//���������ֽ�4
	W5500_SHAR0_REG_0x09		=	0x0009,	//MACӲ����ַ�ֽ�1
	W5500_SHAR1_REG_0x0A		=	0x000A,	//MACӲ����ַ�ֽ�2
	W5500_SHAR2_REG_0x0B		=	0x000B,	//MACӲ����ַ�ֽ�3
	W5500_SHAR3_REG_0x0C		=	0x000C,	//MACӲ����ַ�ֽ�4
	W5500_SHAR4_REG_0x0D		=	0x000D,	//MACӲ����ַ�ֽ�5
	W5500_SHAR5_REG_0x0E		=	0x000E,	//MACӲ����ַ�ֽ�6
	W5500_SIPR0_REG_0x0F		=	0x000F,	//����IP�ֽ�1
	W5500_SIPR1_REG_0x10		=	0x0010,	//����IP�ֽ�2
	W5500_SIPR2_REG_0x11		=	0x0011,	//����IP�ֽ�3
	W5500_SIPR3_REG_0x12		=	0x0012,	//����IP�ֽ�4
	W5500_INTLEVEL0_REG_0x13	=	0x0013,	//�͵�ƽ�ж϶�ʱ���Ĵ���-���ֽ�
	W5500_INTLEVEL1_REG_0x14	=	0x0014,	//�͵�ƽ�ж϶�ʱ���Ĵ���-���ֽ�
	W5500_IR_REG_0x15   		=	0x0015,	//�жϼĴ���
	W5500_IMR_REG_0x16  		=	0x0016,	//�ж� ���� �Ĵ���
	W5500_SIR_REG_0x17	    	=	0x0017,	//Socket �жϼĴ���
	W5500_SIMR_REG_0x18 		=	0x0018,	//Socket �ж����μĴ���
	W5500_RTR0_REG_0x19     	=	0x0019,	//����ʱ��ֵ�Ĵ���-���ֽ�
	W5500_RTR1_REG_0x1A	    	=	0x001A,	//����ʱ��ֵ�Ĵ���-���ֽ�
	W5500_RCR_REG_0x1B  		=	0x001B,	//���Լ�����
	W5500_PTIMER_REG_0x1C   	=	0x001C,	//PPP���ӿ���Э������ʱ�Ĵ���
	W5500_PMAGIC_REG_0x1D   	=	0x001D,	//PPP���ӿ���Э������Ĵ���
	W5500_PHAR0_REG_0x1E		=	0x001E,	//PPPoE ģʽ��Ŀ�� MAC �Ĵ���
	W5500_PHAR1_REG_0x1F		=	0x001F,	//PPPoE ģʽ��Ŀ�� MAC �Ĵ���
	W5500_PHAR2_REG_0x20		=	0x0020,	//PPPoE ģʽ��Ŀ�� MAC �Ĵ���
	W5500_PHAR3_REG_0x21		=	0x0021,	//PPPoE ģʽ��Ŀ�� MAC �Ĵ���
	W5500_PHAR4_REG_0x22		=	0x0022,	//PPPoE ģʽ��Ŀ�� MAC �Ĵ���
	W5500_PHAR5_REG_0x23		=	0x0023,	//PPPoE ģʽ��Ŀ�� MAC �Ĵ���
	W5500_PSID0_REG_0x24		=	0x0024,	//PPPoE ģʽ�»Ự ID �Ĵ���
	W5500_PSID1_REG_0x25		=	0x0025,	//PPPoE ģʽ�»Ự ID �Ĵ���
	W5500_PMRU0_REG_0x26		=	0x0026,	//PPPoEģʽ�������յ�Ԫ
	W5500_PMRU1_REG_0x27		=	0x0027,	//PPPoEģʽ�������յ�Ԫ
	W5500_UIPR0_REG_0x28		=	0x0028,	//�޷��ִ� IP ��ַ�Ĵ���
	W5500_UIPR1_REG_0x28		=	0x0029,	//�޷��ִ� IP ��ַ�Ĵ���
	W5500_UIPR2_REG_0x2A		=	0x002A,	//�޷��ִ� IP ��ַ�Ĵ���
	W5500_UIPR3_REG_0x2B		=	0x002B,	//�޷��ִ� IP ��ַ�Ĵ���
	W5500_UPORTR0_REG_0x2C	    =	0x002C,	//�޷��ִ� �˿� ��ַ�Ĵ���
	W5500_UPORTR1_REG_0x2D  	=	0x002D,	//�޷��ִ� �˿� ��ַ�Ĵ���
	W5500_PHYCFGR_REG_0x2E  	=	0x002E,	//W5500 PHY ���ü� ����
	W5500_VERSIONR_REG_0x39 	=	0x0039,	//оƬ�汾��Ϣ��Ĭ��Ϊ0x04
}W5500_REG_ADDR;


//socket n�Ĵ���
typedef enum
{
	Sn_MR_0x00			=	0x0000,	//ģʽ�Ĵ���
	Sn_CR_0x01			=	0x0001,	//���üĴ���
	Sn_IR_0x02			=	0X0002,	//�жϼĴ���
	Sn_SR_0x03			=	0X0003,	//״̬�Ĵ���
	Sn_PORT_0x04		=	0X0004,	//Դ�˿ڼĴ���
	Sn_DHAR_0x06		=	0X0006,	//Ŀ��MAC��ַ�Ĵ���
	Sn_DIPR_0x0C		=	0X000C,	//Ŀ��IP��ַ�Ĵ���
	Sn_DPORT_0x10		=	0X0010,	//Ŀ��˿ڼĴ���
	Sn_MSSR_0x12		=	0X0012,	//���ֶμĴ���
	Sn_TOS_0x15			=	0X0015,	//�������ͼĴ���
	Sn_TTL_0x16			=	0X0016,	//����ʱ��Ĵ���
	Sn_RXBUFF_SIZE_0x1E	=	0X001E,	//���ջ�������С�Ĵ���
	Sn_TXBUFF_SIZE_0x1F	=	0X001F,	//���ͻ�������С�Ĵ���
	Sn_TX_FSR_0x20		=	0X0020,	//���з��ͻ���Ĵ���
	Sn_TX_RD_0x22		=	0X0022,	//���Ͷ�ָ��Ĵ���
	Sn_TX_WR_0x24		=	0X0024,	//����дָ��Ĵ���
	Sn_RX_RSR_0x26		=	0X0026,	//���н��ջ���Ĵ���
	Sn_RX_RD_0x28		=	0X0028,	//���ն�ָ��Ĵ���
	Sn_RX_WR_0x2A		=	0X002A,	//����дָ��Ĵ���
	Sn_IMR_0x2C			=	0X002C,	//�ж����μĴ���
	Sn_FRAG_0x2D		=	0X002D,	//�ֶμĴ���
	Sn_KPALVTR_0x2F		=	0X002F,	//����ʱ��Ĵ���
}W5500_SOCKET_REG;



//SOCKETѡ��
typedef enum
{
	W5500_SECKET0		=	0,	//SOCKET0
	W5500_SECKET1		=	1,	//SOCKET1
	W5500_SECKET2		=	2,	//SOCKET2
	W5500_SECKET3		=	3,	//SOCKET3
	W5500_SECKET4		=	4,	//SOCKET4
	W5500_SECKET5		=	5,	//SOCKET5
	W5500_SECKET6		=	6,	//SOCKET6
	W5500_SECKET7		=	7,	//SOCKET7
}W5500_SOCKET_NUM;



//TCP/IPЭ��
typedef enum
{
	SOCKET_CLOSED		=	0,
	SOCKET_TCP			=	1,	
	SOCKET_UDP			=	2,
	SOCKET_MACRAW		=	3,
}W5500_PROTOCOL;




//���Ӵ���״̬
typedef enum
{
	SOCKET_OK				= 0,	//SOCKET OK
	SOCKET_OPEN_ERROR 		= 1,	//��SOCKET����
	SOCKET_TIMEOUT 			= 2,	//��ʱ
	SOCKET_TXBUFF_SIZE		= 3,	//���ͻ�����ʣ��ռ䲻��
	SOCKET_ERROR 			= 0xff,	//socket�Ƿ�
}W5500_SOCKET_ERROR;


//socket����
typedef enum
{
	SOCKET_CMD_OPEN		=	0x01,	//open
	SOCKET_CMD_LISTEN	=	0x02,	//LISTEN
	SOCKET_CMD_CONHECT	=	0x04,	
	SOCKET_CMD_DISCON	=	0x08,
	SOCKET_CMD_CLOSE	=	0x10,
	SOCKET_CMD_SEND		=	0x20,
	SOCKET_CMD_SEND_MAC	=	0x21,
	SOCKET_CMD_SEND_KEEP=	0x22,
	SOCKET_CMD_RECV		=	0x40,
}W5500_SOCKET_CMD;


//ĳһ��socket�����ж�״̬
#define	SOCKET_IR_SEND_OK_BIT		((u8)1<<4)	//�������
#define	SOCKET_IR_TIMEOUT_BIT		((u8)1<<3)	//APR��TCP���ͳ�ʱ
#define	SOCKET_IR_RECV_BIT			((u8)1<<2)	//�յ�������
#define	SOCKET_IR_DISCON_BIT		((u8)1<<1)	//�Է��Ͽ�����
#define	SOCKET_IR_CON_BIT			((u8)1<<0)	//���ӽ����ɹ�
#define SOCKET_IR_ALL               ((u8)0xFF)    



//SOCKET״̬
typedef enum
{
	SOCK_CLOSED		=	0x00,	//�ر�״̬
	SOCK_INIT		=	0x13,	//��ʼ��
	SOCK_LISTEN		=	0x14,	//����
	SOCK_ESTABLISHED=	0x17,	//TCP�ͻ������ӳɹ�
	SOCK_CLOSE_WAIT	=	0x1C,	//���յ��Ͽ�ָ��
	SOCK_UDP		=	0x22,	//UDPģʽ��,open
	SOCK_MACRAW		=	0x02,	//MACRAWģʽ
	SOCK_SYNSENT	=	0x15,	//�Ѿ�������������
	SOCK_SYNRECV	=	0x16,	//���յ����ӳɹ�ָ��
	SOCK_FIN_WAIT	=	0x18,	//���ڹر�socket
	SOCK_CLOSING 	=	0x1A,	//���ڹر�socket
	SOCK_TIME_WAIT	=	0x1B,	//��ʱ�ر�socket
	SOCK_LAST_ACK	=	0x1D,	//socket���ڹر�״̬
}W5500_SOCKET_STATUS;



//W5500 PHY����ģʽ����
typedef enum
{
	W5500_OPMDC_10M_HALF		=	0,			//10M��˫�����ر��Զ�Э��
	W5500_OPMDC_10M_FULL		=	1,			//10Mȫ˫�����ر��Զ�Э��
	W5500_OPMDC_100M_HALF		=	2,			//100M��˫�����ر��Զ�Э��
	W5500_OPMDC_100M_FULL		=	3,			//100Mȫ˫�����ر��Զ�Э��
	W5500_OPMDC_100M_HALF_AUTO	=	4,			//100M��˫���������Զ�Э��
	W5500_OPMDC_NONE			=	5,			//��Ч��δ����״̬
	W5500_OPMDC_POWER_DOWN		=	6,			//���磬�͹���
	W5500_OPMDC_FULL_FUNC_AUTO	=	7,			//ȫ���ܣ��Զ�Э��
}W5500_PHY_OPMDC;


//W5500 �շ���������С����
typedef enum
{
    SOCKET_BUFF_SIZE_1KB        =   1,          //1KB
    SOCKET_BUFF_SIZE_2KB        =   2,          //2KB 
    SOCKET_BUFF_SIZE_4KB        =   4,          //4KB
    SOCKET_BUFF_SIZE_8KB        =   8,          //8KB
    SOCKET_BUFF_SIZE_16KB       =   16,         //16KB    
}W5500_SOCKET_BUFF_SIZE;


//W5500ͨ�ü�ģʽ�Ĵ���ֵ����
#define W5500_MODE_WOL_BIT		(1<<5)		//���绽��ģʽ���أ�0���ر����绽��;1���������绽��
#define W5500_MODE_PING_BIT		(1<<4)		//ping blockģʽ��0���ر�ping; 1:����ping
#define W5500_MODE_PPPOE_BIT	(1<<3)		//PPPOEģʽ���أ�0���ر�PPPoE;1:����PPPoE
#define W5500_MODE_FARP_BIT		(1<<1)		//ǿ��ARPģʽ��0���ر�ǿ�� ARP ģ��1������ǿ�� ARP ģʽ�� ��ǿ�� ARP ģʽ�£������Ƿ��� ���ݶ���ǿ��ARPARPARP��

//W5500�ж�״̬����
#define W5500_INT_CONFLICT		(1<<7)		//IP��ͻ�жϣ����յ� APR ����ʱ�������ͷ� IP �뱾�� IP �ظ�����λ������ ��1��
#define W5500_INT_UNREACH		(1<<6)		//Ŀ�겻�ɵִ�,�����յ� ICMP��Ŀ�Ķ˿ڲ��ɴ���󣬸�λ�á� 1��
#define W5500_INT_PPPOE			(1<<5)		//PPPoE ���ӹرգ��� PPPoEģʽ�� ,PPPoE���ӶϿ�ʱ��Ч
#define W5500_INT_MP			(1<<4)		//���绽��ģʽ���ܵ��������ݰ��Ǵ���
#define W5500_INT_ALL           0xFF

//W5500 PHY ״̬
#define W5500_PHY_STATUS_DPX_BIT		(1<<2)	//ȫ˫��״̬
#define W5500_PHY_STATUS_SPD_BIT		(1<<1)	//�ٶ�״̬��1:100��0:10M
#define W5500_PHY_STATUS_LNK_BIT		(1<<0)	//��������״̬��1���Ѿ����ӣ�0��δ����


//socket�Ĵ������ƼĴ����б�
extern W5500_BLOCK_SELECT const SELECT_SOCKET_REG[8];
//socket���ͻ������Ĵ����б�
extern W5500_BLOCK_SELECT const SELECT_SOCKET_TXBUFF[8];
//socket���ջ������Ĵ����б�
extern W5500_BLOCK_SELECT const SELECT_SOCKET_RXBUFF[8];	



//W5500Ӳ���ӿھ��
typedef struct
{
    void (*SetCS_IO)(u8 Level);             //CS IO���ýӿڣ�1���ߵ�ƽ��0���͵�ƽ��
    u8  (*GetInt_IO)(void);                 //��ȡ�ж�IO״̬���ߵ�ƽ����1���͵�ƽ����0��
    u8 (*ReadWrtieByte)(u8 data);           //SPI��д�ӿڣ�SPI��дһ�ֽڽӿڣ�
    void (*Sleep)(u32 ms);                	//������ʱ�ӿ�
    u8 MAC[6];                              //MAC��ַ
    char HostName[16+1];                     //��ǰ�������ƣ�����DHCP����ʾ���������ƣ�
    bool isEnablePing;                      //�Ƿ�ʹ��ping
    bool isEnableWOL;                       //�Ƿ������绽��
	
	W5500_SOCKET_STATUS SocketStatus[8];	//socket״̬
	W5500_PROTOCOL SocketProtocol[8];		//socketЭ��
}W5500_HANDLE;

						
//��ʼ��W5500
bool W5500_Init(W5500_HANDLE *pHandle, u8 MAC[6], void (*SetCS_IO)(u8 Level),  u8  (*GetInt_IO)(void), u8 (*ReadWrtieByte)(u8 data), void (*Sleep)(u32 ms), char HostName[16], 
	bool isEnablePing, bool isEnableWOL);
//��ʼ��W5500����
bool W5500_InitConfig(W5500_HANDLE *pHandle);
	
	
/*===================================Socket�ӿ����===================================*/
void W5500_DisconnectOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);																				//�Ͽ�һ��socket����
void W5500_CloseOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);																					//ǿ�ƹر�һ��socket
W5500_SOCKET_ERROR W5500_CreateUdpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort,const  u8 DestIp[4], u16 DestPort, bool isEnableBroadcast);	//����һ��UDP�ͻ���
W5500_SOCKET_ERROR W5500_CreateTcpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort,const  u8 DestIp[4], u16 DestPort, u16 TimeOutSecond);		//����һ��TCP�ͻ���
W5500_SOCKET_ERROR W5500_CreateTcpServer(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LoaclPort);													//W5500����һ��socket�����
W5500_PROTOCOL W5500_GetOneSocketProtocol(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);																	//��ȡһ��socket��Ӧ��Э������
void W5500_ReadOneSocketTcpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen);												//��ȡĳһ��socket TCP���ջ���������
u16 W5500_ReadOneSocketUdpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen, u8 ip[4], u16 *pPort);							//��ȡĳһ��socket UDP���ջ���������
W5500_SOCKET_ERROR W5500_WriteOneSocketTcpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen);							//TCPд�뷢�����ݵ�һ��socket���ͻ�����
W5500_SOCKET_ERROR W5500_WriteOneSocketUdpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen, u8 ip[4], u16 port);		//UDPд�뷢��ָ��IP�˿ڵ����ݵ�һ��socket���ͻ�����
/*==================================================================================*/	
	

//�ײ�Ĵ���ͨѶ�ӿ�
u8 W5500_ReadOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr);					//�������Ĵ���
void W5500_WriteOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 data);		//д�����Ĵ���
void W5500_ReadMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr,  u8 DataBuff[], u16 DataLen);//������Ĵ���
void W5500_WriteMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 DataBuff[], u16 DataLen);//д����Ĵ���
u16 W5500_ReadDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr);				//��ȡ2�������ļĴ���
void W5500_WriteDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u16 data);	//д��2�������ļĴ���


//���ÿ�ݲ���-ϵͳ����
void W5500_ClearSysInterrupt(W5500_HANDLE *pHandle, u8 W5500_INT);					//W5500ϵͳ�ж����
u8 W5500_GetSysInterrupt(W5500_HANDLE *pHandle);									//��ȡW5500ϵͳ�ж�״̬
void W5500_SetSysInterruptEnable(W5500_HANDLE *pHandle, u8 W5500_INT);				//W5500ϵͳ�ж�ʹ������
u8 W5500_GetSysInterruptEnable(W5500_HANDLE *pHandle);								//��ȡW5500ϵͳ�жϿ���״̬
void W5500_SetPHY_OPMDC(W5500_HANDLE *pHandle, W5500_PHY_OPMDC OPMDC);				//W5500 ����PHY�������˿����ã�
W5500_PHY_OPMDC W5500_GetPHY_OPMDC(W5500_HANDLE *pHandle);							//W5500 ��ȡPHY���ã������˿����ã�
u8 W5500_GetPHY_Status(W5500_HANDLE *pHandle);										//W5500 ��ȡPHY״̬
u8 W5500_GetSocketTotalInterruptStatus(W5500_HANDLE *pHandle);						//��ȡsocket���ж�״̬-ÿһ��bit����һ��socket�Ƿ������ж�
void W5500_ClearSocketTotalInterruptStatus(W5500_HANDLE *pHandle, u8 SokctBit);		//���socket���ж�״̬-ÿһ��bit����һ��socket�ж��������ӦλΪ1�������
u8 W5500_GetSocketTotalInterruptEnable(W5500_HANDLE *pHandle);						//��ȡsocket���жϿ���״̬-ÿһ��bit����һ��socket�Ƿ������ж�
void W5500_SetSocketTotalInterruptEnable(W5500_HANDLE *pHandle, u8 SokctBit);		//����socket���ж�״̬-ÿһ��bit����һ��socket�Ƿ������ж�
u8 W5500_GetVersion(W5500_HANDLE *pHandle);						//��ȡW5500оƬ�汾��W5500�汾Ĭ��Ϊ0x04��
void W5500_SetLocalIP(W5500_HANDLE *pHandle, u8 ip[4]);			//���ñ���IP��ַ
void W5500_SetGatewayAddr(W5500_HANDLE *pHandle, u8 ip[4]);		//��������IP��ַ
void W5500_SetMaskAddr(W5500_HANDLE *pHandle, u8 ip[4]);		//������������
void W5500_SetMAC(W5500_HANDLE *pHandle, u8 mac[6]);			//���ñ���MACӲ����ַ
void W5500_GetMAC(W5500_HANDLE *pHandle, u8 mac[6]);			//��ȡ����MACӲ����ַ
void W5500_SoftwareReset(W5500_HANDLE *pHandle);				//W5500��λ-��ö���ȴ���λ���,����1ms
void W5500_SetIntLevel(W5500_HANDLE *pHandle, u16 LevelTime);	//�����жϵ͵���ʱ�䣬2���ж����͵ļ��ʱ��(ʱ������4/CLK)
void W5500_SetRetryTime(W5500_HANDLE *pHandle, u16 Time100us);	//�����ش���ʱʱ��-��λ100us
u16 W5500_GetRetryTime(W5500_HANDLE *pHandle);					//��ȡ�ش���ʱʱ��-��λ100us
void W5500_SetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount);	//W5500���÷������Դ���,RetryCount+1�Σ�������ᴥ����ʱ�ж�
u8 W5500_GetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount);	//W5500��ȡ�������Դ���,RetryCount+1�Σ�������ᴥ����ʱ�ж�
void W5500_GetUnableIP(W5500_HANDLE *pHandle, u8 ip[4]);		//W5500��ȡ�޷��ִ��IP��ַ
u16 W5500_GetUnablePort(W5500_HANDLE *pHandle);					//W5500��ȡ�޷��ִ�Ķ˿�

//���ÿ�ݲ���-����socket����
u8 W5500_GetOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);                        	//W5500 ��ȡĳһ��socket�ľ����ж�״̬
void W5500_ClearOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) ;  	//W5500 ���ĳһ��socket�ľ����ж�״̬
void W5500_SetOneSocketIntEnable(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) ;		//W5500 ����ĳһ��socket��Ӧ��״̬���ݲ����ж�
void W5500_SetOneSocketTcpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//W5500����һ��socktΪTCPģʽ(socketѡ��0-7);
void W5500_SetOneSocketUdpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, bool isEnableBroadcast);	//W5500����һ��socktΪUDPģʽ(socketѡ��0-7)-��Ҫ�������鲥IP��˿ڣ��ٵ��ô˺���
void W5500_SetOneSocket0MacrawMode(W5500_HANDLE *pHandle, bool isMACRAW, bool isEnableBroadcast );		//W5500����һ��socktΪMACRAWģʽ(ֻ��ʹ��socket 0)
void W5500_SendOneSocketCmd(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, W5500_SOCKET_CMD SocketCmd);//W5500����һ��socket����(socketѡ��0-7, SocketCmd:socket����)���ڿ���һ��socket״̬
W5500_SOCKET_STATUS W5500_GetOneSocketStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);			//��ȡһ��socket״̬
void W5500_SetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LocalPort);		//����һ��socket�ı��ض˿�
u16 W5500_GetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);						//��ȡһ��socket�ı��ض˿�
void W5500_SetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6]);				//����һ��socket��Ŀ��MAC��ַ(����UDPģʽ�£�ʹ��Send_MAC���������Ŀ������MAC��ַ�����ߴ洢ARP��ȡ����MAC��ַ)
void W5500_GetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6]);				//��ȡһ��socket��Ŀ��MAC��ַ(����UDPģʽ�£�ʹ��Send_MAC��������ǰ���õ�Ŀ������MAC��ַ�����ߴ洢ARP��ȡ����MAC��ַ)
void W5500_SetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 ip[4]);			//����һ��socket��Ŀ��ip��ַ(����TCP/UDPģʽ����Ŀ������IP��ַ�����ߴ洢TCP/UDP Serverģʽ�´ӻ�IP��ַ)
void W5500_GetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 ip[4]);				//��ȡһ��socket��Ŀ��ip��ַ(���ڻ�ȡTCP/UDPģʽ����Ŀ������IP��ַ�����߻�ȡTCP/UDP Serverģʽ�´ӻ�IP��ַ)
void W5500_SetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 DestPort);			//����һ��socket��Ŀ��˿�(����TCP/UDPģʽ����Ŀ�������˿ڣ����ߴ洢TCP/UDP Serverģʽ�´ӻ��˿�)
u16 W5500_GetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//��ȡһ��socket��Ŀ��˿�(���ڻ�ȡTCP/UDPģʽ����Ŀ�������˿ڣ����߻�ȡTCP/UDP Serverģʽ�´ӻ��˿�)
void W5500_SetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 MaxTransUnit);	//����һ��socket������䵥Ԫ��TCP���1460��UDP���1472��PPPOE TCP���1452 PPPOE UDP���1464 MACRAW�����1514��
u16 W5500_GetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);						//��ȡһ��socket������䵥Ԫ��TCP���1460��UDP���1472��PPPOE TCP���1452 PPPOE UDP���1464 MACRAW�����1514��
void W5500_SetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TOS);					//����һ��socket�ķ��������ֶ�TOS(ͨ������Ϊ0����)
u8 W5500_GetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);								//��ȡһ��socket�ķ��������ֶ�TOS
void W5500_SetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TTL);					//����һ��socket��IP����ʱ��TTL
u8 W5500_GetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);								//��ȡһ��socket��IP����ʱ��TTL
void W5500_SetOneSocketRxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize);	//����һ��socket�Ľ��ջ�������С
void W5500_SetOneSocketTxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize);	//����һ��socket�ķ��ͻ�������С
u16 W5500_GetOneSocketFreeTxBuffSzie(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);					//��ȡһ��socket�ķ��ͻ�����ʣ���С(����ʣ��ռ��С�ķ������ݲ�������)
u16 W5500_GetOneSocketTxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//��ȡһ��socket�ķ��Ͷ�ָ�루����ͨ��OPEN������г�ʼ������TCP�����ڼ�Ҳ�ᱻ��ʼ����SEND����ᷢ��TX_RD��TX_WR֮������ݣ�
void W5500_SetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr);				//����һ��socket�ķ���дָ�루����ͨ��OPEN������г�ʼ������TCP�����ڼ�Ҳ�ᱻ��ʼ����SEND����ᷢ��TX_RD��TX_WR֮������ݣ�ʹ��SEND����ᷢ��TX_RD��TX_WR֮������ݣ�
u16 W5500_GetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//��ȡһ��socket�ķ���дָ�루����ͨ��OPEN������г�ʼ������TCP�����ڼ�Ҳ�ᱻ��ʼ����SEND����ᷢ��TX_RD��TX_WR֮������ݣ�
u16 W5500_GetOneSocketRxDataSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);						//��ȡһ��socket�Ľ������ݴ�С
void W5500_SetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr);				//����һ��socket�Ľ��ն�ָ�루����ͨ��OPEN�����ʼ�������ý��ջ����������ݵ��׵�ַ,����д����Ϊһ����������ջ�������ͨ��RECV����֪ͨW5500����RXRD��
u16 W5500_GetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//��ȡһ��socket�Ľ��ն�ָ�루����ͨ��OPEN�����ʼ������ȡ�����ڽ��ջ����������ݵ��׵�ַ��
u16 W5500_GetOneSocketRxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//��ȡһ��socket�Ľ���дָ�루����ͨ��OPEN������г�ʼ�����������ݵĽ��ջ����������һ��ظ����ǣ�
void W5500_SetOneSocketNotFrag(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//�ر�һ��socket�ֶΣ�һ��ʹ�ö����鲻Ҫ�ֶΣ�
void W5500_SetOneSocketTcpHeartPackTime(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 Time5sCount);//����һ��socket TCP������������ʱ�䣨��λ��5��,�������Ϊ0���رգ�










/***************************************���ò���(����)************************************************************************************/



//��ȡsocket���ж�״̬-ÿһ��bit����һ��socket�Ƿ������ж�
__inline u8 W5500_GetSocketTotalInterruptStatus(W5500_HANDLE *pHandle) 
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_SIR_REG_0x17);
}

//���socket���ж�״̬-ÿһ��bit����һ��socket�ж��������ӦλΪ1�������
__inline void W5500_ClearSocketTotalInterruptStatus(W5500_HANDLE *pHandle, u8 SokctBit) 
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_SIR_REG_0x17, SokctBit);
}

//��ȡsocket���жϿ���״̬-ÿһ��bit����һ��socket�Ƿ������ж�
__inline u8 W5500_GetSocketTotalInterruptEnable(W5500_HANDLE *pHandle) 
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_SIMR_REG_0x18);
}

//����socket���ж�״̬-ÿһ��bit����һ��socket�Ƿ������ж�
__inline void W5500_SetSocketTotalInterruptEnable(W5500_HANDLE *pHandle, u8 SokctBit) 
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_SIMR_REG_0x18, SokctBit);	
}

//��ȡW5500оƬ�汾��W5500�汾Ĭ��Ϊ0x04��
__inline u8 W5500_GetVersion(W5500_HANDLE *pHandle) 
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_VERSIONR_REG_0x39);
}




//���ñ���IP��ַ
__inline void W5500_SetLocalIP(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SIPR0_REG_0x0F, ip, 4);	//����IP
}

//��������IP��ַ
__inline void W5500_SetGatewayAddr(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_GAR0_REG_0x01, ip, 4);	//��������
}

//������������
__inline void W5500_SetMaskAddr(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SUBR0_REG_0x05, ip, 4);	//������������
}

//���ñ���MACӲ����ַ
__inline void W5500_SetMAC(W5500_HANDLE *pHandle, u8 mac[6])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SHAR0_REG_0x09, mac, 6);	//����MACӲ����ַ
}

//��ȡ����MACӲ����ַ
__inline void W5500_GetMAC(W5500_HANDLE *pHandle, u8 mac[6])
{
	W5500_ReadMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SHAR0_REG_0x09, mac, 6);	//��ȡMACӲ����ַ
}

//W5500��λ-��ö���ȴ���λ���,����1ms
__inline void W5500_SoftwareReset(W5500_HANDLE *pHandle)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E, 0x18);	//PHY��λ
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_MODE_REG_0x00, 0x80);		//�����λ	
}

//�����жϵ͵���ʱ�䣬2���ж����͵ļ��ʱ��(ʱ������4/CLK)
__inline void W5500_SetIntLevel(W5500_HANDLE *pHandle, u16 LevelTime)
{
	W5500_WriteDuadReg(pHandle, SELECT_GREG_00, W5500_INTLEVEL0_REG_0x13, LevelTime);
}

//�����ش���ʱʱ��-��λ100us
__inline void W5500_SetRetryTime(W5500_HANDLE *pHandle, u16 Time100us)
{
	W5500_WriteDuadReg(pHandle, SELECT_GREG_00, W5500_RTR0_REG_0x19, Time100us);	
}

//��ȡ�ش���ʱʱ��-��λ100us
__inline u16 W5500_GetRetryTime(W5500_HANDLE *pHandle)
{
	return W5500_ReadDuadReg(pHandle, SELECT_GREG_00, W5500_RTR0_REG_0x19);		
}

//W5500���÷������Դ���,RetryCount+1�Σ�������ᴥ����ʱ�ж�
__inline void W5500_SetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_RCR_REG_0x1B, RetryCount);	
}

//W5500��ȡ�������Դ���,RetryCount+1�Σ�������ᴥ����ʱ�ж�
__inline u8 W5500_GetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_RCR_REG_0x1B);	
}

//W5500��ȡ�޷��ִ��IP��ַ
__inline void W5500_GetUnableIP(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_ReadMultiReg(pHandle, SELECT_GREG_00, W5500_UIPR0_REG_0x28, ip, 4);	
}

//W5500��ȡ�޷��ִ�Ķ˿�
__inline u16 W5500_GetUnablePort(W5500_HANDLE *pHandle)
{
	return W5500_ReadDuadReg(pHandle, SELECT_GREG_00, W5500_UPORTR0_REG_0x2C);		
}


//W5500����һ��socktΪTCPģʽ(socketѡ��0-7)
__inline void W5500_SetOneSocketTcpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return ;    //������Χ
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00, (SOCKET_TCP&0x07) | BIT5);  //����ΪTCPģʽ������������ʱACK
}

//W5500����һ��socktΪUDPģʽ(socketѡ��0-7)-��Ҫ�������鲥IP��˿ڣ��ٵ��ô˺���
__inline void W5500_SetOneSocketUdpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, bool isEnableBroadcast)
{
    u8 data = 0;
    
    if(socket > 7) return;    		//������Χ
    data = SOCKET_UDP & 0x07;
    if(isEnableBroadcast)           //ʹ���˹㲥
    {
        //data |= BIT7;               //�����㲥-������ֻ�ܷ��͹㲥���ݰ������Բ�Ҫ������ֻ��Ҫ�����㲥���ݰ����ռ���
        data &= ~BIT6;              //�رչ㲥����
    }
    else //û�п����㲥ģʽ
    {
        data |= BIT6;              //���ù㲥����
    }
    //Ĭ��ʹ��IGMP�汾2���رյ���ģʽ����
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00, data);
}

//W5500����һ��socktΪMACRAWģʽ(ֻ��ʹ��socket 0)
__inline void W5500_SetOneSocket0MacrawMode(W5500_HANDLE *pHandle, bool isMACRAW, bool isEnableBroadcast )
{
    u8 data;
    
    data = SOCKET_MACRAW & 0x07;
    if(isEnableBroadcast)           //ʹ���˹㲥
    {
        data |= BIT7;               //�����㲥
        data &= ~BIT5;              //�رչ㲥����
    }
    else
    {
        data |= BIT5;              //���ù㲥����
    }
    data |= BIT4;                   //IPv6���ݰ�����
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[0], Sn_MR_0x00, data);
}

//W5500����һ��socket����(socketѡ��0-7, SocketCmd:socket����)���ڿ���һ��socket״̬
__inline void W5500_SendOneSocketCmd(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, W5500_SOCKET_CMD SocketCmd)
{
    if(socket > 7) return ;    //������Χ
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_CR_0x01, SocketCmd);
}


//��ȡһ��socket״̬
__inline W5500_SOCKET_STATUS W5500_GetOneSocketStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket) 
{
    if(socket > 7) return SOCK_CLOSED;    //������Χ
	return (W5500_SOCKET_STATUS)W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_SR_0x03);
}

//����һ��socket�ı��ض˿�
__inline void W5500_SetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LocalPort)
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_PORT_0x04, LocalPort); 
}

//��ȡһ��socket�ı��ض˿�
__inline u16 W5500_GetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_PORT_0x04); 
}

//����һ��socket��Ŀ��MAC��ַ(����UDPģʽ�£�ʹ��Send_MAC���������Ŀ������MAC��ַ�����ߴ洢ARP��ȡ����MAC��ַ)
__inline void W5500_SetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6])
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DHAR_0x06, MAC, 6); 
}

//��ȡһ��socket��Ŀ��MAC��ַ(����UDPģʽ�£�ʹ��Send_MAC��������ǰ���õ�Ŀ������MAC��ַ�����ߴ洢ARP��ȡ����MAC��ַ)
__inline void W5500_GetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6])
{
    if(socket > 7) return ;    //������Χ
    W5500_ReadMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DHAR_0x06, MAC, 6); 
}

//����һ��socket��Ŀ��ip��ַ(����TCP/UDPģʽ����Ŀ������IP��ַ�����ߴ洢TCP/UDP Serverģʽ�´ӻ�IP��ַ)
__inline void W5500_SetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 ip[4])
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DIPR_0x0C, (u8 *)ip, 4); 
}

//��ȡһ��socket��Ŀ��ip��ַ(���ڻ�ȡTCP/UDPģʽ����Ŀ������IP��ַ�����߻�ȡTCP/UDP Serverģʽ�´ӻ�IP��ַ)
__inline void W5500_GetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 ip[4])
{
    if(socket > 7) return ;    //������Χ
    W5500_ReadMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DIPR_0x0C, ip, 4); 
}

//����һ��socket��Ŀ��˿�(����TCP/UDPģʽ����Ŀ�������˿ڣ����ߴ洢TCP/UDP Serverģʽ�´ӻ��˿�)
__inline void W5500_SetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 DestPort)
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DPORT_0x10, DestPort);
}

//��ȡһ��socket��Ŀ��˿�(���ڻ�ȡTCP/UDPģʽ����Ŀ�������˿ڣ����߻�ȡTCP/UDP Serverģʽ�´ӻ��˿�)
__inline u16 W5500_GetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DPORT_0x10); 
}

//����һ��socket������䵥Ԫ��TCP���1460��UDP���1472��PPPOE TCP���1452 PPPOE UDP���1464 MACRAW�����1514��
__inline void W5500_SetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 MaxTransUnit)
{
    if(socket > 7) return ;    //������Χ
    if(MaxTransUnit < 1) MaxTransUnit = 1;
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MSSR_0x12, MaxTransUnit);
}

//��ȡһ��socket������䵥Ԫ��TCP���1460��UDP���1472��PPPOE TCP���1452 PPPOE UDP���1464 MACRAW�����1514��
__inline u16 W5500_GetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MSSR_0x12); 
}

//����һ��socket�ķ��������ֶ�TOS(ͨ������Ϊ0����)
__inline void W5500_SetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TOS)
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TOS_0x15, TOS);
}

//��ȡһ��socket�ķ��������ֶ�TOS
__inline u8 W5500_GetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TOS_0x15); 
}

//����һ��socket��IP����ʱ��TTL
__inline void W5500_SetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TTL)
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TTL_0x16, TTL);
}

//��ȡһ��socket��IP����ʱ��TTL
__inline u8 W5500_GetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TTL_0x16); 
}

//����һ��socket�Ľ��ջ�������С
__inline void W5500_SetOneSocketRxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize)
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RXBUFF_SIZE_0x1E, BuffSize);
}

//����һ��socket�ķ��ͻ�������С
__inline void W5500_SetOneSocketTxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize)
{
    if(socket > 7) return ;    //������Χ
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TXBUFF_SIZE_0x1F, BuffSize);
}

//��ȡһ��socket�ķ��ͻ�����ʣ���С(����ʣ��ռ��С�ķ������ݲ�������)
__inline u16 W5500_GetOneSocketFreeTxBuffSzie(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_FSR_0x20); 
}
//��ȡһ��socket�ķ��Ͷ�ָ�루����ͨ��OPEN������г�ʼ������TCP�����ڼ�Ҳ�ᱻ��ʼ����SEND����ᷢ��TX_RD��TX_WR֮������ݣ�
__inline u16 W5500_GetOneSocketTxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_RD_0x22); 
}

//����һ��socket�ķ���дָ�루����ͨ��OPEN������г�ʼ������TCP�����ڼ�Ҳ�ᱻ��ʼ����SEND����ᷢ��TX_RD��TX_WR֮������ݣ�ʹ��SEND����ᷢ��TX_RD��TX_WR֮������ݣ�
__inline void W5500_SetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr)
{
    if(socket > 7) return;    //������Χ
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_WR_0x24, ptr); 
}

//��ȡһ��socket�ķ���дָ�루����ͨ��OPEN������г�ʼ������TCP�����ڼ�Ҳ�ᱻ��ʼ����SEND����ᷢ��TX_RD��TX_WR֮������ݣ�
__inline u16 W5500_GetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_WR_0x24); 
}

//��ȡһ��socket�Ľ������ݴ�С
__inline u16 W5500_GetOneSocketRxDataSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_RSR_0x26); 
}

//����һ��socket�Ľ��ն�ָ�루����ͨ��OPEN�����ʼ�������ý��ջ����������ݵ��׵�ַ,����д����Ϊһ����������ջ�������ͨ��RECV����֪ͨW5500����RXRD��
__inline void W5500_SetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr)
{
    if(socket > 7) return;    //������Χ
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_RD_0x28, ptr); 
}

//��ȡһ��socket�Ľ��ն�ָ�루����ͨ��OPEN�����ʼ������ȡ�����ڽ��ջ����������ݵ��׵�ַ��
__inline u16 W5500_GetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_RD_0x28); 
}

//��ȡһ��socket�Ľ���дָ�루����ͨ��OPEN������г�ʼ�����������ݵĽ��ջ����������һ��ظ����ǣ�
__inline u16 W5500_GetOneSocketRxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //������Χ
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_WR_0x2A); 
}


//�ر�һ��socket�ֶΣ�һ��ʹ�ö����鲻Ҫ�ֶΣ�
__inline void W5500_SetOneSocketNotFrag(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return;   //������Χ
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_FRAG_0x2D, 0x4000);     //��Ҫ�ֶ�
}

//����һ��socket TCP������������ʱ�䣨��λ��5��,�������Ϊ0���رգ�
__inline void W5500_SetOneSocketTcpHeartPackTime(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 Time5sCount)
{
    if(socket > 7) return;   //������Χ
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_KPALVTR_0x2F, Time5sCount);     //������ʱ��
}

/*************************************************************************************************************************************/


#endif /*_W5500_H_*/

