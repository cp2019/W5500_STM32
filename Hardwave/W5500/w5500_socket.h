/*************************************************************************************************************
 * �ļ���:			w5500_socket.h
 * ����:			socket�ӿں���
 * ����:			cp1300@139.com
 * ����ʱ��:		2016-02-22
 * ����޸�ʱ��:	2019-01-14
 * ��ϸ:			TCP/UDPЭ��ջ,�����OS��ʹ�ã���Ҫϵͳ��ʱ�Լ��ź���֧��
*************************************************************************************************************/
#ifndef _W5500_SOCKET_H_
#define _W5500_SOCKET_H_
#include "system.h"
#include "W5500.h"

//��ʱ
#ifdef _UCOS_II_//ʹ���˲���ϵͳ
#include "ucos_ii.h"
#define socket_sleep(x) 	OSTimeDlyHMSM(0,0,x/1000,x%1000)
#else
#include "delay.h"
#define socket_sleep(x) 	Delay_MS(x)
#endif //_UCOS_II_






//ͳһ��ַ�ṹ
typedef struct 
{
	u8 addr[4];
	u16 port;
}sockaddr,SOCKADDR_IN,socketaddr;




//SOCKETͨ������
#define SOCKET_CH_NUM	8	//���ͨ������8
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


//ͨ��Э�鶨��
typedef enum
{
	IPPROTO_TCP			=		0,		//TCPЭ��
	IPPROTO_UDP			=		1,		//UDP����
	IPPROTO_UDP_BROAD	=		2,		//UDP�㲥-����ǹ㲥���ͣ���Ҫ��IP����Ϊ255
}IPPROTO;

//SOCKET opt���õ�ֵ
#define SO_RCVTIMEO		1		//�������ݳ�ʱʱ�䣬��λms,��С10ms
#define SO_SNDTIMEO		2		//�������ݳ�ʱʱ�䣬��λms,��С10ms
#define SO_CONNTIMEO	3		//���ӳ�ʱʱ�䣬��λms,��С10ms

//SOCKET cmd���õ�ֵ
#define SOL_FIONREAD	1		//��ȡ���ջ����������ݳ���
#define SOL_SOCKET		2		//��ȡһ������״̬�Ƿ���Ч-��Ч����1����Ч����0
#define SOL_IPPROTO		3		//��ȡ��ǰsocketЭ������

//����״̬
#define INVALID_SOCKET			-1		//��Ч




//ϵͳ�������ʼ����ؽӿ�
int WSAStartup(void);														//Э��ջӲ����ʼ��
void netsh_ip_addr(u8 ip[4]);												//���ñ���ip��ַ
void netsh_ip_mask(u8 ip[4]);												//���ñ�����������
void netsh_ip_gateway(u8 ip[4]);											//���ñ������ص�ַ
bool InternetGetConnectedState(void);										//��ȡ��������״̬
const char *socket_GetError(void);											//��ȡϵͳ����״̬

//ͨ��socket�ӿ�
SOCKET socket(IPPROTO protocol);											//��ʼ��socket
int connect(SOCKET socket, const sockaddr* address);						//���ӵ�ָ��������
int bind(SOCKET socket, const sockaddr* address);							//��
int recv(SOCKET socket, u8* buff, u32 buffsize);							//��������
int recvfrom(SOCKET socket, u8* buff, u32 bufflen, socketaddr* from);		//��������-���ضԷ���ַ
int sendto(SOCKET socket, const u8* buff, u32 size, const sockaddr *to);	//�������ݵ�ָ����ַ
int send(SOCKET socket, const u8* buff, u32 size);							//��������
void closesocket(SOCKET socket);											//�ر�һ��socket
void setsockopt(SOCKET socket, int optname, u32 opt);						//����socket��ѡ��
int getsockopt(SOCKET socket, int cmd);										//��ȡsocket��ز��� cmd=SOL_FIONREAD:��ȡ�������ݳ���

//�����ӿ�-����DHCPʱʹ��
void getLocalMAC(u8 mac[6]);												//��ȡ����MAC��ַ
void getLocalHostName(char HostName[16+1], u8 BuffSize);					//��ȡ��������


#endif /*_W5500_SOCKET_H_*/

