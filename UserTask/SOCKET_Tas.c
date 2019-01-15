/*************************************************************************************************************
 * �ļ���:			SOCKET_Task.c
 * ����:			SOCKET�����߳�
 * ����:			cp1300@139.com
 * ����ʱ��:		2019-01-15
 * ����޸�ʱ��:	2019-01-15
 * ��ϸ:			�ײ����				
*************************************************************************************************************/
#include "system.h"
#include "usart.h"
#include "main.h"
#include "userTASK.h"
#include "board.h"
#include "wdg.h"
#include "BOARD.h"
#include "W5500_Socket.h"
#include "DHCP.h"

//TCP �ͻ����շ����ݲ���
static void TCP_Client_Test(void);
//TCP�������ղ���
static void TCP_Client_BlockingRece(void);
//UDP��������
static void UDP_Client_BlockingRece(void);
//UDP����˲���
static void UDP_Server_BlockingRece(void);
//UDP����˹㲥����
static void UDP_Server_BroadBlockingRece(void);
//UDP�㲥����
static void UDP_Client_BroadSend(void);
//ʹ��UDP���ͳ�ʱ̽��IP�Ƿ����
static void UDP_ScanningIP(void);


static u8 DataBuff[2048];

//socket��������
void SOCKET_Task(void *pdata)
{
	u8 ip[4] = {0,0,0,0};
	u8 mask[4] = {0,0,0,0};
	u8 gateway[4] = {0,0,0,0};
	u8 dhcp_ret;
	
	while(WSAStartup()!=0)				//��ʼ������Ӳ��
	{
		OSTimeDlyHMSM(0,0,3,0);
	}
	
	
	while(InternetGetConnectedState() == FALSE)
	{
		uart_printf("�ȴ���������...\r\n");
		OSTimeDlyHMSM(0,0,1,0);
	}
	
	uart_printf("��ʼ��DHCP����ʼ���л�ȡIP...\r\n");
	netsh_ip_addr(ip);				//���ñ���ip��ַ
	netsh_ip_mask(mask);				//���ñ�����������
	netsh_ip_gateway(gateway);			//���ñ������ص�ַ
	DHCP_init(SOCKET_INVALID, DataBuff);		//��ʼ��DHCP����Ҫһ���ܴ�Ļ������������1KB����
	do	//ѭ������DHCP
	{
		dhcp_ret = DHCP_run();//�ص���DHCP_run()���ú���ʵ�ֶ�̬����IP�Ĺ���
		OSTimeDlyHMSM(0,0,1,0);	
		DHCP_time_handler();
	}while(dhcp_ret != DHCP_IP_LEASED);
	DHCP_stop();			//ֹͣDHCP���ͷ�socket
	
	if(dhcp_ret == DHCP_IP_LEASED) //�Զ���ȡIP�ɹ�
	{
		uart_printf("\r\n");
		uart_printf("      IP:%d.%d.%d.%d\r\n", DHCP_allocated_ip[0],DHCP_allocated_ip[1],DHCP_allocated_ip[2],DHCP_allocated_ip[3]);
		uart_printf("  ��  ��:%d.%d.%d.%d\r\n", DHCP_allocated_gw[0],DHCP_allocated_gw[1],DHCP_allocated_gw[2],DHCP_allocated_gw[3]);
		uart_printf("��������:%d.%d.%d.%d\r\n", DHCP_allocated_sn[0],DHCP_allocated_sn[1],DHCP_allocated_sn[2],DHCP_allocated_sn[3]);
		uart_printf("     DNS:%d.%d.%d.%d\r\n", DHCP_allocated_dns[0],DHCP_allocated_dns[1],DHCP_allocated_dns[2],DHCP_allocated_dns[3]);
		uart_printf("\r\n");
		
		netsh_ip_addr(DHCP_allocated_ip);				//���ñ���ip��ַ
		netsh_ip_mask(DHCP_allocated_sn);				//���ñ�����������
		netsh_ip_gateway(DHCP_allocated_gw);			//���ñ������ص�ַ
	}
	
	
	
	
	
	
	while(1) //��ʼ����ɺ�ʼ���в���
	{
		//TCP_Client_Test();					//TCP�ͻ��˲���
		//TCP_Client_BlockingRece();			//TCP�ͻ��˽�����������		
		//UDP_Client_BlockingRece();//UDP��������
		//UDP_Server_BlockingRece();			//UDP����˲���
		//UDP_Server_BroadBlockingRece();		//UDP�㲥����˲���
		//UDP_Client_BroadSend();					//UDP�㲥����
		UDP_ScanningIP();						//ɨ������
	}
}



