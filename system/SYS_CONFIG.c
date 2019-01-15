/*******************************************************************************
//�ļ���:	SYS_CONFIG.c
//����:	�洢,����ϵͳ�������
//����:	cp1300@139.com
//����ʱ��:	2013-05-05 07:43
//�޸�ʱ��:2013-05-05
//�޶�˵��:
//����:	��Ҫ�洢֧��	
********************************************************************************/
#include "system.h"
#include "SYS_CONFIG.h"
#include "ff.h"
//#include "SetPage.h"


#if TOUCH_CONFIG_
#include "touch.h"
#endif //TOUCH_CONFIG_



//�����ļ���
#define CONFIG_FILE_NAME	"0:/SysConf.sys"





#define SYS_CONFIG_BUFF_SIZE		sizeof(SYS_CONFIG)	//ϵͳ�����ļ���������С
static u8	SysConfigBuff[SYS_CONFIG_BUFF_SIZE];		//ϵͳ���û�����
SYS_CONFIG	*pSysConfig	 = (SYS_CONFIG *)SysConfigBuff;	//ϵͳ���ýṹָ��






static bool SYS_SaveConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize);	//�洢���������ļ����洢��
static bool SYS_LoadConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize);	//�Ӵ洢���������������ļ�




/*************************************************************************************************************************
* ����	:	bool SYS_SaveConfigFile(const char *ConfName, u8*pConfBuff, u32 ConfFileSize)
* ����	:	�洢�������ݵ��洢��
* ����	:	ConfName:�����ļ�����
			pConfBuff:�����ļ��洢������ָ��
			ConfFileSize:�����ļ���С
* ����	:	TRUE:�������óɹ�;FALSE:��������ʧ��
* ����	:	FATFS
* ����	:	cp1300@139.com
* ʱ��	:	2013-05-01
* ����޸�ʱ�� : 2013-05-01
* ˵��	: 	ʹ���ļ�ϵͳ�洢�����ļ���SD��
*************************************************************************************************************************/
static bool SYS_SaveConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize)
{
	FRESULT status;
	FIL file;
	UINT bw;
	bool sta;

	
	status = f_open(&file,(TCHAR *)ConfName, FA_WRITE | FA_CREATE_ALWAYS);		//����һ�������ļ�,����ļ���������и���
	if(status != FR_OK)															//���ļ�����
	{
		uart_printf("\r\nsys_config : create \"%s\" error!(%d)  ", ConfName, status);		//�����ļ�����
		sta = FALSE;	
	}
	else
	{
		status = f_write(&file, pConfBuff, ConfFileSize, &bw);
		if(status == FR_OK && bw == ConfFileSize)
		{
			uart_printf("sys_config : write \"%s\" ok!\r\n", ConfName);
			sta = TRUE;
		}
		else
		{
			uart_printf("sys_config : write \"%s\" error!(%d)\r\n", ConfName, status);	
			sta = FALSE;
		}
	}
	
	f_close(&file);	//�ر��ļ�
	
	return sta;
}




/*************************************************************************************************************************
* ����	:	bool SYS_LoadConfigFile(const char *ConfName, u8	*pConfBuff, u32 ConfFileSize)
* ����	:	�Ӵ洢���������������ļ�
* ����	:	ConfName:�����ļ�����
			pConfBuff:�����ļ��洢������ָ��
			ConfFileSize:�����ļ���С
* ����	:	TRUE:�������óɹ�;FALSE:��������ʧ��
* ����	:	FATFS
* ����	:	cp1300@139.com
* ʱ��	:	2013-05-01
* ����޸�ʱ�� : 2013-05-01
* ˵��	: 	�ļ�ϵͳ֧��
*************************************************************************************************************************/
static bool SYS_LoadConfigFile(const char *ConfName, u8 *pConfBuff, u32 ConfFileSize)
{
	FRESULT status;
	FIL file;
	UINT bw;
	bool sta;

	
	status = f_open(&file,(TCHAR *)ConfName, FA_READ | FA__ERROR);				//�������ļ�,����ļ��������򷵻ش���
	if(status != FR_OK)															//���ļ�����
	{
		uart_printf("\r\nopen \"%s\" error!(%d)  ", ConfName, status);			//���ļ�����
		sta = FALSE;	
	}
	else
	{
		ConfFileSize = (ConfFileSize > file.fsize) ? file.fsize : ConfFileSize;	//��ֹҪ��ȡ�����ݳ����ļ���С
		status = f_read(&file, pConfBuff, ConfFileSize, &bw);
		if(status == FR_OK && bw == ConfFileSize)
		{
			uart_printf("read \"%s\" ok!\r\n", ConfName);
			sta = TRUE;
		}
		else
		{
			uart_printf("read \"%s\" error!(%d)\r\n", ConfName, status);	
			sta = FALSE;
		}
	}
	f_close(&file);	//�ر��ļ�
	
	return sta;
}





