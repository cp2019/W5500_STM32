/*************************************************************************************************************
 * 文件名:			W5500_TASK.c
 * 功能:			以太网模块W5500处理线程
 * 作者:			cp1300@139.com
 * 创建时间:		2018-12-20
 * 最后修改时间:	2018-12-20
 * 详细:			底层相关				
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



//TCP客户端测试
void TCP_Client_Test(void);
//UDP客户端测试
void UDP_Client_Test(void);
//TCP服务端测试
void TCP_Server_Test(void);


//W5500处理任务
void TaskW5500(void *pdata)
{
	u8 SocketIntBit;
	u8 SocketIntStatus = 0;
	u8 MAC[6] = {0x50,0x46,0x5D, 0X5A, 0XAD, 0X58};
	u8 ch = 0;	//记录哪个socket发送的数据
	//u8 LocalIP[4] = {192,168,1,200};
	//u8 GatewayIP[4] = {192,168,1,1};
	u8 LocalIP[4] = {10,254,1,200};
	u8 GatewayIP[4] = {10,254,1,1};	
	u8 Mask[4] = {255,255,255,0};

	
	W5500_PowerON();			//W5500电源使能
	W5500_HardwaveInit();		//W5500硬件接口初始化
	OSTimeDlyHMSM(0,0,1,100);
	W5500_Init(&g_W5500Handle, MAC, W5500_SetCS, W5500_GetInt, W5500_ReadWrtieByte, OS_Sleep, "STM32", TRUE, FALSE);//初始化W5500
	
	

	W5500_SetLocalIP(&g_W5500Handle, LocalIP);						//设置本地IP
	W5500_SetGatewayAddr(&g_W5500Handle, GatewayIP);				//设置网关IP
	W5500_SetMaskAddr(&g_W5500Handle, Mask);						//设置子网掩码
	OSTimeDlyHMSM(0,0,1,0);	
	
	uart_printf("初始化W5500成功!\r\n");
	
	
	while(1)
	{
		TCP_Client_Test();		//TCP客户端测试
		//UDP_Client_Test();		//UDP客户端测试
		//TCP_Server_Test();			//TCP服务端测试
	}
}


//如果网线未连接，则等待，直到网络连接或超时
bool WaitNetworkConnection(u16 TimeOutSecond)
{
	u8 PHY_Status;
	u32 count = 0;
	
	do
	{
		PHY_Status = W5500_GetPHY_Status(&g_W5500Handle);					//W5500 获取PHY状态
		if((PHY_Status & W5500_PHY_STATUS_LNK_BIT) == 0)	//网络未连接
		{
			/*count ++;
			OSTimeDlyHMSM(0,0,1,0);
			if(count > TimeOutSecond)	//超时了
			{
				uart_printf("等待连接网络超时！\r\n");
				return FALSE;
			}*/
			uart_printf("等待连接网络...\r\n");
			return TRUE;
		}
		else
		{
			uart_printf("网络连接成功！\r\n");
			return TRUE;
		}
		
	}while(1);
	
	
	
}


