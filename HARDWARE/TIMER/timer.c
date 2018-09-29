#include "bsp.h"
#include "user_config.h"
#include "display.h"
#include "can.h"
#include "bmpdisplay.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F7开发板
//定时器中断驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/11/27
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

TIM_HandleTypeDef TIM3_Handler;      //定时器句柄 
TIM_HandleTypeDef TIM6_Handler;      //定时器6句柄 

uint32_t KMeter;
uint32_t minute ;

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!(定时器3挂在APB1上，时钟为HCLK/2)
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //通用定时器3
    TIM3_Handler.Init.Prescaler=psc;                     //分频
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM3_Handler.Init.Period=arr;                        //自动装载值
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE    
}

//通用定时器6初始化
//arr：自动重装值。
//psc：时钟预分频数。
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器6!(定时器6挂在APB1上，时钟为HCLK/2)
void TIM6_Init(u16 arr,u16 psc)
{  
    TIM6_Handler.Instance=TIM6;                          //通用定时器6
    TIM6_Handler.Init.Prescaler=psc;                     //分频
    TIM6_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM6_Handler.Init.Period=arr;                        //自动装载值
    TIM6_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&TIM6_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM6_Handler); //使能定时器6和定时器6中断   
}

//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    __HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
    __HAL_RCC_TIM6_CLK_ENABLE();            //使能TIM6时钟
    
    HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
    HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断 

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn,0,3);//设置中断优先级，抢占优先级0，子优先级3
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);      //开启ITM6中断 
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

//定时器6中断服务函数
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM6_Handler);
}

//定义定时中断全局变量
vu8 frameup; 
U16 time3_counter1=0;
U16 time3_counter2=0;
U16 time3_counter3=0;
U16 time3_counter4=0;
U16 time6_counter1=0;
U16 time6_counter2=0;

//add by mch 20180609
U16 time3_run_counter=0;
U16 time3_fan_counter=0;
//add by mch 20180824
U16 time3_sweeper_counter=0;
U16 time3_sweeper_counter1=0;
U16 time3_sweeper_counter2=0;
U16 time3_sweeper_counter3=0;
U16 time3_sweeper_counter4=0;

//室温相关全局变量
U16 tempCounter=0;
U32 tempSum=0;
U16 tempCounter_real=0;
//AD采样相关全局变量
U16 ADcounter=0;
U32  ad1Sum=0;
U32  ad2Sum=0;
U32  ad5Sum=0;
U32  ad6Sum=0;

U8 flag_shortpress = 0;
U8 flag_longpress = 0;
U8 flag_timing = 0;

