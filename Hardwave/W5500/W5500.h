/*************************************************************************************************************
 * 文件名:			W5500.c
 * 功能:			W5500网络芯片驱动
 * 作者:			cp1300@139.com
 * 创建时间:		2015-01-25
 * 最后修改时间:	2015-01-25
 * 详细:			网络芯片驱动,支持TCP,UDP协议
					如果使用了UCOS,所有操作必须在线程中执行
*************************************************************************************************************/
#ifndef _W5500_H_
#define _W5500_H_
#include "system.h"


//16位整形数高低对调
#ifndef SWAP16
#define SWAP16(x)   (((x & 0xff00) >> 8) | ((x & 0xff) << 8))
#endif //SWAP16


//寄存器选择定义
typedef enum
{
	SELECT_GREG_00		    	=	0,	//通用寄存器
	SELECT_SECKET0_01	        =	1,	//socket0寄存器
	SELECT_SECKET1_05	        =	5,	//socket1寄存器
	SELECT_SECKET2_09   	    =	9,	//socket2寄存器
	SELECT_SECKET3_13   	    =	13,	//socket3寄存器
	SELECT_SECKET4_17   	    =	17,	//socket4寄存器
	SELECT_SECKET5_21       	=	21,	//socket5寄存器
	SELECT_SECKET6_25       	=	25,	//socket6寄存器
	SELECT_SECKET7_29       	=	29,	//socket7寄存器
	SELECT_SECKET0_TXREG_02 	=	2,	//socket0发送缓冲区
	SELECT_SECKET1_TXREG_06	    =	6,	//socket0发送缓冲区
	SELECT_SECKET2_TXREG_10 	=	10,	//socket0发送缓冲区
	SELECT_SECKET3_TXREG_14	    =	14,	//socket0发送缓冲区
	SELECT_SECKET4_TXREG_18	    =	18,	//socket0发送缓冲区
	SELECT_SECKET5_TXREG_22 	=	22,	//socket0发送缓冲区
	SELECT_SECKET6_TXREG_26 	=	26,	//socket0发送缓冲区
	SELECT_SECKET7_TXREG_30 	=	30,	//socket0发送缓冲区
	SELECT_SECKET0_RXREG_03 	=	3,	//socket0接收缓冲区
	SELECT_SECKET1_RXREG_07	    =	7,	//socket0接收缓冲区
	SELECT_SECKET2_RXREG_11	    =	11,	//socket0接收缓冲区
	SELECT_SECKET3_RXREG_15	    =	15,	//socket0接收缓冲区
	SELECT_SECKET4_RXREG_19	    =	19,	//socket0接收缓冲区
	SELECT_SECKET5_RXREG_23	    =	23,	//socket0接收缓冲区
	SELECT_SECKET6_RXREG_27	    =	27,	//socket0接收缓冲区
	SELECT_SECKET7_RXREG_31 	=	31,	//socket0接收缓冲区
}W5500_BLOCK_SELECT;


