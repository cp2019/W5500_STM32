/*************************************************************************************************************
 * �ļ���:			W5500_TASK.c
 * ����:			��̫��ģ��W5500�����߳�
 * ����:			cp1300@139.com
 * ����ʱ��:		2018-12-20
 * ����޸�ʱ��:	2018-12-20
 * ��ϸ:			�ײ����				
*************************************************************************************************************/
#include "system.h"
#include "usart.h"
#include "main.h"
#include "w5500.h"
#include "userTASK.h"
#include "board.h"
#include "wdg.h"
#include "BOARD.h"

W5500_HANDLE g_W5500Handle;

u8 DataBuff[2048];



//TCP�ͻ��˲���
void TCP_Client_Test(void);
//UDP�ͻ��˲���
void UDP_Client_Test(void);
//TCP����˲���
void TCP_Server_Test(void);


//W5500��������
void TaskW5500(void *pdata)
{
	u8 SocketIntBit;
	u8 SocketIntStatus = 0;
	u8 MAC[6] = {0x50,0x46,0x5D, 0X5A, 0XAD, 0X58};
	u8 ch = 0;	//��¼�ĸ�socket���͵�����
	//u8 LocalIP[4] = {192,168,1,200};
	//u8 GatewayIP[4] = {192,168,1,1};
	u8 LocalIP[4] = {10,254,1,200};
	u8 GatewayIP[4] = {10,254,1,1};	
	u8 Mask[4] = {255,255,255,0};

	
	W5500_PowerON();			//W5500��Դʹ��
	W5500_HardwaveInit();		//W5500Ӳ���ӿڳ�ʼ��
	OSTimeDlyHMSM(0,0,1,100);
	W5500_Init(&g_W5500Handle, MAC, W5500_SetCS, W5500_GetInt, W5500_ReadWrtieByte, OS_Sleep, "STM32", TRUE, FALSE);//��ʼ��W5500
	
	

	W5500_SetLocalIP(&g_W5500Handle, LocalIP);						//���ñ���IP
	W5500_SetGatewayAddr(&g_W5500Handle, GatewayIP);				//��������IP
	W5500_SetMaskAddr(&g_W5500Handle, Mask);						//������������
	OSTimeDlyHMSM(0,0,1,0);	
	
	uart_printf("��ʼ��W5500�ɹ�!\r\n");
	
	
	while(1)
	{
		TCP_Client_Test();		//TCP�ͻ��˲���
		//UDP_Client_Test();		//UDP�ͻ��˲���
		//TCP_Server_Test();			//TCP����˲���
	}
}


//�������δ���ӣ���ȴ���ֱ���������ӻ�ʱ
bool WaitNetworkConnection(u16 TimeOutSecond)
{
	u8 PHY_Status;
	u32 count = 0;
	
	do
	{
		PHY_Status = W5500_GetPHY_Status(&g_W5500Handle);					//W5500 ��ȡPHY״̬
		if((PHY_Status & W5500_PHY_STATUS_LNK_BIT) == 0)	//����δ����
		{
			/*count ++;
			OSTimeDlyHMSM(0,0,1,0);
			if(count > TimeOutSecond)	//��ʱ��
			{
				uart_printf("�ȴ��������糬ʱ��\r\n");
				return FALSE;
			}*/
			uart_printf("�ȴ���������...\r\n");
			return TRUE;
		}
		else
		{
			uart_printf("�������ӳɹ���\r\n");
			return TRUE;
		}
		
	}while(1);
	
	
	
}


