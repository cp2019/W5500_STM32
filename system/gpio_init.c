#include "GPIO_INIT.H"
#include "SYSTEM.H"


/*************************************************************************************************************************
* 函数	:	void GPIOx_Init(GPIO_TypeDef *GPIOx,vu16 BIT_N,vu8 IO_MODE,vu8 SPEED_MODE)
* 功能	:	IO初始化函数
* 参数	:	*GPIOx : 对应IO的基址指针(如GPIOA),BIT_N : 对应的IO位,IO_MODE : IO模式,SPEED_MODE : IO速度
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111105
* 最后修改时间 : 20111105
* 说明	: 	一次初始化多个IO
*************************************************************************************************************************/
void GPIOx_Init(GPIO_TypeDef *GPIOx,vu16 BIT_N,vu8 IO_MODE,vu8 SPEED_MODE)
{
 	u8 i;

	for(i = 0;i < 16;i ++)
	{
	 	if(BIT_N & 0x1)
		{
		 	if(i < 8)
			{
				GPIOx->CRL &= ~(0xf << 4 * i);
				GPIOx->CRL |= (IO_MODE & 0x3) << 4 * i + 2;
				if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //如果是输入模式,MODE = 00
					GPIOx->CRL |= SPEED_MODE << i * 4;
			}
			else
			{
				GPIOx->CRH &= ~(0xf << 4 * (i - 8));
				GPIOx->CRH |= (IO_MODE & 0x3) << 4 * (i - 8) + 2;
				if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //如果是输入模式,MODE = 00
					GPIOx->CRH |= SPEED_MODE << (i - 8) * 4;
			}
			if(IO_MODE == IN_IPT)
				GPIOx->ODR &= ~(1 << i);  //下拉输入
			else if(IO_MODE == IN_IPU)
				GPIOx->ODR |= (1 << i);  //上拉输入
		}
		BIT_N >>= 1;
	}	
}




/*************************************************************************************************************************
* 函数	:	void GPIOx_OneInit(GPIO_TypeDef *GPIOx,vu16 i,vu8 IO_MODE,vu8 SPEED_MODE)
* 功能	:	单IO初始化函数
* 参数	:	*GPIOx : 对应IO的基址指针(如GPIOA),i : 对应的IO位,IO_MODE : IO模式,SPEED_MODE : IO速度
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	20111105
* 最后修改时间 : 20111105
* 说明	: 	一次初始化多个IO
*************************************************************************************************************************/
void GPIOx_OneInit(GPIO_TypeDef *GPIOx,vu16 i,vu8 IO_MODE,vu8 SPEED_MODE)
{
	if(i > 15) return;

	if(i < 8)
	{
		GPIOx->CRL &= ~(0xf << 4 * i);
		GPIOx->CRL |= (IO_MODE & 0x3) << 4 * i + 2;
		if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //如果是输入模式,MODE = 00
			GPIOx->CRL |= SPEED_MODE << i * 4;
	}
	else
	{
		GPIOx->CRH &= ~(0xf << 4 * (i - 8));
		GPIOx->CRH |= (IO_MODE & 0x3) << 4 * (i - 8) + 2;
		if(((IO_MODE != IN_AIN) && (IO_MODE != IN_FLOATING) && (IO_MODE != IN_IPT) && (IO_MODE != IN_IPU))) //如果是输入模式,MODE = 00
			GPIOx->CRH |= SPEED_MODE << (i - 8) * 4;
	}
	if(IO_MODE == IN_IPT)
		GPIOx->ODR &= ~(1 << i);  //下拉输入
	else if(IO_MODE == IN_IPU)
		GPIOx->ODR |= (1 << i);  //上拉输入
}





/*************************************************************************************************************************
* 函数	:	void GPIOx_PD01_Init(void)
* 功能	:	初始化PD0,PD1为普通IO
* 参数	:	无
* 返回	:	无
* 依赖	:	底层宏定义
* 作者	:	cp1300@139.com
* 时间	:	2014-08-19
* 最后修改时间 : 2014-08-19
* 说明	: 	需要开启PD0 PD1重映射
*************************************************************************************************************************/
void GPIOx_PD01_Init(void)
{
	RCC->APB2ENR |= 1 << 0;    //开启辅助时钟
	
	AFIO->MAPR &= ~BIT15;
	AFIO->MAPR |= BIT15;	//PD0 PD1映射为普通IO
	DeviceClockEnable(DEV_AFIO, ENABLE);
	DeviceClockEnable(DEV_GPIOD, ENABLE);
}


