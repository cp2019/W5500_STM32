#include "SYSTEM.H"
#include "DELAY.H"




							    

/********************************************************************************************************/
/*	MS级别延时	 
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864 
void Delay_MS(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL=0x01 ;          //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器				  
	SysTick->VAL =0X00;       //清空计数器	  	    
}   
 												  */

/********************************************************************************************************/
/*	US级别延时	    								   
void Delay_US(u16 nus)
{		
	u32 temp;	    	 								  
	SysTick->LOAD=nus*fac_us; //时间加载	  		 
	SysTick->VAL=0x00;        //清空计数器
	SysTick->CTRL=0x01 ;      //开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   			 
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}
																  	 */	
 /******************************************************************************************************************************************/
/*	软件延时	可以在中断里面使用*/
//延时时间1 - 6553 US  误差0.5US
void Delay_US(vu16 time)
{	
	u8 i;
	while(time --)
	{
	 	for(i = 0;i < 5;i ++)
	 		nop;
	}
}

/******************************************************************************************************************************************/
/*	软件延时	可以在中断里面使用*/
//延时时间1 - 6553 MS  误差10US
void Delay_MS(vu16 time)
{
	u16 i;
	while(time --)
	{
	 	for(i = 0;i < 5998;i ++)
	 	{
		 	nop;
		}
	}
}


//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;	//LOAD的值	    	 
	ticks=SYSTEM_GetClkSpeed()/8/1000/1000*nus; 			//需要的节拍数	  		 
	tcnt=0;
	//OSSchedLock();				//阻止ucos调度，防止打断us延时
	told=SysTick->VAL;        		//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;//时间超过/等于要延迟的时间,则退出.
		}  
	};
	//OSSchedUnlock();			//开启ucos调度 									    
}
