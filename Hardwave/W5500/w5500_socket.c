/*************************************************************************************************************
 * �ļ���:			w5500_socket.c
 * ����:			socket�ӿں���
 * ����:			cp1300@139.com
 * ����ʱ��:		2016-02-22
 * ����޸�ʱ��:	2019-01-14
 * ��ϸ:			TCP/UDPЭ��ջ,�����OS��ʹ�ã���Ҫϵͳ��ʱ�Լ��ź���֧�֡�
					���ʹ�õ�ǰsocket��ʽ����W5500���벻Ҫֱ�ӵ���W5500 API��ȷ�����в�������ʹ�õ�ǰ���е�socket�ӿڡ�
					��ʹ��ϵͳ�ź���ȷ�����̰߳�ȫ��
					
*************************************************************************************************************/
#include "system.h"
#include "W5500.h"
#include "w5500_socket.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "board.h"

static const char * pError = "";							//ȫ�ִ���״̬
static bool isDeviceInit = FALSE;							//�豸�Ƿ��ʼ��
static W5500_HANDLE g_W5500Handle;							//W5500���

static int WaitingReception(SOCKET socket, u32 timeout);	//�ȴ���������
static bool CheckSocketConnectionState(SOCKET socket);		//�����������״̬
static int WaitingSuccessfully(SOCKET socket, u32 timeout);	//�ȴ����ͳɹ�

static W5500_HANDLE *sg_This_W5500_Handle = &g_W5500Handle;	//��ǰ��soket���W5500���
//socket����
typedef struct
{
	sockaddr	addr;		//��ַ
	IPPROTO		ipproto;	//ͨ��Э��
	u32			SendTimeOut;//���ͳ�ʱʱ�䣬��λms
	u32			RecvTimeOut;//���ճ�ʱʱ�䣬��λms
	u32			ConnTimeOut;//���ӳ�ʱʱ�䣬��λms
	u16 		LocalPort;	//���ض˿�
	bool		isServer;	//�Ƿ�Ϊ������
	bool 		isBind;		//�Ƿ�󶨳ɹ�
	bool		isInit;		//�Ƿ��ʼ��
	bool		isOccupy;	//�Ƿ�ռ��
}socket_status;

socket_status	SocketStatusBuff[SOCKET_CH_NUM];	//socket���Լ���

//Э��ջӲ����ʼ��
int WSAStartup(void)
{
	u8 MAC[6];
	u16 temp;
	u8 i;
	
	isDeviceInit = TRUE;	//Ӳ����ʼ��
	memset(SocketStatusBuff,0,sizeof(socket_status)*SOCKET_CH_NUM);
/****************************************************************************************************************************************/	
//�˴�������Ҫ��������Ӳ��ƽ̨������ֲ	
	W5500_PowerON();			//W5500��Դʹ��
	W5500_HardwaveInit();		//W5500Ӳ���ӿڳ�ʼ��
	OSTimeDlyHMSM(0,0,1,0);		//��ʱ1��ȷ��Ӳ���ϵ����
	//MAC��ַ���˴�ʹ��cpuid���ɣ���CPU ID����CRC16�����м�2�ֽڣ����2�ֽڹ̶������4�ֽ�ʹ��CPU ID���4�ֽ�
	temp = CRC16((const u8*)g_CPU_ID, 12);	
	
	MAC[0] = 0x44;
	MAC[1] = 0x58;
	MAC[2] = temp >> 8;
	MAC[3] = temp;
	MAC[4] = g_CPU_ID[10];
	MAC[5] = g_CPU_ID[11];
	
	/*MAC[0] = 0x50;
	MAC[1] = 0x46;
	MAC[2] = 0x5D;
	MAC[3] = 0x5A;
	MAC[4] = 0xAD;
	MAC[5] = 0x58;*/
	
	uart_printf("����MAC��ַ:");
	for(i = 0;i < 6;i ++)
	{
		uart_printf("%02X",MAC[i]);
	}
	uart_printf("\r\n");
		
	if(W5500_Init(sg_This_W5500_Handle, MAC, W5500_SetCS, W5500_GetInt, W5500_ReadWrtieByte, OS_Sleep, "cp1300@139.com", TRUE, FALSE) == FALSE) //��ʼ��W5500
	{
		return -1;	//��ʼ��ʧ��
	}
/****************************************************************************************************************************************/		
	
	return 0;		//��ʼ���ɹ�
}