//TCP �ͻ����շ����ݲ���
static void TCP_Client_Test(void)
{
	SOCKET TestSocket = socket(IPPROTO_TCP);		//��ʼ��socket-TCPЭ��
	sockaddr address;
	int len;
	u32 count = 0;
	
	
	if(SOCKET_INVALID == TestSocket)				//��ʼ��ʧ��
	{
		uart_printf("��ʼ��socketʧ�ܣ�����%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//������ɴ�����
		}
	}
	uart_printf("��ʼ��socket�ɹ�,׼�����ӷ�����\r\n");
	
	while(1)
	{
		//���ӷ�����
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 117;
			address.port = 1204;		//Ŀ���������IP��˿�
			if(connect(TestSocket, &address) < 0)		//���ӵ�ָ��������
			{
				uart_printf("���ӷ�����ʧ�ܣ�����%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//���ӳɹ�
			}
		}
		
		
		//�շ����ݲ���
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);
			
			len = sprintf((char *)DataBuff, "�������ݲ��ԣ�%d��\r\n", count++);
			len = send(TestSocket, DataBuff, len);							//��������
			if(len < 0)
			{
				//�������Ͽ�������
				break;
			}
			
			//�������ݲ���
			len = recv(TestSocket, DataBuff, 2047);							//��������
			if(len < 0)
			{
				//�������Ͽ�������
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("�յ�����(%dB):%s\r\n", len, (char *)DataBuff);
			}
		}
	}
}



//TCP�������ղ���
static void TCP_Client_BlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_TCP);		//��ʼ��socket-TCPЭ��
	sockaddr address;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//��ʼ��ʧ��
	{
		uart_printf("��ʼ��socketʧ�ܣ�����%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//������ɴ�����
		}
	}
	uart_printf("��ʼ��socket�ɹ�,׼�����ӷ�����\r\n");
	
	while(1)
	{
		//���ӷ�����
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 117;
			address.port = 1204;		//Ŀ���������IP��˿�
			if(connect(TestSocket, &address) < 0)		//���ӵ�ָ��������
			{
				uart_printf("���ӷ�����ʧ�ܣ�����%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//���ӳɹ�
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);						//���ý��ճ�ʱʱ�䣬����һ���ܴ��ʱ�䣬�൱������
		//�շ����ݲ���
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//�������ݲ���-��������
			len = recv(TestSocket, DataBuff, 2047);							//������������
			if(len < 0)
			{
				//�������Ͽ�������
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("�յ�����(%dB):%s\r\n", len, (char *)DataBuff);
				
				send(TestSocket, DataBuff, len);							//���յ������ݷ��ͳ�ȥ
			}
			else //�������������ճ�ʱ�����µȴ�
			{
				uart_printf("���ճ�ʱ\r\n");
			}
		}
	}
}




//UDP��������
static void UDP_Client_BlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP);		//��ʼ��socket-UDPЭ��
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//��ʼ��ʧ��
	{
		uart_printf("��ʼ��socketʧ�ܣ�����%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//������ɴ�����
		}
	}
	uart_printf("��ʼ��socket�ɹ�,׼�����ӷ�����\r\n");
	
	while(1)
	{
		//���ӷ�����
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 117;
			address.port = 1204;		//Ŀ���������IP��˿�
			if(connect(TestSocket, &address) < 0)		//���ӵ�ָ��������
			{
				uart_printf("���ӷ�����ʧ�ܣ�����%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//���ӳɹ�
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);									//���ý��ճ�ʱʱ�䣬����һ���ܴ��ʱ�䣬�൱������
		//�շ����ݲ���
		sendto(TestSocket, DataBuff, 1, &address);										//���յ������ݷ��ͳ�ȥ-����һ�η��ͣ�UDP���������޸�֪�ģ�������ȷ������ݹ�ȥ
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//�������ݲ���-��������
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//������������
			if(len < 0)
			{
				//�������Ͽ�������
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("�յ�����%d.%d.%d.%d:%d����(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				sendto(TestSocket, DataBuff, len, &from);								//���յ������ݷ��ͳ�ȥ
			}
			else //�������������ճ�ʱ�����µȴ�
			{
				sendto(TestSocket, DataBuff, 1, &address);								//��������һ������
				uart_printf("���ճ�ʱ\r\n");
			}
		}
	}
}




//UDP�㲥����
static void UDP_Client_BroadSend(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP);		//��ʼ��socket-UDPЭ��
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//��ʼ��ʧ��
	{
		uart_printf("��ʼ��socketʧ�ܣ�����%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//������ɴ�����
		}
	}
	uart_printf("��ʼ��socket�ɹ�,׼�����ӷ�����\r\n");
	
	while(1)
	{
		//���ӷ�����
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 255;
			address.port = 1204;		//Ŀ���������IP��˿�
			if(connect(TestSocket, &address) < 0)		//���ӵ�ָ��������
			{
				uart_printf("���ӷ�����ʧ�ܣ�����%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//���ӳɹ�
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);									//���ý��ճ�ʱʱ�䣬����һ���ܴ��ʱ�䣬�൱������
		
		
		//�շ����ݲ���
		sendto(TestSocket, DataBuff, 1, &address);										//���յ������ݷ��ͳ�ȥ-����һ�η��ͣ�UDP���������޸�֪�ģ�������ȷ������ݹ�ȥ
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//�������ݲ���-��������
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//������������
			if(len < 0)
			{
				//�������Ͽ�������
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("�յ�����%d.%d.%d.%d:%d����(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				sendto(TestSocket, DataBuff, len, &from);								//���յ������ݷ��ͳ�ȥ
			}
			else //�������������ճ�ʱ�����µȴ�
			{
				uart_printf("���ճ�ʱ\r\n");
			}
		}
	}
}