//TCP�ͻ��˲���
void TCP_Client_Test(void)
{
	bool isConnectServer = FALSE;				//���ӷ�����״̬
	W5500_SOCKET_ERROR SocketError;				//����״̬
	W5500_SOCKET_NUM socket = W5500_SECKET5;	//����һ��socket���в���
	u8 DestIp[4] = {10,254,1,100};
	u8 SocketIntBit;							//socket���ж�״̬
	u8 SocketIntStatus;
	u16 len;
	u8 ip[4];
	u16 port;
	u8 CheckNetworkDelay = 200;
	W5500_SOCKET_STATUS SocketStatus;
	
	//�˴����ڲ���UDPģʽ��arp��ʱ-ò����ô���ᳬʱ
	SocketError = W5500_CreateUdpClient(&g_W5500Handle, socket, 67, DestIp, 5000, FALSE);			//����һ��UDP�ͻ��˲����ӷ�����
	if(SocketError == SOCKET_OK)
	{
		//Ϊ��Ҫ�ȷ���һ�Σ���һ�η��Ϳ϶��ᳬʱ��ԭ��δ֪
		//W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 10,DestIp, 5000);	//���յ������ݷ��ͳ�ȥ
		//OSTimeDlyHMSM(0,0,1,500);
		W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, 0XFF);
		
		uart_printf("ϵͳ�ж�״̬1��%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
		SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
		uart_printf("�ж�״̬1��%02X\r\n", SocketIntStatus);
		DataBuff[0] = 'a';
		

		W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 10,DestIp, 5000);	//���յ������ݷ��ͳ�ȥ
			OSTimeDlyHMSM(0,0,0,500);
		
		W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 10,DestIp, 5000);	//���յ������ݷ��ͳ�ȥ
		
		SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
		uart_printf("�ж�״̬2��%02X\r\n", SocketIntStatus);
		OSTimeDlyHMSM(0,0,1,0);
		SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
		
		uart_printf("�ж�״̬3��%02X\r\n", SocketIntStatus);		
		uart_printf("ϵͳ�ж�״̬2��%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
	}
	
	
	while(1)
	{

		//���ӷ�����ʧ��
		while(isConnectServer == FALSE)	//����������״̬��Ч����һֱ���ӷ�����
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 5)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
			}
			
			uart_printf("ϵͳ�ж�״̬��%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
			
			SocketError = W5500_CreateTcpClient(&g_W5500Handle, socket, 1234, DestIp, 1209, 5);			//����һ��TCP�ͻ��˲����ӷ�����
			if(SocketError == SOCKET_OK) //���ӳɹ�
			{
				isConnectServer = TRUE;		
				break;
			}
			else //����ʧ�ܣ���ʱ������
			{
				OSTimeDlyHMSM(0,0,1,0);
			}
		}
		
		//���ӷ������ɹ�
		while(isConnectServer == TRUE)	//���������ӳɹ����������ݽ��պ�ش�
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 50)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
				
				//��Ҫ��ʱ��ѯÿ��socket״̬����Ϊ�쳣����������»��������Ͽ�������ȴ���ܼ�ʱ����
				SocketStatus = W5500_GetOneSocketStatus(&g_W5500Handle, socket);
				uart_printf("socket״̬��0x%02X\r\n",SocketStatus);
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					uart_printf("�������쳣�Ͽ������ˣ����Ͻ���������\r\n");
					isConnectServer = FALSE;
				}
				
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//��ȡsocket���ж�״̬��ÿһ��bit����һ��socket
				uart_printf("socket���ж�״̬=%02X\r\n",SocketIntBit);
								
			}
			
			
			if(W5500_GetInt() == 0)		//�����ж�
			{
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//��ȡsocket���ж�״̬��ÿһ��bit����һ��socket
				uart_printf("socket���ж�״̬=%02X\r\n",SocketIntBit);
				if(SocketIntBit & (1<<socket))	//��ǰsocket���ж�
				{
					SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
					W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, SocketIntStatus);				//����ж�
					if(SocketIntStatus & SOCKET_IR_DISCON_BIT)					//�������Ͽ�������
					{
						isConnectServer = FALSE;
						uart_printf("�������Ͽ������ˣ����Ͻ���������\r\n");
						break;
					}
					else if(SocketIntStatus & SOCKET_IR_RECV_BIT)									//�յ�������
					{
						len = W5500_GetOneSocketRxDataSize(&g_W5500Handle, socket);					//��ȡ��ǰ�������ݴ�С
						if(len > 0)
						{
							W5500_GetOneSocketDestIP(&g_W5500Handle, socket,ip);					//��ȡ�Է�IP
							port = W5500_GetOneSocketDestPort(&g_W5500Handle, socket);				//��ȡ�Է��˿�
							W5500_ReadOneSocketTcpRxData(&g_W5500Handle, socket, DataBuff, len);	//��ȡĳһ��socket���ջ���������
							if(len > 2047)len = 2047;
							DataBuff[len] = 0;	//���ӽ�����
							uart_printf(">�յ�����%d.%d.%d.%d:%d ����%dB:%s\r\n",ip[0],ip[1],ip[2],ip[3],port,len,(char *)DataBuff);
							
							
							W5500_WriteOneSocketTcpTxData(&g_W5500Handle, socket, DataBuff, len);		//���յ������ݷ��ͳ�ȥ
						}
					}
				}
			}
			OSTimeDlyHMSM(0,0,0,100);
		}
		
		OSTimeDlyHMSM(0,0,1,0);
	}
}