//��ȡ����״̬
const char *socket_GetError(void)
{
	return pError;
}


/*************************************************************************************************************************
* ����			:	SOCKET socket(IPPROTO protocol)
* ����			:	��ʼ��socket
* ����			:	protocol��ͨ��Э�鶨��
* ����			:	-1����ʼ��ʧ�ܣ�������socket���
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2018-01-14
* ˵��			: 	��
*************************************************************************************************************************/
SOCKET socket(IPPROTO protocol)
{
	u8 i;
	
	if(sg_This_W5500_Handle == NULL)
	{
		pError = "��Ч�ľ��!";
		return SOCKET_INVALID;
	}
	if(isDeviceInit == FALSE)
	{
		pError = "Ӳ��δ��ʼ��!";
		return SOCKET_INVALID;
	}
	switch(protocol)
	{
		case IPPROTO_TCP:		//TCPЭ��
		case IPPROTO_UDP:		//UDP����
		case IPPROTO_UDP_BROAD:	//UDP�㲥
		{
			
		}break;
		default:
		{
			pError = "��Ч��Э��!";
			return SOCKET_INVALID;
		}
	}
	
	
	for(i = 0;i < SOCKET_CH_NUM;i ++)				//Ѱ�ҿ��е�socket
	{
		if(SocketStatusBuff[i].isOccupy == FALSE)	//����δ��ʹ�õ�socket
		{			
			memset(&SocketStatusBuff[i], 0, sizeof(socket_status));
			SocketStatusBuff[i].isOccupy = TRUE;	//��ǰsocket�Ѿ���ռ��
			SocketStatusBuff[i].ipproto = protocol;	//��¼ͨ��Э��
			SocketStatusBuff[i].SendTimeOut = 0;	//���ͳ�ʱʱ�䣬��λms-Ĭ��0���ȴ�
			SocketStatusBuff[i].RecvTimeOut = 0;	//���ճ�ʱʱ�䣬��λms-Ĭ��0���ȴ�
			SocketStatusBuff[i].ConnTimeOut = 10000;//���ӳ�ʱʱ�䣬��λms-Ĭ��10��
			pError = "";
			return (SOCKET)i;
		}
	}
	
	pError = "��ȡsocketʧ�ܣ���Դ����!";
	return SOCKET_INVALID;
}



/*************************************************************************************************************************
* ����			:	u16 RandPort(void)
* ����			:	����һ������˿�
* ����			:	socket:socket��address:��ַ
* ����			:	�˿�
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-14
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	��
*************************************************************************************************************************/
u16 RandPort(void)
{
	u8 i;
	u16 port = rand()%60000 + 2000;
	
	srand(SYS_GetOSRunTime());								//�������������-�˴�ʹ�õ�OSϵͳ����ʱ����Ϊ��������ӣ��������ֲ�������Ϊ�Լ�����������ӣ������������ʱ��ȵ�
	for(i = 0;i < SOCKET_CH_NUM;i ++)
	{
		if(SocketStatusBuff[i].LocalPort == port) break;	//�ҵ�����ͬ�Ķ˿�
	}
	if(i < SOCKET_CH_NUM)	//����ͬ�Ķ˿�
	{
		uart_printf("***��������ͬ������˿�:%d\r\n", port);
		port += rand()%1000;
		uart_printf("***�������ɶ˿�:%d\r\n", port);
	}
	else
	{
		uart_printf("***������ɶ˿�:%d\r\n", port);
	}
	
	return port;
}

