#ifndef __CAN_H
#define __CAN_H
#include "sys.h"
//#include "stm32f7xx_hal_can.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//CAN��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/12/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

//CAN1����RX0�ж�ʹ��
#define CAN1_RX0_INT_ENABLE	1		//0,��ʹ��;1,ʹ��.
#define CAN3_RX0_INT_ENABLE	1		//0,��ʹ��;1,ʹ��.

u8 CAN1_Mode_Init(u32 tsjw,u32 tbs2,u32 tbs1,u16 brp,u32 mode);//CAN��ʼ��
u8 CAN3_Mode_Init(u32 tsjw,u32 tbs2,u32 tbs1,u16 brp,u32 mode);//CAN��ʼ��

u8 CAN1_Send_Msg(u8* msg,u8 len);						//��������
u8 CAN1_Receive_Msg(u8 *buf);							//��������
extern uint16_t FanVoltage,FanSpeed, FanCtlTemp;
extern uint8_t FanCurrent, FanErrorCode, FanTemp;
extern uint16_t BatteryVoltage, BatteryQuantity, OBC_Voltage, OBC_Current;
extern int16_t BatteryCurrent;
extern uint8_t BatteryTemp, BatteryMaxTemp, BatteryMinTemp;
extern uint8_t Charge_Info, Highest_Alarm_Level, Alarm_Level;  //, Alarm_Code
extern uint8_t flag_fan;
extern uint8_t Alarm_Code[8];
extern uint8_t error_code[64];
extern uint8_t error_count, error_count_old;
extern uint8_t alarm_code;

//add by mch 20180524
extern uint16_t RunSpeed, RunMileage, RunVoltage, RunCurrent, RunTemp, RunCtlTemp;
extern uint8_t RunStatus, RunErrorCode, RunLowPower;
extern uint8_t flag_run, flag_getmileage, flag_run_item, flag_fan_item, flag_run_done, flag_fan_done;
extern uint16_t counter_fan, counter_run;

//add by mch 20180824
extern uint16_t SweeperSpeed, SweeperVoltage, SweeperCurrent, SweeperTemp, SweeperCtlTemp, SweeperErrorCode;            //��ǰɨˢ������
extern uint8_t SweeperStatus;
extern uint16_t SweeperSpeed1, SweeperVoltage1, SweeperCurrent1, SweeperTemp1, SweeperCtlTemp1, SweeperErrorCode1;            //��ǰɨˢ������
extern uint8_t SweeperStatus1;
extern uint16_t SweeperSpeed2, SweeperVoltage2, SweeperCurrent2, SweeperTemp2, SweeperCtlTemp2, SweeperErrorCode2;            //��ǰɨˢ������
extern uint8_t SweeperStatus2;
extern uint16_t SweeperSpeed3, SweeperVoltage3, SweeperCurrent3, SweeperTemp3, SweeperCtlTemp3, SweeperErrorCode3;            //��ǰɨˢ������
extern uint8_t SweeperStatus3;
extern uint16_t SweeperSpeed4, SweeperVoltage4, SweeperCurrent4, SweeperTemp4, SweeperCtlTemp4, SweeperErrorCode4;            //��ǰɨˢ������
extern uint8_t SweeperStatus4;
extern uint8_t flag_sweeper_item, flag_sweeper_item1, flag_sweeper_item2, flag_sweeper_item3, flag_sweeper_item4;
extern uint8_t flag_sweeper_done, flag_sweeper_done1, flag_sweeper_done2, flag_sweeper_done3, flag_sweeper_done4;
extern uint8_t flag_sweeper, flag_sweeper1, flag_sweeper2, flag_sweeper3, flag_sweeper4;
extern uint8_t flag_init_run, flag_init_fan, flag_init_sweeper, flag_init_sweeper1, flag_init_sweeper2, flag_init_sweeper3, flag_init_sweeper4;
#endif
