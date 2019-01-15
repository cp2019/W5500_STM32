/*************************************************************************************************************
 * 文件名:			W5500.c
 * 功能:			W5500网络芯片驱动
 * 作者:			cp1300@139.com
 * 创建时间:		2015-01-25
 * 最后修改时间:	2015-01-25
 * 详细:			网络芯片驱动,支持TCP,UDP协议
					如果使用了UCOS,所有操作必须在线程中执行
					关于UDP模式的说明：UDP是不分主从机的，比如我当前UDP监测端口为123，目标IP为1.2.3.4，端口为100，如果目标IP端口开启了监控，当前W5500是可以发送数据到目标服务器的
						但是如果对方是UDP主动发送数据到本机IP与端口123，则本机一样可以收到数据，也就是那个端口100是无法控制其他设备发送数据到本机的
					UDP模式下的广播接收需要开启，但是不要开启广播发送，开启广播发送后将无法发送单播数据包，如果需要发送广播包，只需要将IP设置为255即可
*************************************************************************************************************/
#include "system.h"
#include "W5500.h"
#include "SPI.h"
#include "board.h"
#include "string.h"


//socket寄存器控制寄存器选择列表
W5500_BLOCK_SELECT const SELECT_SOCKET_REG[8] = {SELECT_SECKET0_01, SELECT_SECKET1_05, SELECT_SECKET2_09, SELECT_SECKET3_13, SELECT_SECKET4_17, SELECT_SECKET5_21,
	SELECT_SECKET6_25, SELECT_SECKET7_29};
//socket发送缓冲区寄存器选择列表
W5500_BLOCK_SELECT const SELECT_SOCKET_TXBUFF[8] = {SELECT_SECKET0_TXREG_02, SELECT_SECKET1_TXREG_06, SELECT_SECKET2_TXREG_10, SELECT_SECKET3_TXREG_14, SELECT_SECKET4_TXREG_18, SELECT_SECKET5_TXREG_22,
	SELECT_SECKET6_TXREG_26, SELECT_SECKET7_TXREG_30};
//socket接收缓冲区寄存器选择列表
W5500_BLOCK_SELECT const SELECT_SOCKET_RXBUFF[8] = {SELECT_SECKET0_RXREG_03, SELECT_SECKET1_RXREG_07, SELECT_SECKET2_RXREG_11, SELECT_SECKET3_RXREG_15, SELECT_SECKET4_RXREG_19, SELECT_SECKET5_RXREG_23,
	SELECT_SECKET6_RXREG_27, SELECT_SECKET7_RXREG_31};



//数据包生存时间
#define SOCKET_TTL		        128
#define SOCKET_RETRY_COUNT		3			//失败重试3+1次
#define SOCKET_TIME_OUT			(10*200)	//单次发送超时时间200ms
#define SOCKET_SEND_TIME_OUT	((SOCKET_RETRY_COUNT+2)*(SOCKET_TIME_OUT/10))	//数据包发送超时时间
	
	
//最大传输单元
#define SOCKET_UDP_PACKSIZE		1472 		//1-1472
#define SOCKET_TCP_PACKSIZE		1460 		//1-1460
#define SOCKET_PPPOE_PACKSIZE	1464 		//1-1464
#define SOCKET_MACRAW_PACKSIZE	1514 		//1-1514

//读写定义
#define W5500_READ_MODE		0	//读
#define W5500_WRTIE_MODE	1	//写


void W5500_OneSocketRxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);   //W5500 复位一个socket接收缓冲区
void W5500_OneSocketTxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket);  	//W5500 复位一个socket发送缓冲区  
    

