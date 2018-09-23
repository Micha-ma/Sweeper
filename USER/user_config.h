#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H


#include "display.h"
#include "ds18b20.h"
#include "adc.h"

typedef struct {
	U16 LeftLight; //��ת1������0Ϩ��
	U16 RightLight;//������ɢ�ź�1������Ч��0������Ч
	U16 ForwardLight;
	U16 BackWardLight;
	U16 CellWarning;
	U16 WaterTempWarning;
	U16 PreHeatLight;
	U16 HandBrakeLight;
	U16 FootBrakeLight;
	U16 EngineOilLight;
	U16 DippedLight;//����
	U16 FilterWarning;
	U16 ChargingLight;
	U16 LowWaterWarning;
	U16 UserKey1;
	U16 UserKey2Debug;
	U16 UserKey3;
	U16 Car;
	
	double Oilmass_R;//��������
	double WaterTemp;  //ˮ��
	U32 WorkTime;
	U32 WorkTimeCounter;
	U32 OdoMeter;
	U32 OdoMeterCounter;
	U32 SweeperSpeed;  
	U32 ESpeedPulseCounter;
	U32 Speed;
	U32 SpeedPusleCounter;
	
	U32 FanSpeed;
	
	short RoomTemp;//����
	float WorkVoltage;//������ѹ 
	float CellVoltage;//��ص�ѹ
	float CellCurrent;//��ص���
	float SweeperCurrent;//ɨˢ����
//	float SwapperVoltage;//ɨˢ��ѹ
} CI_PARAMETER;

typedef struct {
	U16 P_LeftLight_Counter;
	U16 P_RightLight_Counter;
	U16 P_ForwardLight_Counter;
	U16 P_BackLight_Counter;
	U16 P_PreHeatLight_Counter;
	U16 P_HandBrakeLight_Counter;
	U16 P_FootBrakeLight_Counter;
	U16 P_DippedLight_Counter;
	U16 P_CellWarning_Counter;
	U16 P_EngineOilLight_Counter;
	U16 P_FilterWarning_Counter;
	U16 P_ChargingLight_Counter;
	U16 P_LowWaterWarning_Counter;
	U16 P_UserKey1_Counter;
	U16 P_UserKey2Debug_Counter;
	U16 P_UserKey3_Counter;
	U16	P_ODOReset_Counter;
	U16	P_ODOKey2_Counter;
	U16	P_ODOKey3_Counter;
	
	U16	P_RoomTemp_Counter;

	U16 P_SweeperSpeed_Counter;           //timer6
	U16 P_Speed_Counter;                  //timer6

	U16 P_LeftLight_Value;
	U16 P_RightLight_Value;
	U16 P_ForwardLight_Value;
	U16 P_BackLight_Value;
	U16 P_PreHeatLight_Value;
	U16 P_HandBrakeLight_Value;
	U16 P_FootBrakeLight_Value;
	U16 P_DippedLight_Value;
	U16 P_CellWarning_Value;
	U16 P_EngineOilLight_Value;
	U16 P_FilterWarning_Value;
	U16 P_ChargingLight_Value;
	U16 P_LowWaterWarning_Value;
	U16 P_UserKey1_Value;
	U16 P_UserKey2Debug_Value;
	U16 P_UserKey3_Value;
	
	U16 P_RoomTemp_Value;

	U16 P_SweeperSpeed_Value;//timer6
	U16 P_Speed_Value;//timer6
	
}P_DISIO_IN_Calc;

extern CI_PARAMETER ci_parameter;
extern CI_PARAMETER ci_parameter_old;
extern P_DISIO_IN_Calc p_disio_in_calc;
extern U16 display_init_OK;

//add by mch 20180824
typedef struct {
	U16 Speed;            //ɨˢת��
	U16 Voltage;          //ɨˢ��ѹ
	U16 Current;          //ɨˢ����
	U16 Temp;             //ɨˢ����¶�
	U16 CtlTemp;          //ɨˢ�������¶�
	U16 ErrorCode;        //ɨˢ���ϴ���
	
	U8 WorkStatus;        //ɨˢ������з�ʽ
	U8 Flag_Single_Val;   //�ڶ�ʱ���е�����λ��־
	U8 Flag_Final_Val;    //������������λ��־
	U8 Flag_Init_Judge;   //����ѭ���г�ʼ���жϱ�־�������ظ���ʼ��
	U8 Flag_Done_Judge;   //�ڶ�ʱ������ɨˢCAN�����жϱ�־�������ظ��ж��Ƿ�ֹͣ����
	
}CAN_SWEEPER;
extern CAN_SWEEPER rf_sweeper, lf_sweeper, rb_sweeper, lb_sweeper, mid_sweeper;
extern CAN_SWEEPER rf_sweeper_old, lf_sweeper_old, rb_sweeper_old, lb_sweeper_old, mid_sweeper_old;

typedef struct {
	U16 Speed;            //���ת��
	U16 Voltage;          //�����ѹ
	U16 Current;          //�������, �ϵ�Ϊ U8
	U16 Temp;             //�������¶�, �ϵ�Ϊ U8
	U16 CtlTemp;          //����������¶�, �ϵ� ��
	U16 ErrorCode;        //������ϴ���
	
	U8 WorkStatus;        //���������з�ʽ
	U8 Flag_Single_Val;   //�ڶ�ʱ���е�����λ��־
	U8 Flag_Final_Val;    //������������λ��־
	U8 Flag_Init_Judge;   //����ѭ���г�ʼ���жϱ�־�������ظ���ʼ��
	U8 Flag_Done_Judge;   //�ڶ�ʱ�����޷��CAN�����жϱ�־�������ظ��ж��Ƿ�ֹͣ����
	
}CAN_FANMOTOR;
extern CAN_FANMOTOR fan_motor, fan_motor_old;

