/*************************************************************************************************************
 * 文件名:			w5500_socket.c
 * 功能:			socket接口函数
 * 作者:			cp1300@139.com
 * 创建时间:		2016-02-22
 * 最后修改时间:	2019-01-14
 * 详细:			TCP/UDP协议栈,最好在OS中使用，需要系统延时以及信号量支持。
					如果使用当前socket方式控制W5500，请不要直接调用W5500 API，确保所有操作都是使用当前库中的socket接口。
					会使用系统信号量确保多线程安全。
					
*************************************************************************************************************/
#include "system.h"
#include "W5500.h"
#include "w5500_socket.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "board.h"

static const char * pError = "";							//全局错误状态
static bool isDeviceInit = FALSE;							//设备是否初始化
static W5500_HANDLE g_W5500Handle;							//W5500句柄

static int WaitingReception(SOCKET socket, u32 timeout);	//等待接收数据
static bool CheckSocketConnectionState(SOCKET socket);		//检查网络连接状态
static int WaitingSuccessfully(SOCKET socket, u32 timeout);	//等待发送成功

static W5500_HANDLE *sg_This_W5500_Handle = &g_W5500Handle;	//当前的soket库的W5500句柄
//socket属性
typedef struct
{
	sockaddr	addr;		//地址
	IPPROTO		ipproto;	//通信协议
	u32			SendTimeOut;//发送超时时间，单位ms
	u32			RecvTimeOut;//接收超时时间，单位ms
	u32			ConnTimeOut;//连接超时时间，单位ms
	u16 		LocalPort;	//本地端口
	bool		isServer;	//是否为服务器
	bool 		isBind;		//是否绑定成功
	bool		isInit;		//是否初始化
	bool		isOccupy;	//是否占用
}socket_status;

socket_status	SocketStatusBuff[SOCKET_CH_NUM];	//socket属性集合

//协议栈硬件初始化
int WSAStartup(void)
{
	u8 MAC[6];
	u16 temp;
	u8 i;
	
	isDeviceInit = TRUE;	//硬件初始化
	memset(SocketStatusBuff,0,sizeof(socket_status)*SOCKET_CH_NUM);
/****************************************************************************************************************************************/	
//此处代码需要根据自身硬件平台进行移植	
	W5500_PowerON();			//W5500电源使能
	W5500_HardwaveInit();		//W5500硬件接口初始化
	OSTimeDlyHMSM(0,0,1,0);		//延时1秒确保硬件上电完成
	//MAC地址，此处使用cpuid生成，将CPU ID进行CRC16生成中间2字节，最高2字节固定，最后4字节使用CPU ID最后4字节
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
	
	uart_printf("本机MAC地址:");
	for(i = 0;i < 6;i ++)
	{
		uart_printf("%02X",MAC[i]);
	}
	uart_printf("\r\n");
		
	if(W5500_Init(sg_This_W5500_Handle, MAC, W5500_SetCS, W5500_GetInt, W5500_ReadWrtieByte, OS_Sleep, "cp1300@139.com", TRUE, FALSE) == FALSE) //初始化W5500
	{
		return -1;	//初始化失败
	}
/****************************************************************************************************************************************/		
	
	return 0;		//初始化成功
}



//获取错误状态
const char *socket_GetError(void)
{
	return pError;
}