//通用寄存器地址定义
typedef enum 
{
	W5500_MODE_REG_0x00		    =	0x0000,	//模式寄存器
	W5500_GAR0_REG_0x01 		=	0x0001,	//网关地址字节1
	W5500_GAR1_REG_0x02 		=	0x0002,	//网关地址字节2
	W5500_GAR2_REG_0x03 		=	0x0003,	//网关地址字节3
	W5500_GAR3_REG_0x04 		=	0x0004,	//网关地址字节4
	W5500_SUBR0_REG_0x05		=	0x0005,	//子网掩码字节1
	W5500_SUBR1_REG_0x06		=	0x0006,	//子网掩码字节2
	W5500_SUBR2_REG_0x07		=	0x0007,	//子网掩码字节3
	W5500_SUBR3_REG_0x08		=	0x0008,	//子网掩码字节4
	W5500_SHAR0_REG_0x09		=	0x0009,	//MAC硬件地址字节1
	W5500_SHAR1_REG_0x0A		=	0x000A,	//MAC硬件地址字节2
	W5500_SHAR2_REG_0x0B		=	0x000B,	//MAC硬件地址字节3
	W5500_SHAR3_REG_0x0C		=	0x000C,	//MAC硬件地址字节4
	W5500_SHAR4_REG_0x0D		=	0x000D,	//MAC硬件地址字节5
	W5500_SHAR5_REG_0x0E		=	0x000E,	//MAC硬件地址字节6
	W5500_SIPR0_REG_0x0F		=	0x000F,	//本地IP字节1
	W5500_SIPR1_REG_0x10		=	0x0010,	//本地IP字节2
	W5500_SIPR2_REG_0x11		=	0x0011,	//本地IP字节3
	W5500_SIPR3_REG_0x12		=	0x0012,	//本地IP字节4
	W5500_INTLEVEL0_REG_0x13	=	0x0013,	//低电平中断定时器寄存器-高字节
	W5500_INTLEVEL1_REG_0x14	=	0x0014,	//低电平中断定时器寄存器-低字节
	W5500_IR_REG_0x15   		=	0x0015,	//中断寄存器
	W5500_IMR_REG_0x16  		=	0x0016,	//中断 屏蔽 寄存器
	W5500_SIR_REG_0x17	    	=	0x0017,	//Socket 中断寄存器
	W5500_SIMR_REG_0x18 		=	0x0018,	//Socket 中断屏蔽寄存器
	W5500_RTR0_REG_0x19     	=	0x0019,	//重试时间值寄存器-高字节
	W5500_RTR1_REG_0x1A	    	=	0x001A,	//重试时间值寄存器-低字节
	W5500_RCR_REG_0x1B  		=	0x001B,	//重试计数器
	W5500_PTIMER_REG_0x1C   	=	0x001C,	//PPP连接控制协议请求定时寄存器
	W5500_PMAGIC_REG_0x1D   	=	0x001D,	//PPP连接控制协议幻数寄存器
	W5500_PHAR0_REG_0x1E		=	0x001E,	//PPPoE 模式下目标 MAC 寄存器
	W5500_PHAR1_REG_0x1F		=	0x001F,	//PPPoE 模式下目标 MAC 寄存器
	W5500_PHAR2_REG_0x20		=	0x0020,	//PPPoE 模式下目标 MAC 寄存器
	W5500_PHAR3_REG_0x21		=	0x0021,	//PPPoE 模式下目标 MAC 寄存器
	W5500_PHAR4_REG_0x22		=	0x0022,	//PPPoE 模式下目标 MAC 寄存器
	W5500_PHAR5_REG_0x23		=	0x0023,	//PPPoE 模式下目标 MAC 寄存器
	W5500_PSID0_REG_0x24		=	0x0024,	//PPPoE 模式下会话 ID 寄存器
	W5500_PSID1_REG_0x25		=	0x0025,	//PPPoE 模式下会话 ID 寄存器
	W5500_PMRU0_REG_0x26		=	0x0026,	//PPPoE模式下最大接收单元
	W5500_PMRU1_REG_0x27		=	0x0027,	//PPPoE模式下最大接收单元
	W5500_UIPR0_REG_0x28		=	0x0028,	//无法抵达 IP 地址寄存器
	W5500_UIPR1_REG_0x28		=	0x0029,	//无法抵达 IP 地址寄存器
	W5500_UIPR2_REG_0x2A		=	0x002A,	//无法抵达 IP 地址寄存器
	W5500_UIPR3_REG_0x2B		=	0x002B,	//无法抵达 IP 地址寄存器
	W5500_UPORTR0_REG_0x2C	    =	0x002C,	//无法抵达 端口 地址寄存器
	W5500_UPORTR1_REG_0x2D  	=	0x002D,	//无法抵达 端口 地址寄存器
	W5500_PHYCFGR_REG_0x2E  	=	0x002E,	//W5500 PHY 配置寄 存器
	W5500_VERSIONR_REG_0x39 	=	0x0039,	//芯片版本信息，默认为0x04
}W5500_REG_ADDR;


//socket n寄存器
typedef enum
{
	Sn_MR_0x00			=	0x0000,	//模式寄存器
	Sn_CR_0x01			=	0x0001,	//配置寄存器
	Sn_IR_0x02			=	0X0002,	//中断寄存器
	Sn_SR_0x03			=	0X0003,	//状态寄存器
	Sn_PORT_0x04		=	0X0004,	//源端口寄存器
	Sn_DHAR_0x06		=	0X0006,	//目的MAC地址寄存器
	Sn_DIPR_0x0C		=	0X000C,	//目标IP地址寄存器
	Sn_DPORT_0x10		=	0X0010,	//目标端口寄存器
	Sn_MSSR_0x12		=	0X0012,	//最大分段寄存器
	Sn_TOS_0x15			=	0X0015,	//服务类型寄存器
	Sn_TTL_0x16			=	0X0016,	//生存时间寄存器
	Sn_RXBUFF_SIZE_0x1E	=	0X001E,	//接收缓冲区大小寄存器
	Sn_TXBUFF_SIZE_0x1F	=	0X001F,	//发送缓冲区大小寄存器
	Sn_TX_FSR_0x20		=	0X0020,	//空闲发送缓存寄存器
	Sn_TX_RD_0x22		=	0X0022,	//发送读指针寄存器
	Sn_TX_WR_0x24		=	0X0024,	//发送写指针寄存器
	Sn_RX_RSR_0x26		=	0X0026,	//空闲接收缓存寄存器
	Sn_RX_RD_0x28		=	0X0028,	//接收读指针寄存器
	Sn_RX_WR_0x2A		=	0X002A,	//接收写指针寄存器
	Sn_IMR_0x2C			=	0X002C,	//中断屏蔽寄存器
	Sn_FRAG_0x2D		=	0X002D,	//分段寄存器
	Sn_KPALVTR_0x2F		=	0X002F,	//在线时间寄存器
}W5500_SOCKET_REG;



//SOCKET选择
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



//TCP/IP协议
typedef enum
{
	SOCKET_CLOSED		=	0,
	SOCKET_TCP			=	1,	
	SOCKET_UDP			=	2,
	SOCKET_MACRAW		=	3,
}W5500_PROTOCOL;




