#include "SYSTEM.H"
#include "DELAY.H"




							    

/********************************************************************************************************/
/*	MS������ʱ	 
//SYSCLK��λΪHz,nms��λΪms
//��72M������,nms<=1864 
void Delay_MS(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL =0x00;           //��ռ�����
	SysTick->CTRL=0x01 ;          //��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;       //�رռ�����				  
	SysTick->VAL =0X00;       //��ռ�����	  	    
}   
 												  */

/********************************************************************************************************/
/*	US������ʱ	    								   
void Delay_US(u16 nus)
{		
	u32 temp;	    	 								  
	SysTick->LOAD=nus*fac_us; //ʱ�����	  		 
	SysTick->VAL=0x00;        //��ռ�����
	SysTick->CTRL=0x01 ;      //��ʼ���� 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��   			 
	SysTick->CTRL=0x00;       //�رռ�����
	SysTick->VAL =0X00;       //��ռ�����	 
}
																  	 */	
 /******************************************************************************************************************************************/
/*	�����ʱ	�������ж�����ʹ��*/
//��ʱʱ��1 - 6553 US  ���0.5US
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
/*	�����ʱ	�������ж�����ʹ��*/
//��ʱʱ��1 - 6553 MS  ���10US
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


//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;	//LOAD��ֵ	    	 
	ticks=SYSTEM_GetClkSpeed()/8/1000/1000*nus; 			//��Ҫ�Ľ�����	  		 
	tcnt=0;
	//OSSchedLock();				//��ֹucos���ȣ���ֹ���us��ʱ
	told=SysTick->VAL;        		//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
	//OSSchedUnlock();			//����ucos���� 									    
}