/*************************************************************************************************************************
* 函数			:	SOCKET socket(IPPROTO protocol)
* 功能			:	初始化socket
* 参数			:	protocol：通信协议定义
* 返回			:	-1：初始化失败；其它：socket编号
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2018-01-14
* 说明			: 	无
*************************************************************************************************************************/
SOCKET socket(IPPROTO protocol)
{
	u8 i;
	
	if(sg_This_W5500_Handle == NULL)
	{
		pError = "无效的句柄!";
		return SOCKET_INVALID;
	}
	if(isDeviceInit == FALSE)
	{
		pError = "硬件未初始化!";
		return SOCKET_INVALID;
	}
	switch(protocol)
	{
		case IPPROTO_TCP:		//TCP协议
		case IPPROTO_UDP:		//UDP单播
		case IPPROTO_UDP_BROAD:	//UDP广播
		{
			
		}break;
		default:
		{
			pError = "无效的协议!";
			return SOCKET_INVALID;
		}
	}
	
	
	for(i = 0;i < SOCKET_CH_NUM;i ++)				//寻找空闲的socket
	{
		if(SocketStatusBuff[i].isOccupy == FALSE)	//空闲未被使用的socket
		{			
			memset(&SocketStatusBuff[i], 0, sizeof(socket_status));
			SocketStatusBuff[i].isOccupy = TRUE;	//当前socket已经被占用
			SocketStatusBuff[i].ipproto = protocol;	//记录通信协议
			SocketStatusBuff[i].SendTimeOut = 0;	//发送超时时间，单位ms-默认0不等待
			SocketStatusBuff[i].RecvTimeOut = 0;	//接收超时时间，单位ms-默认0不等待
			SocketStatusBuff[i].ConnTimeOut = 10000;//连接超时时间，单位ms-默认10秒
			pError = "";
			return (SOCKET)i;
		}
	}
	
	pError = "获取socket失败，资源不足!";
	return SOCKET_INVALID;
}



/*************************************************************************************************************************
* 函数			:	u16 RandPort(void)
* 功能			:	产生一个随机端口
* 参数			:	socket:socket；address:地址
* 返回			:	端口
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-14
* 最后修改时间	:	2019-01-14
* 说明			: 	无
*************************************************************************************************************************/
u16 RandPort(void)
{
	u8 i;
	u16 port = rand()%60000 + 2000;
	
	srand(SYS_GetOSRunTime());								//设置随机数种子-此处使用的OS系统运行时间作为随机数种子，如果你移植报错，请改为自己的随机数种子，比如程序运行时间等等
	for(i = 0;i < SOCKET_CH_NUM;i ++)
	{
		if(SocketStatusBuff[i].LocalPort == port) break;	//找到了相同的端口
	}
	if(i < SOCKET_CH_NUM)	//有相同的端口
	{
		uart_printf("***生成了相同的随机端口:%d\r\n", port);
		port += rand()%1000;
		uart_printf("***重新生成端口:%d\r\n", port);
	}
	else
	{
		uart_printf("***随机生成端口:%d\r\n", port);
	}
	
	return port;
}

