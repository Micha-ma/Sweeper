#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//��ʱ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
extern TIM_HandleTypeDef TIM3_Handler;      //��ʱ��3��� 
extern TIM_HandleTypeDef TIM6_Handler;      //��ʱ��6��� 
extern uint32_t KMeter;
extern uint32_t minute ;
extern uint16_t ADcounter;
extern uint8_t flag_shortpress;
extern uint8_t flag_longpress;
extern uint8_t flag_timing;
void TIM3_Init(u16 arr,u16 psc);
void TIM6_Init(u16 arr,u16 psc);
#endif