/*************************************************************************************************************************
* ����			:	int bind(SOCKET socket, const sockaddr* address)
* ����			:	��socket
* ����			:	socket:socket��address:��ַ
* ����			:	-1����ʼ��ʧ�ܣ�socket��ţ��󶨳ɹ�
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
int bind(SOCKET socket, const sockaddr* address)
{
	W5500_SOCKET_ERROR W5500Error;
	u8 ip[4] = {0,0,0,0};
	
	if((socket<0) || (socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return INVALID_SOCKET;
	}
	SocketStatusBuff[socket].isInit = FALSE;	//û�г�ʼ��
	switch(SocketStatusBuff[socket].ipproto)	//ͨ��Э��
	{
		case IPPROTO_TCP			://				//TCPЭ��
		{
			W5500Error = W5500_CreateTcpServer(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, address->port);					//����һ��TCP�����
		}break;
		case IPPROTO_UDP			://				//UDP����
		{
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, address->port, ip, 0, FALSE);	//UDP����
		}break;
		case IPPROTO_UDP_BROAD		://				//UDP�㲥
		{
			//ʹ�öಥģʽ����Ҫ�ڿ���ǰ����Ŀ��IP��ַ��Ŀ��˿�Ϊ�㲥��ַ
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, address->port, address->addr, address->port, TRUE);		//UDP�㲥
		}break;
		default:
		{
			pError = "��Ч��Э��!";
			return INVALID_SOCKET;
		}
	}
	switch(W5500Error)
	{
		case SOCKET_OK				:	//�ɹ�
		{
			SocketStatusBuff[socket].LocalPort = address->port;	//��¼���ض˿�
			SocketStatusBuff[socket].isInit = TRUE;				//��ʼ���ɹ�
			pError = "";
			return socket;			
		}
		case SOCKET_OPEN_ERROR 		:	//��SOCKET����
		{
			pError = "��socket����";
		}break;
		case SOCKET_TIMEOUT 		:	//��ʱ
		{
			pError = "��socket��ʱ";
		}break;
		default:
		{
			pError = "δ֪����";
		}break;
	}

	return -1;	//��ʼ��ʧ��
}



/*************************************************************************************************************************
* ����			:	int connect(SOCKET socket, const sockaddr* address)
* ����			:	���ӷ�����
* ����			:	socket:socket��address:��ַ
* ����			:	-1����ʼ��ʧ�ܣ�socket��ţ��󶨳ɹ�
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	��
*************************************************************************************************************************/
int connect(SOCKET socket, const sockaddr* address)
{
	W5500_SOCKET_ERROR W5500Error;
	
	if(((int)socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return INVALID_SOCKET;
	}
	SocketStatusBuff[socket].isInit = FALSE;	//û�г�ʼ��
	switch(SocketStatusBuff[socket].ipproto)	//ͨ��Э��
	{
		case IPPROTO_TCP			://				//TCPЭ��
		{
			W5500Error = W5500_CreateTcpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, RandPort(), address->addr, address->port, SocketStatusBuff[socket].ConnTimeOut/1000+1);		//����һ��TCP�ͻ���
		}break;
		case IPPROTO_UDP			://				//UDP����
		{
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, RandPort(), address->addr, address->port, FALSE);	//UDP����
		}break;
		case IPPROTO_UDP_BROAD		://				//UDP�㲥
		{
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, RandPort(), address->addr, address->port, TRUE);		//UDP�㲥
		}break;
		default:
		{
			pError = "��Ч��Э��!";
			return INVALID_SOCKET;
		}
	}
	switch(W5500Error)
	{
		case SOCKET_OK				:	//�ɹ�
		{
			SocketStatusBuff[socket].isInit = TRUE;		//��ʼ���ɹ�
			pError = "";
			return socket;			
		}
		case SOCKET_OPEN_ERROR 		:	//��SOCKET����
		{
			pError = "��socket����";
		}break;
		case SOCKET_TIMEOUT 		:	//��ʱ
		{
			pError = "���ӳ�ʱ";
		}break;
		default:
		{
			pError = "δ֪����";
		}break;
	}

	return -1;	//��ʼ��ʧ��
}


/*************************************************************************************************************************
* ����			:	static int WaitingReception(SOCKET socket, u32 timeout)
* ����			:	�ȴ���������
* ����			:	socket:socket��timeout:��ʱʱ�䣬��λms
* ����			:	-1����ȡʧ�ܣ�>=0�����յ������ݳ���
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	������Ҫ�ȴ���������
*************************************************************************************************************************/
static int WaitingReception(SOCKET socket, u32 timeout)
{
	u32 i;
	u32 time20ms = (timeout)/20 + 1;
	u8 CheckConnectCnt = 0;						//��ʱ�����������
	
	if(SocketStatusBuff[socket].RecvTimeOut > 10)
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;											//���ӶϿ�
		for(i = 0;i < time20ms;i ++)
		{
			
			if(W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket) == 0)			//û���յ����ݣ�ֱ����ʱ
			{
				socket_sleep(20);																			//��ʱ20ms
			}
			else 
			{
				socket_sleep(2);	
				break;	//�յ������ˣ��˳�
			}
			
			//��ʱ�����������
			CheckConnectCnt ++;
			if(CheckConnectCnt  > 15)
			{
				CheckConnectCnt  = 0;
				if(CheckSocketConnectionState(socket) == FALSE) return -1;									//���ӶϿ�
			}
		}	
	}
	else if(SocketStatusBuff[socket].RecvTimeOut > 0) 		//0-10ms
	{
		if(W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket) == 0)				//û���յ����ݣ�ֱ����ʱ
		{
			socket_sleep(SocketStatusBuff[socket].RecvTimeOut);	//С��10msֱ����ʱ�ȴ�
		}	
	}

	return W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);					//���ػ�ȡ��������С
}


