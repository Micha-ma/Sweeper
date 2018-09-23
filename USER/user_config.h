#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H


#include "display.h"
#include "ds18b20.h"
#include "adc.h"

typedef struct {
	U16 LeftLight; //左转1点亮，0熄灭
	U16 RightLight;//所有离散信号1代表有效，0代表无效
	U16 ForwardLight;
	U16 BackWardLight;
	U16 CellWarning;
	U16 WaterTempWarning;
	U16 PreHeatLight;
	U16 HandBrakeLight;
	U16 FootBrakeLight;
	U16 EngineOilLight;
	U16 DippedLight;//近光
	U16 FilterWarning;
	U16 ChargingLight;
	U16 LowWaterWarning;
	U16 UserKey1;
	U16 UserKey2Debug;
	U16 UserKey3;
	U16 Car;
	
	double Oilmass_R;//油量电阻
	double WaterTemp;  //水温
	U32 WorkTime;
	U32 WorkTimeCounter;
	U32 OdoMeter;
	U32 OdoMeterCounter;
	U32 SweeperSpeed;  
	U32 ESpeedPulseCounter;
	U32 Speed;
	U32 SpeedPusleCounter;
	
	U32 FanSpeed;
	
	short RoomTemp;//室温
	float WorkVoltage;//工作电压 
	float CellVoltage;//电池电压
	float CellCurrent;//电池电流
	float SweeperCurrent;//扫刷电流
//	float SwapperVoltage;//扫刷电压
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
	U16 Speed;            //扫刷转速
	U16 Voltage;          //扫刷电压
	U16 Current;          //扫刷电流
	U16 Temp;             //扫刷电机温度
	U16 CtlTemp;          //扫刷控制器温度
	U16 ErrorCode;        //扫刷故障代码
	
	U8 WorkStatus;        //扫刷电机运行方式
	U8 Flag_Single_Val;   //在定时器中单次置位标志
	U8 Flag_Final_Val;    //最后工作情况的置位标志
	U8 Flag_Init_Judge;   //在主循环中初始化判断标志，避免重复初始化
	U8 Flag_Done_Judge;   //在定时器中无扫刷CAN数据判断标志，避免重复判断是否停止工作
	
}CAN_SWEEPER;
extern CAN_SWEEPER rf_sweeper, lf_sweeper, rb_sweeper, lb_sweeper, mid_sweeper;
extern CAN_SWEEPER rf_sweeper_old, lf_sweeper_old, rb_sweeper_old, lb_sweeper_old, mid_sweeper_old;

typedef struct {
	U16 Speed;            //风机转速
	U16 Voltage;          //风机电压
	U16 Current;          //风机电流, 老的为 U8
	U16 Temp;             //风机电机温度, 老的为 U8
	U16 CtlTemp;          //风机控制器温度, 老的 无
	U16 ErrorCode;        //风机故障代码
	
	U8 WorkStatus;        //风机电机运行方式
	U8 Flag_Single_Val;   //在定时器中单次置位标志
	U8 Flag_Final_Val;    //最后工作情况的置位标志
	U8 Flag_Init_Judge;   //在主循环中初始化判断标志，避免重复初始化
	U8 Flag_Done_Judge;   //在定时器中无风机CAN数据判断标志，避免重复判断是否停止工作
	
}CAN_FANMOTOR;
extern CAN_FANMOTOR fan_motor, fan_motor_old;

typedef struct {
	U16 Speed;            //行走电机转速
	U16 Voltage;          //行走电机电压
	U16 Current;          //行走电机电流
	U16 Temp;             //行走电机温度
	U16 CtlTemp;          //行走电机控制器温度
	U16 ErrorCode;        //行走电机故障代码
	
	U8 Mileage;           //行驶里程
	U8 WorkStatus;        //行走电机运行方式
	U8 Flag_Single_Val;   //在定时器中单次置位标志
	U8 Flag_Final_Val;    //最后工作情况的置位标志
	U8 Flag_Init_Judge;   //在主循环中初始化判断标志，避免重复初始化
	U8 Flag_Done_Judge;   //在定时器中无行走电机CAN数据判断标志，避免重复判断是否停止工作
	
}CAN_RUNMOTOR;
extern CAN_RUNMOTOR run_motor, run_motor_old;