//TCP客户端测试
void TCP_Client_Test(void)
{
	bool isConnectServer = FALSE;				//连接服务器状态
	W5500_SOCKET_ERROR SocketError;				//错误状态
	W5500_SOCKET_NUM socket = W5500_SECKET5;	//随意一个socket进行测试
	u8 DestIp[4] = {10,254,1,100};
	u8 SocketIntBit;							//socket总中断状态
	u8 SocketIntStatus;
	u16 len;
	u8 ip[4];
	u16 port;
	u8 CheckNetworkDelay = 200;
	W5500_SOCKET_STATUS SocketStatus;
	
	//此处用于测试UDP模式下arp超时-貌似怎么都会超时
	SocketError = W5500_CreateUdpClient(&g_W5500Handle, socket, 67, DestIp, 5000, FALSE);			//创建一个UDP客户端并连接服务器
	if(SocketError == SOCKET_OK)
	{
		//为何要先发送一次，第一次发送肯定会超时，原因未知
		//W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 10,DestIp, 5000);	//将收到的数据发送出去
		//OSTimeDlyHMSM(0,0,1,500);
		W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, 0XFF);
		
		uart_printf("系统中断状态1：%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
		SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//获取当前socket中断，判断中断原因
		uart_printf("中断状态1：%02X\r\n", SocketIntStatus);
		DataBuff[0] = 'a';
		

		W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 10,DestIp, 5000);	//将收到的数据发送出去
			OSTimeDlyHMSM(0,0,0,500);
		
		W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 10,DestIp, 5000);	//将收到的数据发送出去
		
		SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//获取当前socket中断，判断中断原因
		uart_printf("中断状态2：%02X\r\n", SocketIntStatus);
		OSTimeDlyHMSM(0,0,1,0);
		SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//获取当前socket中断，判断中断原因
		
		uart_printf("中断状态3：%02X\r\n", SocketIntStatus);		
		uart_printf("系统中断状态2：%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
	}
	
	
	while(1)
	{

		//连接服务器失败
		while(isConnectServer == FALSE)	//服务器连接状态无效，则一直连接服务器
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 5)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
			}
			
			uart_printf("系统中断状态：%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
			
			SocketError = W5500_CreateTcpClient(&g_W5500Handle, socket, 1234, DestIp, 1209, 5);			//创建一个TCP客户端并连接服务器
			if(SocketError == SOCKET_OK) //连接成功
			{
				isConnectServer = TRUE;		
				break;
			}
			else //连接失败，延时后重试
			{
				OSTimeDlyHMSM(0,0,1,0);
			}
		}
		
		//连接服务器成功
		while(isConnectServer == TRUE)	//服务器连接成功，进行数据接收后回传
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 50)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
				
				//需要定时查询每个socket状态，因为异常的网络情况下会造成网络断开，但是却不能及时发现
				SocketStatus = W5500_GetOneSocketStatus(&g_W5500Handle, socket);
				uart_printf("socket状态：0x%02X\r\n",SocketStatus);
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					uart_printf("服务器异常断开连接了，马上进行重连！\r\n");
					isConnectServer = FALSE;
				}
				
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//获取socket总中断状态，每一个bit代表一个socket
				uart_printf("socket总中断状态=%02X\r\n",SocketIntBit);
								
			}
			
			
			if(W5500_GetInt() == 0)		//发生中断
			{
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//获取socket总中断状态，每一个bit代表一个socket
				uart_printf("socket总中断状态=%02X\r\n",SocketIntBit);
				if(SocketIntBit & (1<<socket))	//当前socket的中断
				{
					SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//获取当前socket中断，判断中断原因
					W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, SocketIntStatus);				//清除中断
					if(SocketIntStatus & SOCKET_IR_DISCON_BIT)					//服务器断开连接了
					{
						isConnectServer = FALSE;
						uart_printf("服务器断开连接了，马上进行重连！\r\n");
						break;
					}
					else if(SocketIntStatus & SOCKET_IR_RECV_BIT)									//收到数据了
					{
						len = W5500_GetOneSocketRxDataSize(&g_W5500Handle, socket);					//获取当前接收数据大小
						if(len > 0)
						{
							W5500_GetOneSocketDestIP(&g_W5500Handle, socket,ip);					//获取对方IP
							port = W5500_GetOneSocketDestPort(&g_W5500Handle, socket);				//获取对方端口
							W5500_ReadOneSocketTcpRxData(&g_W5500Handle, socket, DataBuff, len);	//读取某一个socket接收缓冲区数据
							if(len > 2047)len = 2047;
							DataBuff[len] = 0;	//增加结束符
							uart_printf(">收到来自%d.%d.%d.%d:%d 数据%dB:%s\r\n",ip[0],ip[1],ip[2],ip[3],port,len,(char *)DataBuff);
							
							
							W5500_WriteOneSocketTcpTxData(&g_W5500Handle, socket, DataBuff, len);		//将收到的数据发送出去
						}
					}
				}
			}
			OSTimeDlyHMSM(0,0,0,100);
		}
		
		OSTimeDlyHMSM(0,0,1,0);
	}
}





