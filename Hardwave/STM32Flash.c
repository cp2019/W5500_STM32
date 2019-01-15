/*************************************************************************************************************
 * 文件名:		STM32Flash.c
 * 功能:		STM32 内部FLASH编程驱动函数
 * 作者:		cp1300@139.com
 * 创建时间:	2013-10-20
 * 最后修改时间:2013-10-20
 * 详细:		用于STM32内部flash读写驱动
				2016-01-07:自动判断flash大小
				2017-11-27：增加编程后返回状态，可以知道是否成功
				2018-01-13：STM32FLASH_Write_NoCheck增加解锁与上锁支持
*************************************************************************************************************/
#include "STM32Flash.h"
#include "delay.h"
#include "system.h"





/*************************************************************************************************************************
* 函数			:	void STM32FLASH_Unlock(void)
* 功能			:	解锁STM32的FLASH
* 参数			:	无
* 返回			:	无
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
void STM32FLASH_Unlock(void)
{
	FLASH->KEYR=FLASH_KEY1;	//写入解锁序列.
	FLASH->KEYR=FLASH_KEY2;
}



/*************************************************************************************************************************
* 函数			:	void STM32FLASH_Lock(void)
* 功能			:	上锁STM32的FLASH
* 参数			:	无
* 返回			:	无
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
void STM32FLASH_Lock(void)
{
	FLASH->CR|=1<<7;			//上锁
}





/*************************************************************************************************************************
* 函数			:	u8 STM32FLASH_GetStatus(void)
* 功能			:	得到FLASH状态
* 参数			:	无
* 返回			:	0:完成;1:忙;2:错误;3:写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
u8 STM32FLASH_GetStatus(void)
{	
	u32 res;	
	
	res=FLASH->SR; 
	if(res&(1<<0))return 1;		    //忙
	else if(res&(1<<2))return 2;	//编程错误
	else if(res&(1<<4))return 3;	//写保护错误
	return 0;						//操作完成
}



/*************************************************************************************************************************
* 函数			:	u8 STM32FLASH_WaitDone(u16 time)
* 功能			:	等待操作完成
* 参数			:	time:要延时的长短,单位us
* 返回			:	0:完成;1:忙;2:错误;3:写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
u8 STM32FLASH_WaitDone(u16 time)
{
	u8 res;
	
	do
	{
		res=STM32FLASH_GetStatus();
		if(res!=1)break;//非忙,无需等待了,直接退出.
		Delay_US(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}



/*************************************************************************************************************************
* 函数			:	u8 STM32FLASH_ErasePage(u32 paddr)
* 功能			:	擦除页
* 参数			:	paddr:页地址
* 返回			:	0:完成;1:忙;2:错误;3:写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
u8 STM32FLASH_ErasePage(u32 paddr)
{
	u8 res=0;
	
	res=STM32FLASH_WaitDone(0X5FFF);//等待上次操作结束,>20ms    
	if(res==0)
	{ 
		FLASH->CR|=1<<1;//页擦除
		FLASH->AR=paddr;//设置页地址 
		FLASH->CR|=1<<6;//开始擦除		  
		res=STM32FLASH_WaitDone(0X5FFF);//等待操作结束,>20ms  
		if(res!=1)//非忙
		{
			FLASH->CR&=~(1<<1);//清除页擦除标志.
		}
	}
	return res;
}




/*************************************************************************************************************************
* 函数			:	u8 STM32FLASH_WriteHalfWord(u32 faddr, u16 dat)
* 功能			:	在FLASH指定地址写入半字
* 参数			:	faddr:指定地址(此地址必须为2的倍数!!);dat:要写入的数据
* 返回			:	0:完成;1:忙;2:错误;3:写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
u8 STM32FLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	u8 res;	
			
	res=STM32FLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR|=1<<0;//编程使能
		*(vu16*)faddr=dat;//写入数据
		res=STM32FLASH_WaitDone(0XFF);//等待操作完成
		if(res!=1)//操作成功
		{
			FLASH->CR&=~(1<<0);//清除PG位.
		}
	} 
	return res;
} 



/*************************************************************************************************************************
* 函数			:	u16 STM32FLASH_ReadHalfWord(u32 faddr)
* 功能			:	读取指定地址的半字(16位数据) 
* 参数			:	faddr:指定地址(此地址必须为2的倍数!!);
* 返回			:	数据
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
u16 STM32FLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}





/*************************************************************************************************************************
* 函数			:	u8 STM32FLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)  
* 功能			:	不检查的写入
* 参数			:	WriteAddr:起始地址;pBuffer:数据指针;NumToWrite:半字(16位)数 
* 返回			:	状态
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	2017-11-27：增加返回值支持
					2018-01-13：STM32FLASH_Write_NoCheck增加解锁与上锁支持
*************************************************************************************************************************/
u8 STM32FLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	u8 res;
	
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 2;//非法地址
	STM32FLASH_Unlock();						//解锁
	
	for(i=0;i<NumToWrite;i++)
	{
		res = STM32FLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2.
	}
	STM32FLASH_Lock();//上锁
	
	return res;
} 