//连接错误状态
typedef enum
{
	SOCKET_OK				= 0,	//SOCKET OK
	SOCKET_OPEN_ERROR 		= 1,	//打开SOCKET错误
	SOCKET_TIMEOUT 			= 2,	//超时
	SOCKET_TXBUFF_SIZE		= 3,	//发送缓冲区剩余空间不足
	SOCKET_ERROR 			= 0xff,	//socket非法
}W5500_SOCKET_ERROR;


//socket命令
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


//某一个socket具体中断状态
#define	SOCKET_IR_SEND_OK_BIT		((u8)1<<4)	//发送完成
#define	SOCKET_IR_TIMEOUT_BIT		((u8)1<<3)	//APR或TCP发送超时
#define	SOCKET_IR_RECV_BIT			((u8)1<<2)	//收到新数据
#define	SOCKET_IR_DISCON_BIT		((u8)1<<1)	//对方断开连接
#define	SOCKET_IR_CON_BIT			((u8)1<<0)	//连接建立成功
#define SOCKET_IR_ALL               ((u8)0xFF)    



//SOCKET状态
typedef enum
{
	SOCK_CLOSED		=	0x00,	//关闭状态
	SOCK_INIT		=	0x13,	//初始化
	SOCK_LISTEN		=	0x14,	//监听
	SOCK_ESTABLISHED=	0x17,	//TCP客户端连接成功
	SOCK_CLOSE_WAIT	=	0x1C,	//接收到断开指令
	SOCK_UDP		=	0x22,	//UDP模式下,open
	SOCK_MACRAW		=	0x02,	//MACRAW模式
	SOCK_SYNSENT	=	0x15,	//已经发送连接请求
	SOCK_SYNRECV	=	0x16,	//接收到连接成功指令
	SOCK_FIN_WAIT	=	0x18,	//正在关闭socket
	SOCK_CLOSING 	=	0x1A,	//正在关闭socket
	SOCK_TIME_WAIT	=	0x1B,	//超时关闭socket
	SOCK_LAST_ACK	=	0x1D,	//socket处于关闭状态
}W5500_SOCKET_STATUS;



//W5500 PHY工作模式设置
typedef enum
{
	W5500_OPMDC_10M_HALF		=	0,			//10M半双工，关闭自动协商
	W5500_OPMDC_10M_FULL		=	1,			//10M全双工，关闭自动协商
	W5500_OPMDC_100M_HALF		=	2,			//100M半双工，关闭自动协商
	W5500_OPMDC_100M_FULL		=	3,			//100M全双工，关闭自动协商
	W5500_OPMDC_100M_HALF_AUTO	=	4,			//100M半双工，启动自动协商
	W5500_OPMDC_NONE			=	5,			//无效，未启用状态
	W5500_OPMDC_POWER_DOWN		=	6,			//掉电，低功耗
	W5500_OPMDC_FULL_FUNC_AUTO	=	7,			//全功能，自动协商
}W5500_PHY_OPMDC;


//W5500 收发缓冲区大小定义
typedef enum
{
    SOCKET_BUFF_SIZE_1KB        =   1,          //1KB
    SOCKET_BUFF_SIZE_2KB        =   2,          //2KB 
    SOCKET_BUFF_SIZE_4KB        =   4,          //4KB
    SOCKET_BUFF_SIZE_8KB        =   8,          //8KB
    SOCKET_BUFF_SIZE_16KB       =   16,         //16KB    
}W5500_SOCKET_BUFF_SIZE;


//W5500通用寄模式寄存器值定义
#define W5500_MODE_WOL_BIT		(1<<5)		//网络唤醒模式开关，0：关闭网络唤醒;1：开启网络唤醒
#define W5500_MODE_PING_BIT		(1<<4)		//ping block模式，0：关闭ping; 1:开启ping
#define W5500_MODE_PPPOE_BIT	(1<<3)		//PPPOE模式开关；0：关闭PPPoE;1:开启PPPoE
#define W5500_MODE_FARP_BIT		(1<<1)		//强迫ARP模式；0：关闭强迫 ARP 模；1：启用强迫 ARP 模式； 在强迫 ARP 模式下，无论是否发送 数据都会强迫ARPARPARP请

//W5500中断状态定义
#define W5500_INT_CONFLICT		(1<<7)		//IP冲突中断，在收到 APR 请求时，发现送方 IP 与本地 IP 重复，该位将被置 ‘1’
#define W5500_INT_UNREACH		(1<<6)		//目标不可抵达,当接收到 ICMP（目的端口不可达）包后，该位置‘ 1’
#define W5500_INT_PPPOE			(1<<5)		//PPPoE 连接关闭，当 PPPoE模式下 ,PPPoE连接断开时生效
#define W5500_INT_MP			(1<<4)		//网络唤醒模式下受到唤醒数据包是触发
#define W5500_INT_ALL           0xFF

//W5500 PHY 状态
#define W5500_PHY_STATUS_DPX_BIT		(1<<2)	//全双工状态
#define W5500_PHY_STATUS_SPD_BIT		(1<<1)	//速度状态；1:100；0:10M
#define W5500_PHY_STATUS_LNK_BIT		(1<<0)	//网线连接状态；1：已经连接；0：未连接