//UDP�ͻ��˲���-UDPʵ���ϲ����ֿͻ���������
void UDP_Client_Test(void)
{
	bool isConnectServer = FALSE;				//���ӷ�����״̬
	W5500_SOCKET_ERROR SocketError;				//����״̬
	W5500_SOCKET_NUM socket = W5500_SECKET5;	//����һ��socket���в���
	u8 DestIp[4] = {10,254,1,100};
	u8 SocketIntBit;							//socket���ж�״̬
	u8 SocketIntStatus;
	u16 len;
	u8 ip[4];
	u16 port;
	u8 CheckNetworkDelay = 200;
	W5500_SOCKET_STATUS SocketStatus;
	

	while(1)
	{

		//���ӷ�����ʧ��
		while(isConnectServer == FALSE)	//����������״̬��Ч����һֱ���ӷ�����
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 5)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
			}
			
			uart_printf("ϵͳ�ж�״̬��%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
			
			SocketError = W5500_CreateUdpClient(&g_W5500Handle, socket, 67, DestIp, 60000, TRUE);			//����һ��UDP�ͻ��˲����ӷ�����
			if(SocketError == SOCKET_OK) //���ӳɹ�
			{
				isConnectServer = TRUE;		
				break;
			}
			else //����ʧ�ܣ���ʱ������
			{
				OSTimeDlyHMSM(0,0,1,0);
			}
		}
		
		
		
		//���ӷ������ɹ�
		while(isConnectServer == TRUE)	//���������ӳɹ����������ݽ��պ�ش�
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 50)
			{
				//W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 1, DestIp, 60000);	//���յ������ݷ��ͳ�ȥ
				
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);

				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//��ȡsocket���ж�״̬��ÿһ��bit����һ��socket
				uart_printf("socket���ж�״̬=%02X\r\n",SocketIntBit);
								
			}
			
			
			if(W5500_GetInt() == 0)		//�����ж�
			{
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//��ȡsocket���ж�״̬��ÿһ��bit����һ��socket
				uart_printf("socket���ж�״̬=%02X\r\n",SocketIntBit);
				if(SocketIntBit & (1<<socket))	//��ǰsocket���ж�
				{
					SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
					W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, SocketIntStatus);				//����ж�
					if(SocketIntStatus & SOCKET_IR_DISCON_BIT)					//�������Ͽ�������
					{
						isConnectServer = FALSE;
						uart_printf("�������Ͽ������ˣ����Ͻ���������\r\n");
						break;
					}
					else if(SocketIntStatus & SOCKET_IR_RECV_BIT)								//�յ�������
					{
						len = W5500_GetOneSocketRxDataSize(&g_W5500Handle, socket);				//��ȡ��ǰ�������ݴ�С
						if(len > 0)
						{
							len = W5500_ReadOneSocketUdpRxData(&g_W5500Handle, socket, DataBuff, len, ip, &port);//��ȡĳһ��socket UDP���ջ���������
							if(len > 2047)len = 2047;
							DataBuff[len] = 0;	//���ӽ�����
							uart_printf(">�յ�����%d.%d.%d.%d:%d ����%dB:%s\r\n",ip[0],ip[1],ip[2],ip[3],port,len,(char *)DataBuff);
							
							
							W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, len, ip, port);	//���յ������ݷ��ͳ�ȥ
						}
					}
				}
			}
			OSTimeDlyHMSM(0,0,0,100);
		}
		
		OSTimeDlyHMSM(0,0,1,0);
	}
}