/*************************************************************************************************************************
* 函数			:	u8 W5500_ReadOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr)
* 功能			:	读单个寄存器
* 参数			:	pHandle:W5500接口句柄；RegBlockSelect:寄存器模块选择;RegAddr:寄存器地址
* 返回			:	读取的数据
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
u8 W5500_ReadOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr)
{
	u8 temp;
	
	temp = RegBlockSelect << 3;					//BSB
	temp |= (W5500_READ_MODE&1)<<2;				//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);			//写入寄存器地址,先写入高位
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);				//写入命令
	//读取
	temp = pHandle->ReadWrtieByte(0xff);		//读取1字节
	pHandle->SetCS_IO(1);
	
	return temp;
}




/*************************************************************************************************************************
* 函数			:	void W5500_WriteOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 data)
* 功能			:	写单个寄存器
* 参数			:	pHandle:W5500接口句柄；RegBlockSelect:寄存器模块选择;RegAddr:寄存器地址;data:需要写入的数据
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
void W5500_WriteOneReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 data)
{
	u8 temp;
	
	temp = RegBlockSelect << 3;					//BSB
	temp |= (W5500_WRTIE_MODE&1)<<2;			//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);			//写入寄存器地址,先写入高位
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);				//写入命令
	//写入
	pHandle->ReadWrtieByte(data);				//写入1字节
	pHandle->SetCS_IO(1);
}



/*************************************************************************************************************************
* 函数			:	void W5500_ReadMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr,  u8 DataBuff[], u16 DataLen)
* 功能			:	读多个寄存器
* 参数			:	pHandle:W5500接口句柄；RegBlockSelect:寄存器模块选择;RegAddr:寄存器地址;DataBuff:寄存器读取缓冲区;DataLen:读取的长度
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
void W5500_ReadMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr,  u8 DataBuff[], u16 DataLen)
{
	u8 temp;
	u16 i;
	
	temp = RegBlockSelect << 3;						//BSB
	temp |= (W5500_READ_MODE&1)<<2;					//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);				//写入寄存器地址,先写入高位
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);					//写入命令
	//循环读取
	for(i = 0;i < DataLen;i ++)
	{
		DataBuff[i] = pHandle->ReadWrtieByte(0xff);	//读取1字节
	}
	pHandle->SetCS_IO(1);
}



/*************************************************************************************************************************
* 函数			:	void W5500_WriteMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 DataBuff[], u16 DataLen)
* 功能			:	写多个寄存器
* 参数			:	pHandle:W5500接口句柄；RegBlockSelect:寄存器模块选择;RegAddr:寄存器地址;DataBuff:寄存器写入缓冲区;DataLen:写入的长度
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
void W5500_WriteMultiReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u8 DataBuff[], u16 DataLen)
{
	u8 temp;
	u16 i;
	
	temp = RegBlockSelect << 3;					//BSB
	temp |= (W5500_WRTIE_MODE&1)<<2;			//RWB
	pHandle->SetCS_IO(0);
	pHandle->ReadWrtieByte(RegAddr>>8);			//写入寄存器地址,先写入高位
	pHandle->ReadWrtieByte((u8)RegAddr);
	pHandle->ReadWrtieByte(temp);				//写入命令
	//循环写入
	for(i = 0;i < DataLen;i ++)
	{
		pHandle->ReadWrtieByte(DataBuff[i]);	//写入1字节
	}
	pHandle->SetCS_IO(1);
}



/*************************************************************************************************************************
* 函数			:	u16 W5500_ReadDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr)
* 功能			:	读取2个连续的寄存器(16位寄存器)
* 参数			:	pHandle:W5500接口句柄；RegBlockSelect:寄存器模块选择;RegAddr:寄存器地址
* 返回			:	16位寄存器数据
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
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
* 函数			:	void W5500_WriteDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u16 data)
* 功能			:	写入2个连续的寄存器(16位寄存器)
* 参数			:	pHandle:W5500接口句柄；RegBlockSelect:寄存器模块选择;RegAddr:寄存器地址;data:需要写入的16位数据
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
void W5500_WriteDuadReg(W5500_HANDLE *pHandle, W5500_BLOCK_SELECT RegBlockSelect, u16 RegAddr, u16 data)
{
	u8 buff[2];
	
	buff[0] = data >> 8;
	buff[1] = (u8)data;
	W5500_WriteMultiReg(pHandle, RegBlockSelect, RegAddr, buff, 2);
}















/*************************************************************************************************************************
* 函数			:	void W5500_SetMode(W5500_HANDLE *pHandle, u8 W5500_MODE)
* 功能			:	W5500模式设置
* 参数			:	pHandle:W5500接口句柄；W5500_MODE：见模式定义，如：W5500_MODE_WOL_BIT，W5500_MODE_PING_BIT，W5500_MODE_PPPOE_BIT，W5500_MODE_FARP_BIT
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	W5500_MODE_WOL_BIT		(1<<5)		//网络唤醒模式开关，0：关闭网络唤醒;1：开启网络唤醒
					W5500_MODE_PING_BIT		(1<<4)		//ping block模式，0：关闭ping; 1:开启ping
					W5500_MODE_PPPOE_BIT	(1<<3)		//PPPOE模式开关；0：关闭PPPoE;1:开启PPPoE
					W5500_MODE_FARP_BIT		(1<<1)		//强迫ARP模式；0：关闭强迫 ARP 模；1：启用强迫 ARP 模式； 
						在强迫 ARP 模式下，无论是否发送 数据都会强迫ARPARPARP请
*************************************************************************************************************************/
void W5500_SetMode(W5500_HANDLE *pHandle, u8 W5500_MODE)
{
	W5500_MODE &= 0x7F;	//不允许最高位写1，最高位写1后会将内部寄存器初始化，必须复位后恢复
	
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_MODE_REG_0x00, W5500_MODE);		//写寄存器
}


/*************************************************************************************************************************
* 函数			:	u8 W5500_GetMode(W5500_HANDLE *pHandle)
* 功能			:	获取W5500模式
* 参数			:	pHandle:W5500接口句柄；
* 返回			:	见模式定义，如：W5500_MODE_WOL_BIT，W5500_MODE_PING_BIT，W5500_MODE_PPPOE_BIT，W5500_MODE_FARP_BIT
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	W5500_MODE_WOL_BIT		(1<<5)		//网络唤醒模式开关，0：关闭网络唤醒;1：开启网络唤醒
					W5500_MODE_PING_BIT		(1<<4)		//ping block模式，0：关闭ping; 1:开启ping
					W5500_MODE_PPPOE_BIT	(1<<3)		//PPPOE模式开关；0：关闭PPPoE;1:开启PPPoE
					W5500_MODE_FARP_BIT		(1<<1)		//强迫ARP模式；0：关闭强迫 ARP 模；1：启用强迫 ARP 模式； 
						在强迫 ARP 模式下，无论是否发送 数据都会强迫ARPARPARP请
*************************************************************************************************************************/
u8 W5500_GetMode(W5500_HANDLE *pHandle)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_MODE_REG_0x00);		//读寄存器
}



/*************************************************************************************************************************
* 函数			:	void W5500_ClearSysInterrupt(W5500_HANDLE *pHandle, u8 W5500_INT)
* 功能			:	W5500系统中断清除
* 参数			:	pHandle:W5500接口句柄；W5500_INT：中断选择；见W5500_INT_CONFLICT；W5500_INT_UNREACH；W5500_INT_PPPOE；W5500_INT_MP
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	W5500_INT_CONFLICT		(1<<7)	//IP冲突中断，在收到 APR 请求时，发现送方 IP 与本地 IP 重复，该位将被置 ‘1’
					W5500_INT_UNREACH		(1<<6)	//目标不可抵达,当接收到 ICMPICMPICMPICMP（目的端口不可达）包后，该位置‘ 1’
					W5500_INT_PPPOE			(1<<5)	//PPPoE 连接关闭，当 PPPoE模式下 ,PPPoE连接断开时生效
					W5500_INT_MP			(1<<4)	//网络唤醒模式下受到唤醒数据包是触发
*************************************************************************************************************************/
void W5500_ClearSysInterrupt(W5500_HANDLE *pHandle, u8 W5500_INT)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_IR_REG_0x15, W5500_INT);		//写寄存器
}

/*************************************************************************************************************************
* 函数			:	u8 W5500_GetSysInterrupt(W5500_HANDLE *pHandle)
* 功能			:	获取W5500系统中断状态
* 参数			:	pHandle:W5500接口句柄；
* 返回			:	W5500_INT：中断状态；见W5500_INT_CONFLICT；W5500_INT_UNREACH；W5500_INT_PPPOE；W5500_INT_MP
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	W5500_INT_CONFLICT		(1<<7)	//IP冲突中断，在收到 APR 请求时，发现送方 IP 与本地 IP 重复，该位将被置 ‘1’
					W5500_INT_UNREACH		(1<<6)	//目标不可抵达,当接收到 ICMP（目的端口不可达）包后，该位置‘ 1’
					W5500_INT_PPPOE			(1<<5)	//PPPoE 连接关闭，当 PPPoE模式下 ,PPPoE连接断开时生效
					W5500_INT_MP			(1<<4)	//网络唤醒模式下受到唤醒数据包是触发
*************************************************************************************************************************/
u8 W5500_GetSysInterrupt(W5500_HANDLE *pHandle)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_IR_REG_0x15);		//读寄存器
}