//socket寄存器控制寄存器列表
extern W5500_BLOCK_SELECT const SELECT_SOCKET_REG[8];
//socket发送缓冲区寄存器列表
extern W5500_BLOCK_SELECT const SELECT_SOCKET_TXBUFF[8];
//socket接收缓冲区寄存器列表
extern W5500_BLOCK_SELECT const SELECT_SOCKET_RXBUFF[8];	



//W5500硬件接口句柄
typedef struct
{
    void (*SetCS_IO)(u8 Level);             //CS IO设置接口（1：高电平；0：低电平）
    u8  (*GetInt_IO)(void);                 //获取中断IO状态（高电平返回1，低电平返回0）
    u8 (*ReadWrtieByte)(u8 data);           //SPI读写接口（SPI读写一字节接口）
    void (*Sleep)(u32 ms);                	//毫秒延时接口
    u8 MAC[6];                              //MAC地址
    char HostName[16+1];                     //当前主机名称（用于DHCP中显示的主机名称）
    bool isEnablePing;                      //是否使能ping
    bool isEnableWOL;                       //是否开启网络唤醒
	
	W5500_SOCKET_STATUS SocketStatus[8];	//socket状态
	W5500_PROTOCOL SocketProtocol[8];		//socket协议
}W5500_HANDLE;

						
//初始化W5500
bool W5500_Init(W5500_HANDLE *pHandle, u8 MAC[6], void (*SetCS_IO)(u8 Level),  u8  (*GetInt_IO)(void), u8 (*ReadWrtieByte)(u8 data), void (*Sleep)(u32 ms), char HostName[16], 
	bool isEnablePing, bool isEnableWOL);
//初始化W5500配置
bool W5500_InitConfig(W5500_HANDLE *pHandle);
	
	
/*===================================Socket接口相关===================================*/
void W5500_DisconnectOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);																				//断开一个socket连接
void W5500_CloseOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);																					//强制关闭一个socket
W5500_SOCKET_ERROR W5500_CreateUdpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort,const  u8 DestIp[4], u16 DestPort, bool isEnableBroadcast);	//创建一个UDP客户端
W5500_SOCKET_ERROR W5500_CreateTcpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort,const  u8 DestIp[4], u16 DestPort, u16 TimeOutSecond);		//创建一个TCP客户端
W5500_SOCKET_ERROR W5500_CreateTcpServer(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LoaclPort);													//W5500创建一个socket服务端
W5500_PROTOCOL W5500_GetOneSocketProtocol(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);																	//获取一个socket对应的协议类型
void W5500_ReadOneSocketTcpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen);												//读取某一个socket TCP接收缓冲区数据
u16 W5500_ReadOneSocketUdpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen, u8 ip[4], u16 *pPort);							//读取某一个socket UDP接收缓冲区数据
W5500_SOCKET_ERROR W5500_WriteOneSocketTcpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen);							//TCP写入发送数据到一个socket发送缓冲区
W5500_SOCKET_ERROR W5500_WriteOneSocketUdpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen, u8 ip[4], u16 port);		//UDP写入发送指定IP端口的数据到一个socket发送缓冲区
/*==================================================================================*/	
	

//底层寄存器通讯接口
u8 W5500_ReadOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr);					//读单个寄存器
void W5500_WriteOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 data);		//写单个寄存器
void W5500_ReadMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr,  u8 DataBuff[], u16 DataLen);//读多个寄存器
void W5500_WriteMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 DataBuff[], u16 DataLen);//写多个寄存器
u16 W5500_ReadDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr);				//读取2个连续的寄存器
void W5500_WriteDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u16 data);	//写入2个连续的寄存器