//TCP����˲���
void TCP_Server_Test(void)
{
	bool isConnectServer = FALSE;				//���ӷ�����״̬
	W5500_SOCKET_ERROR SocketError;				//����״̬
	W5500_SOCKET_NUM socket = W5500_SECKET5;	//����һ��socket���в���
	u8 DestIp[4] = {192,168,1,117};
	u8 SocketIntBit;							//socket���ж�״̬
	u8 SocketIntStatus;
	u16 len;
	u8 ip[4];
	u16 port;
	u8 CheckNetworkDelay = 200;
	W5500_SOCKET_STATUS SocketStatus;
	

	while(1)
	{

		//���ӷ�����ʧ��
		while(isConnectServer == FALSE)	//����������״̬��Ч����һֱ���ӷ�����
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 5)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
			}
			
			uart_printf("ϵͳ�ж�״̬��%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
			
			SocketError = W5500_CreateTcpServer(&g_W5500Handle, socket, 100);			//����һ��TCP�����
			if(SocketError == SOCKET_OK) //���ӳɹ�
			{
				isConnectServer = TRUE;		
				break;
			}
			else //����ʧ�ܣ���ʱ������
			{
				OSTimeDlyHMSM(0,0,1,0);
			}
		}
		
		
		
		//���ӷ������ɹ�
		while(isConnectServer == TRUE)	//���������ӳɹ����������ݽ��պ�ش�
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 50)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);

				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//��ȡsocket���ж�״̬��ÿһ��bit����һ��socket
				uart_printf("socket���ж�״̬=%02X\r\n",SocketIntBit);
								
			}
			
			
			if(W5500_GetInt() == 0)		//�����ж�
			{
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//��ȡsocket���ж�״̬��ÿһ��bit����һ��socket
				uart_printf("socket���ж�״̬=%02X\r\n",SocketIntBit);
				if(SocketIntBit & (1<<socket))	//��ǰsocket���ж�
				{
					SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);		//��ȡ��ǰsocket�жϣ��ж��ж�ԭ��
					W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, SocketIntStatus);		//����ж�
					if(SocketIntStatus & SOCKET_IR_DISCON_BIT)									//�ͻ��˶Ͽ�����
					{
						uart_printf("�ͻ��Ͽ������ˣ�\r\n");
						break;
					}
					else if(SocketIntStatus & SOCKET_IR_RECV_BIT)								//�յ�������
					{
						len = W5500_GetOneSocketRxDataSize(&g_W5500Handle, socket);					//��ȡ��ǰ�������ݴ�С
						if(len > 0)
						{
							W5500_GetOneSocketDestIP(&g_W5500Handle, socket,ip);					//��ȡ�Է�IP
							port = W5500_GetOneSocketDestPort(&g_W5500Handle, socket);				//��ȡ�Է��˿�
							W5500_ReadOneSocketTcpRxData(&g_W5500Handle, socket, DataBuff, len);	//��ȡĳһ��socket���ջ���������
							if(len > 2047)len = 2047;
							DataBuff[len] = 0;	//���ӽ�����
							uart_printf(">�յ�����%d.%d.%d.%d:%d ����%dB:%s\r\n",ip[0],ip[1],ip[2],ip[3],port,len,(char *)DataBuff);
							
							
							W5500_WriteOneSocketTcpTxData(&g_W5500Handle, socket, DataBuff, len);		//���յ������ݷ��ͳ�ȥ
						}
					}
					else if(SocketIntStatus & SOCKET_IR_CON_BIT)								//�ͻ��˽�������
					{
						W5500_GetOneSocketDestIP(&g_W5500Handle, socket,ip);					//��ȡ�Է�IP
						port = W5500_GetOneSocketDestPort(&g_W5500Handle, socket);				//��ȡ�Է��˿�
						uart_printf(">�ͻ�%d.%d.%d.%d:%d ����\r\n",ip[0],ip[1],ip[2],ip[3],port);
					}
				}
			}
			OSTimeDlyHMSM(0,0,0,100);
		}
		
		OSTimeDlyHMSM(0,0,1,0);
	}
}