/*************************************************************************************************************************
* 函数			:	void W5500_SetSysInterruptEnable(W5500_HANDLE *pHandle, u8 W5500_INT)
* 功能			:	W5500系统中断使能设置
* 参数			:	pHandle:W5500接口句柄；W5500_INT：中断使能选择；见W5500_INT_CONFLICT；W5500_INT_UNREACH；W5500_INT_PPPOE；W5500_INT_MP
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	W5500_INT_CONFLICT		(1<<7)	//IP冲突中断，在收到 APR 请求时，发现送方 IP 与本地 IP 重复，该位将被置 ‘1’
					W5500_INT_UNREACH		(1<<6)	//目标不可抵达,当接收到 ICMP（目的端口不可达）包后，该位置‘ 1’
					W5500_INT_PPPOE			(1<<5)	//PPPoE 连接关闭，当 PPPoE模式下 ,PPPoE连接断开时生效
					W5500_INT_MP			(1<<4)	//网络唤醒模式下受到唤醒数据包是触发
*************************************************************************************************************************/
void W5500_SetSysInterruptEnable(W5500_HANDLE *pHandle, u8 W5500_INT)
{
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_IMR_REG_0x16, W5500_INT);		//写寄存器
}


/*************************************************************************************************************************
* 函数			:	u8 W5500_GetSysInterruptEnable(W5500_HANDLE *pHandle)
* 功能			:	获取W5500系统中断开启状态
* 参数			:	pHandle:W5500接口句柄；
* 返回			:	W5500_INT：中断开启状态；见W5500_INT_CONFLICT；W5500_INT_UNREACH；W5500_INT_PPPOE；W5500_INT_MP
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	W5500_INT_CONFLICT		(1<<7)	//IP冲突中断，在收到 APR 请求时，发现送方 IP 与本地 IP 重复，该位将被置 ‘1’
					W5500_INT_UNREACH		(1<<6)	//目标不可抵达,当接收到 ICMPICMPICMPICMP（目的端口不可达）包后，该位置‘ 1’
					W5500_INT_PPPOE			(1<<5)	//PPPoE 连接关闭，当 PPPoE模式下 ,PPPoE连接断开时生效
					W5500_INT_MP			(1<<4)	//网络唤醒模式下受到唤醒数据包是触发
*************************************************************************************************************************/
u8 W5500_GetSysInterruptEnable(W5500_HANDLE *pHandle)
{
	return W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_IMR_REG_0x16);		//读寄存器
}


/*************************************************************************************************************************
* 函数			:	void W5500_SetPHY_OPMDC(W5500_HANDLE *pHandle, W5500_PHY_OPMDC OPMDC)
* 功能			:	W5500 配置PHY（物理层端口配置）
* 参数			:	pHandle:W5500接口句柄；OPMDC：配置见W5500_PHY_OPMDC
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	
*************************************************************************************************************************/
void W5500_SetPHY_OPMDC(W5500_HANDLE *pHandle, W5500_PHY_OPMDC OPMDC)
{
	u8 temp = OPMDC;
	
	temp &= 0x07;
	temp <<=3;
	temp|=BIT7;	//最高位必须为1，否则会发生复位
	temp|=BIT6;//PHY工作模式选择；1：通过OPMDC配置
	W5500_WriteOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E, temp);		//写寄存器
}





/*************************************************************************************************************************
* 函数			:	W5500_PHY_OPMDC W5500_GetPHY_OPMDC(W5500_HANDLE *pHandle)
* 功能			:	W5500 获取PHY配置（物理层端口配置）
* 参数			:	pHandle:W5500接口句柄；
* 返回			:	W5500_PHY_OPMDC
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	
*************************************************************************************************************************/
W5500_PHY_OPMDC W5500_GetPHY_OPMDC(W5500_HANDLE *pHandle)
{
	u8 temp = W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E);
	
	temp >>= 3;
	temp &= 0x07;
	
	return (W5500_PHY_OPMDC)temp;
}



/*************************************************************************************************************************
* 函数			:	u8 W5500_GetPHY_Status(W5500_HANDLE *pHandle)
* 功能			:	W5500 获取PHY状态
* 参数			:	pHandle:W5500接口句柄；
* 返回			:	状态：W5500_PHY_STATUS_DPX_BIT；W5500_PHY_STATUS_SPD_BIT；W5500_PHY_STATUS_LNK_BIT
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	W5500_PHY_STATUS_DPX_BIT		(1<<2)	//全双工状态
					W5500_PHY_STATUS_SPD_BIT		(1<<1)	//速度状态；1:100；0:10M
					W5500_PHY_STATUS_LNK_BIT		(1<<0)	//网线连接状态；1：已经连接；0：未连接
*************************************************************************************************************************/
u8 W5500_GetPHY_Status(W5500_HANDLE *pHandle)
{
	u8 temp = W5500_ReadOneReg(pHandle, SELECT_GREG_00, W5500_PHYCFGR_REG_0x2E);
	temp &= 0x07;
	
	return temp;
}


/*************************************************************************************************************************
* 函数			:	u8 W5500_GetOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket) 
* 功能			:	W5500 获取某一个socket的具体中断状态
* 参数			:	pHandle:W5500接口句柄；socket：socket选择，0-7
* 返回			:	状态：SOCKET_IR_SEND_OK_BIT；SOCKET_IR_TIMEOUT_BIT；SOCKET_IR_RECV_BIT；SOCKET_IR_DISCON_BIT；SOCKET_IR_CON_BIT
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-12
* 最后修改时间	:	2019-01-12
* 说明			: 	SOCKET_IR_SEND_OK_BIT		(1<<4),	//发送完成
                    SOCKET_IR_TIMEOUT_BIT		(1<<3),	//APR或TCP发送超时
                    SOCKET_IR_RECV_BIT			(1<<2),	//收到新数据
                    SOCKET_IR_DISCON_BIT		(1<<1),	//对方断开连接
                    SOCKET_IR_CON_BIT			(1<<0)	//连接建立成功
*************************************************************************************************************************/
u8 W5500_GetOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket) 
{
    if(socket > 7) return 0;    //超出范围
	return W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_IR_0x02);
}