//常用快捷操作-系统操作
void W5500_ClearSysInterrupt(W5500_HANDLE *pHandle, u8 W5500_INT);					//W5500系统中断清除
u8 W5500_GetSysInterrupt(W5500_HANDLE *pHandle);									//获取W5500系统中断状态
void W5500_SetSysInterruptEnable(W5500_HANDLE *pHandle, u8 W5500_INT);				//W5500系统中断使能设置
u8 W5500_GetSysInterruptEnable(W5500_HANDLE *pHandle);								//获取W5500系统中断开启状态
void W5500_SetPHY_OPMDC(W5500_HANDLE *pHandle, W5500_PHY_OPMDC OPMDC);				//W5500 配置PHY（物理层端口配置）
W5500_PHY_OPMDC W5500_GetPHY_OPMDC(W5500_HANDLE *pHandle);							//W5500 获取PHY配置（物理层端口配置）
u8 W5500_GetPHY_Status(W5500_HANDLE *pHandle);										//W5500 获取PHY状态
u8 W5500_GetSocketTotalInterruptStatus(W5500_HANDLE *pHandle);						//获取socket总中断状态-每一个bit代表一个socket是否发生了中断
void W5500_ClearSocketTotalInterruptStatus(W5500_HANDLE *pHandle, u8 SokctBit);		//清除socket总中断状态-每一个bit代表一个socket中断清除（对应位为1则清除）
u8 W5500_GetSocketTotalInterruptEnable(W5500_HANDLE *pHandle);						//获取socket总中断开启状态-每一个bit代表一个socket是否开启了中断
void W5500_SetSocketTotalInterruptEnable(W5500_HANDLE *pHandle, u8 SokctBit);		//设置socket总中断状态-每一个bit代表一个socket是否开启了中断
u8 W5500_GetVersion(W5500_HANDLE *pHandle);						//获取W5500芯片版本（W5500版本默认为0x04）
void W5500_SetLocalIP(W5500_HANDLE *pHandle, u8 ip[4]);			//设置本机IP地址
void W5500_SetGatewayAddr(W5500_HANDLE *pHandle, u8 ip[4]);		//设置网关IP地址
void W5500_SetMaskAddr(W5500_HANDLE *pHandle, u8 ip[4]);		//设置子网掩码
void W5500_SetMAC(W5500_HANDLE *pHandle, u8 mac[6]);			//设置本机MAC硬件地址
void W5500_GetMAC(W5500_HANDLE *pHandle, u8 mac[6]);			//获取本机MAC硬件地址
void W5500_SoftwareReset(W5500_HANDLE *pHandle);				//W5500软复位-最好额外等待复位完成,至少1ms
void W5500_SetIntLevel(W5500_HANDLE *pHandle, u16 LevelTime);	//设置中断低电间隔时间，2次中断拉低的间隔时间(时间间隔：4/CLK)
void W5500_SetRetryTime(W5500_HANDLE *pHandle, u16 Time100us);	//设置重传超时时间-单位100us
u16 W5500_GetRetryTime(W5500_HANDLE *pHandle);					//获取重传超时时间-单位100us
void W5500_SetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount);	//W5500设置发送重试次数,RetryCount+1次，超出后会触发超时中断
u8 W5500_GetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount);	//W5500获取发送重试次数,RetryCount+1次，超出后会触发超时中断
void W5500_GetUnableIP(W5500_HANDLE *pHandle, u8 ip[4]);		//W5500获取无法抵达的IP地址
u16 W5500_GetUnablePort(W5500_HANDLE *pHandle);					//W5500获取无法抵达的端口

//常用快捷操作-单个socket操作
u8 W5500_GetOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);                        	//W5500 获取某一个socket的具体中断状态
void W5500_ClearOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) ;  	//W5500 清除某一个socket的具体中断状态
void W5500_SetOneSocketIntEnable(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) ;		//W5500 设置某一个socket对应的状态数据产生中断
void W5500_SetOneSocketTcpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//W5500设置一个sockt为TCP模式(socket选择：0-7);
void W5500_SetOneSocketUdpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, bool isEnableBroadcast);	//W5500设置一个sockt为UDP模式(socket选择：0-7)-需要先设置组播IP与端口，再调用此函数
void W5500_SetOneSocket0MacrawMode(W5500_HANDLE *pHandle, bool isMACRAW, bool isEnableBroadcast );		//W5500设置一个sockt为MACRAW模式(只能使用socket 0)
void W5500_SendOneSocketCmd(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, W5500_SOCKET_CMD SocketCmd);//W5500发送一个socket命令(socket选择：0-7, SocketCmd:socket命令)用于控制一个socket状态
W5500_SOCKET_STATUS W5500_GetOneSocketStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);			//获取一个socket状态
void W5500_SetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LocalPort);		//设置一个socket的本地端口
u16 W5500_GetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);						//获取一个socket的本地端口
void W5500_SetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6]);				//设置一个socket的目标MAC地址(用于UDP模式下，使用Send_MAC配置命令的目标主机MAC地址，或者存储ARP获取到的MAC地址)
void W5500_GetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6]);				//获取一个socket的目标MAC地址(用于UDP模式下，使用Send_MAC配置命令前配置的目标主机MAC地址，或者存储ARP获取到的MAC地址)
void W5500_SetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 ip[4]);			//设置一个socket的目标ip地址(用于TCP/UDP模式设置目标主机IP地址，或者存储TCP/UDP Server模式下从机IP地址)
void W5500_GetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 ip[4]);				//获取一个socket的目标ip地址(用于获取TCP/UDP模式设置目标主机IP地址，或者获取TCP/UDP Server模式下从机IP地址)
void W5500_SetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 DestPort);			//设置一个socket的目标端口(用于TCP/UDP模式设置目标主机端口，或者存储TCP/UDP Server模式下从机端口)
u16 W5500_GetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//获取一个socket的目标端口(用于获取TCP/UDP模式设置目标主机端口，或者获取TCP/UDP Server模式下从机端口)
void W5500_SetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 MaxTransUnit);	//设置一个socket的最大传输单元（TCP最大1460，UDP最大1472，PPPOE TCP最大1452 PPPOE UDP最大1464 MACRAW下最大1514）
u16 W5500_GetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);						//获取一个socket的最大传输单元（TCP最大1460，UDP最大1472，PPPOE TCP最大1452 PPPOE UDP最大1464 MACRAW下最大1514）
void W5500_SetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TOS);					//设置一个socket的服务类型字段TOS(通常保持为0即可)
u8 W5500_GetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);								//获取一个socket的服务类型字段TOS
void W5500_SetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TTL);					//设置一个socket的IP生存时间TTL
u8 W5500_GetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);								//获取一个socket的IP生存时间TTL
void W5500_SetOneSocketRxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize);	//设置一个socket的接收缓冲区大小
void W5500_SetOneSocketTxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize);	//设置一个socket的发送缓冲区大小
u16 W5500_GetOneSocketFreeTxBuffSzie(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);					//获取一个socket的发送缓冲区剩余大小(超出剩余空间大小的发送数据不被允许)
u16 W5500_GetOneSocketTxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//获取一个socket的发送读指针（可用通过OPEN命令进行初始化，在TCP连接期间也会被初始化，SEND命令会发送TX_RD到TX_WR之间的数据）
void W5500_SetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr);				//设置一个socket的发送写指针（可用通过OPEN命令进行初始化，在TCP连接期间也会被初始化，SEND命令会发送TX_RD到TX_WR之间的数据，使用SEND命令会发送TX_RD到TX_WR之间的数据）
u16 W5500_GetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//获取一个socket的发送写指针（可用通过OPEN命令进行初始化，在TCP连接期间也会被初始化，SEND命令会发送TX_RD到TX_WR之间的数据）
u16 W5500_GetOneSocketRxDataSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);						//获取一个socket的接收数据大小
void W5500_SetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr);				//设置一个socket的接收读指针（可以通过OPEN命令初始化，设置接收缓冲区中数据的首地址,将读写设置为一样则清除接收缓冲区，通过RECV命令通知W5500更新RXRD）
u16 W5500_GetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//获取一个socket的接收读指针（可以通过OPEN命令初始化，读取保存在接收缓冲区中数据的首地址）
u16 W5500_GetOneSocketRxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//获取一个socket的接收写指针（可用通过OPEN命令进行初始化，随着数据的接收会自增，并且会重复覆盖）
void W5500_SetOneSocketNotFrag(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);							//关闭一个socket分段（一般使用都建议不要分段）
void W5500_SetOneSocketTcpHeartPackTime(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 Time5sCount);//设置一个socket TCP连接下心跳包时间（单位：5秒,如果设置为0将关闭）