/*************************************************************************************************************************
* 函数			:	int bind(SOCKET socket, const sockaddr* address)
* 功能			:	绑定socket
* 参数			:	socket:socket；address:地址
* 返回			:	-1：初始化失败；socket编号：绑定成功
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
int bind(SOCKET socket, const sockaddr* address)
{
	W5500_SOCKET_ERROR W5500Error;
	u8 ip[4] = {0,0,0,0};
	
	if((socket<0) || (socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return INVALID_SOCKET;
	}
	SocketStatusBuff[socket].isInit = FALSE;	//没有初始化
	switch(SocketStatusBuff[socket].ipproto)	//通信协议
	{
		case IPPROTO_TCP			://				//TCP协议
		{
			W5500Error = W5500_CreateTcpServer(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, address->port);					//创建一个TCP服务端
		}break;
		case IPPROTO_UDP			://				//UDP单播
		{
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, address->port, ip, 0, FALSE);	//UDP单播
		}break;
		case IPPROTO_UDP_BROAD		://				//UDP广播
		{
			//使用多播模式，需要在开启前设置目标IP地址与目标端口为广播地址
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, address->port, address->addr, address->port, TRUE);		//UDP广播
		}break;
		default:
		{
			pError = "无效的协议!";
			return INVALID_SOCKET;
		}
	}
	switch(W5500Error)
	{
		case SOCKET_OK				:	//成功
		{
			SocketStatusBuff[socket].LocalPort = address->port;	//记录本地端口
			SocketStatusBuff[socket].isInit = TRUE;				//初始化成功
			pError = "";
			return socket;			
		}
		case SOCKET_OPEN_ERROR 		:	//打开SOCKET错误
		{
			pError = "打开socket错误";
		}break;
		case SOCKET_TIMEOUT 		:	//超时
		{
			pError = "打开socket超时";
		}break;
		default:
		{
			pError = "未知错误";
		}break;
	}

	return -1;	//初始化失败
}



/*************************************************************************************************************************
* 函数			:	int connect(SOCKET socket, const sockaddr* address)
* 功能			:	连接服务器
* 参数			:	socket:socket；address:地址
* 返回			:	-1：初始化失败；socket编号：绑定成功
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
int connect(SOCKET socket, const sockaddr* address)
{
	W5500_SOCKET_ERROR W5500Error;
	
	if(((int)socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return INVALID_SOCKET;
	}
	SocketStatusBuff[socket].isInit = FALSE;	//没有初始化
	switch(SocketStatusBuff[socket].ipproto)	//通信协议
	{
		case IPPROTO_TCP			://				//TCP协议
		{
			W5500Error = W5500_CreateTcpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, RandPort(), address->addr, address->port, SocketStatusBuff[socket].ConnTimeOut/1000+1);		//创建一个TCP客户端
		}break;
		case IPPROTO_UDP			://				//UDP单播
		{
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, RandPort(), address->addr, address->port, FALSE);	//UDP单播
		}break;
		case IPPROTO_UDP_BROAD		://				//UDP广播
		{
			W5500Error = W5500_CreateUdpClient(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, RandPort(), address->addr, address->port, TRUE);		//UDP广播
		}break;
		default:
		{
			pError = "无效的协议!";
			return INVALID_SOCKET;
		}
	}
	switch(W5500Error)
	{
		case SOCKET_OK				:	//成功
		{
			SocketStatusBuff[socket].isInit = TRUE;		//初始化成功
			pError = "";
			return socket;			
		}
		case SOCKET_OPEN_ERROR 		:	//打开SOCKET错误
		{
			pError = "打开socket错误";
		}break;
		case SOCKET_TIMEOUT 		:	//超时
		{
			pError = "连接超时";
		}break;
		default:
		{
			pError = "未知错误";
		}break;
	}

	return -1;	//初始化失败
}


/*************************************************************************************************************************
* 函数			:	static int WaitingReception(SOCKET socket, u32 timeout)
* 功能			:	等待接收数据
* 参数			:	socket:socket；timeout:超时时间，单位ms
* 返回			:	-1：读取失败；>=0：接收到的数据长度
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	用于需要等待接收数据
*************************************************************************************************************************/
static int WaitingReception(SOCKET socket, u32 timeout)
{
	u32 i;
	u32 time20ms = (timeout)/20 + 1;
	u8 CheckConnectCnt = 0;						//定时检查网络连接
	
	if(SocketStatusBuff[socket].RecvTimeOut > 10)
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;											//连接断开
		for(i = 0;i < time20ms;i ++)
		{
			
			if(W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket) == 0)			//没有收到数据，直接延时
			{
				socket_sleep(20);																			//延时20ms
			}
			else 
			{
				socket_sleep(2);	
				break;	//收到数据了，退出
			}
			
			//定时检查网络连接
			CheckConnectCnt ++;
			if(CheckConnectCnt  > 15)
			{
				CheckConnectCnt  = 0;
				if(CheckSocketConnectionState(socket) == FALSE) return -1;									//连接断开
			}
		}	
	}
	else if(SocketStatusBuff[socket].RecvTimeOut > 0) 		//0-10ms
	{
		if(W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket) == 0)				//没有收到数据，直接延时
		{
			socket_sleep(SocketStatusBuff[socket].RecvTimeOut);	//小于10ms直接延时等待
		}	
	}

	return W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);					//返回获取接收数大小
}