/*************************************************************************************************************************
* ����	:	bool SYS_SaveConfig(const char *ConfName, u8*pConfBuff, u32 ConfFileSize)
* ����	:	�洢��������
* ����	:	ConfName:�����ļ�����
			pConfBuff:�����ļ��洢������ָ��
			ConfFileSize:�����ļ���С
* ����	:	TRUE:�������óɹ�;FALSE:��������ʧ��
* ����	:	FATFS
* ����	:	cp1300@139.com
* ʱ��	:	2013-05-05
* ����޸�ʱ�� : 2013-05-05
* ˵��	: 	��
*************************************************************************************************************************/
void SYS_SaveConfig(void)
{
	uart_printf("�����ļ���С:%dB", SYS_CONFIG_BUFF_SIZE);
	
		//�洢����������		
#if TOUCH_CONFIG_
		pSysConfig->xfac = Pen_Point.xfac; 
		pSysConfig->yfac = Pen_Point.yfac;
		pSysConfig->xoff = Pen_Point.xoff;
		pSysConfig->yoff = Pen_Point.yoff;
#endif //TOUCH_CONFIG_
	
	
	if(SYS_SaveConfigFile(CONFIG_FILE_NAME, SysConfigBuff, SYS_CONFIG_BUFF_SIZE) == TRUE)
	{
		uart_printf("�洢���óɹ�!\r\n");
	}
	else
	{
		uart_printf("�洢����ʧ��!\r\n");
	}
}



/*************************************************************************************************************************
* ����	:	bool SYS_SaveConfig(const char *ConfName, u8*pConfBuff, u32 ConfFileSize)
* ����	:	������������
* ����	:	ConfName:�����ļ�����
			pConfBuff:�����ļ��洢������ָ��
			ConfFileSize:�����ļ���С
* ����	:	TRUE:�������óɹ�;FALSE:��������ʧ��
* ����	:	FATFS
* ����	:	cp1300@139.com
* ʱ��	:	2013-05-05
* ����޸�ʱ�� : 2013-05-05
* ˵��	: 	��
*************************************************************************************************************************/
void SYS_LoadConfig(void)
{
	u32 i;
	
	uart_printf("�����ļ���С:%dB", SYS_CONFIG_BUFF_SIZE);
	if(SYS_LoadConfigFile(CONFIG_FILE_NAME, SysConfigBuff, SYS_CONFIG_BUFF_SIZE) == TRUE)
	{
		uart_printf("�������óɹ�!\r\n");	
	}
	else
	{
		uart_printf("��������ʧ��!\r\n");
		for(i = 0;i < SYS_CONFIG_BUFF_SIZE;i ++)
		{
			SysConfigBuff[i] = 0;				//��������ʧ��,��ȫ���������
		}
	}
	
	//���ش���������		
#if TOUCH_CONFIG_
		Pen_Point.xfac = pSysConfig->xfac;
		Pen_Point.yfac = pSysConfig->yfac;
		Pen_Point.xoff = pSysConfig->xoff;
		Pen_Point.yoff = pSysConfig->yoff;
#endif //TOUCH_CONFIG_
		

}