/*************************************************************************************************************************
* ����			:	static int WaitingSuccessfully(SOCKET socket, u32 timeout)
* ����			:	�ȴ����ͳɹ�
* ����			:	socket:socket��timeout:��ʱʱ�䣬��λms
* ����			:	-1�����ͳ�ʱ�����ӶϿ���0���������
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	ͨ���ȴ��ж��Ƿ����˷��ͳɹ��жϻ��ͳ�ʱ�ж�
*************************************************************************************************************************/
static int WaitingSuccessfully(SOCKET socket, u32 timeout)
{
	u32 i;
	u32 time20ms = (timeout)/20 + 1;
	u8 CheckConnectCnt = 0;						//��ʱ�����������
	u8 SocketIntStatus;
	
	if(SocketStatusBuff[socket].RecvTimeOut > 10)
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;											//���ӶϿ�
		for(i = 0;i < time20ms;i ++)
		{		
			SocketIntStatus = W5500_GetOneSocketIntStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);							//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
			if(SocketIntStatus & SOCKET_IR_SEND_OK_BIT)			//���������
			{
				return 1;
			}
			else if(SocketIntStatus & SOCKET_IR_TIMEOUT_BIT)	//���ͳ�ʱ��
			{
				return -1;
			}
			else if(SocketIntStatus & SOCKET_IR_DISCON_BIT)		//�Է��Ͽ�������
			{
				return -1;
			}
			else
			{
				socket_sleep(20);																			//��ʱ20ms
			}

			//��ʱ�����������
			CheckConnectCnt ++;
			if(CheckConnectCnt  > 15)
			{
				CheckConnectCnt  = 0;
				if(CheckSocketConnectionState(socket) == FALSE) return -1;									//���ӶϿ�
			}
		}	
	}
	else if(SocketStatusBuff[socket].RecvTimeOut > 0) 		//0-10ms
	{
		socket_sleep(SocketStatusBuff[socket].RecvTimeOut);	//С��10msֱ����ʱ�ȴ�
	}
		
	SocketIntStatus = W5500_GetOneSocketIntStatus(sg_This_W5500_Handle,  (W5500_SOCKET_NUM)socket);									//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
	if(SocketIntStatus & SOCKET_IR_SEND_OK_BIT)			//���������
	{
		return 1;
	}
	else if(SocketIntStatus & SOCKET_IR_TIMEOUT_BIT)	//���ͳ�ʱ��
	{
		return -1;
	}
	else if(SocketIntStatus & SOCKET_IR_DISCON_BIT)		//�Է��Ͽ�������
	{
		return -1;
	}
	return -1;	//��ʱ
}


/*************************************************************************************************************************
* ����			:	static bool CheckSocketConnectionState(SOCKET socket)
* ����			:	�����������״̬
* ����			:	socket:socket��
* ����			:	FALSE:δ���ӣ�TRUE:������
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	���ڼ������״̬
*************************************************************************************************************************/
static bool CheckSocketConnectionState(SOCKET socket)
{
	W5500_SOCKET_STATUS SocketStatus;
	
	switch(SocketStatusBuff[socket].ipproto)	//ͨ��Э��
	{
		case IPPROTO_TCP		:				//TCPЭ��
		{		
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//��ȡsocket״̬
			if(SocketStatusBuff[socket].isBind == TRUE)	//����˼���
			{
				if(SocketStatus != SOCK_LISTEN)
				{
					pError = "�����δ�������!";
					return FALSE;
				}
			}
			else
			{
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					pError = "δ����!";
					return FALSE;
				}
			}
		}break;
		case IPPROTO_UDP			://				//UDP����
		case IPPROTO_UDP_BROAD		://				//UDP�㲥
		{
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//��ȡsocket״̬
			if(SocketStatus != SOCK_UDP)
			{
				pError = "δ����!";
				return FALSE;
			}			
		}break;
		default:
		{
			pError = "��Ч��Э��!";
			return FALSE;
		}
	}
	
	return TRUE;
}