typedef struct {
	U16 Voltage;          //电池电压
	U16 Current;          //电池电流
	U16 Quantity;         //电池电量
	U16 Temp;             //电池平均温度
	U16 MaxTemp;          //电池最高温度
	U16 MinTemp;          //电池最低温度
	
	U8 ErrorCode[8];      //电池故障代码
	U8 ErrorMaxLevel;     //电池最高故障等级
	U8 WorkStatus;        //电池工作状态
	
}CAN_BATTERY;
extern CAN_BATTERY battery, battery_old;

#define P_LeftLight  			HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_2) //12V/open 	P_DIN_12V_1左转
#define P_RightLight 			HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_3) //12V/open	P_DIN_12V_2右转
#define P_ForwardLight  		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1) //12V/open 	P_DIN_12V_3前进
#define P_BackLight 			HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_11) //12V/open	P_DIN_12V_4后退
#define P_FootBrakeLight 		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7) //12V/open	P_DIN_12V_7脚刹
//#define P_PreHeatLight		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7) //12V/open	P_DIN_12V_7预热
#define P_HandBrakeLight		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3) //12V/open	P_DIN_GND_2 手刹
//#define P_HandBrakeLight		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) //12V/open	P_DIN_12V_6手刹
#define P_DippedLight		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9) //12V/open	P_DIN_12V_5近光

//#define P_EngineSpeed		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_10)//pulse	P_DIN_12V_9发动机转速
#define P_SweeperSpeed		HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_8) //12V/open	P_DIN_GND_7扫刷转速
//#define P_SweeperSpeed		HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_6) //12V/open	P_DIN_12V_8扫刷转速
//#define P_Speed			HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_12) //pulse	P_DIN_12V_11车轮转速
#define P_Speed				HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_7)	//轮速为脉冲信号，接通时为低，断开时为上拉12V，车轮转速

#define P_CellWarning		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2) 	//GND/open	P_DIN_GND1
#define P_EngineOilLight	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)//GND/open	P_DIN_GND2机油指示
#define P_FilterWarning		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)//GND/open	P_DIN_GND3过滤器告警
#define P_ChargingLight		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)//GND/open	P_DIN_GND4 充电指示
#define P_LowWaterWarning	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_6)//GND/open	P_DIN_GND5冷却水低水位告警

#define P_UserKey1        	HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11)  //KEY1按键PB11
#define P_UserKey2Debug		HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)  //KEY2按键PA8
#define P_UserKey3        	HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_3)  //KEY3按键PD3

//LED端口定义
#define LED0(n)					(n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET))
#define LED0_ON					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET)
#define LED0_OFF				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET)
#define LED0_Toggle 			(HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1)) //IO电平翻转

#define P_VideoSwitch(n)    (n?HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET)) // 视频切换
#define USB_CTRL(n)				(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,GPIO_PIN_RESET)) // USB POWER
#define BEEP_CTRL(n)			(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_12,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_12,GPIO_PIN_RESET)) // BEEP
#define DisOUT1(n)				(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET)) // DISOUT 1
#define DisOUT2(n)				(n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET)) // DISOUT 2
#define DisOUT2_Toggle 			(HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_14)) //P_DO2电平翻转
#define P_DO2       	HAL_GPIO_ReadPin(GPIOG,GPIO_PIN_14)  //读取P_DO2电平

#define HighWaterTempCtrl(n)  	DisOUT1(n)

#define USER_KEY1        	HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11)  //KEY1按键PB11
#define USER_KEY2        	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8)  //KEY2按键PA8
#define USER_KEY3        	HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_3)  //KEY3按键PD3

extern u8 flag_key3;
extern u16 ds18b20_init_err;
extern u16 main_count,can_counter,main_counter2;

#endif