/***************************************常用操作(内联)************************************************************************************/



//获取socket总中断状态-每一个bit代表一个socket是否发生了中断
__inline u8 W5500_GetSocketTotalInterruptStatus(W5500_HANDLE *pHandle) 
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_SIR_REG_0x17);
}

//清除socket总中断状态-每一个bit代表一个socket中断清除（对应位为1则清除）
__inline void W5500_ClearSocketTotalInterruptStatus(W5500_HANDLE *pHandle, u8 SokctBit) 
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_SIR_REG_0x17, SokctBit);
}

//获取socket总中断开启状态-每一个bit代表一个socket是否开启了中断
__inline u8 W5500_GetSocketTotalInterruptEnable(W5500_HANDLE *pHandle) 
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_SIMR_REG_0x18);
}

//设置socket总中断状态-每一个bit代表一个socket是否开启了中断
__inline void W5500_SetSocketTotalInterruptEnable(W5500_HANDLE *pHandle, u8 SokctBit) 
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_SIMR_REG_0x18, SokctBit);	
}

//获取W5500芯片版本（W5500版本默认为0x04）
__inline u8 W5500_GetVersion(W5500_HANDLE *pHandle) 
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_VERSIONR_REG_0x39);
}




//设置本机IP地址
__inline void W5500_SetLocalIP(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SIPR0_REG_0x0F, ip, 4);	//设置IP
}

//设置网关IP地址
__inline void W5500_SetGatewayAddr(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_GAR0_REG_0x01, ip, 4);	//设置网关
}

//设置子网掩码
__inline void W5500_SetMaskAddr(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SUBR0_REG_0x05, ip, 4);	//设置子网掩码
}

//设置本机MAC硬件地址
__inline void W5500_SetMAC(W5500_HANDLE *pHandle, u8 mac[6])
{
	W5500_WriteMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SHAR0_REG_0x09, mac, 6);	//设置MAC硬件地址
}

//获取本机MAC硬件地址
__inline void W5500_GetMAC(W5500_HANDLE *pHandle, u8 mac[6])
{
	W5500_ReadMultiReg(pHandle, SELECT_GREG_00, (u16)W5500_SHAR0_REG_0x09, mac, 6);	//获取MAC硬件地址
}

//W5500软复位-最好额外等待复位完成,至少1ms
__inline void W5500_SoftwareReset(W5500_HANDLE *pHandle)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E, 0x18);	//PHY复位
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_MODE_REG_0x00, 0x80);		//软件复位	
}

//设置中断低电间隔时间，2次中断拉低的间隔时间(时间间隔：4/CLK)
__inline void W5500_SetIntLevel(W5500_HANDLE *pHandle, u16 LevelTime)
{
	W5500_WriteDuadReg(pHandle, SELECT_GREG_00, W5500_INTLEVEL0_REG_0x13, LevelTime);
}