/*************************************************************************************************************************
* ����			:	recvfrom(SOCKET socket, u8* buff, u32 len, socketaddr* from)
* ����			:	��ȡ����
* ����			:	socket:socket��buff:���ݻ�������len�����ݻ�������󳤶ȣ�from�����ͷ���ַ
* ����			:	-1����ȡʧ�ܣ�>=0�����յ������ݳ���
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	����UDPģʽ
*************************************************************************************************************************/
int recvfrom(SOCKET socket, u8* buff, u32 buffsize, socketaddr* from)
{
	int len;
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socketû�г�ʼ��!";
		return INVALID_SOCKET;
	}
	
	if(SocketStatusBuff[socket].RecvTimeOut > 0)								//��Ҫ�ȴ���ʱ
	{
		len = WaitingReception(socket, SocketStatusBuff[socket].RecvTimeOut); 	//�ȴ���������
	}
	else
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;																		//���ӶϿ�
		len = W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);												//��ȡ��������С
	}
	if(len <= 0)  return len;						//û���յ�����
	
	switch(SocketStatusBuff[socket].ipproto)		//ͨ��Э��
	{
		case IPPROTO_TCP			://				//TCPЭ��
		{
			W5500_GetOneSocketDestIP(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket,from->addr);										//��ȡ�Է�IP
			from->port = W5500_GetOneSocketDestPort(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);									//��ȡ�Է��˿�
			
			if(len > buffsize) len = buffsize;
			W5500_ReadOneSocketTcpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len);									//��ȡĳһ��socket TCP���ջ���������
			
			pError = "";
			return len;
		}
		case IPPROTO_UDP			://				//UDP����
		case IPPROTO_UDP_BROAD		://				//UDP�㲥
		{
			if((len-8) > buffsize) len = (buffsize+8);
			len = W5500_ReadOneSocketUdpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len, from->addr, &from->port);	//��ȡĳһ��socket UDP���ջ���������
			
			pError = "";
			return len;
		}
		default:
		{
			pError = "��Ч��Э��!";
			return INVALID_SOCKET;
		}
	}
}






/*************************************************************************************************************************
* ����			:	int recv(SOCKET socket, u8* buff, u32 buffsize)
* ����			:	��ȡ����
* ����			:	socket:socket��buff:���ݻ�������len�����ݻ�������󳤶ȣ�
* ����			:	-1����ȡʧ�ܣ�>=0�����յ������ݳ���
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	����TCPģʽ
*************************************************************************************************************************/
int recv(SOCKET socket, u8* buff, u32 buffsize)
{
	int len;
	u8 ip[4];
	u16 port;
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socketû�г�ʼ��!";
		return INVALID_SOCKET;
	}
	
	if(SocketStatusBuff[socket].RecvTimeOut > 0)								//��Ҫ�ȴ���ʱ
	{
		len = WaitingReception(socket, SocketStatusBuff[socket].RecvTimeOut); 	//�ȴ���������
	}
	else
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;																		//���ӶϿ�
		len = W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);												//��ȡ��������С
	}
	if(len <= 0)  return len;						//û���յ�����
	
	switch(SocketStatusBuff[socket].ipproto)	//ͨ��Э��
	{
		case IPPROTO_TCP			://				//TCPЭ��
		{
			if(len > buffsize) len = buffsize;
			W5500_ReadOneSocketTcpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len);									//��ȡĳһ��socket TCP���ջ���������
			
			pError = "";
			return len;
		}
		case IPPROTO_UDP			://				//UDP����
		case IPPROTO_UDP_BROAD		://				//UDP�㲥
		{
			if((len-8) > buffsize) len = (buffsize+8);
			len = W5500_ReadOneSocketUdpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len, ip, &port);					//��ȡĳһ��socket UDP���ջ���������
			
			pError = "";
			return len;
		}
		default:
		{
			pError = "����:��Ч��Э��!";
			return INVALID_SOCKET;
		}
	}
}



