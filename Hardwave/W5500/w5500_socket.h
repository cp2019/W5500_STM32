/*************************************************************************************************************
 * 文件名:			w5500_socket.h
 * 功能:			socket接口函数
 * 作者:			cp1300@139.com
 * 创建时间:		2016-02-22
 * 最后修改时间:	2019-01-14
 * 详细:			TCP/UDP协议栈,最好在OS中使用，需要系统延时以及信号量支持
*************************************************************************************************************/
#ifndef _W5500_SOCKET_H_
#define _W5500_SOCKET_H_
#include "system.h"
#include "W5500.h"

//延时
#ifdef _UCOS_II_//使用了操作系统
#include "ucos_ii.h"
#define socket_sleep(x) 	OSTimeDlyHMSM(0,0,x/1000,x%1000)
#else
#include "delay.h"
#define socket_sleep(x) 	Delay_MS(x)
#endif //_UCOS_II_






//统一地址结构
typedef struct 
{
	u8 addr[4];
	u16 port;
}sockaddr,SOCKADDR_IN,socketaddr;




//SOCKET通道定义
#define SOCKET_CH_NUM	8	//最大通道数量8
typedef enum
{
	SOCKET_CH1		=	0,
	SOCKET_CH2		=	1,
	SOCKET_CH3		=	2,
	SOCKET_CH4		=	3,
	SOCKET_CH5		=	4,
	SOCKET_CH6		=	5,
	SOCKET_CH7		=	6,
	SOCKET_CH8		=	7,
	SOCKET_INVALID	=	-1,
}SOCKET;


//通信协议定义
typedef enum
{
	IPPROTO_TCP			=		0,		//TCP协议
	IPPROTO_UDP			=		1,		//UDP单播
	IPPROTO_UDP_BROAD	=		2,		//UDP广播-如果是广播发送，需要把IP设置为255
}IPPROTO;

//SOCKET opt设置的值
#define SO_RCVTIMEO		1		//接收数据超时时间，单位ms,最小10ms
#define SO_SNDTIMEO		2		//发送数据超时时间，单位ms,最小10ms
#define SO_CONNTIMEO	3		//连接超时时间，单位ms,最小10ms

//SOCKET cmd设置的值
#define SOL_FIONREAD	1		//获取接收缓冲区中数据长度
#define SOL_SOCKET		2		//获取一个连接状态是否有效-有效返回1；无效返回0
#define SOL_IPPROTO		3		//获取当前socket协议类型

//返回状态
#define INVALID_SOCKET			-1		//无效




//系统配置与初始化相关接口
int WSAStartup(void);														//协议栈硬件初始化
void netsh_ip_addr(u8 ip[4]);												//设置本地ip地址
void netsh_ip_mask(u8 ip[4]);												//设置本地子网掩码
void netsh_ip_gateway(u8 ip[4]);											//设置本地网关地址
bool InternetGetConnectedState(void);										//获取网络连接状态
const char *socket_GetError(void);											//获取系统错误状态

//通用socket接口
SOCKET socket(IPPROTO protocol);											//初始化socket
int connect(SOCKET socket, const sockaddr* address);						//连接到指定服务器
int bind(SOCKET socket, const sockaddr* address);							//绑定
int recv(SOCKET socket, u8* buff, u32 buffsize);							//接收数据
int recvfrom(SOCKET socket, u8* buff, u32 bufflen, socketaddr* from);		//接收数据-返回对方地址
int sendto(SOCKET socket, const u8* buff, u32 size, const sockaddr *to);	//发送数据到指定地址
int send(SOCKET socket, const u8* buff, u32 size);							//发送数据
void closesocket(SOCKET socket);											//关闭一个socket
void setsockopt(SOCKET socket, int optname, u32 opt);						//设置socket的选项
int getsockopt(SOCKET socket, int cmd);										//获取socket相关参数 cmd=SOL_FIONREAD:获取接收数据长度

//其它接口-用于DHCP时使用
void getLocalMAC(u8 mac[6]);												//获取本机MAC地址
void getLocalHostName(char HostName[16+1], u8 BuffSize);					//获取本机名称


#endif /*_W5500_SOCKET_H_*/

