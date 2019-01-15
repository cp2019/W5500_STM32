/*************************************************************************************************************
 * 文件名:			SOCKET_Task.c
 * 功能:			SOCKET测试线程
 * 作者:			cp1300@139.com
 * 创建时间:		2019-01-15
 * 最后修改时间:	2019-01-15
 * 详细:			底层相关				
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

//TCP 客户端收发数据测试
static void TCP_Client_Test(void);
//TCP阻塞接收测试
static void TCP_Client_BlockingRece(void);
//UDP阻塞测试
static void UDP_Client_BlockingRece(void);
//UDP服务端测试
static void UDP_Server_BlockingRece(void);
//UDP服务端广播测试
static void UDP_Server_BroadBlockingRece(void);
//UDP广播发送
static void UDP_Client_BroadSend(void);
//使用UDP发送超时探测IP是否存在
static void UDP_ScanningIP(void);


static u8 DataBuff[2048];

//socket测试任务
void SOCKET_Task(void *pdata)
{
	u8 ip[4] = {0,0,0,0};
	u8 mask[4] = {0,0,0,0};
	u8 gateway[4] = {0,0,0,0};
	u8 dhcp_ret;
	
	while(WSAStartup()!=0)				//初始化网络硬件
	{
		OSTimeDlyHMSM(0,0,3,0);
	}
	
	
	while(InternetGetConnectedState() == FALSE)
	{
		uart_printf("等待网络连接...\r\n");
		OSTimeDlyHMSM(0,0,1,0);
	}
	
	uart_printf("初始化DHCP，开始进行获取IP...\r\n");
	netsh_ip_addr(ip);				//设置本地ip地址
	netsh_ip_mask(mask);				//设置本地子网掩码
	netsh_ip_gateway(gateway);			//设置本地网关地址
	DHCP_init(SOCKET_INVALID, DataBuff);		//初始化DHCP，需要一个很大的缓冲区，建议给1KB以上
	do	//循环进行DHCP
	{
		dhcp_ret = DHCP_run();//重点是DHCP_run()，该函数实现动态申请IP的功能
		OSTimeDlyHMSM(0,0,1,0);	
		DHCP_time_handler();
	}while(dhcp_ret != DHCP_IP_LEASED);
	DHCP_stop();			//停止DHCP，释放socket
	
	if(dhcp_ret == DHCP_IP_LEASED) //自动获取IP成功
	{
		uart_printf("\r\n");
		uart_printf("      IP:%d.%d.%d.%d\r\n", DHCP_allocated_ip[0],DHCP_allocated_ip[1],DHCP_allocated_ip[2],DHCP_allocated_ip[3]);
		uart_printf("  网  关:%d.%d.%d.%d\r\n", DHCP_allocated_gw[0],DHCP_allocated_gw[1],DHCP_allocated_gw[2],DHCP_allocated_gw[3]);
		uart_printf("子网掩码:%d.%d.%d.%d\r\n", DHCP_allocated_sn[0],DHCP_allocated_sn[1],DHCP_allocated_sn[2],DHCP_allocated_sn[3]);
		uart_printf("     DNS:%d.%d.%d.%d\r\n", DHCP_allocated_dns[0],DHCP_allocated_dns[1],DHCP_allocated_dns[2],DHCP_allocated_dns[3]);
		uart_printf("\r\n");
		
		netsh_ip_addr(DHCP_allocated_ip);				//设置本地ip地址
		netsh_ip_mask(DHCP_allocated_sn);				//设置本地子网掩码
		netsh_ip_gateway(DHCP_allocated_gw);			//设置本地网关地址
	}
	
	
	
	
	
	
	while(1) //初始化完成后开始进行测试
	{
		//TCP_Client_Test();					//TCP客户端测试
		//TCP_Client_BlockingRece();			//TCP客户端接收阻塞测试		
		//UDP_Client_BlockingRece();//UDP阻塞测试
		//UDP_Server_BlockingRece();			//UDP服务端测试
		//UDP_Server_BroadBlockingRece();		//UDP广播服务端测试
		//UDP_Client_BroadSend();					//UDP广播发送
		UDP_ScanningIP();						//扫描主机
	}
}



//TCP 客户端收发数据测试
static void TCP_Client_Test(void)
{
	SOCKET TestSocket = socket(IPPROTO_TCP);		//初始化socket-TCP协议
	sockaddr address;
	int len;
	u32 count = 0;
	
	
	if(SOCKET_INVALID == TestSocket)				//初始化失败
	{
		uart_printf("初始化socket失败，错误：%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//自行完成错误处理
		}
	}
	uart_printf("初始化socket成功,准备连接服务器\r\n");
	
	while(1)
	{
		//连接服务器
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 117;
			address.port = 1204;		//目标服务器的IP与端口
			if(connect(TestSocket, &address) < 0)		//连接到指定服务器
			{
				uart_printf("连接服务器失败，错误：%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//连接成功
			}
		}
		
		
		//收发数据测试
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);
			
			len = sprintf((char *)DataBuff, "发送数据测试：%d次\r\n", count++);
			len = send(TestSocket, DataBuff, len);							//发送数据
			if(len < 0)
			{
				//服务器断开连接了
				break;
			}
			
			//接收数据测试
			len = recv(TestSocket, DataBuff, 2047);							//接收数据
			if(len < 0)
			{
				//服务器断开连接了
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("收到数据(%dB):%s\r\n", len, (char *)DataBuff);
			}
		}
	}
}



//TCP阻塞接收测试
static void TCP_Client_BlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_TCP);		//初始化socket-TCP协议
	sockaddr address;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//初始化失败
	{
		uart_printf("初始化socket失败，错误：%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//自行完成错误处理
		}
	}
	uart_printf("初始化socket成功,准备连接服务器\r\n");
	
	while(1)
	{
		//连接服务器
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 117;
			address.port = 1204;		//目标服务器的IP与端口
			if(connect(TestSocket, &address) < 0)		//连接到指定服务器
			{
				uart_printf("连接服务器失败，错误：%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//连接成功
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);						//设置接收超时时间，设置一个很大的时间，相当于阻塞
		//收发数据测试
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//接收数据测试-阻塞接收
			len = recv(TestSocket, DataBuff, 2047);							//阻塞接收数据
			if(len < 0)
			{
				//服务器断开连接了
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("收到数据(%dB):%s\r\n", len, (char *)DataBuff);
				
				send(TestSocket, DataBuff, len);							//把收到的数据发送出去
			}
			else //阻塞结束，接收超时，重新等待
			{
				uart_printf("接收超时\r\n");
			}
		}
	}
}




//UDP阻塞测试
static void UDP_Client_BlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP);		//初始化socket-UDP协议
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//初始化失败
	{
		uart_printf("初始化socket失败，错误：%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//自行完成错误处理
		}
	}
	uart_printf("初始化socket成功,准备连接服务器\r\n");
	
	while(1)
	{
		//连接服务器
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 117;
			address.port = 1204;		//目标服务器的IP与端口
			if(connect(TestSocket, &address) < 0)		//连接到指定服务器
			{
				uart_printf("连接服务器失败，错误：%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//连接成功
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);									//设置接收超时时间，设置一个很大的时间，相当于阻塞
		//收发数据测试
		sendto(TestSocket, DataBuff, 1, &address);										//把收到的数据发送出去-启动一次发送，UDP是无连接无感知的，你必须先发送数据过去
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//接收数据测试-阻塞接收
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//阻塞接收数据
			if(len < 0)
			{
				//服务器断开连接了
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("收到来自%d.%d.%d.%d:%d数据(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				sendto(TestSocket, DataBuff, len, &from);								//把收到的数据发送出去
			}
			else //阻塞结束，接收超时，重新等待
			{
				sendto(TestSocket, DataBuff, 1, &address);								//主动发送一次数据
				uart_printf("接收超时\r\n");
			}
		}
	}
}




//UDP广播发送
static void UDP_Client_BroadSend(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP);		//初始化socket-UDP协议
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//初始化失败
	{
		uart_printf("初始化socket失败，错误：%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//自行完成错误处理
		}
	}
	uart_printf("初始化socket成功,准备连接服务器\r\n");
	
	while(1)
	{
		//连接服务器
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 255;
			address.port = 1204;		//目标服务器的IP与端口
			if(connect(TestSocket, &address) < 0)		//连接到指定服务器
			{
				uart_printf("连接服务器失败，错误：%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//连接成功
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);									//设置接收超时时间，设置一个很大的时间，相当于阻塞
		
		
		//收发数据测试
		sendto(TestSocket, DataBuff, 1, &address);										//把收到的数据发送出去-启动一次发送，UDP是无连接无感知的，你必须先发送数据过去
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//接收数据测试-阻塞接收
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//阻塞接收数据
			if(len < 0)
			{
				//服务器断开连接了
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("收到来自%d.%d.%d.%d:%d数据(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				sendto(TestSocket, DataBuff, len, &from);								//把收到的数据发送出去
			}
			else //阻塞结束，接收超时，重新等待
			{
				uart_printf("接收超时\r\n");
			}
		}
	}
}


//UDP服务端测试
static void UDP_Server_BlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP);		//初始化socket-UDP协议
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//初始化失败
	{
		uart_printf("初始化socket失败，错误：%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//自行完成错误处理
		}
	}
	uart_printf("初始化socket成功,准备连接服务器\r\n");
	
	while(1)
	{
		//连接服务器
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 200;
			address.port = 1204;		//目标服务器的IP与端口
			if(bind(TestSocket, &address) < 0)		//连接到指定服务器
			{
				uart_printf("绑定服务器失败，错误：%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//连接成功
			}
		}
			
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);						//设置接收超时时间，设置一个很大的时间，相当于阻塞
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//接收数据测试-阻塞接收
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//阻塞接收数据
			if(len < 0)
			{
				//服务器断开连接了
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("收到来自%d.%d.%d.%d:%d数据(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				sendto(TestSocket, DataBuff, len, &from);								//把收到的数据发送出去
			}
			else //阻塞结束，接收超时，重新等待
			{
				uart_printf("接收超时\r\n");
			}
		}
	}
}





//UDP服务端广播测试
static void UDP_Server_BroadBlockingRece(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP_BROAD);	//初始化socket-UDP广播协议
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//初始化失败
	{
		uart_printf("初始化socket失败，错误：%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//自行完成错误处理
		}
	}
	uart_printf("初始化socket成功,准备连接服务器\r\n");
	
	while(1)
	{
		//连接服务器
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 255;
			address.port = 1204;		//目标服务器的IP与端口
			if(bind(TestSocket, &address) < 0)		//连接到指定服务器
			{
				uart_printf("绑定服务器失败，错误：%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//连接成功
			}
		}
			
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);						//设置接收超时时间，设置一个很大的时间，相当于阻塞
		while(1)
		{
			OSTimeDlyHMSM(0,0,0,10);
			
			//接收数据测试-阻塞接收
			len = recvfrom(TestSocket, DataBuff, 2047, &from);							//阻塞接收数据
			if(len < 0)
			{
				//服务器断开连接了
				break;
			}
			else if(len > 0)
			{
				DataBuff[len] = 0;
				uart_printf("收到来自%d.%d.%d.%d:%d数据(%dB):%s\r\n",from.addr[0],from.addr[1], from.addr[2], from.addr[3], from.port , len, (char *)DataBuff);
				
				
				sendto(TestSocket, DataBuff, len, &from);								//把收到的数据发送出去
			}
			else //阻塞结束，接收超时，重新等待
			{
				uart_printf("接收超时\r\n");
			}
		}
	}
}



//使用UDP发送超时探测IP是否存在
static void UDP_ScanningIP(void)
{
	SOCKET TestSocket = socket(IPPROTO_UDP_BROAD);		//初始化socket-UDP协议
	sockaddr address;
	sockaddr from;
	int len;
	
	
	if(SOCKET_INVALID == TestSocket)				//初始化失败
	{
		uart_printf("初始化socket失败，错误：%s\r\n", socket_GetError());
		while(1)
		{
			OSTimeDlyHMSM(0,0,1,0);					//自行完成错误处理
		}
	}
	uart_printf("初始化socket成功,准备连接服务器\r\n");
	
	while(1)
	{
		//连接服务器
		while(1)
		{
			address.addr[0] = 192;
			address.addr[1] = 168;
			address.addr[2] = 1;
			address.addr[3] = 100;
			address.port = 1204;		//目标服务器的IP与端口
			if(connect(TestSocket, &address) < 0)		//连接到指定服务器
			{
				uart_printf("连接服务器失败，错误：%s\r\n", socket_GetError());
				OSTimeDlyHMSM(0,0,1,0);					
			}
			else
			{
				break;	//连接成功
			}
		}
		
		
		setsockopt(TestSocket, SO_RCVTIMEO, 1000*5);									//设置接收超时时间，设置一个很大的时间，相当于阻塞
		setsockopt(TestSocket, SO_SNDTIMEO, 1000);										//设置发送超时时间，一般建议大于800ms
		
		address.addr[0] = 192;
		address.addr[1] = 168;
		address.addr[2] = 1;
		address.addr[3] = 100;
		address.port = 5000;	//随便给个端口
		
		while(1)
		{
			len = sprintf((char *)DataBuff, "hello");
			
			
			uart_printf("扫描主机 %d.%d.%d.%d \t", address.addr[0],address.addr[1],address.addr[2],address.addr[3]);
			len = sendto(TestSocket, DataBuff, len, &address);
			if(len > 0)			//发送成功
			{
				uart_printf("成功\r\n");
			}
			else if(len == 0) 	//发送超时
			{
				uart_printf("\r\n");
			}
			else //连接失败-UDP一般不会到此处
			{
				//异常处理
				uart_printf("发生了异常\r\n");
			}
			
			(address.addr[3]) += 1;
			
			OSTimeDlyHMSM(0,0,0,200);
		}
	}
}