/*************************************************************************************************************************
* 函数			:	static int WaitingSuccessfully(SOCKET socket, u32 timeout)
* 功能			:	等待发送成功
* 参数			:	socket:socket；timeout:超时时间，单位ms
* 返回			:	-1：发送超时或连接断开；0：发送完成
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	通过等待判断是否发生了发送成功中断或发送超时中断
*************************************************************************************************************************/
static int WaitingSuccessfully(SOCKET socket, u32 timeout)
{
	u32 i;
	u32 time20ms = (timeout)/20 + 1;
	u8 CheckConnectCnt = 0;						//定时检查网络连接
	u8 SocketIntStatus;
	
	if(SocketStatusBuff[socket].RecvTimeOut > 10)
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;											//连接断开
		for(i = 0;i < time20ms;i ++)
		{		
			SocketIntStatus = W5500_GetOneSocketIntStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);							//获取当前socket中断，判断中断原因
			if(SocketIntStatus & SOCKET_IR_SEND_OK_BIT)			//发送完成了
			{
				return 1;
			}
			else if(SocketIntStatus & SOCKET_IR_TIMEOUT_BIT)	//发送超时了
			{
				return -1;
			}
			else if(SocketIntStatus & SOCKET_IR_DISCON_BIT)		//对方断开连接了
			{
				return -1;
			}
			else
			{
				socket_sleep(20);																			//延时20ms
			}

			//定时检查网络连接
			CheckConnectCnt ++;
			if(CheckConnectCnt  > 15)
			{
				CheckConnectCnt  = 0;
				if(CheckSocketConnectionState(socket) == FALSE) return -1;									//连接断开
			}
		}	
	}
	else if(SocketStatusBuff[socket].RecvTimeOut > 0) 		//0-10ms
	{
		socket_sleep(SocketStatusBuff[socket].RecvTimeOut);	//小于10ms直接延时等待
	}
		
	SocketIntStatus = W5500_GetOneSocketIntStatus(sg_This_W5500_Handle,  (W5500_SOCKET_NUM)socket);									//获取当前socket中断，判断中断原因
	if(SocketIntStatus & SOCKET_IR_SEND_OK_BIT)			//发送完成了
	{
		return 1;
	}
	else if(SocketIntStatus & SOCKET_IR_TIMEOUT_BIT)	//发送超时了
	{
		return -1;
	}
	else if(SocketIntStatus & SOCKET_IR_DISCON_BIT)		//对方断开连接了
	{
		return -1;
	}
	return -1;	//超时
}


/*************************************************************************************************************************
* 函数			:	static bool CheckSocketConnectionState(SOCKET socket)
* 功能			:	检查网络连接状态
* 参数			:	socket:socket；
* 返回			:	FALSE:未连接；TRUE:已连接
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	用于检查网络状态
*************************************************************************************************************************/
static bool CheckSocketConnectionState(SOCKET socket)
{
	W5500_SOCKET_STATUS SocketStatus;
	
	switch(SocketStatusBuff[socket].ipproto)	//通信协议
	{
		case IPPROTO_TCP		:				//TCP协议
		{		
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//获取socket状态
			if(SocketStatusBuff[socket].isBind == TRUE)	//服务端监听
			{
				if(SocketStatus != SOCK_LISTEN)
				{
					pError = "服务端未启动监控!";
					return FALSE;
				}
			}
			else
			{
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					pError = "未连接!";
					return FALSE;
				}
			}
		}break;
		case IPPROTO_UDP			://				//UDP单播
		case IPPROTO_UDP_BROAD		://				//UDP广播
		{
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//获取socket状态
			if(SocketStatus != SOCK_UDP)
			{
				pError = "未连接!";
				return FALSE;
			}			
		}break;
		default:
		{
			pError = "无效的协议!";
			return FALSE;
		}
	}
	
	return TRUE;
}