/*************************************************************************************************************************
* 函数			:	void W5500_ClearOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
* 功能			:	W5500 清除某一个socket的具体中断状态
* 参数			:	pHandle:W5500接口句柄；socket：socket选择，0-7；SocketIntBit：中断bit见
                        SOCKET_IR_SEND_OK_BIT；SOCKET_IR_TIMEOUT_BIT；SOCKET_IR_RECV_BIT；SOCKET_IR_DISCON_BIT；SOCKET_IR_CON_BIT
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-12
* 最后修改时间	:	2019-01-12
* 说明			: 	SOCKET_IR_SEND_OK_BIT		(1<<4),	//发送完成
                    SOCKET_IR_TIMEOUT_BIT		(1<<3),	//APR或TCP发送超时
                    SOCKET_IR_RECV_BIT			(1<<2),	//收到新数据
                    SOCKET_IR_DISCON_BIT		(1<<1),	//对方断开连接
                    SOCKET_IR_CON_BIT			(1<<0)	//连接建立成功
                    对应位为1则清除对应中断状态
*************************************************************************************************************************/
void W5500_ClearOneSocketIntStatus(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
{
    if(socket > 7) return ;    //超出范围
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_IR_0x02, SocketIntBit);
}




/*************************************************************************************************************************
* 函数			:	void W5500_SetOneSocketIntEnable(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
* 功能			:	W5500 设置某一个socket对应的状态数据产生中断
* 参数			:	pHandle:W5500接口句柄；socket：socket选择，0-7；SocketIntBit：中断bit见
                        SOCKET_IR_SEND_OK_BIT；SOCKET_IR_TIMEOUT_BIT；SOCKET_IR_RECV_BIT；SOCKET_IR_DISCON_BIT；SOCKET_IR_CON_BIT
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-12
* 最后修改时间	:	2019-01-12
* 说明			: 	SOCKET_IR_SEND_OK_BIT		(1<<4),	//发送完成
                    SOCKET_IR_TIMEOUT_BIT		(1<<3),	//APR或TCP发送超时
                    SOCKET_IR_RECV_BIT			(1<<2),	//收到新数据
                    SOCKET_IR_DISCON_BIT		(1<<1),	//对方断开连接
                    SOCKET_IR_CON_BIT			(1<<0)	//连接建立成功
                    对应位为1则开启对应中断，否则将不会开启
*************************************************************************************************************************/
void W5500_SetOneSocketIntEnable(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 SocketIntBit) 
{
    if(socket > 7) return ;    //超出范围
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_IMR_0x2C, SocketIntBit);
}



/*************************************************************************************************************************
* 函数			:	void W5500_OneSocketRxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* 功能			:	W5500 复位一个socket接收缓冲区
* 参数			:	pHandle:W5500接口句柄；socket:socket选择
* 返回			:	无
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2019-01-13
* 说明			: 	用于清除接收缓冲区
*************************************************************************************************************************/
void W5500_OneSocketRxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	u16 ptr;

	ptr = W5500_GetOneSocketRxBuffWR(pHandle, socket);               	//获取接收缓冲区写指针位置getSn_RX_WR(socket);
	W5500_SetOneSocketRxBuffRD(pHandle, socket, ptr);                	//将读指针=写指针，复位接收缓冲区
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_RECV);			//执行RECV命令，更新读取指针
	pHandle->Sleep(1);
}





/*************************************************************************************************************************
* 函数			:	void W5500_OneSocketTxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* 功能			:	W5500 复位一个socket发送缓冲区
* 参数			:	pHandle:W5500接口句柄；socket:socket选择
* 返回			:	无
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2019-01-13
* 说明			: 	用于清除发送缓冲区
*************************************************************************************************************************/
void W5500_OneSocketTxBuffReset(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	u16 ptr;

	ptr = W5500_GetOneSocketTxBuffRD(pHandle, socket);           	//获取发送的读指针位置
	W5500_SetOneSocketTxBuffWR(pHandle, socket, ptr);            	//设置写指针=读指针，复位发送缓冲区指针
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_SEND);		//执行send命令更新发送指针位置
	pHandle->Sleep(1);
}


/*************************************************************************************************************************
* 函数			:	void W5500_ReadOneSocketTcpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen)
* 功能			:	读取某一个socket TCP接收缓冲区数据
* 参数			:	pHandle:W5500接口句柄；socket:socket选择,DataBuff:数据缓冲区,DataLen:要读取数据长度
* 返回			:	无
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2019-01-13
* 说明			: 	必须先初始化
*************************************************************************************************************************/
void W5500_ReadOneSocketTcpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen)
{
	u16 ptr;
	if(DataLen == 0) return;
	
	ptr = W5500_GetOneSocketRxBuffRD(pHandle, socket);                                   //获取当前socket的接收缓冲区读指针
	W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr,  DataBuff, DataLen);  //读取数据
	W5500_SetOneSocketRxBuffRD(pHandle, socket, ptr+DataLen);                            //读取数据后更新接收缓冲区读指针位置
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_RECV);		                    //发送RECV命令，更新读取指针
}


/*************************************************************************************************************************
* 函数			:	void W5500_ReadOneSocketUdpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen, u8 ip[4], u16 *pPort)
* 功能			:	读取某一个socket UDP接收缓冲区数据
* 参数			:	pHandle:W5500接口句柄；socket:socket选择,DataBuff:数据缓冲区,DataLen:接收到的数据长度，包含IP端口数据占用的8字节数据;ip:对方IP；pPort：对方端口
* 返回			:	实际数据长度
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2019-01-14
* 最后修改时间	:	2019-01-14
* 说明			: 	UDP数据格式 IP1 IP2 IP3 IP4 PORT_H PORTL LEN_H LEN_L 数据
*************************************************************************************************************************/
u16 W5500_ReadOneSocketUdpRxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen, u8 ip[4], u16 *pPort)
{
	u16 ptr;
	u8 buff[8];			//前面8字节存放对方IP地址，端口，数据长度信息
	u16 temp;
	
	if(DataLen == 0) return 0;
	if(DataLen < 9)		//最少9字节
	{
		ptr = W5500_GetOneSocketRxBuffRD(pHandle, socket);                                   	//获取当前socket的接收缓冲区读指针
		W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr,  DataBuff, DataLen);  	//读取数据
		*pPort = 0;																				//无效的端口
		
		return 0;
	}
	
	ptr = W5500_GetOneSocketRxBuffRD(pHandle, socket);                                   //获取当前socket的接收缓冲区读指针
	W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr,  buff, 8);  			 //读取数据-先读取8字节
	//解析帧头数据
	memcpy(ip, buff, 4);	//IP
	temp = buff[4];	
	temp <<= 8;
	temp |= buff[5];
	*pPort = temp;			//端口号
	temp = buff[6];
	temp <<= 8;
	temp |= buff[7];		//数据长度
	if(temp > (DataLen-8)) temp = DataLen-8;
	
	W5500_ReadMultiReg(pHandle, SELECT_SOCKET_RXBUFF[socket], ptr+8,  DataBuff, temp);  //读取数据
	W5500_SetOneSocketRxBuffRD(pHandle, socket, ptr+DataLen);                            //读取数据后更新接收缓冲区读指针位置
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_RECV);		                     //发送RECV命令，更新读取指针
	
	return temp;			//返回长度
}