P_DISIO_IN_Calc p_disio_in_calc;
//获取左转向灯的状态
void GetP_LeftLight(void)
{
	U16 temp;
	temp=P_LeftLight;  //获取左转向对应的IO口的电平
	if(temp==p_disio_in_calc.P_LeftLight_Value)   //P_LeftLight_Value没有初始化，可以这样？
	{
		p_disio_in_calc.P_LeftLight_Counter++;
		if(p_disio_in_calc.P_LeftLight_Counter>=5)//连续5次50ms采集到相同的值才认为是有效值
		{
			ci_parameter.LeftLight=p_disio_in_calc.P_LeftLight_Value;
			Show_Light(ci_parameter.LeftLight, ci_parameter_old.LeftLight, ZUOZHUAN, 1, 10, 15);                //左转
			p_disio_in_calc.P_LeftLight_Counter=0;
		}
	}
	else
	{
		p_disio_in_calc.P_LeftLight_Value=temp;
		p_disio_in_calc.P_LeftLight_Counter=0;
	}
	
}
//获取右转向灯的状态
void GetP_RightLight(void)
{
	U16 temp;
	temp=P_RightLight;
	if(temp==p_disio_in_calc.P_RightLight_Value)
		{
		p_disio_in_calc.P_RightLight_Counter++;
		if(p_disio_in_calc.P_RightLight_Counter>=5)//连续5次50ms采集到相同的值才认为是有效值
			{
			ci_parameter.RightLight=p_disio_in_calc.P_RightLight_Value;
			Show_Light(ci_parameter.RightLight, ci_parameter_old.RightLight, YOUZHUAN, 1, 591, 15);             //右转
			p_disio_in_calc.P_RightLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_RightLight_Value=temp;
		p_disio_in_calc.P_RightLight_Counter=0;
		}
	
}
//获取前向灯的状态
void GetP_ForwardLight(void)
{
	U16 temp;
	temp=P_ForwardLight;
	if(temp==p_disio_in_calc.P_ForwardLight_Value)
		{
		p_disio_in_calc.P_ForwardLight_Counter++;
		if(p_disio_in_calc.P_ForwardLight_Counter>=5)//连续5次50ms采集到相同的值才认为是有效值
			{
			ci_parameter.ForwardLight=p_disio_in_calc.P_ForwardLight_Value;
			//ShowForwardLight();
			p_disio_in_calc.P_ForwardLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_ForwardLight_Value=temp;
		p_disio_in_calc.P_ForwardLight_Counter=0;
		}
	
}
//获取后退灯的状态
void GetP_BackLight(void)
{
	U16 temp;
	temp=P_BackLight;
	if(temp==p_disio_in_calc.P_BackLight_Value)
		{
		p_disio_in_calc.P_BackLight_Counter++;
		if(p_disio_in_calc.P_BackLight_Counter>=5)//连续5次50ms采集到相同的值才认为是有效值
			{
			ci_parameter.BackWardLight=p_disio_in_calc.P_BackLight_Value;
			//ShowBackwardLight();
			p_disio_in_calc.P_BackLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_BackLight_Value=temp;
		p_disio_in_calc.P_BackLight_Counter=0;
		}
	
}

void GetP_PreHeatLight(void)
{
//	U16 temp;
//	temp=P_PreHeatLight;
//	if(temp==p_disio_in_calc.P_PreHeatLight_Value)
//		{
//		p_disio_in_calc.P_PreHeatLight_Counter++;
//		if(p_disio_in_calc.P_PreHeatLight_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
//			{
//			ci_parameter.PreHeatLight=p_disio_in_calc.P_PreHeatLight_Value;
//			p_disio_in_calc.P_PreHeatLight_Counter=0;
//			}
//		}
//	else
//		{
//		p_disio_in_calc.P_PreHeatLight_Value=temp;
//		p_disio_in_calc.P_PreHeatLight_Counter=0;
//		}
	
}
//获取手刹的状态
void GetP_HandBrakeLight(void)
{
	U16 temp;
	temp=P_HandBrakeLight;
	if(temp==p_disio_in_calc.P_HandBrakeLight_Value)
		{
		p_disio_in_calc.P_HandBrakeLight_Counter++;
		if(p_disio_in_calc.P_HandBrakeLight_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
			{
			ci_parameter.HandBrakeLight=p_disio_in_calc.P_HandBrakeLight_Value;
			//ShowHandBrakeLight();
			p_disio_in_calc.P_HandBrakeLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_HandBrakeLight_Value=temp;
		p_disio_in_calc.P_HandBrakeLight_Counter=0;
		}
} 
//获取脚刹的状态
void GetP_FootBrakeLight(void)
{
	U16 temp;
	temp=P_FootBrakeLight;
	if(temp==p_disio_in_calc.P_FootBrakeLight_Value)
		{
		p_disio_in_calc.P_FootBrakeLight_Counter++;
		if(p_disio_in_calc.P_FootBrakeLight_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
			{
			ci_parameter.FootBrakeLight=p_disio_in_calc.P_FootBrakeLight_Value;
			//ShowFootBrakeLight();
			p_disio_in_calc.P_FootBrakeLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_FootBrakeLight_Value=temp;
		p_disio_in_calc.P_FootBrakeLight_Counter=0;
		}
}
//获取近光灯的状态
void GetP_DippedLight(void)
{
	U16 temp;
	temp=P_DippedLight;
	if(temp==p_disio_in_calc.P_DippedLight_Value)
		{
		p_disio_in_calc.P_DippedLight_Counter++;
		if(p_disio_in_calc.P_DippedLight_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
			{
			ci_parameter.DippedLight=p_disio_in_calc.P_DippedLight_Value;
			//ShowDippedLight();
			p_disio_in_calc.P_DippedLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_DippedLight_Value=temp;
		p_disio_in_calc.P_DippedLight_Counter=0;
		}
}	

void GetP_CellWarning(void)
{
//	U16 temp;
//	temp=P_CellWarning;
//	if(temp==p_disio_in_calc.P_CellWarning_Value)
//		{
//		p_disio_in_calc.P_CellWarning_Counter++;
//		if(p_disio_in_calc.P_CellWarning_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
//			{
//			ci_parameter.CellWarning=p_disio_in_calc.P_CellWarning_Value;
//			p_disio_in_calc.P_CellWarning_Counter=0;
//			}
//		}
//	else
//		{
//		p_disio_in_calc.P_CellWarning_Value=temp;
//		p_disio_in_calc.P_CellWarning_Counter=0;
//		}
}	

void GetP_EngineOilLight(void)
{
//	U16 temp;
//	temp=P_EngineOilLight;
//	if(temp==p_disio_in_calc.P_EngineOilLight_Value)
//		{
//		p_disio_in_calc.P_EngineOilLight_Counter++;
//		if(p_disio_in_calc.P_EngineOilLight_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
//			{
//			ci_parameter.EngineOilLight=p_disio_in_calc.P_EngineOilLight_Value;
//			p_disio_in_calc.P_EngineOilLight_Counter=0;
//			}
//		}
//	else
//		{
//		p_disio_in_calc.P_EngineOilLight_Value=temp;
//		p_disio_in_calc.P_EngineOilLight_Counter=0;
//		}
}

void GetP_FilterWarning(void)
{
//	U16 temp;
//	temp=P_FilterWarning;
//	if(temp==p_disio_in_calc.P_FilterWarning_Value)
//		{
//		p_disio_in_calc.P_FilterWarning_Counter++;
//		if(p_disio_in_calc.P_FilterWarning_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
//			{
//			ci_parameter.FilterWarning=p_disio_in_calc.P_FilterWarning_Value;
//			p_disio_in_calc.P_FilterWarning_Counter=0;
//			}
//		}
//	else
//		{
//		p_disio_in_calc.P_FilterWarning_Value=temp;
//		p_disio_in_calc.P_FilterWarning_Counter=0;
//		}
}

void GetP_ChargingLight(void)
{
	U16 temp;
	temp=P_ChargingLight;
	if(temp==p_disio_in_calc.P_ChargingLight_Value)
		{
		p_disio_in_calc.P_ChargingLight_Counter++;
		if(p_disio_in_calc.P_ChargingLight_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
			{
			ci_parameter.ChargingLight=!p_disio_in_calc.P_ChargingLight_Value;
			p_disio_in_calc.P_ChargingLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_ChargingLight_Value=temp;
		p_disio_in_calc.P_ChargingLight_Counter=0;
		}
}	

//获取低水位告警状态
void GetP_LowWaterWarning(void)
{
	U16 temp;
	temp=P_LowWaterWarning;
	if(temp==p_disio_in_calc.P_LowWaterWarning_Value)
		{
		p_disio_in_calc.P_LowWaterWarning_Counter++;
		if(p_disio_in_calc.P_LowWaterWarning_Counter>=10)//连续10次，100ms采集到相同的值才认为是有效值
			{
			ci_parameter.LowWaterWarning=p_disio_in_calc.P_LowWaterWarning_Value;
			//ShowLowWaterWarning();
			p_disio_in_calc.P_LowWaterWarning_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_LowWaterWarning_Value=temp;
		p_disio_in_calc.P_LowWaterWarning_Counter=0;
		}
}

//获取按键2的状态，切换界面
void GetP_UserKey2Debug(void)
{
	U16 temp;
	temp=P_UserKey2Debug;
	if(temp==p_disio_in_calc.P_UserKey2Debug_Value)
		{
		p_disio_in_calc.P_UserKey2Debug_Counter++;
		if(p_disio_in_calc.P_UserKey2Debug_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
			{
			ci_parameter.UserKey2Debug=p_disio_in_calc.P_UserKey2Debug_Value;
			p_disio_in_calc.P_UserKey2Debug_Counter=0;
			if(temp==0)
			{
				p_disio_in_calc.P_ODOKey2_Counter++;	//100ms加1			
			}
			}
		}
	else
		{
		p_disio_in_calc.P_UserKey2Debug_Value=temp;
		p_disio_in_calc.P_UserKey2Debug_Counter=0;
		p_disio_in_calc.P_ODOKey2_Counter=0;
		}
}

//获取按键1的状态
void GetP_UserKey1(void)
{
	U16 temp;
	temp=P_UserKey1;
	if(temp==p_disio_in_calc.P_UserKey1_Value)
		{
		p_disio_in_calc.P_UserKey1_Counter++;
		if(p_disio_in_calc.P_UserKey1_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
			{
			ci_parameter.UserKey1=p_disio_in_calc.P_UserKey1_Value;
			p_disio_in_calc.P_UserKey1_Counter=0;
			if(temp==0)
				{
					p_disio_in_calc.P_ODOReset_Counter++;	//100ms加1			
				}
			
			}
		}
	else
		{
		p_disio_in_calc.P_UserKey1_Value=temp;
		p_disio_in_calc.P_UserKey1_Counter=0;
		p_disio_in_calc.P_ODOReset_Counter=0;
		}
}

//获取按键3的值
void GetP_UserKey3(void)
{
	U16 temp;
	temp=P_UserKey3;
	
	if(temp == 0 && flag_longpress == 0)
	{
		flag_shortpress = 1;
	}

	if(temp==p_disio_in_calc.P_UserKey3_Value)
		{
		p_disio_in_calc.P_UserKey3_Counter++;
		if(p_disio_in_calc.P_UserKey3_Counter>=10)//连续10次100ms采集到相同的值才认为是有效值
			{
			ci_parameter.UserKey3=p_disio_in_calc.P_UserKey3_Value;
			p_disio_in_calc.P_UserKey3_Counter=0;
			if(temp==0)
				{
					p_disio_in_calc.P_ODOKey3_Counter++;	//100ms加1		
					flag_longpress = 1;
					
				}
			
			}
		}
	else
	{
		p_disio_in_calc.P_UserKey3_Value=temp;
		p_disio_in_calc.P_UserKey3_Counter=0;
		p_disio_in_calc.P_ODOKey3_Counter=0;
		flag_key3 = 0;
		flag_longpress = 0;
	}
}

U16 ESpeedCounter=0;
U16 getESpeedPulse=0;
//获取扫刷速度
void GetP_SweeperSpeed(void)
{
	U16 temp;
	time6_counter2++;
	temp=P_SweeperSpeed;
	if(temp==p_disio_in_calc.P_SweeperSpeed_Value)
		{
		p_disio_in_calc.P_SweeperSpeed_Counter++;
		if(p_disio_in_calc.P_SweeperSpeed_Counter>=5)//连续4次0.5ms采集到相同的值才认为是有效值
			{
			//ci_parameter.EngineSpeed=p_disio_in_calc.P_EngineSpeed_Value;
			p_disio_in_calc.P_SweeperSpeed_Counter=0;
			if(temp==1)
				{
				if(getESpeedPulse==0)
					{
					ESpeedCounter++;
					getESpeedPulse=1;
					}
				}
			}
		}
	else
	{
		p_disio_in_calc.P_SweeperSpeed_Value=temp;
		p_disio_in_calc.P_SweeperSpeed_Counter=0;
		getESpeedPulse=0;
	}
	if(time6_counter2>=20000)// 每2秒算一次
	{		
		ci_parameter.ESpeedPulseCounter=ESpeedCounter;	
		ci_parameter.SweeperSpeed=ESpeedCounter*30/120;//单位r/min be 发电机转速参数，发电机每转一圈输出6个脉冲信号，发电机转速和发动机比为65:110。
		ESpeedCounter=0;
		time6_counter2=0;
	}
}

U16 SpeedCounter=0;
U16 getSpeedPulse=0;
//获取行走速度
void GetP_Speed(void)
{
	U16 temp;

	time6_counter1++;   													//每0.1ms计数一次
	temp=P_Speed;
	if(temp==p_disio_in_calc.P_Speed_Value)
	{
		p_disio_in_calc.P_Speed_Counter++;
		if(p_disio_in_calc.P_Speed_Counter>=5)			//连续10次1ms采集到相同的值才认为是有效值
		{
			//ci_parameter.Speed=p_disio_in_calc.P_Speed_Value;
			p_disio_in_calc.P_Speed_Counter=0;
			if(temp==1)
			{
				if(getSpeedPulse==0)
				{
					SpeedCounter++;
					ci_parameter.OdoMeterCounter++;				//里程计数

					if(ci_parameter.OdoMeterCounter>=8530)          //100m
					{
						ci_parameter.OdoMeterCounter=0;
						ci_parameter.OdoMeter =ci_parameter.OdoMeter+100;
						AT24CXX_WriteLenByte(4,ci_parameter.OdoMeter,4);	
					}		
					
					getSpeedPulse=1;
				}
			}
		}
	}
	else
	{
		p_disio_in_calc.P_Speed_Value=temp;
		p_disio_in_calc.P_Speed_Counter=0;
		getSpeedPulse=0;
	}
	if(time6_counter1>=20000)// 每2秒算一次
	{		
		ci_parameter.SpeedPusleCounter=SpeedCounter;	
		ci_parameter.Speed=(SpeedCounter*3.14*0.00054)*30*60/8/18.12;//单位km/h 
					
		SpeedCounter=0;
		time6_counter1=0;
	}
}

//获取室温
u8 flagroomtemp = 0;
u32 roomtemp_old = 0;
u32 RoomTemp_Counter = 0;
void GetRoomTemp(void)
{
	short temp;
	
	if(ds18b20_init_err==0)//ds18b20_init OK
	{
		tempCounter++;
		temp=DS18B20_Get_Temp();	
		if(temp<500 && temp>-200)
			{
			tempSum+=temp;
			tempCounter_real++;
			}
		if(tempCounter>=100)
			{
			tempCounter=0;
			ci_parameter.RoomTemp=(U32)tempSum/tempCounter_real;
			tempCounter_real=0;		
			tempSum=0;
			}							
	}
	
/*	temp=DS18B20_Get_Temp();
	if(flagroomtemp == 0)
	{
		RoomTemp_Counter++;
		ci_parameter.RoomTemp=temp;
		if(RoomTemp_Counter>=500)//连续10次，100ms采集到相同的值才认为是有效值
		{
			flagroomtemp = 1;
			roomtemp_old = temp;
		}
		
	}
	else
	{
		tempCounter++;	
		if(fabs(temp - roomtemp_old) < 50)
		{
			tempSum+=temp;
			tempCounter_real++;
			roomtemp_old = temp;
		}
		if(tempCounter>=100)
		{
			tempCounter=0;
			ci_parameter.RoomTemp=(U32)tempSum/tempCounter_real;
			tempCounter_real=0;		
			tempSum=0;
		}							
	}	*/
		
		
}

//获取AD值,1S更新一次
//double temp1,temp2;
void 	GetADValue(void)
{
		char *str;
		U16 adtemp;
		U32 ad1val;
//		U32 ad2val;
//		U32 ad5val;
		U32 ad6val;	

		ADcounter++;
		adtemp=Get_Adc(ADC_CHANNEL_4);
		ad1Sum+=adtemp;
//		adtemp=Get_Adc(ADC_CHANNEL_5);
//		ad2Sum+=adtemp;
//		adtemp=Get_Adc(ADC_CHANNEL_14);
//		ad5Sum+=adtemp;
		adtemp=Get_Adc(ADC_CHANNEL_15);
		ad6Sum+=adtemp;	
	
	//采集100个数据，分别求平均
	if(ADcounter>=100)
	{					
		ad1val=ad1Sum/ADcounter;
//		ad2val=ad2Sum/ADcounter;
//		ad5val=ad5Sum/ADcounter;
		ad6val=ad6Sum/ADcounter;
		ADcounter=0;
		ad1Sum=0;
//		ad2Sum=0;
//		ad5Sum=0;
		ad6Sum=0;
		
		//根据硬件电路和参数指标进行AD转换
		ci_parameter.WorkVoltage=(float)ad1val*(3.3/4096)*(1+12.7);                  //工作电压
//		ci_parameter.CellVoltage=(float)ad2val*(3.3/4096)*31;	                       //电池电压
//		ci_parameter.CellCurrent=(float)ad5val*(3.3/4096*1000/100)*(125/4)-125;      //电池电流
		ci_parameter.SweeperCurrent=(float)ad6val*(3.3/4096*1000/100)*(25/4)-25;	   //扫刷电流

		//扫刷电流异常值处理
		if(ci_parameter.SweeperCurrent<0)
		{
			ci_parameter.SweeperCurrent=0;
		}
		else if(ci_parameter.SweeperCurrent>100)
		{
			ci_parameter.SweeperCurrent=99.9;
		}
	}
		//	    watertemp
			
//		temp1=(double)ad6val*(3.3/4096);
//		if(temp1>2.4)
//		{
//		temp2=9999;
//		}
//		else
//		{				
//		temp2=(double)temp1*100/((2.50-temp1));
//		}
//		
//		//根据电阻计算水温
//		if(temp2>=1743)//0度以下都按0度处理
//		{
//			temp1=0;			
//		}
//		else if(temp2>=439.3)
//		{
//			temp1=(-0.000001*temp2*temp2*temp2+0.0045*temp2*temp2-8.2049*temp2+5796.3)/100;
//		}
//		else if (temp2>=134.1)
//		{
//			temp1=0.0002*temp2*temp2-0.2087*temp2+83.899 ;//(30,60)
//		}
//		else if (temp2>=51.3)
//			{
//			temp1=0.0021*temp2*temp2-0.7462*temp2+122.28;// (60,90)	
//			}
//		else if(temp2>=33.5)
//			{
//			temp1=0.011*temp2*temp2-1.7716*temp2+151.92;//  (90,105)	
//			}
//		else
//			{
//			temp1=0.0308*temp2*temp2-3.0872*temp2+173.75;//    (105,120)	
//			}

//		ci_parameter.WaterTemp=temp1;
//		
//			// oilmass
//		temp1=(double)ad4val*(3.3/4096);//电压
//		if(temp1>2.4)
//		{
//			temp2=9999;
//		}
//		else
//		{				
//			temp2=(double)temp1*100/((2.50-temp1));//电阻
//		}
//		ci_parameter.Oilmass_R=temp2;
////		temp1=0.000005*temp2*temp2*temp2+0.0004*temp2*temp2+1.0884*temp2+34.63;	
////		if(temp1>300)
////			{
////				temp1=300;
////			}
//////		ci_parameter.Oilmass=temp1;			
//		}
	}	

//定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		
    if(htim==(&TIM3_Handler))       			//定时器3 10ms
    {   
			time3_counter1++;                   //定时器3计数1迭加
			
			if(time3_counter1>=100)
			{
				time3_counter1=0;
				//ci_parameter.WorkTimeCounter++;	  //WorkTimeCounter每s加1
				LED0_Toggle;                      //LED0每秒闪烁
				BEEPN = !BEEPN;                   //蜂鸣器每秒响灭
			} 
			if(flag_timing == 1)                //只有行走速度大于0才开始计时
			{
				time3_counter2++;                   //定时器3计数2迭加
				if(time3_counter2>=6000)						//每分钟写入一次eeprom
				{
					time3_counter2=0;
					minute = minute + 1;              //工作时间(分钟数)加1
					AT24CXX_WriteLenByte(0,minute,4); //将工作时间写入EEPROM

				} 	
			}		

			if(flagbattery==0)
			{
				time3_counter3++;
				if(time3_counter3>=2000)          //每20秒将flagbattery置1，让电池状态更新
				{
					time3_counter3=0;
					flagbattery = 1;
				}
			}
			//add by mch 20180609
			//flag_run_item在行走电机的CAN接收程序中置1，在定时器中置0
			//若连续50次flag_run_item均为0，代表行走电机未工作,一旦flag_run_done置1就无需
			if(flag_run_done == 0)
			{
				if(flag_run_item == 0)
				{
					time3_run_counter++;
				}
				else
				{
					time3_run_counter = 0;
				}
				flag_run_item = 0;
				if(time3_run_counter > 50)
				{
					flag_run = 0;
					flag_getmileage = 1;           //需要重新获取里程基准值
					flag_run_done = 1;
				}
			}
			
			//flag_fan_item在风机的CAN接收程序中置1，在定时器中置0
			//若连续50次flag_fan_item均为0，代表风机未工作
			if(flag_fan_done == 0)
			{
				if(flag_fan_item == 0)
				{
					time3_fan_counter++;
				}
				else
				{
					time3_fan_counter = 0;
				}
				flag_fan_item = 0;
				if(time3_fan_counter > 50)
				{
					flag_fan = 0;
					flag_fan_done = 1;
				}
			}
			
			//flag_fan_item在风机的CAN接收程序中置1，在定时器中置0
			//若连续50次flag_fan_item均为0，代表风机未工作
			if(flag_sweeper_done == 0)
			{
				if(flag_sweeper_item == 0)
				{
					time3_sweeper_counter++;
				}
				else
				{
					time3_sweeper_counter = 0;
				}
				flag_sweeper_item = 0;
				if(time3_sweeper_counter > 50)
				{
					flag_sweeper = 0;
					flag_sweeper_done = 1;
				}
			}
			
			if(flag_sweeper_done1 == 0)
			{
				if(flag_sweeper_item1 == 0)
				{
					time3_sweeper_counter1++;
				}
				else
				{
					time3_sweeper_counter1 = 0;
				}
				flag_sweeper_item1 = 0;
				if(time3_sweeper_counter1 > 50)
				{
					flag_sweeper1 = 0;
					flag_sweeper_done1 = 1;
				}
			}
			
			if(flag_sweeper_done2 == 0)
			{
				if(flag_sweeper_item2 == 0)
				{
					time3_sweeper_counter2++;
				}
				else
				{
					time3_sweeper_counter2 = 0;
				}
				flag_sweeper_item2 = 0;
				if(time3_sweeper_counter2 > 50)
				{
					flag_sweeper2 = 0;
					flag_sweeper_done2 = 1;
				}
			}
			
			if(flag_sweeper_done3 == 0)
			{
				if(flag_sweeper_item3 == 0)
				{
					time3_sweeper_counter3++;
				}
				else
				{
					time3_sweeper_counter3 = 0;
				}
				flag_sweeper_item3 = 0;
				if(time3_sweeper_counter3 > 50)
				{
					flag_sweeper3 = 0;
					flag_sweeper_done3 = 1;
				}
			}
			
			if(flag_sweeper_done4 == 0)
			{
				if(flag_sweeper_item4 == 0)
				{
					time3_sweeper_counter4++;
				}
				else
				{
					time3_sweeper_counter4 = 0;
				}
				flag_sweeper_item4 = 0;
				if(time3_sweeper_counter4 > 50)
				{
					flag_sweeper4 = 0;
					flag_sweeper_done4 = 1;
				}
			}
			
			// DS18B20
			GetRoomTemp();                      //获取室温
			GetADValue();                       //获取AD采样值
			GetP_LeftLight();                   //获取左转向灯的状态(信号)
			GetP_RightLight();                  //获取右转向灯的状态(信号)
			GetP_ForwardLight();                //获取前向灯的状态(信号)
			GetP_BackLight();                   //获取后退灯的状态(信号)
			//GetP_PreHeatLight();               
			GetP_HandBrakeLight();              //获取手刹的状态(信号)
			GetP_FootBrakeLight();              //获取脚刹的状态(信号)
			GetP_DippedLight();                 //获取近光灯的状态(信号)
			GetP_LowWaterWarning();	            //获取低水位告警状态
			GetP_UserKey1();                    //获取按键1的状态(是否按下)
			GetP_UserKey2Debug();               //获取按键2的状态(是否按下)
			GetP_UserKey3();                    //获取按键3的状态(是否按下)
			
    }
    else if(htim==(&TIM6_Handler))  			//定时器6
    {
			GetP_SweeperSpeed();                //获取扫刷转速
			//change by mch 20180524  如果未接收到行走电机的CAN数据，则使用脉冲计数
			if(flag_run == 0)
			{
				  GetP_Speed();                       //获取行进速度
			}
			
    }
}