typedef struct {
	U16 Speed;            //���ߵ��ת��
	U16 Voltage;          //���ߵ����ѹ
	U16 Current;          //���ߵ������
	U16 Temp;             //���ߵ���¶�
	U16 CtlTemp;          //���ߵ���������¶�
	U16 ErrorCode;        //���ߵ�����ϴ���
	
	U8 Mileage;           //��ʻ���
	U8 WorkStatus;        //���ߵ�����з�ʽ
	U8 Flag_Single_Val;   //�ڶ�ʱ���е�����λ��־
	U8 Flag_Final_Val;    //������������λ��־
	U8 Flag_Init_Judge;   //����ѭ���г�ʼ���жϱ�־�������ظ���ʼ��
	U8 Flag_Done_Judge;   //�ڶ�ʱ���������ߵ��CAN�����жϱ�־�������ظ��ж��Ƿ�ֹͣ����
	
}CAN_RUNMOTOR;
extern CAN_RUNMOTOR run_motor, run_motor_old;

typedef struct {
	U16 Voltage;          //��ص�ѹ
	U16 Current;          //��ص���
	U16 Quantity;         //��ص���
	U16 Temp;             //���ƽ���¶�
	U16 MaxTemp;          //�������¶�
	U16 MinTemp;          //�������¶�
	
	U8 ErrorCode[8];      //��ع��ϴ���
	U8 ErrorMaxLevel;     //�����߹��ϵȼ�
	U8 WorkStatus;        //��ع���״̬
	
}CAN_BATTERY;
extern CAN_BATTERY battery, battery_old;

#define P_LeftLight  			HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_2) //12V/open 	P_DIN_12V_1��ת
#define P_RightLight 			HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_3) //12V/open	P_DIN_12V_2��ת
#define P_ForwardLight  		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1) //12V/open 	P_DIN_12V_3ǰ��
#define P_BackLight 			HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_11) //12V/open	P_DIN_12V_4����
#define P_FootBrakeLight 		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7) //12V/open	P_DIN_12V_7��ɲ
//#define P_PreHeatLight		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7) //12V/open	P_DIN_12V_7Ԥ��
#define P_HandBrakeLight		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3) //12V/open	P_DIN_GND_2 ��ɲ
//#define P_HandBrakeLight		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) //12V/open	P_DIN_12V_6��ɲ
#define P_DippedLight		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9) //12V/open	P_DIN_12V_5����

//#define P_EngineSpeed		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_10)//pulse	P_DIN_12V_9������ת��
#define P_SweeperSpeed		HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_8) //12V/open	P_DIN_GND_7ɨˢת��
//#define P_SweeperSpeed		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_6) //12V/open	P_DIN_12V_8ɨˢת��
//#define P_Speed			HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_12) //pulse	P_DIN_12V_11����ת��
#define P_Speed				HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_7)	//����Ϊ�����źţ���ͨʱΪ�ͣ��Ͽ�ʱΪ����12V������ת��

#define P_CellWarning		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2) 	//GND/open	P_DIN_GND1
#define P_EngineOilLight	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)//GND/open	P_DIN_GND2����ָʾ
#define P_FilterWarning		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)//GND/open	P_DIN_GND3�������澯
#define P_ChargingLight		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)//GND/open	P_DIN_GND4 ���ָʾ
#define P_LowWaterWarning	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_6)//GND/open	P_DIN_GND5��ȴˮ��ˮλ�澯

#define P_UserKey1        	HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11)  //KEY1����PB11
#define P_UserKey2Debug		HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)  //KEY2����PA8
#define P_UserKey3        	HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_3)  //KEY3����PD3

//LED�˿ڶ���
#define LED0(n)					(n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET))
#define LED0_ON					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET)
#define LED0_OFF				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET)
#define LED0_Toggle 			(HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1)) //IO��ƽ��ת

#define P_VideoSwitch(n)    (n?HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET)) // ��Ƶ�л�
#define USB_CTRL(n)				(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,GPIO_PIN_RESET)) // USB POWER
#define BEEP_CTRL(n)			(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_12,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_12,GPIO_PIN_RESET)) // BEEP
#define DisOUT1(n)				(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET)) // DISOUT 1
#define DisOUT2(n)				(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET)) // DISOUT 2
#define DisOUT2_Toggle 			(HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14)) //P_DO2��ƽ��ת
#define P_DO2       	HAL_GPIO_ReadPin(GPIOG,GPIO_PIN_14)  //��ȡP_DO2��ƽ

#define HighWaterTempCtrl(n)  	DisOUT1(n)

#define USER_KEY1        	HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11)  //KEY1����PB11
#define USER_KEY2        	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)  //KEY2����PA8
#define USER_KEY3        	HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_3)  //KEY3����PD3

extern u8 flag_key3;
extern u16 ds18b20_init_err;
extern u16 main_count,can_counter,main_counter2;

#endif




