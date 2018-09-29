#include "bsp.h"
#include "user_config.h"
#include "display.h"
#include "can.h"
#include "bmpdisplay.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//��ʱ���ж���������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

TIM_HandleTypeDef TIM3_Handler;      //��ʱ����� 
TIM_HandleTypeDef TIM6_Handler;      //��ʱ��6��� 

uint32_t KMeter;
uint32_t minute ;

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!(��ʱ��3����APB1�ϣ�ʱ��ΪHCLK/2)
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //ͨ�ö�ʱ��3
    TIM3_Handler.Init.Prescaler=psc;                     //��Ƶ
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM3_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE    
}

//ͨ�ö�ʱ��6��ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ����
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��6!(��ʱ��6����APB1�ϣ�ʱ��ΪHCLK/2)
void TIM6_Init(u16 arr,u16 psc)
{  
    TIM6_Handler.Instance=TIM6;                          //ͨ�ö�ʱ��6
    TIM6_Handler.Init.Prescaler=psc;                     //��Ƶ
    TIM6_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM6_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM6_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&TIM6_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM6_Handler); //ʹ�ܶ�ʱ��6�Ͷ�ʱ��6�ж�   
}

//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    __HAL_RCC_TIM3_CLK_ENABLE();            //ʹ��TIM3ʱ��
    __HAL_RCC_TIM6_CLK_ENABLE();            //ʹ��TIM6ʱ��
    
    HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�3
    HAL_NVIC_EnableIRQ(TIM3_IRQn);          //����ITM3�ж� 

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn,0,3);//�����ж����ȼ�����ռ���ȼ�0�������ȼ�3
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);      //����ITM6�ж� 
}

//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

//��ʱ��6�жϷ�����
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM6_Handler);
}

//���嶨ʱ�ж�ȫ�ֱ���
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

//�������ȫ�ֱ���
U16 tempCounter=0;
U32 tempSum=0;
U16 tempCounter_real=0;
//AD�������ȫ�ֱ���
U16 ADcounter=0;
U32  ad1Sum=0;
U32  ad2Sum=0;
U32  ad5Sum=0;
U32  ad6Sum=0;

U8 flag_shortpress = 0;
U8 flag_longpress = 0;
U8 flag_timing = 0;