//UDP客户端测试-UDP实际上不区分客户端与服务端
void UDP_Client_Test(void)
{
	bool isConnectServer = FALSE;				//连接服务器状态
	W5500_SOCKET_ERROR SocketError;				//错误状态
	W5500_SOCKET_NUM socket = W5500_SECKET5;	//随意一个socket进行测试
	u8 DestIp[4] = {10,254,1,100};
	u8 SocketIntBit;							//socket总中断状态
	u8 SocketIntStatus;
	u16 len;
	u8 ip[4];
	u16 port;
	u8 CheckNetworkDelay = 200;
	W5500_SOCKET_STATUS SocketStatus;
	

	while(1)
	{

		//连接服务器失败
		while(isConnectServer == FALSE)	//服务器连接状态无效，则一直连接服务器
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 5)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
			}
			
			uart_printf("系统中断状态：%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
			
			SocketError = W5500_CreateUdpClient(&g_W5500Handle, socket, 67, DestIp, 60000, TRUE);			//创建一个UDP客户端并连接服务器
			if(SocketError == SOCKET_OK) //连接成功
			{
				isConnectServer = TRUE;		
				break;
			}
			else //连接失败，延时后重试
			{
				OSTimeDlyHMSM(0,0,1,0);
			}
		}
		
		
		
		//连接服务器成功
		while(isConnectServer == TRUE)	//服务器连接成功，进行数据接收后回传
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 50)
			{
				//W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, 1, DestIp, 60000);	//将收到的数据发送出去
				
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);

				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//获取socket总中断状态，每一个bit代表一个socket
				uart_printf("socket总中断状态=%02X\r\n",SocketIntBit);
								
			}
			
			
			if(W5500_GetInt() == 0)		//发生中断
			{
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//获取socket总中断状态，每一个bit代表一个socket
				uart_printf("socket总中断状态=%02X\r\n",SocketIntBit);
				if(SocketIntBit & (1<<socket))	//当前socket的中断
				{
					SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);	//获取当前socket中断，判断中断原因
					W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, SocketIntStatus);				//清除中断
					if(SocketIntStatus & SOCKET_IR_DISCON_BIT)					//服务器断开连接了
					{
						isConnectServer = FALSE;
						uart_printf("服务器断开连接了，马上进行重连！\r\n");
						break;
					}
					else if(SocketIntStatus & SOCKET_IR_RECV_BIT)								//收到数据了
					{
						len = W5500_GetOneSocketRxDataSize(&g_W5500Handle, socket);				//获取当前接收数据大小
						if(len > 0)
						{
							len = W5500_ReadOneSocketUdpRxData(&g_W5500Handle, socket, DataBuff, len, ip, &port);//读取某一个socket UDP接收缓冲区数据
							if(len > 2047)len = 2047;
							DataBuff[len] = 0;	//增加结束符
							uart_printf(">收到来自%d.%d.%d.%d:%d 数据%dB:%s\r\n",ip[0],ip[1],ip[2],ip[3],port,len,(char *)DataBuff);
							
							
							W5500_WriteOneSocketUdpTxData(&g_W5500Handle, socket, DataBuff, len, ip, port);	//将收到的数据发送出去
						}
					}
				}
			}
			OSTimeDlyHMSM(0,0,0,100);
		}
		
		OSTimeDlyHMSM(0,0,1,0);
	}
}