/*************************************************************************************************************************
* 函数			:	recvfrom(SOCKET socket, u8* buff, u32 len, socketaddr* from)
* 功能			:	读取数据
* 参数			:	socket:socket；buff:数据缓冲区；len：数据缓冲区最大长度；from：发送方地址
* 返回			:	-1：读取失败；>=0：接收到的数据长度
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	用于UDP模式
*************************************************************************************************************************/
int recvfrom(SOCKET socket, u8* buff, u32 buffsize, socketaddr* from)
{
	int len;
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socket没有初始化!";
		return INVALID_SOCKET;
	}
	
	if(SocketStatusBuff[socket].RecvTimeOut > 0)								//需要等待超时
	{
		len = WaitingReception(socket, SocketStatusBuff[socket].RecvTimeOut); 	//等待接收数据
	}
	else
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;																		//连接断开
		len = W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);												//获取接收数大小
	}
	if(len <= 0)  return len;						//没有收到数据
	
	switch(SocketStatusBuff[socket].ipproto)		//通信协议
	{
		case IPPROTO_TCP			://				//TCP协议
		{
			W5500_GetOneSocketDestIP(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket,from->addr);										//获取对方IP
			from->port = W5500_GetOneSocketDestPort(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);									//获取对方端口
			
			if(len > buffsize) len = buffsize;
			W5500_ReadOneSocketTcpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len);									//读取某一个socket TCP接收缓冲区数据
			
			pError = "";
			return len;
		}
		case IPPROTO_UDP			://				//UDP单播
		case IPPROTO_UDP_BROAD		://				//UDP广播
		{
			if((len-8) > buffsize) len = (buffsize+8);
			len = W5500_ReadOneSocketUdpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len, from->addr, &from->port);	//读取某一个socket UDP接收缓冲区数据
			
			pError = "";
			return len;
		}
		default:
		{
			pError = "无效的协议!";
			return INVALID_SOCKET;
		}
	}
}






/*************************************************************************************************************************
* 函数			:	int recv(SOCKET socket, u8* buff, u32 buffsize)
* 功能			:	读取数据
* 参数			:	socket:socket；buff:数据缓冲区；len：数据缓冲区最大长度；
* 返回			:	-1：读取失败；>=0：接收到的数据长度
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2019-01-14
* 说明			: 	用于TCP模式
*************************************************************************************************************************/
int recv(SOCKET socket, u8* buff, u32 buffsize)
{
	int len;
	u8 ip[4];
	u16 port;
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socket没有初始化!";
		return INVALID_SOCKET;
	}
	
	if(SocketStatusBuff[socket].RecvTimeOut > 0)								//需要等待超时
	{
		len = WaitingReception(socket, SocketStatusBuff[socket].RecvTimeOut); 	//等待接收数据
	}
	else
	{
		if(CheckSocketConnectionState(socket) == FALSE) return -1;																		//连接断开
		len = W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);												//获取接收数大小
	}
	if(len <= 0)  return len;						//没有收到数据
	
	switch(SocketStatusBuff[socket].ipproto)	//通信协议
	{
		case IPPROTO_TCP			://				//TCP协议
		{
			if(len > buffsize) len = buffsize;
			W5500_ReadOneSocketTcpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len);									//读取某一个socket TCP接收缓冲区数据
			
			pError = "";
			return len;
		}
		case IPPROTO_UDP			://				//UDP单播
		case IPPROTO_UDP_BROAD		://				//UDP广播
		{
			if((len-8) > buffsize) len = (buffsize+8);
			len = W5500_ReadOneSocketUdpRxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, len, ip, &port);					//读取某一个socket UDP接收缓冲区数据
			
			pError = "";
			return len;
		}
		default:
		{
			pError = "错误:无效的协议!";
			return INVALID_SOCKET;
		}
	}
}