P_DISIO_IN_Calc p_disio_in_calc;
//��ȡ��ת��Ƶ�״̬
void GetP_LeftLight(void)
{
	U16 temp;
	temp=P_LeftLight;  //��ȡ��ת���Ӧ��IO�ڵĵ�ƽ
	if(temp==p_disio_in_calc.P_LeftLight_Value)   //P_LeftLight_Valueû�г�ʼ��������������
	{
		p_disio_in_calc.P_LeftLight_Counter++;
		if(p_disio_in_calc.P_LeftLight_Counter>=5)//����5��50ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
		{
			ci_parameter.LeftLight=p_disio_in_calc.P_LeftLight_Value;
			Show_Light(ci_parameter.LeftLight, ci_parameter_old.LeftLight, ZUOZHUAN, 1, 10, 15);                //��ת
			p_disio_in_calc.P_LeftLight_Counter=0;
		}
	}
	else
	{
		p_disio_in_calc.P_LeftLight_Value=temp;
		p_disio_in_calc.P_LeftLight_Counter=0;
	}
	
}
//��ȡ��ת��Ƶ�״̬
void GetP_RightLight(void)
{
	U16 temp;
	temp=P_RightLight;
	if(temp==p_disio_in_calc.P_RightLight_Value)
		{
		p_disio_in_calc.P_RightLight_Counter++;
		if(p_disio_in_calc.P_RightLight_Counter>=5)//����5��50ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
			{
			ci_parameter.RightLight=p_disio_in_calc.P_RightLight_Value;
			Show_Light(ci_parameter.RightLight, ci_parameter_old.RightLight, YOUZHUAN, 1, 591, 15);             //��ת
			p_disio_in_calc.P_RightLight_Counter=0;
			}
		}
	else
		{
		p_disio_in_calc.P_RightLight_Value=temp;
		p_disio_in_calc.P_RightLight_Counter=0;
		}
	
}
//��ȡǰ��Ƶ�״̬
void GetP_ForwardLight(void)
{
	U16 temp;
	temp=P_ForwardLight;
	if(temp==p_disio_in_calc.P_ForwardLight_Value)
		{
		p_disio_in_calc.P_ForwardLight_Counter++;
		if(p_disio_in_calc.P_ForwardLight_Counter>=5)//����5��50ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//��ȡ���˵Ƶ�״̬
void GetP_BackLight(void)
{
	U16 temp;
	temp=P_BackLight;
	if(temp==p_disio_in_calc.P_BackLight_Value)
		{
		p_disio_in_calc.P_BackLight_Counter++;
		if(p_disio_in_calc.P_BackLight_Counter>=5)//����5��50ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//		if(p_disio_in_calc.P_PreHeatLight_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//��ȡ��ɲ��״̬
void GetP_HandBrakeLight(void)
{
	U16 temp;
	temp=P_HandBrakeLight;
	if(temp==p_disio_in_calc.P_HandBrakeLight_Value)
		{
		p_disio_in_calc.P_HandBrakeLight_Counter++;
		if(p_disio_in_calc.P_HandBrakeLight_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//��ȡ��ɲ��״̬
void GetP_FootBrakeLight(void)
{
	U16 temp;
	temp=P_FootBrakeLight;
	if(temp==p_disio_in_calc.P_FootBrakeLight_Value)
		{
		p_disio_in_calc.P_FootBrakeLight_Counter++;
		if(p_disio_in_calc.P_FootBrakeLight_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//��ȡ����Ƶ�״̬
void GetP_DippedLight(void)
{
	U16 temp;
	temp=P_DippedLight;
	if(temp==p_disio_in_calc.P_DippedLight_Value)
		{
		p_disio_in_calc.P_DippedLight_Counter++;
		if(p_disio_in_calc.P_DippedLight_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//		if(p_disio_in_calc.P_CellWarning_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//		if(p_disio_in_calc.P_EngineOilLight_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
//		if(p_disio_in_calc.P_FilterWarning_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
		if(p_disio_in_calc.P_ChargingLight_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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

//��ȡ��ˮλ�澯״̬
void GetP_LowWaterWarning(void)
{
	U16 temp;
	temp=P_LowWaterWarning;
	if(temp==p_disio_in_calc.P_LowWaterWarning_Value)
		{
		p_disio_in_calc.P_LowWaterWarning_Counter++;
		if(p_disio_in_calc.P_LowWaterWarning_Counter>=10)//����10�Σ�100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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

//��ȡ����2��״̬���л�����
void GetP_UserKey2Debug(void)
{
	U16 temp;
	temp=P_UserKey2Debug;
	if(temp==p_disio_in_calc.P_UserKey2Debug_Value)
		{
		p_disio_in_calc.P_UserKey2Debug_Counter++;
		if(p_disio_in_calc.P_UserKey2Debug_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
			{
			ci_parameter.UserKey2Debug=p_disio_in_calc.P_UserKey2Debug_Value;
			p_disio_in_calc.P_UserKey2Debug_Counter=0;
			if(temp==0)
			{
				p_disio_in_calc.P_ODOKey2_Counter++;	//100ms��1			
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

//��ȡ����1��״̬
void GetP_UserKey1(void)
{
	U16 temp;
	temp=P_UserKey1;
	if(temp==p_disio_in_calc.P_UserKey1_Value)
		{
		p_disio_in_calc.P_UserKey1_Counter++;
		if(p_disio_in_calc.P_UserKey1_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
			{
			ci_parameter.UserKey1=p_disio_in_calc.P_UserKey1_Value;
			p_disio_in_calc.P_UserKey1_Counter=0;
			if(temp==0)
				{
					p_disio_in_calc.P_ODOReset_Counter++;	//100ms��1			
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

//��ȡ����3��ֵ
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
		if(p_disio_in_calc.P_UserKey3_Counter>=10)//����10��100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
			{
			ci_parameter.UserKey3=p_disio_in_calc.P_UserKey3_Value;
			p_disio_in_calc.P_UserKey3_Counter=0;
			if(temp==0)
				{
					p_disio_in_calc.P_ODOKey3_Counter++;	//100ms��1		
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
//��ȡɨˢ�ٶ�
void GetP_SweeperSpeed(void)
{
	U16 temp;
	time6_counter2++;
	temp=P_SweeperSpeed;
	if(temp==p_disio_in_calc.P_SweeperSpeed_Value)
		{
		p_disio_in_calc.P_SweeperSpeed_Counter++;
		if(p_disio_in_calc.P_SweeperSpeed_Counter>=5)//����4��0.5ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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
	if(time6_counter2>=20000)// ÿ2����һ��
	{		
		ci_parameter.ESpeedPulseCounter=ESpeedCounter;	
		ci_parameter.SweeperSpeed=ESpeedCounter*30/120;//��λr/min be �����ת�ٲ����������ÿתһȦ���6�������źţ������ת�ٺͷ�������Ϊ65:110��
		ESpeedCounter=0;
		time6_counter2=0;
	}
}

U16 SpeedCounter=0;
U16 getSpeedPulse=0;
//��ȡ�����ٶ�
void GetP_Speed(void)
{
	U16 temp;

	time6_counter1++;   													//ÿ0.1ms����һ��
	temp=P_Speed;
	if(temp==p_disio_in_calc.P_Speed_Value)
	{
		p_disio_in_calc.P_Speed_Counter++;
		if(p_disio_in_calc.P_Speed_Counter>=5)			//����10��1ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
		{
			//ci_parameter.Speed=p_disio_in_calc.P_Speed_Value;
			p_disio_in_calc.P_Speed_Counter=0;
			if(temp==1)
			{
				if(getSpeedPulse==0)
				{
					SpeedCounter++;
					ci_parameter.OdoMeterCounter++;				//��̼���

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
	if(time6_counter1>=20000)// ÿ2����һ��
	{		
		ci_parameter.SpeedPusleCounter=SpeedCounter;	
		ci_parameter.Speed=(SpeedCounter*3.14*0.00054)*30*60/8/18.12;//��λkm/h 
					
		SpeedCounter=0;
		time6_counter1=0;
	}
}

//��ȡ����
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
		if(RoomTemp_Counter>=500)//����10�Σ�100ms�ɼ�����ͬ��ֵ����Ϊ����Чֵ
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

//��ȡADֵ,1S����һ��
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
	
	//�ɼ�100�����ݣ��ֱ���ƽ��
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
		
		//����Ӳ����·�Ͳ���ָ�����ADת��
		ci_parameter.WorkVoltage=(float)ad1val*(3.3/4096)*(1+12.7);                  //������ѹ
//		ci_parameter.CellVoltage=(float)ad2val*(3.3/4096)*31;	                       //��ص�ѹ
//		ci_parameter.CellCurrent=(float)ad5val*(3.3/4096*1000/100)*(125/4)-125;      //��ص���
		ci_parameter.SweeperCurrent=(float)ad6val*(3.3/4096*1000/100)*(25/4)-25;	   //ɨˢ����

		//ɨˢ�����쳣ֵ����
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
//		//���ݵ������ˮ��
//		if(temp2>=1743)//0�����¶���0�ȴ���
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
//		temp1=(double)ad4val*(3.3/4096);//��ѹ
//		if(temp1>2.4)
//		{
//			temp2=9999;
//		}
//		else
//		{				
//			temp2=(double)temp1*100/((2.50-temp1));//����
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

//��ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		
    if(htim==(&TIM3_Handler))       			//��ʱ��3 10ms
    {   
			time3_counter1++;                   //��ʱ��3����1����
			
			if(time3_counter1>=100)
			{
				time3_counter1=0;
				//ci_parameter.WorkTimeCounter++;	  //WorkTimeCounterÿs��1
				LED0_Toggle;                      //LED0ÿ����˸
				BEEPN = !BEEPN;                   //������ÿ������
			} 
			if(flag_timing == 1)                //ֻ�������ٶȴ���0�ſ�ʼ��ʱ
			{
				time3_counter2++;                   //��ʱ��3����2����
				if(time3_counter2>=6000)						//ÿ����д��һ��eeprom
				{
					time3_counter2=0;
					minute = minute + 1;              //����ʱ��(������)��1
					AT24CXX_WriteLenByte(0,minute,4); //������ʱ��д��EEPROM

				} 	
			}		

			if(flagbattery==0)
			{
				time3_counter3++;
				if(time3_counter3>=2000)          //ÿ20�뽫flagbattery��1���õ��״̬����
				{
					time3_counter3=0;
					flagbattery = 1;
				}
			}
			//add by mch 20180609
			//flag_run_item�����ߵ����CAN���ճ�������1���ڶ�ʱ������0
			//������50��flag_run_item��Ϊ0���������ߵ��δ����,һ��flag_run_done��1������
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
					flag_getmileage = 1;           //��Ҫ���»�ȡ��̻�׼ֵ
					flag_run_done = 1;
				}
			}
			
			//flag_fan_item�ڷ����CAN���ճ�������1���ڶ�ʱ������0
			//������50��flag_fan_item��Ϊ0��������δ����
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
			
			//flag_fan_item�ڷ����CAN���ճ�������1���ڶ�ʱ������0
			//������50��flag_fan_item��Ϊ0��������δ����
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
			GetRoomTemp();                      //��ȡ����
			GetADValue();                       //��ȡAD����ֵ
			GetP_LeftLight();                   //��ȡ��ת��Ƶ�״̬(�ź�)
			GetP_RightLight();                  //��ȡ��ת��Ƶ�״̬(�ź�)
			GetP_ForwardLight();                //��ȡǰ��Ƶ�״̬(�ź�)
			GetP_BackLight();                   //��ȡ���˵Ƶ�״̬(�ź�)
			//GetP_PreHeatLight();               
			GetP_HandBrakeLight();              //��ȡ��ɲ��״̬(�ź�)
			GetP_FootBrakeLight();              //��ȡ��ɲ��״̬(�ź�)
			GetP_DippedLight();                 //��ȡ����Ƶ�״̬(�ź�)
			GetP_LowWaterWarning();	            //��ȡ��ˮλ�澯״̬
			GetP_UserKey1();                    //��ȡ����1��״̬(�Ƿ���)
			GetP_UserKey2Debug();               //��ȡ����2��״̬(�Ƿ���)
			GetP_UserKey3();                    //��ȡ����3��״̬(�Ƿ���)
			
    }
    else if(htim==(&TIM6_Handler))  			//��ʱ��6
    {
			GetP_SweeperSpeed();                //��ȡɨˢת��
			//change by mch 20180524  ���δ���յ����ߵ����CAN���ݣ���ʹ���������
			if(flag_run == 0)
			{
				  GetP_Speed();                       //��ȡ�н��ٶ�
			}
			
    }
}