/*************************************************************************************************************************
* 函数			:	W5500_SOCKET_ERROR W5500_WriteOneSocketTcpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u8 DataBuff[], u16 DataLen)
* 功能			:	TCP模式写入发送数据到一个socket发送缓冲区
* 参数			:	pHandle:W5500接口句柄；socket:socket选择,DataBuff:数据缓冲区,DataLen:数据长度
* 返回			:	W5500_SOCKET_ERROR
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2019-01-13
* 说明			: 	必须先初始化，用于TCP客户端发送数据
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_WriteOneSocketTcpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen)
{
	u16 ptr;
	u16 SurSize;
    
	if(DataLen == 0) return SOCKET_ERROR;
	SurSize = W5500_GetOneSocketFreeTxBuffSzie(pHandle, socket);				//获取发送缓冲区剩余空间
	if(DataLen > SurSize) return SOCKET_TXBUFF_SIZE;                			//待发缓冲区空间不足
 
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_SEND_OK_BIT|SOCKET_IR_TIMEOUT_BIT);	//清除发送完成中断与接收超时中断
    ptr = W5500_GetOneSocketTxBuffRD(pHandle, socket);                       	//获取发送的读指针位置
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_TXBUFF[socket], ptr,  (u8 *)DataBuff, DataLen);//写入数据到发送缓冲区中  
	W5500_SetOneSocketTxBuffWR(pHandle, socket, ptr+DataLen);                	//发送后更新写指针位置
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_SEND);	            	//执行send命令更新发送指针位置
	
	return SOCKET_OK;
}



/*************************************************************************************************************************
* 函数			:	W5500_SOCKET_ERROR W5500_WriteOneSocketUdpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen, u8 ip[4], u16 port)
* 功能			:	UDP写入发送指定IP端口的数据到一个socket发送缓冲区
* 参数			:	pHandle:W5500接口句柄；socket:socket选择,DataBuff:数据缓冲区,DataLen:数据长度;ip:目标IP地址(如果为NULL则不进行设置)；port：目标端口(如果为0则不进行设置)
* 返回			:	W5500_SOCKET_ERROR
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2019-01-13
* 说明			: 	必须先初始化，用于UDP发送数据
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_WriteOneSocketUdpTxData(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, const u8 DataBuff[], u16 DataLen, u8 ip[4], u16 port)
{
	u16 ptr;
	u16 SurSize;
    
	if(DataLen == 0) return SOCKET_ERROR;
	SurSize = W5500_GetOneSocketFreeTxBuffSzie(pHandle, socket);				//获取发送缓冲区剩余空间
	if(DataLen > SurSize) return SOCKET_TXBUFF_SIZE;                			//待发缓冲区空间不足
 
	if(port != 0 && ip != NULL)	//无效的端口，不进行设置目标端口与IP地址
	{
		W5500_SetOneSocketDestIP(pHandle, socket, ip);							//写入目标IP地址
		W5500_SetOneSocketDestPort(pHandle, socket, port);						//写入目标端口
	}
	
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_SEND_OK_BIT|SOCKET_IR_TIMEOUT_BIT);	//清除发送完成中断与接收超时中断
    ptr = W5500_GetOneSocketTxBuffRD(pHandle, socket);                       	//获取发送的读指针位置
    W5500_WriteMultiReg(pHandle, SELECT_SOCKET_TXBUFF[socket], ptr,  (u8 *)DataBuff, DataLen);//写入数据到发送缓冲区中  
	W5500_SetOneSocketTxBuffWR(pHandle, socket, ptr+DataLen);                	//发送后更新写指针位置
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_SEND);	            	//执行send命令更新发送指针位置
	
	return SOCKET_OK;
}

/*************************************************************************************************************************
* 函数			:	W5500_PROTOCOL W5500_GetOneSocketProtocol(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* 功能			:	获取一个socket对应的协议类型
* 参数			:	pHandle:W5500接口句柄；socket:socket选择
* 返回			:	W5500_PROTOCOL
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	必须先初始化
*************************************************************************************************************************/
W5500_PROTOCOL W5500_GetOneSocketProtocol(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
    if(socket > 7) return SOCKET_CLOSED;    															//超出范围
	return (W5500_PROTOCOL)(W5500_ReadOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00)&0xf);		//获取socket协议模式
}



/*************************************************************************************************************************
* 函数			:	void W5500_CloseOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* 功能			:	强制关闭一个socket
* 参数			:	pHandle:W5500接口句柄；socket:socket选择
* 返回			:	W5500_PROTOCOL
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2019-01-13
* 最后修改时间	:	2019-01-13
* 说明			: 	必须先初始化
*************************************************************************************************************************/
void W5500_CloseOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	W5500_SOCKET_STATUS SocketStatus;
    if((u8)socket > 7) return;    //超出范围

	SocketStatus = W5500_GetOneSocketStatus(pHandle, socket);					//获取socket状态 
	if(SocketStatus == SOCK_LISTEN || SocketStatus == SOCK_ESTABLISHED)			//TCP监听状态或连接状态则发送断开命令
	{
		W5500_DisconnectOneSocket(pHandle, socket);								//发送一个SOCKET命令关闭当前socket-会通知对方断开连接
	}
	
	W5500_WriteOneReg(pHandle, SELECT_SOCKET_REG[socket], Sn_MR_0x00, 0x00);	//强制关闭一个socket
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);				//清除当前socket所有中断状态
	W5500_SetOneSocketIntEnable(pHandle, socket, 0);							//关闭所有中断
	
	uart_printf("强制关闭socket %d\r\n", socket);
}