/*************************************************************************************************************************
* 函数			:	int send(SOCKET socket, const u8* buff, u32 size)
* 功能			:	发送数据
* 参数			:	socket:socket；buff:数据缓冲区；size：发送数据长度；
* 返回			:	-1：发送失败；>0：发送的数据长度;0:发送超时；
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-14
* 最后修改时间	:	2019-01-14
* 说明			: 	用于TCP发送数据
*************************************************************************************************************************/
int send(SOCKET socket, const u8* buff, u32 size)
{
	W5500_SOCKET_ERROR W5500Error = SOCKET_OPEN_ERROR;
	W5500_SOCKET_STATUS SocketStatus;
	
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socket没有初始化!";
		return INVALID_SOCKET;
	}
	
	if(size > 2048) size = 2048;				//限制最大长度
	switch(SocketStatusBuff[socket].ipproto)	//通信协议
	{
		case IPPROTO_TCP		://				//TCP协议
		{		
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//获取socket状态
			if(SocketStatusBuff[socket].isBind == TRUE)	//服务端监听
			{
				if(SocketStatus != SOCK_LISTEN)
				{
					pError = "服务端未启动监控!";
					return INVALID_SOCKET;
				}
			}
			else
			{
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					pError = "未连接!";
					return INVALID_SOCKET;
				}
			}
			
			
			W5500Error = W5500_WriteOneSocketTcpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size);	//TCP发送数据
		}break;
		case IPPROTO_UDP			://				//UDP单播
		case IPPROTO_UDP_BROAD		://				//UDP广播
		{
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//获取socket状态
			if(SocketStatus != SOCK_UDP)
			{
				pError = "未连接!";
				return INVALID_SOCKET;
			}
			
			W5500Error = W5500_WriteOneSocketUdpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size, NULL, 0);
		}break;
		default:
		{
			pError = "无效的协议!";
			return INVALID_SOCKET;
		}
	}
	
	switch(W5500Error)
	{
		case SOCKET_OK			:	//SOCKET OK
		{
			//发送OK时检查是否发生了超时
			if(SocketStatusBuff[socket].SendTimeOut > 0)				//设置了才检查
			{
				if(WaitingSuccessfully(socket, SocketStatusBuff[socket].SendTimeOut) < 0)	//等待发送成功
				{
					pError = "发送超时";
					return 0;
				}
				
			}
			
			pError = "";			
			return size;
		}
		case SOCKET_OPEN_ERROR 	:	//打开SOCKET错误
		{
			pError = "无效的 socket!";
			return INVALID_SOCKET;
		}
		case SOCKET_TIMEOUT 	:	//超时
		{
			pError = "连接超时";
			return INVALID_SOCKET;
		}
		case SOCKET_TXBUFF_SIZE	:	//发送缓冲区剩余空间不足
		{
			pError = "发送忙或数据超出范围!";
			return INVALID_SOCKET;
		}
		case SOCKET_ERROR 		:	//socket非法
		{
			pError = "非法的 socket!";
			return INVALID_SOCKET;
		}
		default:
		{
			pError = "未知错误!";
			return INVALID_SOCKET;
		}
	}
}


/*************************************************************************************************************************
* 函数			:	int sendto(SOCKET socket, const u8* buff, u32 size, const sockaddr *to)
* 功能			:	发送数据到指定地址
* 参数			:	socket:socket；buff:数据缓冲区；size：发送数据长度；to：目标地址
* 返回			:	-1：发送失败；>0：发送的数据长度；0:发送超时；
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	用于UDP发送数据或TCP server
*************************************************************************************************************************/
int sendto(SOCKET socket, const u8* buff, u32 size, const sockaddr *to)
{
	W5500_SOCKET_ERROR W5500Error;
	W5500_SOCKET_STATUS SocketStatus;
	
	
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return INVALID_SOCKET;
	}
	if(SocketStatusBuff[socket].isInit==FALSE)
	{
		pError = "socket没有初始化!";
		return INVALID_SOCKET;
	}
	
	if(size > 2048) size = 2048;				//限制最大长度
	switch(SocketStatusBuff[socket].ipproto)	//通信协议
	{
		case IPPROTO_TCP		:				//TCP协议
		{		
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//获取socket状态
			if(SocketStatusBuff[socket].isBind == TRUE)	//服务端监听
			{
				if(SocketStatus != SOCK_LISTEN)
				{
					pError = "服务端未启动监控!";
					return INVALID_SOCKET;
				}
			}
			else
			{
				if(SocketStatus != SOCK_ESTABLISHED)
				{
					pError = "未连接!";
					return INVALID_SOCKET;
				}
			}
			
			
			W5500Error = W5500_WriteOneSocketTcpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size);	//TCP发送数据
		}break;
		case IPPROTO_UDP			://				//UDP单播
		case IPPROTO_UDP_BROAD		://				//UDP广播
		{
			SocketStatus = W5500_GetOneSocketStatus(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);	//获取socket状态
			if(SocketStatus != SOCK_UDP)
			{
				pError = "未连接!";
				return INVALID_SOCKET;
			}
			
			W5500Error = W5500_WriteOneSocketUdpTxData(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket, buff, size, (u8 *)to->addr, to->port);
		}break;
		default:
		{
			pError = "无效的协议!";
			return INVALID_SOCKET;
		}
	}
	
	switch(W5500Error)
	{
		case SOCKET_OK			:	//SOCKET OK
		{
			//发送OK时检查是否发生了超时
			if(SocketStatusBuff[socket].SendTimeOut > 0)				//设置了才检查
			{
				if(WaitingSuccessfully(socket, SocketStatusBuff[socket].SendTimeOut) < 0)	//等待发送成功
				{
					pError = "发送超时";
					return 0;
				}				
			}

			pError = "";
			return size;
		}
		case SOCKET_OPEN_ERROR 	:	//打开SOCKET错误
		{
			pError = "无效的 socket!";
			return INVALID_SOCKET;
		}
		case SOCKET_TIMEOUT 	:	//超时
		{
			pError = "连接超时";
			return INVALID_SOCKET;
		}
		case SOCKET_TXBUFF_SIZE	:	//发送缓冲区剩余空间不足
		{
			pError = "发送忙或数据超出范围!";
			return INVALID_SOCKET;
		}
		case SOCKET_ERROR 		:	//socket非法
		{
			pError = "非法的 socket!";
			return INVALID_SOCKET;
		}
		default:
		{
			pError = "未知错误!";
			return INVALID_SOCKET;
		}
	}
}