//设置重传超时时间-单位100us
__inline void W5500_SetRetryTime(W5500_HANDLE *pHandle, u16 Time100us)
{
	W5500_WriteDuadReg(pHandle, SELECT_GREG_00, W5500_RTR0_REG_0x19, Time100us);	
}

//获取重传超时时间-单位100us
__inline u16 W5500_GetRetryTime(W5500_HANDLE *pHandle)
{
	return W5500_ReadDuadReg(pHandle, SELECT_GREG_00, W5500_RTR0_REG_0x19);		
}

//W5500设置发送重试次数,RetryCount+1次，超出后会触发超时中断
__inline void W5500_SetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_RCR_REG_0x1B, RetryCount);	
}

//W5500获取发送重试次数,RetryCount+1次，超出后会触发超时中断
__inline u8 W5500_GetRetryCount(W5500_HANDLE *pHandle, u8 RetryCount)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_RCR_REG_0x1B);	
}

//W5500获取无法抵达的IP地址
__inline void W5500_GetUnableIP(W5500_HANDLE *pHandle, u8 ip[4])
{
	W5500_ReadMultiReg(pHandle, SELECT_GREG_00, W5500_UIPR0_REG_0x28, ip, 4);	
}

//W5500获取无法抵达的端口
__inline u16 W5500_GetUnablePort(W5500_HANDLE *pHandle)
{
	return W5500_ReadDuadReg(pHandle, SELECT_GREG_00, W5500_UPORTR0_REG_0x2C);		
}


//W5500设置一个sockt为TCP模式(socket选择：0-7)
__inline void W5500_SetOneSocketTcpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return ;    //超出范围
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00, (SOCKET_TCP&0x07) | BIT5);  //设置为TCP模式，并开启无延时ACK
}

//W5500设置一个sockt为UDP模式(socket选择：0-7)-需要先设置组播IP与端口，再调用此函数
__inline void W5500_SetOneSocketUdpMode(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, bool isEnableBroadcast)
{
    u8 data = 0;
    
    if(socket > 7) return;    		//超出范围
    data = SOCKET_UDP & 0x07;
    if(isEnableBroadcast)           //使能了广播
    {
        //data |= BIT7;               //开启广播-开启后只能发送广播数据包，所以不要开启，只需要开启广播数据包接收即可
        data &= ~BIT6;              //关闭广播屏蔽
    }
    else //没有开启广播模式
    {
        data |= BIT6;              //启用广播屏蔽
    }
    //默认使用IGMP版本2，关闭单播模式屏蔽
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00, data);
}

//W5500设置一个sockt为MACRAW模式(只能使用socket 0)
__inline void W5500_SetOneSocket0MacrawMode(W5500_HANDLE *pHandle, bool isMACRAW, bool isEnableBroadcast )
{
    u8 data;
    
    data = SOCKET_MACRAW & 0x07;
    if(isEnableBroadcast)           //使能了广播
    {
        data |= BIT7;               //开启广播
        data &= ~BIT5;              //关闭广播屏蔽
    }
    else
    {
        data |= BIT5;              //启用广播屏蔽
    }
    data |= BIT4;                   //IPv6数据包屏蔽
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[0], Sn_MR_0x00, data);
}

//W5500发送一个socket命令(socket选择：0-7, SocketCmd:socket命令)用于控制一个socket状态
__inline void W5500_SendOneSocketCmd(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, W5500_SOCKET_CMD SocketCmd)
{
    if(socket > 7) return ;    //超出范围
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_CR_0x01, SocketCmd);
}


//获取一个socket状态
__inline W5500_SOCKET_STATUS W5500_GetOneSocketStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket) 
{
    if(socket > 7) return SOCK_CLOSED;    //超出范围
	return (W5500_SOCKET_STATUS)W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_SR_0x03);
}

//设置一个socket的本地端口
__inline void W5500_SetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LocalPort)
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_PORT_0x04, LocalPort); 
}

//获取一个socket的本地端口
__inline u16 W5500_GetOneSocketLoaclPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_PORT_0x04); 
}

//设置一个socket的目标MAC地址(用于UDP模式下，使用Send_MAC配置命令的目标主机MAC地址，或者存储ARP获取到的MAC地址)
__inline void W5500_SetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6])
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DHAR_0x06, MAC, 6); 
}

//获取一个socket的目标MAC地址(用于UDP模式下，使用Send_MAC配置命令前配置的目标主机MAC地址，或者存储ARP获取到的MAC地址)
__inline void W5500_GetOneSocketDestMAC(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 MAC[6])
{
    if(socket > 7) return ;    //超出范围
    W5500_ReadMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DHAR_0x06, MAC, 6); 
}

//设置一个socket的目标ip地址(用于TCP/UDP模式设置目标主机IP地址，或者存储TCP/UDP Server模式下从机IP地址)
__inline void W5500_SetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 ip[4])
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DIPR_0x0C, (u8 *)ip, 4); 
}

//获取一个socket的目标ip地址(用于获取TCP/UDP模式设置目标主机IP地址，或者获取TCP/UDP Server模式下从机IP地址)
__inline void W5500_GetOneSocketDestIP(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 ip[4])
{
    if(socket > 7) return ;    //超出范围
    W5500_ReadMultiReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DIPR_0x0C, ip, 4); 
}