#if(STM32_FLASH_WREN)	//如果使能了写   
/*************************************************************************************************************************
* 函数			:	u8 STM32FLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
* 功能			:	从指定地址开始写入指定长度的数据
* 参数			:	WriteAddr:起始地址(此地址必须为2的倍数!!);pBuffer:数据指针;NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
* 返回			:	0:完成;1:忙;2:错误;3:写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2016-01-07
* 说明			: 	2016-01-07:自动判断flash大小
*************************************************************************************************************************/ 
u16 STM32FLASH_BUFF[2048/2];//最多是2K字节
u8 STM32FLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	   
 	u16 i;    
	u32 offaddr;   //去掉0X08000000后的地址
	u16 STM32_SECTOR_SIZE;
	u8 res = 0;
	
	if((*((u16*)0x1FFFF7E0)) < 256)	//小于256KB ,自动判断flash大小
	{
		STM32_SECTOR_SIZE = 1024;
	}
	else
	{
		STM32_SECTOR_SIZE = 2048;
	}
	
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return 2;//非法地址
	STM32FLASH_Unlock();						//解锁
	offaddr=WriteAddr-STM32_FLASH_BASE;			//实际偏移地址.
	secpos=offaddr/STM32_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff=(offaddr%STM32_SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain=STM32_SECTOR_SIZE/2-secoff;		//扇区剩余空间大小   
	if(NumToWrite<=secremain)secremain=NumToWrite;//不大于该扇区范围
	while(1) 
	{	
		STM32FLASH_Read(secpos*STM32_SECTOR_SIZE+STM32_FLASH_BASE,STM32FLASH_BUFF,STM32_SECTOR_SIZE/2);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(STM32FLASH_BUFF[secoff+i]!=0XFFFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			STM32FLASH_ErasePage(secpos*STM32_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
			for(i=0;i<secremain;i++)//复制
			{
				STM32FLASH_BUFF[i+secoff]=pBuffer[i];	  
			}
			res = STM32FLASH_Write_NoCheck(secpos*STM32_SECTOR_SIZE+STM32_FLASH_BASE,STM32FLASH_BUFF,STM32_SECTOR_SIZE/2);//写入整个扇区  
		}
		else 
		{
			res = STM32FLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间. 
		}				   
		if(NumToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
		   	pBuffer+=secremain;  	//指针偏移
			WriteAddr+=secremain;	//写地址偏移	   
		   	NumToWrite-=secremain;	//字节(16位)数递减
			if(NumToWrite>(STM32_SECTOR_SIZE/2))secremain=STM32_SECTOR_SIZE/2;//下一个扇区还是写不完
			else secremain=NumToWrite;//下一个扇区可以写完了
		}	 
	};	
	STM32FLASH_Lock();//上锁
	
	return res;
}
#endif	//STM32_FLASH_WREN




/*************************************************************************************************************************
* 函数			:	void STM32FLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)  	
* 功能			:	从指定地址开始读出指定长度的数据
* 参数			:	ReadAddr:起始地址;pBuffer:数据指针;NumToWrite:半字(16位)数
* 返回			:	0:完成;1:忙;2:错误;3:写保护
* 依赖			:	底层
* 作者			:	cp1300@139.com
* 时间			:	2013-10-20
* 最后修改时间 	: 	2013-10-20
* 说明			: 	
*************************************************************************************************************************/
void STM32FLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STM32FLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}


