/*************************************************************************************************************************
* 函数			:	void closesocket(SOCKET socket)
* 功能			:	关闭一个socket
* 参数			:	socket:socket；
* 返回			:	无
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-14
* 最后修改时间	:	2019-01-14
* 说明			: 	用于关闭一个socket
*************************************************************************************************************************/
void closesocket(SOCKET socket)
{
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return;
	}
	W5500_DisconnectOneSocket(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);		//断开当前连接
	memset(&SocketStatusBuff[socket], 0, sizeof(socket_status));								//清空socket缓冲区内存
	W5500_CloseOneSocket(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);			//强制关闭一个socket
}



/*************************************************************************************************************************
* 函数			:	void setsockopt(SOCKET socket, int optname, u32 opt)
* 功能			:	设置socket的选项
* 参数			:	socket:socket；optname:需设置的选项;opt:设置的值
* 返回			:	无
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-14
* 最后修改时间	:	2019-01-14
* 说明			: 	SO_RCVTIMEO			//接收数据超时时间，单位ms,最小10ms，默认设置为0，不阻塞
					SO_SNDTIMEO			//发送数据超时时间，单位ms,最小10ms，默认设置为0，不阻塞
					SO_CONNTIMEO		//连接超时时间，单位ms,最小10ms，默认设置为10000
*************************************************************************************************************************/
void setsockopt(SOCKET socket, int optname, u32 opt)
{
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "socket无效!";
		return;
	}
	
	switch(optname) //需设置的选项
	{
		case SO_SNDTIMEO:	//发送超时时间
		{
			SocketStatusBuff[socket].SendTimeOut = opt;		//发送超时时间，单位ms-默认0不等待
		}break;
		case SO_RCVTIMEO:	//接收超时时间
		{
			SocketStatusBuff[socket].RecvTimeOut = opt;		//接收超时时间，单位ms-默认0不等待
		}break;
		case SO_CONNTIMEO:	//连接超时时间
		{
			if(opt < 1000) opt = 1000;						//连接超时最小给1秒
			SocketStatusBuff[socket].ConnTimeOut = opt;		//连接超时时间，单位ms-默认10秒
		}break;
		default:break;
	}
}