//TCP服务端测试
void TCP_Server_Test(void)
{
	bool isConnectServer = FALSE;				//连接服务器状态
	W5500_SOCKET_ERROR SocketError;				//错误状态
	W5500_SOCKET_NUM socket = W5500_SECKET5;	//随意一个socket进行测试
	u8 DestIp[4] = {192,168,1,117};
	u8 SocketIntBit;							//socket总中断状态
	u8 SocketIntStatus;
	u16 len;
	u8 ip[4];
	u16 port;
	u8 CheckNetworkDelay = 200;
	W5500_SOCKET_STATUS SocketStatus;
	

	while(1)
	{

		//连接服务器失败
		while(isConnectServer == FALSE)	//服务器连接状态无效，则一直连接服务器
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 5)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);
			}
			
			uart_printf("系统中断状态：%02X\r\n", W5500_GetSysInterrupt(&g_W5500Handle));
			
			SocketError = W5500_CreateTcpServer(&g_W5500Handle, socket, 100);			//创建一个TCP服务端
			if(SocketError == SOCKET_OK) //连接成功
			{
				isConnectServer = TRUE;		
				break;
			}
			else //连接失败，延时后重试
			{
				OSTimeDlyHMSM(0,0,1,0);
			}
		}
		
		
		
		//连接服务器成功
		while(isConnectServer == TRUE)	//服务器连接成功，进行数据接收后回传
		{
			CheckNetworkDelay ++;
			if(CheckNetworkDelay > 50)
			{
				CheckNetworkDelay = 0;
				WaitNetworkConnection(3000);

				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//获取socket总中断状态，每一个bit代表一个socket
				uart_printf("socket总中断状态=%02X\r\n",SocketIntBit);
								
			}
			
			
			if(W5500_GetInt() == 0)		//发生中断
			{
				SocketIntBit = W5500_GetSocketTotalInterruptStatus(&g_W5500Handle);		//获取socket总中断状态，每一个bit代表一个socket
				uart_printf("socket总中断状态=%02X\r\n",SocketIntBit);
				if(SocketIntBit & (1<<socket))	//当前socket的中断
				{
					SocketIntStatus = W5500_GetOneSocketIntStatus(&g_W5500Handle, socket);		//获取当前socket中断，判断中断原因
					W5500_ClearOneSocketIntStatus(&g_W5500Handle, socket, SocketIntStatus);		//清除中断
					if(SocketIntStatus & SOCKET_IR_DISCON_BIT)									//客户端断开连接
					{
						uart_printf("客户断开连接了！\r\n");
						break;
					}
					else if(SocketIntStatus & SOCKET_IR_RECV_BIT)								//收到数据了
					{
						len = W5500_GetOneSocketRxDataSize(&g_W5500Handle, socket);					//获取当前接收数据大小
						if(len > 0)
						{
							W5500_GetOneSocketDestIP(&g_W5500Handle, socket,ip);					//获取对方IP
							port = W5500_GetOneSocketDestPort(&g_W5500Handle, socket);				//获取对方端口
							W5500_ReadOneSocketTcpRxData(&g_W5500Handle, socket, DataBuff, len);	//读取某一个socket接收缓冲区数据
							if(len > 2047)len = 2047;
							DataBuff[len] = 0;	//增加结束符
							uart_printf(">收到来自%d.%d.%d.%d:%d 数据%dB:%s\r\n",ip[0],ip[1],ip[2],ip[3],port,len,(char *)DataBuff);
							
							
							W5500_WriteOneSocketTcpTxData(&g_W5500Handle, socket, DataBuff, len);		//将收到的数据发送出去
						}
					}
					else if(SocketIntStatus & SOCKET_IR_CON_BIT)								//客户端建立链接
					{
						W5500_GetOneSocketDestIP(&g_W5500Handle, socket,ip);					//获取对方IP
						port = W5500_GetOneSocketDestPort(&g_W5500Handle, socket);				//获取对方端口
						uart_printf(">客户%d.%d.%d.%d:%d 连接\r\n",ip[0],ip[1],ip[2],ip[3],port);
					}
				}
			}
			OSTimeDlyHMSM(0,0,0,100);
		}
		
		OSTimeDlyHMSM(0,0,1,0);
	}
}