//UDP����˲���
static void UDP_Server_BlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP);		//��ʼ��socket-UDPЭ��
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//��ʼ��ʧ��
	{
		uart_printf("��ʼ��socketʧ�ܣ�����%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//������ɴ�����
		}
	}
	uart_printf("��ʼ��socket�ɹ�,׼�����ӷ�����\r\n");
	
	while(1)
	{
		//���ӷ�����
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 200;
			address.port = 1204;		//Ŀ���������IP��˿�
			if(bind(TestSocket, &address) < 0)		//���ӵ�ָ��������
			{
				uart_printf("�󶨷�����ʧ�ܣ�����%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//���ӳɹ�
			}
		}
			
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);						//���ý��ճ�ʱʱ�䣬����һ���ܴ��ʱ�䣬�൱������
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//�������ݲ���-��������
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//������������
			if(len < 0)
			{
				//�������Ͽ�������
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("�յ�����%d.%d.%d.%d:%d����(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				sendto(TestSocket, DataBuff, len, &from);								//���յ������ݷ��ͳ�ȥ
			}
			else //�������������ճ�ʱ�����µȴ�
			{
				uart_printf("���ճ�ʱ\r\n");
			}
		}
	}
}





//UDP����˹㲥����
static void UDP_Server_BroadBlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP_BROAD);	//��ʼ��socket-UDP�㲥Э��
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//��ʼ��ʧ��
	{
		uart_printf("��ʼ��socketʧ�ܣ�����%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//������ɴ�����
		}
	}
	uart_printf("��ʼ��socket�ɹ�,׼�����ӷ�����\r\n");
	
	while(1)
	{
		//���ӷ�����
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 255;
			address.port = 1204;		//Ŀ���������IP��˿�
			if(bind(TestSocket, &address) < 0)		//���ӵ�ָ��������
			{
				uart_printf("�󶨷�����ʧ�ܣ�����%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//���ӳɹ�
			}
		}
			
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);						//���ý��ճ�ʱʱ�䣬����һ���ܴ��ʱ�䣬�൱������
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//�������ݲ���-��������
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//������������
			if(len < 0)
			{
				//�������Ͽ�������
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("�յ�����%d.%d.%d.%d:%d����(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				
				sendto(TestSocket, DataBuff, len, &from);								//���յ������ݷ��ͳ�ȥ
			}
			else //�������������ճ�ʱ�����µȴ�
			{
				uart_printf("���ճ�ʱ\r\n");
			}
		}
	}
}



//ʹ��UDP���ͳ�ʱ̽��IP�Ƿ����
static void UDP_ScanningIP(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP_BROAD);		//��ʼ��socket-UDPЭ��
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//��ʼ��ʧ��
	{
		uart_printf("��ʼ��socketʧ�ܣ�����%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//������ɴ�����
		}
	}
	uart_printf("��ʼ��socket�ɹ�,׼�����ӷ�����\r\n");
	
	while(1)
	{
		//���ӷ�����
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 100;
			address.port = 1204;		//Ŀ���������IP��˿�
			if(connect(TestSocket, &address) < 0)		//���ӵ�ָ��������
			{
				uart_printf("���ӷ�����ʧ�ܣ�����%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//���ӳɹ�
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);									//���ý��ճ�ʱʱ�䣬����һ���ܴ��ʱ�䣬�൱������
		setsockopt(TestSocket, SO_SNDTIMEO, 1000);										//���÷��ͳ�ʱʱ�䣬һ�㽨�����800ms
		
		address.addr[0] = 192;
		address.addr[1] = 168;
		address.addr[2] = 1;
		address.addr[3] = 100;
		address.port = 5000;	//�������˿�
		
		while(1)
		{
			len = sprintf((char *)DataBuff, "hello");
			
			
			uart_printf("ɨ������ %d.%d.%d.%d \t", address.addr[0],address.addr[1],address.addr[2],address.addr[3]);
			len = sendto(TestSocket, DataBuff, len, &address);
			if(len > 0)			//���ͳɹ�
			{
				uart_printf("�ɹ�\r\n");
			}
			else if(len == 0) 	//���ͳ�ʱ
			{
				uart_printf("\r\n");
			}
			else //����ʧ��-UDPһ�㲻�ᵽ�˴�
			{
				//�쳣����
				uart_printf("�������쳣\r\n");
			}
			
			(address.addr[3]) += 1;
			
			OSTimeDlyHMSM(0,0,0,200);
		}
	}
}