/*************************************************************************************************************************
* 函数			:	int getsockopt(SOCKET socket, int cmd)
* 功能			:	获取socket相关参数
* 参数			:	socket:socket；cmd:命令参数
* 返回			:	返回需要获取的值
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	SOL_FIONREAD	1		//获取接收缓冲区中数据长度
					SOL_SOCKET		2		//获取一个连接状态是否有效-有效返回1；无效返回0
					SOL_IPPROTO		3		//获取当前socket协议类型
*************************************************************************************************************************/
int getsockopt(SOCKET socket, int cmd)
{
	if((socket<0) || ((int)socket>=SOCKET_CH_NUM))
	{
		pError = "发送失败:socket无效!";
		return 0;
	}
	
	switch(cmd) //需设置的选项
	{
		case SOL_FIONREAD:	//获取接收缓冲区数据长度
		{
			return W5500_GetOneSocketRxDataSize(sg_This_W5500_Handle, (W5500_SOCKET_NUM)socket);
		}
		case SOL_SOCKET:	//获取连接状态是否有效
		{
			return (CheckSocketConnectionState(socket)==TRUE)?1:0;
		}
		case SOL_IPPROTO:	//获取协议类型
		{
			if(SocketStatusBuff[socket].isInit == FALSE) return SOCKET_CLOSED;	//无效
			return SocketStatusBuff[socket].ipproto;							//返回协议类型
		}
		default: return 0;
	}
}



/*************************************************************************************************************************
* 函数			:	void netsh_ip_addr(u8 ip[4])
* 功能			:	设置本地ip地址
* 参数			:	ip
* 返回			:	无
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	初始化硬件后进行配置
*************************************************************************************************************************/
void netsh_ip_addr(u8 ip[4])
{
	W5500_SetLocalIP(sg_This_W5500_Handle, ip);		//设置本机IP地址
}


/*************************************************************************************************************************
* 函数			:	void netsh_ip_mask(u8 ip[4])
* 功能			:	设置本地子网掩码
* 参数			:	ip
* 返回			:	无
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	初始化硬件后进行配置
*************************************************************************************************************************/
void netsh_ip_mask(u8 ip[4])
{
	W5500_SetMaskAddr(sg_This_W5500_Handle, ip);		//设置子网掩码
}


/*************************************************************************************************************************
* 函数			:	void netsh_ip_gateway(u8 ip[4])
* 功能			:	设置本地网关地址
* 参数			:	ip
* 返回			:	无
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	初始化硬件后进行配置
*************************************************************************************************************************/
void netsh_ip_gateway(u8 ip[4])
{
	W5500_SetGatewayAddr(sg_This_W5500_Handle, ip);		//设置网关IP地址
}

/*************************************************************************************************************************
* 函数			:	bool InternetGetConnectedState(void)
* 功能			:	获取网络连接状态
* 参数			:	无
* 返回			:	TRUE:网络连接正常；FALSE:网络连接异常
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	初始化硬件后获取网络是否连接
*************************************************************************************************************************/
bool InternetGetConnectedState(void)
{
	if((W5500_GetPHY_Status(sg_This_W5500_Handle) & W5500_PHY_STATUS_LNK_BIT) == 0)				//W5500 获取PHY状态
	{
		return FALSE;	//网络未连接
	}
	else
	{
		return TRUE;	//网络连接成功
	}
}

/*************************************************************************************************************************
* 函数			:	void getLocalMAC(u8 mac[6])
* 功能			:	获取本机MAC地址
* 参数			:	mac：mac存储缓冲区
* 返回			:	无
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	
*************************************************************************************************************************/
void getLocalMAC(u8 mac[6])
{
	W5500_GetMAC(sg_This_W5500_Handle, mac);			//获取本机MAC硬件地址
}


/*************************************************************************************************************************
* 函数			:	void getLocalHostName(char HostName[16+1], u8 BuffSize)
* 功能			:	获取本机名称
* 参数			:	HostName：名称缓冲区；BuffSize：buff大小
* 返回			:	无
* 依赖			:	W5500
* 作者			:	cp1300@139.com
* 时间			:	2019-01-15
* 最后修改时间	:	2019-01-15
* 说明			: 	
*************************************************************************************************************************/
void getLocalHostName(char HostName[16+1], u8 BuffSize)
{
	u8 len = strlen(sg_This_W5500_Handle->HostName);
	
	if(BuffSize <= 1 || HostName == NULL) return;
	if(len > (BuffSize-1)) len = BuffSize-1;
	memcpy(HostName, sg_This_W5500_Handle->HostName, len);
	HostName[len] = '\0';
}