/*************************************************************************************************************************
* 函数			:	W5500_PROTOCOL W5500_CreateUdpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort,const  u8 DestIp[4], u16 SestPort, bool isEnableBroadcast)
* 功能			:	创建一个UDP客户端
* 参数			:	pHandle:W5500接口句柄；socket:socket选择；LoaclPort：本地端口；DestIp：目标IP；SestPort：模板端口；isEnableBroadcast：是否使能广播
* 返回			:	W5500_PROTOCOL
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2019-01-13
* 最后修改时间	:	2019-01-13
* 说明			: 	如果开启了广播，只需要将目标ip设置为255即可实现广播或组播
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_CreateUdpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort, const u8 DestIp[4], u16 DestPort, bool isEnableBroadcast)
{
	W5500_SOCKET_STATUS status;
	u8 cnt = 0;
	
    if((u8)socket > 7) return SOCKET_ERROR;    //超出范围
	//读取当前socket状态，如果是开启状态，则先关闭
	status = W5500_GetOneSocketStatus(pHandle, socket);
	if(status != SOCK_CLOSED)					//没有关闭，先关闭
	{
		W5500_CloseOneSocket(pHandle, socket);								//强制关闭
		pHandle->Sleep(1);		
	}
	W5500_SetOneSocketUdpMode(pHandle, socket, isEnableBroadcast);			//设置当前socket为UDP模式
	W5500_SetOneSocketLoaclPort(pHandle, socket, LoaclPort);				//设置本地端口
	W5500_SetOneSocketDestIP(pHandle, socket, DestIp);						//设置目标服务器IP
	W5500_SetOneSocketDestPort(pHandle, socket, DestPort);					//设置目标服务器端口	 
	//发送命令，打开socket
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_OPEN);				//发送一个SOCKET命令 open
	//等待socket开启成功
	for(cnt = 0;cnt < 40;cnt ++)
	{
		status = W5500_GetOneSocketStatus(pHandle, socket);					//获取当前socket状态
		if(status == SOCK_UDP) break;
		pHandle->Sleep(100);
	}
	if(status != SOCK_UDP)
	{
		uart_printf("开启socket %d为UDP模式超时，当前状态：%d\r\n",socket,status );
		W5500_CloseOneSocket(pHandle, socket);								//强制关闭
		return SOCKET_OPEN_ERROR;											//开启socket超时
	}

	W5500_SetOneSocketMaxTransUnit(pHandle, socket, SOCKET_UDP_PACKSIZE);	//设置一个socket的最大传输单元
	W5500_OneSocketRxBuffReset(pHandle, socket);   							//W5500 复位一个socket接收缓冲区
	W5500_OneSocketTxBuffReset(pHandle, socket);  							//W5500 复位一个socket发送缓冲区 	
	W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);			//清除当前socket所有中断状态
	W5500_SetOneSocketIntEnable(pHandle, socket, SOCKET_IR_RECV_BIT);		//开启当前socket接收中断
	//不知为何UDP第一包会丢失，必须延时至少给700ms,此处给800MS延时
	pHandle->Sleep(800);
	
	uart_printf("开启socket %d为UDP模式成功!\r\n", socket);
	return SOCKET_OK;	//连接成功
}


/*************************************************************************************************************************
* 函数			:	W5500_SOCKET_ERROR W5500_CreateTcpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort, const u8 DestIp[4], u16 DestPort, u16 TimeOutSecond)
* 功能			:	创建一个TCP客户端
* 参数			:	pHandle:W5500接口句柄；socket:socket选择；LoaclPort：本地端口；DestIp：目标IP；SestPort：模板端口；TimeOutSecond：连接超时时间，单位秒
* 返回			:	W5500_PROTOCOL
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2019-01-13
* 最后修改时间	:	2019-01-13
* 说明			: 	必须先初始化
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_CreateTcpClient(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket,u16 LoaclPort, const u8 DestIp[4], u16 DestPort, u16 TimeOutSecond)
{
	W5500_SOCKET_STATUS status;
	u8 cnt = 0;
	u16 TimeOutCnt = 0;
	
    if(socket > 7) return SOCKET_ERROR;    							//超出范围
	if(TimeOutSecond < 1) TimeOutSecond = 1;
	//读取当前socket状态，如果是开启状态，则先关闭
	status = W5500_GetOneSocketStatus(pHandle, socket);
	if(status != SOCK_CLOSED)										//没有关闭，先关闭
	{
		W5500_CloseOneSocket(pHandle, socket);						//强制关闭
		pHandle->Sleep(1);	
	}
	W5500_SetOneSocketTcpMode(pHandle, socket);						//设置当前socket为TCP模式
	W5500_SetOneSocketLoaclPort(pHandle, socket, LoaclPort);		//设置本地端口
	W5500_SetOneSocketDestIP(pHandle, socket, DestIp);				//设置目标服务器IP
	W5500_SetOneSocketDestPort(pHandle, socket, DestPort);			//设置模板服务器端口	
	//发送命令，打开socket
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_OPEN);		//发送一个SOCKET命令 open
	//等待socket开启成功-变为init模式
	for(cnt = 0;cnt < 40;cnt ++)
	{
		status = W5500_GetOneSocketStatus(pHandle, socket);			//获取当前socket状态
		if(status == SOCK_INIT) break;
		pHandle->Sleep(100);
	}
	if(status != SOCK_INIT)
	{
		uart_printf("开启socket %d为TCP模式超时，当前状态：%d\r\n",socket,status );
		W5500_CloseOneSocket(pHandle, socket);						//强制关闭
		return SOCKET_OPEN_ERROR;									//开启socket超时
	}
	//执行CONHECT 连接服务器
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_CONHECT);	//发送一个SOCKET命令 CONHECT
	//等待连接服务器成功
	for(TimeOutCnt = 0; TimeOutCnt < TimeOutSecond; TimeOutCnt ++)
	{
		for(cnt = 0;cnt < 10;cnt ++)
		{
			pHandle->Sleep(100);
			status = W5500_GetOneSocketStatus(pHandle, socket);		//获取当前socket状态
			if(status == SOCK_ESTABLISHED) break;
			pHandle->Sleep(100);
		}
	}
	if(status == SOCK_ESTABLISHED) //连接服务器成功
	{
		W5500_SetOneSocketMaxTransUnit(pHandle, socket, SOCKET_TCP_PACKSIZE);	//设置一个socket的最大传输单元
		W5500_SetOneSocketTcpHeartPackTime(pHandle, socket, 2);					//设置一个socket TCP连接下心跳包时间（单位：5秒,如果设置为0将关闭）-10秒
		W5500_OneSocketRxBuffReset(pHandle, socket);   							//W5500 复位一个socket接收缓冲区
		W5500_OneSocketTxBuffReset(pHandle, socket);  							//W5500 复位一个socket发送缓冲区  
		W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);			//清除当前socket所有中断状态
		W5500_SetOneSocketIntEnable(pHandle, socket, SOCKET_IR_RECV_BIT|SOCKET_IR_DISCON_BIT);//接收数据中断,断开连接中断

		uart_printf("开启socket %d为TCP模式成功!\r\n", socket);
		return SOCKET_OK;	//连接成功
	}
	else
	{
		uart_printf("开启socket %d为TCP模式超时，当前状态：%d\r\n",socket,status);
		W5500_CloseOneSocket(pHandle, socket);						//强制关闭
		return SOCKET_TIMEOUT;								//连接超时
	}
}



/*************************************************************************************************************************
* 函数			:	W5500_SOCKET_ERROR W5500_CreateTcpServer(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LoaclPort)
* 功能			:	W5500创建一个socket服务端
* 参数			:	pHandle:W5500接口句柄；socket:socket选择;LoaclPort:本地监控端口
* 返回			:	无
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2019-01-13
* 说明			: 	一个socket服务端只能连接一个客户端
*************************************************************************************************************************/
W5500_SOCKET_ERROR W5500_CreateTcpServer(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket, u16 LoaclPort)
{
	W5500_SOCKET_STATUS status;
	u8 cnt = 0;
	
    if(socket > 7) return SOCKET_ERROR;    //超出范围
	//读取当前socket状态，如果是开启状态，则先关闭
	status = W5500_GetOneSocketStatus(pHandle, socket);
	if(status != SOCK_CLOSED)								//没有关闭，先关闭
	{
		W5500_CloseOneSocket(pHandle, socket);						//强制关闭
		pHandle->Sleep(1);	
	}
	W5500_SetOneSocketTcpMode(pHandle, socket);						//设置当前socket为TCP模式
	W5500_SetOneSocketLoaclPort(pHandle, socket, LoaclPort);			//设置本地端口	
	//发送命令，打开socket
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_OPEN);		//发送一个SOCKET命令 open
	pHandle->Sleep(2);
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_LISTEN);		//发送一个SOCKET命令 SOCKET_CMD_LISTEN
	
	//等待socket变为SOCK_LISTEN模式
	for(cnt = 0;cnt < 40;cnt ++)
	{
		status = W5500_GetOneSocketStatus(pHandle, socket);				//获取当前socket状态
		if(status == SOCK_LISTEN) break;
		pHandle->Sleep(100);
	}
	if(status != SOCK_LISTEN) //开启监听失败
	{
		uart_printf("开启socket %d为TCP服务LISTEN超时，当前状态：%d\r\n",socket,status );
		W5500_CloseOneSocket(pHandle, socket);							//强制关闭
		return SOCKET_OPEN_ERROR;								//开启socket超时
	}
	else //开启监听成功
	{
		W5500_SetOneSocketMaxTransUnit(pHandle, socket, SOCKET_TCP_PACKSIZE);	//设置一个socket的最大传输单元
		W5500_SetOneSocketTcpHeartPackTime(pHandle, socket, 2);					//设置一个socket TCP连接下心跳包时间（单位：5秒,如果设置为0将关闭）-10秒
		W5500_OneSocketRxBuffReset(pHandle, socket);   							//W5500 复位一个socket接收缓冲区
		W5500_OneSocketTxBuffReset(pHandle, socket);  							//W5500 复位一个socket发送缓冲区  
		W5500_ClearOneSocketIntStatus(pHandle, socket, SOCKET_IR_ALL);			//清除当前socket所有中断状态
		W5500_SetOneSocketIntEnable(pHandle, socket, SOCKET_IR_RECV_BIT|SOCKET_IR_DISCON_BIT|SOCKET_IR_CON_BIT);//开启接收数据中断,断开连接中断,新建连接中断
		
		uart_printf("开启socket %d为TCP服务LISTEN成功!\r\n", socket);
		return SOCKET_OK;	//连接成功
	}
}


 
/*************************************************************************************************************************
* 函数			:	void W5500_DisconnectOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
* 功能			:	断开一个socket连接
* 参数			:	pHandle:W5500接口句柄；socket:socket选择；
* 返回			:	W5500_PROTOCOL
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2019-01-14
* 最后修改时间	:	2019-01-14
* 说明			: 	会通知对方断开连接-比较友善的断开连接
*************************************************************************************************************************/
void W5500_DisconnectOneSocket(W5500_HANDLE *pHandle, W5500_SOCKET_NUM socket)
{
	W5500_SOCKET_STATUS status;
	u8 retry;
	
	if((u8)socket > 7) return;    //超出范围
	W5500_SendOneSocketCmd(pHandle, socket, SOCKET_CMD_CLOSE);											//发送一个SOCKET命令关闭当前socket
	for(retry = 0;retry < 20;retry ++)
	{
		pHandle->Sleep(5);	
		status = W5500_GetOneSocketStatus(pHandle, socket);												//获取socket状态
		if(status == SOCK_CLOSED) break;
	}
	if(status != SOCK_CLOSED)
	{
		uart_printf("socket %d 断开连接超时，状态：%d\r\n",socket, status);
	}
}
   
    
/*************************************************************************************************************************
* 函数			:	void W5500_Init(W5500_HANDLE *pHandle, u8 MAC[6])
* 功能			:	初始化W5500
* 参数			:	pHandle:W5500接口句柄；pHandle：句柄；MAC:MAC地址,共6B
					void (*SetCS_IO)(u8 Level);             //CS IO设置接口（1：高电平；0：低电平）
					u8  (*GetInt_IO)(void);                 //获取中断IO状态（高电平返回1，低电平返回0）
					u8 (*ReadWrtieByte)(u8 data);           //SPI读写接口（SPI读写一字节接口）
					void (*Sleep)(u32 ms);               	//毫秒延时接口
					MAC[6];                              	//MAC地址
					HostName[16+1];                      	//当前主机名称（用于DHCP中显示的主机名称）
					isEnablePing;                      		//是否使能ping
					isEnableWOL;                       		//是否开启网络唤醒
* 返回			:	无
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2015-01-26
* 最后修改时间	:	2015-01-26
* 说明			: 	无
*************************************************************************************************************************/
bool W5500_Init(W5500_HANDLE *pHandle, u8 MAC[6], void (*SetCS_IO)(u8 Level),  u8  (*GetInt_IO)(void), u8 (*ReadWrtieByte)(u8 data), void (*Sleep)(u32 ms), char HostName[16], 
	bool isEnablePing, bool isEnableWOL)
{
	u8 len;
	u8 i;
	
	if(pHandle == NULL)
	{
		DEBUG("[W5500]:无效的句柄，初始化失败！\r\n");
		return FALSE;
	}
	if(SetCS_IO == NULL || GetInt_IO == NULL || ReadWrtieByte == NULL || Sleep == NULL)
	{
		DEBUG("[W5500]:无效的通讯接口，初始化失败！\r\n");
		return FALSE;
	}
	
	//初始化句柄与硬件接口
	pHandle->SetCS_IO = SetCS_IO;					//CS IO设置接口（1：高电平；0：低电平）
	pHandle->GetInt_IO = GetInt_IO;					//获取中断IO状态（高电平返回1，低电平返回0）
	pHandle->ReadWrtieByte = ReadWrtieByte;			//SPI读写接口（SPI读写一字节接口）
	pHandle->Sleep = Sleep;							//毫秒延时接口
	len = strlen(HostName);							//名称长度
	if(len > 16) len = 16;
	memcpy(pHandle->HostName, HostName, len);		//主机名称
	pHandle->HostName[len] = 0;
	pHandle->isEnablePing = isEnablePing;			//是否使能ping
	pHandle->isEnableWOL = isEnableWOL;				//是否使能网络唤醒
	memcpy(pHandle->MAC, MAC, 6);					//拷贝MAC地址
	
	//复位
	pHandle->SetCS_IO(1);							//保存片选默认高电平
	pHandle->Sleep(2);
	W5500_SoftwareReset(pHandle);					//软件复位
	pHandle->Sleep(2);
	
	for(i = 0;i < 8;i ++)
	{
		pHandle->SocketStatus[i] = SOCK_CLOSED;		//socket状态
		pHandle->SocketProtocol[i] = SOCKET_CLOSED;	//socket协议
	}
	
	return W5500_InitConfig(pHandle);				//配置设备
}