//设置一个socket的目标端口(用于TCP/UDP模式设置目标主机端口，或者存储TCP/UDP Server模式下从机端口)
__inline void W5500_SetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 DestPort)
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DPORT_0x10, DestPort);
}

//获取一个socket的目标端口(用于获取TCP/UDP模式设置目标主机端口，或者获取TCP/UDP Server模式下从机端口)
__inline u16 W5500_GetOneSocketDestPort(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_DPORT_0x10); 
}

//设置一个socket的最大传输单元（TCP最大1460，UDP最大1472，PPPOE TCP最大1452 PPPOE UDP最大1464 MACRAW下最大1514）
__inline void W5500_SetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 MaxTransUnit)
{
    if(socket > 7) return ;    //超出范围
    if(MaxTransUnit < 1) MaxTransUnit = 1;
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MSSR_0x12, MaxTransUnit);
}

//获取一个socket的最大传输单元（TCP最大1460，UDP最大1472，PPPOE TCP最大1452 PPPOE UDP最大1464 MACRAW下最大1514）
__inline u16 W5500_GetOneSocketMaxTransUnit(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MSSR_0x12); 
}

//设置一个socket的服务类型字段TOS(通常保持为0即可)
__inline void W5500_SetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TOS)
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TOS_0x15, TOS);
}

//获取一个socket的服务类型字段TOS
__inline u8 W5500_GetOneSocketTOS(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TOS_0x15); 
}

//设置一个socket的IP生存时间TTL
__inline void W5500_SetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  u8 TTL)
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TTL_0x16, TTL);
}

//获取一个socket的IP生存时间TTL
__inline u8 W5500_GetOneSocketTTL(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TTL_0x16); 
}

//设置一个socket的接收缓冲区大小
__inline void W5500_SetOneSocketRxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize)
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RXBUFF_SIZE_0x1E, BuffSize);
}

//设置一个socket的发送缓冲区大小
__inline void W5500_SetOneSocketTxBuffSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,  W5500_SOCKET_BUFF_SIZE BuffSize)
{
    if(socket > 7) return ;    //超出范围
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TXBUFF_SIZE_0x1F, BuffSize);
}

//获取一个socket的发送缓冲区剩余大小(超出剩余空间大小的发送数据不被允许)
__inline u16 W5500_GetOneSocketFreeTxBuffSzie(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_FSR_0x20); 
}
//获取一个socket的发送读指针（可用通过OPEN命令进行初始化，在TCP连接期间也会被初始化，SEND命令会发送TX_RD到TX_WR之间的数据）
__inline u16 W5500_GetOneSocketTxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_RD_0x22); 
}

//设置一个socket的发送写指针（可用通过OPEN命令进行初始化，在TCP连接期间也会被初始化，SEND命令会发送TX_RD到TX_WR之间的数据，使用SEND命令会发送TX_RD到TX_WR之间的数据）
__inline void W5500_SetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr)
{
    if(socket > 7) return;    //超出范围
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_WR_0x24, ptr); 
}

//获取一个socket的发送写指针（可用通过OPEN命令进行初始化，在TCP连接期间也会被初始化，SEND命令会发送TX_RD到TX_WR之间的数据）
__inline u16 W5500_GetOneSocketTxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_TX_WR_0x24); 
}

//获取一个socket的接收数据大小
__inline u16 W5500_GetOneSocketRxDataSize(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_RSR_0x26); 
}

//设置一个socket的接收读指针（可以通过OPEN命令初始化，设置接收缓冲区中数据的首地址,将读写设置为一样则清除接收缓冲区，通过RECV命令通知W5500更新RXRD）
__inline void W5500_SetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 ptr)
{
    if(socket > 7) return;    //超出范围
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_RD_0x28, ptr); 
}

//获取一个socket的接收读指针（可以通过OPEN命令初始化，读取保存在接收缓冲区中数据的首地址）
__inline u16 W5500_GetOneSocketRxBuffRD(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_RD_0x28); 
}

//获取一个socket的接收写指针（可用通过OPEN命令进行初始化，随着数据的接收会自增，并且会重复覆盖）
__inline u16 W5500_GetOneSocketRxBuffWR(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return 0;    //超出范围
    return W5500_ReadDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_RX_WR_0x2A); 
}


//关闭一个socket分段（一般使用都建议不要分段）
__inline void W5500_SetOneSocketNotFrag(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return;   //超出范围
    W5500_WriteDuadReg(pHandle, SELECT_SOCKET_REG[socket], Sn_FRAG_0x2D, 0x4000);     //不要分段
}

//设置一个socket TCP连接下心跳包时间（单位：5秒,如果设置为0将关闭）
__inline void W5500_SetOneSocketTcpHeartPackTime(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 Time5sCount)
{
    if(socket > 7) return;   //超出范围
    W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_KPALVTR_0x2F, Time5sCount);     //心跳包时间
}

/*************************************************************************************************************************************/


#endif /*_W5500_H_*/