/*************************************************************************************************************************
* ����			:	int send(SOCKET socket, const u8* buff, u32 size)
* ����			:	��������
* ����			:	socket:socket��buff:���ݻ�������size���������ݳ��ȣ�
* ����			:	-1������ʧ�ܣ�>0�����͵����ݳ���;0:���ͳ�ʱ��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-14
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	����TCP��������
*************************************************************************************************************************/
int send(SOCKET socket, const u8* buff, u32 size)
{
	W5500_SOCKET_ERROR W5500Error = SOCKET_OPEN_ERROR;
	W5500_SOCKET_STATUS SocketStatus;
	
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socketû�г�ʼ��!";
		return INVALID_SOCKET;
	}
	
	if(size > 2048) size = 2048;				//������󳤶�
	switch(SocketStatusBuff[socket].ipproto)	//ͨ��Э��
	{
		case IPPROTO_TCP		://				//TCPЭ��
		{		
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//��ȡsocket״̬
			if(SocketStatusBuff[socket].isBind == TRUE)	//����˼���
			{
				if(SocketStatus != SOCK_LISTEN)
				{
					pError = "�����δ�������!";
					return INVALID_SOCKET;
				}
			}
			else
			{
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					pError = "δ����!";
					return INVALID_SOCKET;
				}
			}
			
			
			W5500Error = W5500_WriteOneSocketTcpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size);	//TCP��������
		}break;
		case IPPROTO_UDP			://				//UDP����
		case IPPROTO_UDP_BROAD		://				//UDP�㲥
		{
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//��ȡsocket״̬
			if(SocketStatus != SOCK_UDP)
			{
				pError = "δ����!";
				return INVALID_SOCKET;
			}
			
			W5500Error = W5500_WriteOneSocketUdpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size, NULL, 0);
		}break;
		default:
		{
			pError = "��Ч��Э��!";
			return INVALID_SOCKET;
		}
	}
	
	switch(W5500Error)
	{
		case SOCKET_OK			:	//SOCKET OK
		{
			//����OKʱ����Ƿ����˳�ʱ
			if(SocketStatusBuff[socket].SendTimeOut > 0)				//�����˲ż��
			{
				if(WaitingSuccessfully(socket, SocketStatusBuff[socket].SendTimeOut) < 0)	//�ȴ����ͳɹ�
				{
					pError = "���ͳ�ʱ";
					return 0;
				}
				
			}
			
			pError = "";			
			return size;
		}
		case SOCKET_OPEN_ERROR 	:	//��SOCKET����
		{
			pError = "��Ч�� socket!";
			return INVALID_SOCKET;
		}
		case SOCKET_TIMEOUT 	:	//��ʱ
		{
			pError = "���ӳ�ʱ";
			return INVALID_SOCKET;
		}
		case SOCKET_TXBUFF_SIZE	:	//���ͻ�����ʣ��ռ䲻��
		{
			pError = "����æ�����ݳ�����Χ!";
			return INVALID_SOCKET;
		}
		case SOCKET_ERROR 		:	//socket�Ƿ�
		{
			pError = "�Ƿ��� socket!";
			return INVALID_SOCKET;
		}
		default:
		{
			pError = "δ֪����!";
			return INVALID_SOCKET;
		}
	}
}