/*************************************************************************************************************************
* 函数			:	bool W5500_ConfigInit(W5500_HANDLE *pHandle, u8 MAC[6])
* 功能			:	配置W5500
* 参数			:	pHandle:W5500接口句柄；MAC:MAC地址,共6B
* 返回			:	TRUE:初始化成功；FALSE:初始化失败
* 依赖			:	SPI
* 作者			:	cp1300@139.com
* 时间			:	2019-01-14
* 最后修改时间	:	2019-01-14
* 说明			: 	会判断版本号是否正确，如果不正确意味着芯片没有正常工作或者通讯异常
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
			uart_printf("检测W5500失败,错误的版本号为：0x%02X,正在重试...\r\n",Version);
		}
		else break;
		pHandle->Sleep(100);
	}
	if(Version!=0x04)
	{
		uart_printf("检测W5500失败,错误的版本号为：0x%02X\r\n",Version);
		return FALSE;
	}
	
	W5500_SetPHY_OPMDC(pHandle, W5500_OPMDC_FULL_FUNC_AUTO);		//W5500 配置PHY-全功能，自动协商	
	W5500_SetMAC(pHandle, pHandle->MAC);							//设置MAC地址
	temp = 0;
	if(pHandle->isEnablePing)	//开启了ping
	{
		temp |= W5500_MODE_PING_BIT;
	}
	if(pHandle->isEnableWOL)	//开启了网络唤醒
	{
		temp |= W5500_MODE_WOL_BIT;
	}
	W5500_SetMode(pHandle, temp);									//设置ping与网络唤醒
	
	W5500_SetIntLevel(pHandle, 1);									//设置中断间隔
	W5500_SetRetryTime(pHandle, SOCKET_TIME_OUT);					//设置重传超时时间-200MS
	W5500_SetRetryCount(pHandle, SOCKET_RETRY_COUNT);				//W5500设置发送重试次数,RetryCount+1次，超出后会触发超时中断
	//初始化8个socket
	for(i = 0;i < 8;i ++)
	{
		W5500_SetOneSocketTOS(pHandle, (W5500_SOCKET_NUM) i,  0);							//设置一个socket的服务类型字段TOS(通常保持为0即可)
		W5500_SetOneSocketTTL(pHandle, (W5500_SOCKET_NUM) i,  SOCKET_TTL);					//设置一个socket的IP生存时间TTL
		W5500_SetOneSocketNotFrag(pHandle, (W5500_SOCKET_NUM) i);							//关闭一个socket分段（一般使用都建议不要分段）
		W5500_SetOneSocketRxBuffSize(pHandle, (W5500_SOCKET_NUM) i,  SOCKET_BUFF_SIZE_2KB);	//设置一个socket的接收缓冲区大小-2KB
		W5500_SetOneSocketTxBuffSize(pHandle, (W5500_SOCKET_NUM) i,  SOCKET_BUFF_SIZE_2KB);	//设置一个socket的发送缓冲区大小-2KB
	}
	W5500_SetSocketTotalInterruptEnable(pHandle, 0xFF);										//设置socket总中断状态-每一个bit代表一个socket是否开启了中断-开启所有socket总中断
	//uart_printf("socket总中断开关=%02X\r\n",W5500_GetSocketTotalInterruptEnable());
	
	return TRUE;
}



/*************************************************************************************************************************
* 函数			:	void W5500_DHCP_ip_assign(void)
* 功能			:	用于DHCP设置默认IP的接口
* 参数			:	无
* 返回			:	无
* 依赖			:	底层宏定义
* 作者			:	cp1300@139.com
* 时间			:	2019-01-11
* 最后修改时间	:	2019-01-11
* 说明			: 	用于DHCP时，初始化为默认ip，全部设置为0.0.0.0
*************************************************************************************************************************/
void W5500_DHCP_ip_assign(void)
{
	/*uart_printf("W5500_DHCP_ip_assign\r\n");
	W5500_SetLocalIP(0, 0, 0, 0);					//设置本地IP
	W5500_SetGatewayAddr(0, 0, 0, 0);				//设置网关IP
	W5500_SetMaskAddr(0, 0, 0, 0);					//设置子网掩码*/
}