/*************************************************************************************************************************
* ����			:	int sendto(SOCKET socket, const u8* buff, u32 size, const sockaddr *to)
* ����			:	�������ݵ�ָ����ַ
* ����			:	socket:socket��buff:���ݻ�������size���������ݳ��ȣ�to��Ŀ���ַ
* ����			:	-1������ʧ�ܣ�>0�����͵����ݳ��ȣ�0:���ͳ�ʱ��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2015-01-26
* ����޸�ʱ��	:	2015-01-26
* ˵��			: 	����UDP�������ݻ�TCP server
*************************************************************************************************************************/
int sendto(SOCKET socket, const u8* buff, u32 size, const sockaddr *to)
{
	W5500_SOCKET_ERROR W5500Error;
	W5500_SOCKET_STATUS SocketStatus;
	
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socketû�г�ʼ��!";
		return INVALID_SOCKET;
	}
	
	if(size > 2048) size = 2048;				//������󳤶�
	switch(SocketStatusBuff[socket].ipproto)	//ͨ��Э��
	{
		case IPPROTO_TCP		:				//TCPЭ��
		{		
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//��ȡsocket״̬
			if(SocketStatusBuff[socket].isBind == TRUE)	//����˼���
			{
				if(SocketStatus != SOCK_LISTEN)
				{
					pError = "�����δ�������!";
					return INVALID_SOCKET;
				}
			}
			else
			{
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					pError = "δ����!";
					return INVALID_SOCKET;
				}
			}
			
			
			W5500Error = W5500_WriteOneSocketTcpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size);	//TCP��������
		}break;
		case IPPROTO_UDP			://				//UDP����
		case IPPROTO_UDP_BROAD		://				//UDP�㲥
		{
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//��ȡsocket״̬
			if(SocketStatus != SOCK_UDP)
			{
				pError = "δ����!";
				return INVALID_SOCKET;
			}
			
			W5500Error = W5500_WriteOneSocketUdpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size, (u8 *)to->addr, to->port);
		}break;
		default:
		{
			pError = "��Ч��Э��!";
			return INVALID_SOCKET;
		}
	}
	
	switch(W5500Error)
	{
		case SOCKET_OK			:	//SOCKET OK
		{
			//����OKʱ����Ƿ����˳�ʱ
			if(SocketStatusBuff[socket].SendTimeOut > 0)				//�����˲ż��
			{
				if(WaitingSuccessfully(socket, SocketStatusBuff[socket].SendTimeOut) < 0)	//�ȴ����ͳɹ�
				{
					pError = "���ͳ�ʱ";
					return 0;
				}				
			}

			pError = "";
			return size;
		}
		case SOCKET_OPEN_ERROR 	:	//��SOCKET����
		{
			pError = "��Ч�� socket!";
			return INVALID_SOCKET;
		}
		case SOCKET_TIMEOUT 	:	//��ʱ
		{
			pError = "���ӳ�ʱ";
			return INVALID_SOCKET;
		}
		case SOCKET_TXBUFF_SIZE	:	//���ͻ�����ʣ��ռ䲻��
		{
			pError = "����æ�����ݳ�����Χ!";
			return INVALID_SOCKET;
		}
		case SOCKET_ERROR 		:	//socket�Ƿ�
		{
			pError = "�Ƿ��� socket!";
			return INVALID_SOCKET;
		}
		default:
		{
			pError = "δ֪����!";
			return INVALID_SOCKET;
		}
	}
}



/*************************************************************************************************************************
* ����			:	void closesocket(SOCKET socket)
* ����			:	�ر�һ��socket
* ����			:	socket:socket��
* ����			:	��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-14
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	���ڹر�һ��socket
*************************************************************************************************************************/
void closesocket(SOCKET socket)
{
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return;
	}
	W5500_DisconnectOneSocket(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);		//�Ͽ���ǰ����
	memset(&SocketStatusBuff[socket], 0, sizeof(socket_status));								//���socket�������ڴ�
	W5500_CloseOneSocket(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);			//ǿ�ƹر�һ��socket
}



/*************************************************************************************************************************
* ����			:	void setsockopt(SOCKET socket, int optname, u32 opt)
* ����			:	����socket��ѡ��
* ����			:	socket:socket��optname:�����õ�ѡ��;opt:���õ�ֵ
* ����			:	��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-14
* ����޸�ʱ��	:	2019-01-14
* ˵��			: 	SO_RCVTIMEO			//�������ݳ�ʱʱ�䣬��λms,��С10ms��Ĭ������Ϊ0��������
					SO_SNDTIMEO			//�������ݳ�ʱʱ�䣬��λms,��С10ms��Ĭ������Ϊ0��������
					SO_CONNTIMEO		//���ӳ�ʱʱ�䣬��λms,��С10ms��Ĭ������Ϊ10000
*************************************************************************************************************************/
void setsockopt(SOCKET socket, int optname, u32 opt)
{
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket��Ч!";
		return;
	}
	
	switch(optname) //�����õ�ѡ��
	{
		case SO_SNDTIMEO:	//���ͳ�ʱʱ��
		{
			SocketStatusBuff[socket].SendTimeOut = opt;		//���ͳ�ʱʱ�䣬��λms-Ĭ��0���ȴ�
		}break;
		case SO_RCVTIMEO:	//���ճ�ʱʱ��
		{
			SocketStatusBuff[socket].RecvTimeOut = opt;		//���ճ�ʱʱ�䣬��λms-Ĭ��0���ȴ�
		}break;
		case SO_CONNTIMEO:	//���ӳ�ʱʱ��
		{
			if(opt < 1000) opt = 1000;						//���ӳ�ʱ��С��1��
			SocketStatusBuff[socket].ConnTimeOut = opt;		//���ӳ�ʱʱ�䣬��λms-Ĭ��10��
		}break;
		default:break;
	}
}


/*************************************************************************************************************************
* ����			:	int getsockopt(SOCKET socket, int cmd)
* ����			:	��ȡsocket��ز���
* ����			:	socket:socket��cmd:�������
* ����			:	������Ҫ��ȡ��ֵ
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	SOL_FIONREAD	1		//��ȡ���ջ����������ݳ���
					SOL_SOCKET		2		//��ȡһ������״̬�Ƿ���Ч-��Ч����1����Ч����0
					SOL_IPPROTO		3		//��ȡ��ǰsocketЭ������
*************************************************************************************************************************/
int getsockopt(SOCKET socket, int cmd)
{
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "����ʧ��:socket��Ч!";
		return 0;
	}
	
	switch(cmd) //�����õ�ѡ��
	{
		case SOL_FIONREAD:	//��ȡ���ջ��������ݳ���
		{
			return W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);
		}
		case SOL_SOCKET:	//��ȡ����״̬�Ƿ���Ч
		{
			return (CheckSocketConnectionState(socket)==TRUE)?1:0;
		}
		case SOL_IPPROTO:	//��ȡЭ������
		{
			if(SocketStatusBuff[socket].isInit == FALSE) return SOCKET_CLOSED;	//��Ч
			return SocketStatusBuff[socket].ipproto;							//����Э������
		}
		default: return 0;
	}
}



/*************************************************************************************************************************
* ����			:	void netsh_ip_addr(u8 ip[4])
* ����			:	���ñ���ip��ַ
* ����			:	ip
* ����			:	��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	��ʼ��Ӳ�����������
*************************************************************************************************************************/
void netsh_ip_addr(u8 ip[4])
{
	W5500_SetLocalIP(sg_This_W5500_Handle, ip);		//���ñ���IP��ַ
}


/*************************************************************************************************************************
* ����			:	void netsh_ip_mask(u8 ip[4])
* ����			:	���ñ�����������
* ����			:	ip
* ����			:	��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	��ʼ��Ӳ�����������
*************************************************************************************************************************/
void netsh_ip_mask(u8 ip[4])
{
	W5500_SetMaskAddr(sg_This_W5500_Handle, ip);		//������������
}


/*************************************************************************************************************************
* ����			:	void netsh_ip_gateway(u8 ip[4])
* ����			:	���ñ������ص�ַ
* ����			:	ip
* ����			:	��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	��ʼ��Ӳ�����������
*************************************************************************************************************************/
void netsh_ip_gateway(u8 ip[4])
{
	W5500_SetGatewayAddr(sg_This_W5500_Handle, ip);		//��������IP��ַ
}

/*************************************************************************************************************************
* ����			:	bool InternetGetConnectedState(void)
* ����			:	��ȡ��������״̬
* ����			:	��
* ����			:	TRUE:��������������FALSE:���������쳣
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	��ʼ��Ӳ�����ȡ�����Ƿ�����
*************************************************************************************************************************/
bool InternetGetConnectedState(void)
{
	if((W5500_GetPHY_Status(sg_This_W5500_Handle) & W5500_PHY_STATUS_LNK_BIT) == 0)				//W5500 ��ȡPHY״̬
	{
		return FALSE;	//����δ����
	}
	else
	{
		return TRUE;	//�������ӳɹ�
	}
}

/*************************************************************************************************************************
* ����			:	void getLocalMAC(u8 mac[6])
* ����			:	��ȡ����MAC��ַ
* ����			:	mac��mac�洢������
* ����			:	��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	
*************************************************************************************************************************/
void getLocalMAC(u8 mac[6])
{
	W5500_GetMAC(sg_This_W5500_Handle, mac);			//��ȡ����MACӲ����ַ
}


/*************************************************************************************************************************
* ����			:	void getLocalHostName(char HostName[16+1], u8 BuffSize)
* ����			:	��ȡ��������
* ����			:	HostName�����ƻ�������BuffSize��buff��С
* ����			:	��
* ����			:	W5500
* ����			:	cp1300@139.com
* ʱ��			:	2019-01-15
* ����޸�ʱ��	:	2019-01-15
* ˵��			: 	
*************************************************************************************************************************/
void getLocalHostName(char HostName[16+1], u8 BuffSize)
{
	u8 len = strlen(sg_This_W5500_Handle->HostName);
	
	if(BuffSize <= 1 || HostName == NULL) return;
	if(len > (BuffSize-1)) len = BuffSize-1;
	memcpy(HostName, sg_This_W5500_Handle->HostName, len);
	HostName[len] = '\0';
}



