#include "can.h"
#include "usart.h"
#include "delay.h"
#include "user_config.h"
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
//All rights reserved									////////////////////////////////////////////////
  
////////////////  //////////////////
CAN_HandleTypeDef   CAN1_Handler;   //CAN1���
CAN_HandleTypeDef   CAN3_Handler;   //CAN3���

CanTxMsgTypeDef     TxMessage;      //������Ϣ
CanRxMsgTypeDef     RxMessage;      //������Ϣ

CanTxMsgTypeDef     Tx3Message;      //CAN3������Ϣ
CanRxMsgTypeDef     Rx3Message;      //CAN3������Ϣ

uint16_t FanVoltage=0,              //�����ѹ
				 FanSpeed=0,                //���ת��
				 FanCtlTemp=0;              //�������¶�


uint8_t FanCurrent=0,               //�������
				FanTemp=50,                 //����¶�
				FanErrorCode=1;             //���������

uint16_t RunSpeed=0,                //���ߵ��ת��
         RunMileage=0,              //��ʻ���
				 RunVoltage=0,              //���ߵ����ѹ
				 RunCurrent=0,              //���ߵ������
				 RunTemp=0,                 //���ߵ���¶�
				 RunCtlTemp=0;              //���ߵ���¶�

uint8_t RunStatus=0,                //���ߵ��ģʽ
        RunErrorCode=0,             //���ߵ��������
				RunLowPower=0;              //���ߵ���͹���
				
uint16_t SweeperSpeed=0,                //��ǰɨˢת��
				 SweeperVoltage=0,              //��ǰɨˢ��ѹ
				 SweeperCurrent=0,              //��ǰɨˢ����
				 SweeperTemp=0,                 //��ǰɨˢ����¶�
				 SweeperCtlTemp=0,              //��ǰɨˢ�������¶�
				 SweeperErrorCode=0;            //��ǰɨˢ������

uint8_t SweeperStatus=0;                //��ǰɨˢ����ģʽ

uint16_t SweeperSpeed1=0,                //�Һ�ɨˢת��
				 SweeperVoltage1=0,              //�Һ�ɨˢ��ѹ
				 SweeperCurrent1=0,              //�Һ�ɨˢ����
				 SweeperTemp1=0,                 //�Һ�ɨˢ����¶�
				 SweeperCtlTemp1=0,              //�Һ�ɨˢ�������¶�
				 SweeperErrorCode1=0;            //�Һ�ɨˢ������

uint8_t SweeperStatus1=0;                //�Һ�ɨˢ����ģʽ

uint16_t SweeperSpeed2=0,                //��ǰɨˢת��
				 SweeperVoltage2=0,              //��ǰɨˢ��ѹ
				 SweeperCurrent2=0,              //��ǰɨˢ����
				 SweeperTemp2=0,                 //��ǰɨˢ����¶�
				 SweeperCtlTemp2=0,              //��ǰɨˢ�������¶�
				 SweeperErrorCode2=0;            //��ǰɨˢ������

uint8_t SweeperStatus2=0;                //��ǰɨˢ����ģʽ

uint16_t SweeperSpeed3=0,                //���ɨˢת��
				 SweeperVoltage3=0,              //���ɨˢ��ѹ
				 SweeperCurrent3=0,              //���ɨˢ����
				 SweeperTemp3=0,                 //���ɨˢ����¶�
				 SweeperCtlTemp3=0,              //���ɨˢ�������¶�
				 SweeperErrorCode3=0;            //���ɨˢ������

uint8_t SweeperStatus3=0;                //���ɨˢ����ģʽ

uint16_t SweeperSpeed4=0,                //����ɨˢת��
				 SweeperVoltage4=0,              //����ɨˢ��ѹ
				 SweeperCurrent4=0,              //����ɨˢ����
				 SweeperTemp4=0,                 //����ɨˢ����¶�
				 SweeperCtlTemp4=0,              //����ɨˢ�������¶�
				 SweeperErrorCode4=0;            //����ɨˢ������

uint8_t SweeperStatus4=0;                //����ɨˢ����ģʽ

uint16_t BatteryVoltage=0,          //��ص�ѹ
				 BatteryQuantity=0;         //��ص���
				 
uint16_t OBC_Voltage=0,             //��س���ѹ
				 OBC_Current=0;             //��س�����
				 
int16_t BatteryCurrent=10000;        //��ص��������ڸ���
uint8_t BatteryTemp=50;               //����¶ȣ����ڸ���	
uint8_t BatteryMaxTemp=50;            //�������¶ȣ����ڸ���	
uint8_t BatteryMinTemp=50;            //�������¶ȣ����ڸ���	

uint8_t Charge_Info=0,              //��س���־λ
				Highest_Alarm_Level=0,      //�����߱����ȼ�
				Alarm_Level=0;              //��ع��ϵȼ�
				//Alarm_Code=255,               //��ع�����
uint8_t Alarm_Code[8];

u8 flag_fan=0;                      //���������־��1��������
u16 counter_fan = 0;                //���CAN���ռ���

u8 flag_getmileage = 1;             //Ĭ��Ҫ��ȡ��׼��ʻ���
u8 flag_run=0;                      //���ߵ��������־��1��������
u16 counter_run = 0;                //���ߵ��CAN���ռ���

//add by mch 20180609
u8 flag_run_item = 0;
u8 flag_fan_item = 0;
u8 flag_run_done = 0;
u8 flag_fan_done = 0;

//add by mch 201808024
u8 flag_init_run = 0;
u8 flag_init_fan = 0;
u8 flag_init_sweeper = 0;
u8 flag_init_sweeper1 = 0;
u8 flag_init_sweeper2 = 0;
u8 flag_init_sweeper3 = 0;
u8 flag_init_sweeper4 = 0;
u8 flag_sweeper_item = 0;
u8 flag_sweeper_item1 = 0;
u8 flag_sweeper_item2 = 0;
u8 flag_sweeper_item3 = 0;
u8 flag_sweeper_item4 = 0;
u8 flag_sweeper_done = 0;
u8 flag_sweeper_done1 = 0;
u8 flag_sweeper_done2 = 0;
u8 flag_sweeper_done3 = 0;
u8 flag_sweeper_done4 = 0;
u8 flag_sweeper=0;
u8 flag_sweeper1=0;
u8 flag_sweeper2=0;
u8 flag_sweeper3=0;
u8 flag_sweeper4=0;

u8 error_code[64];
u8 error_count=0;
u8 error_count_old=100;
u8 alarm_code;

////CAN��ʼ��
//tsjw:����ͬ����Ծʱ�䵥Ԫ.��Χ:CAN_SJW_1TQ~CAN_SJW_4TQ
//tbs2:ʱ���2��ʱ�䵥Ԫ.   ��Χ:CAN_BS2_1TQ~CAN_BS2_8TQ;
//tbs1:ʱ���1��ʱ�䵥Ԫ.   ��Χ:CAN_BS1_1TQ~CAN_BS1_16TQ
//brp :�����ʷ�Ƶ��.��Χ:1~1024; tq=(brp)*tpclk1
//������=Fpclk1/((tbs1+1+tbs2+1+1)*brp);
//mode:CAN_MODE_NORMAL,��ͨģʽ;CAN_MODE_LOOPBACK,�ػ�ģʽ;
//Fpclk1��ʱ���ڳ�ʼ����ʱ������Ϊ54M,�������CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_11tq,6,CAN_MODE_LOOPBACK);
//������Ϊ:54M/((6+11+1)*6)=500Kbps
//����ֵ:0,��ʼ��OK;
//    ����,��ʼ��ʧ��; 

u8 CAN1_Mode_Init(u32 tsjw,u32 tbs2,u32 tbs1,u16 brp,u32 mode)
{
    CAN_FilterConfTypeDef  CAN1_FilerConf;
    
    CAN1_Handler.Instance=CAN1; 
    CAN1_Handler.pTxMsg=&TxMessage;     //������Ϣ
    CAN1_Handler.pRxMsg=&RxMessage;     //������Ϣ
    CAN1_Handler.Init.Prescaler=brp;    //��Ƶϵ��(Fdiv)Ϊbrp+1
    CAN1_Handler.Init.Mode=mode;        //ģʽ���� 
    CAN1_Handler.Init.SJW=tsjw;         //����ͬ����Ծ���(Tsjw)Ϊtsjw+1��ʱ�䵥λ CAN_SJW_1TQ~CAN_SJW_4TQ
    CAN1_Handler.Init.BS1=tbs1;         //tbs1��ΧCAN_BS1_1TQ~CAN_BS1_16TQ
    CAN1_Handler.Init.BS2=tbs2;         //tbs2��ΧCAN_BS2_1TQ~CAN_BS2_8TQ
    CAN1_Handler.Init.TTCM=DISABLE;     //��ʱ�䴥��ͨ��ģʽ 
    CAN1_Handler.Init.ABOM=DISABLE;     //����Զ����߹���
    CAN1_Handler.Init.AWUM=DISABLE;     //˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)
    CAN1_Handler.Init.NART=ENABLE;      //��ֹ�����Զ����� 
    CAN1_Handler.Init.RFLM=DISABLE;     //���Ĳ�����,�µĸ��Ǿɵ� 
    CAN1_Handler.Init.TXFP=DISABLE;     //���ȼ��ɱ��ı�ʶ������ 
    if(HAL_CAN_Init(&CAN1_Handler)!=HAL_OK) return 1;   //��ʼ��
    
    CAN1_FilerConf.FilterIdHigh=0X0000;     //32λID
    CAN1_FilerConf.FilterIdLow=0X0000;
    CAN1_FilerConf.FilterMaskIdHigh=0X0000; //32λMASK
    CAN1_FilerConf.FilterMaskIdLow=0X0000;  
    CAN1_FilerConf.FilterFIFOAssignment=CAN_FILTER_FIFO0;//������0������FIFO0
    CAN1_FilerConf.FilterNumber=0;          //������0
    CAN1_FilerConf.FilterMode=CAN_FILTERMODE_IDMASK;
    CAN1_FilerConf.FilterScale=CAN_FILTERSCALE_32BIT;
    CAN1_FilerConf.FilterActivation=ENABLE; //�����˲���0
    CAN1_FilerConf.BankNumber=14;
    if(HAL_CAN_ConfigFilter(&CAN1_Handler,&CAN1_FilerConf)!=HAL_OK) return 2;//�˲�����ʼ��
    return 0;
}

u8 CAN3_Mode_Init(u32 tsjw,u32 tbs2,u32 tbs1,u16 brp,u32 mode)
{
    CAN_FilterConfTypeDef  CAN3_FilerConf;
    
    CAN3_Handler.Instance=CAN3; 
    CAN3_Handler.pTxMsg=&Tx3Message;     //������Ϣ
    CAN3_Handler.pRxMsg=&Rx3Message;     //������Ϣ
    CAN3_Handler.Init.Prescaler=brp;    //��Ƶϵ��(Fdiv)Ϊbrp+1
    CAN3_Handler.Init.Mode=mode;        //ģʽ���� 
    CAN3_Handler.Init.SJW=tsjw;         //����ͬ����Ծ���(Tsjw)Ϊtsjw+1��ʱ�䵥λ CAN_SJW_1TQ~CAN_SJW_4TQ
    CAN3_Handler.Init.BS1=tbs1;         //tbs1��ΧCAN_BS1_1TQ~CAN_BS1_16TQ
    CAN3_Handler.Init.BS2=tbs2;         //tbs2��ΧCAN_BS2_1TQ~CAN_BS2_8TQ
    CAN3_Handler.Init.TTCM=DISABLE;     //��ʱ�䴥��ͨ��ģʽ 
    CAN3_Handler.Init.ABOM=DISABLE;     //����Զ����߹���
    CAN3_Handler.Init.AWUM=DISABLE;     //˯��ģʽͨ���������(���CAN->MCR��SLEEPλ)
    CAN3_Handler.Init.NART=ENABLE;      //��ֹ�����Զ����� 
    CAN3_Handler.Init.RFLM=DISABLE;     //���Ĳ�����,�µĸ��Ǿɵ� 
    CAN3_Handler.Init.TXFP=DISABLE;     //���ȼ��ɱ��ı�ʶ������ 
    if(HAL_CAN_Init(&CAN3_Handler)!=HAL_OK) return 1;   //��ʼ��
    
    CAN3_FilerConf.FilterIdHigh=0X0000;     //32λID
    CAN3_FilerConf.FilterIdLow=0X0000;
    CAN3_FilerConf.FilterMaskIdHigh=0X0000; //32λMASK
    CAN3_FilerConf.FilterMaskIdLow=0X0000;  
    CAN3_FilerConf.FilterFIFOAssignment=CAN_FILTER_FIFO0;//������0������FIFO0
    CAN3_FilerConf.FilterNumber=0;          //������0
    CAN3_FilerConf.FilterMode=CAN_FILTERMODE_IDMASK;
    CAN3_FilerConf.FilterScale=CAN_FILTERSCALE_32BIT;
    CAN3_FilerConf.FilterActivation=ENABLE; //�����˲���0
    CAN3_FilerConf.BankNumber=14;
    if(HAL_CAN_ConfigFilter(&CAN3_Handler,&CAN3_FilerConf)!=HAL_OK) return 2;//�˲�����ʼ��
    return 0;
}

//CAN�ײ��������������ã�ʱ�����ã��ж�����
//�˺����ᱻHAL_CAN_Init()����
//hcan:CAN���
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_CAN1_CLK_ENABLE();                //ʹ��CAN1ʱ��
		__HAL_RCC_CAN3_CLK_ENABLE();                //ʹ��CAN3ʱ��
    __HAL_RCC_GPIOA_CLK_ENABLE();			    //����GPIOAʱ��
		__HAL_RCC_GPIOB_CLK_ENABLE();			    //����GPIOBʱ��
	
    GPIO_Initure.Pin=GPIO_PIN_11|GPIO_PIN_12;   //PA11,12
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //���츴��
    GPIO_Initure.Pull=GPIO_PULLUP;              //����
    GPIO_Initure.Speed=GPIO_SPEED_FAST;         //����
    GPIO_Initure.Alternate=GPIO_AF9_CAN1;       //����ΪCAN1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);         //��ʼ��
	
		GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4;     //PB3,4
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //���츴��
    GPIO_Initure.Pull=GPIO_PULLUP;              //����
    GPIO_Initure.Speed=GPIO_SPEED_FAST;         //����
    GPIO_Initure.Alternate=GPIO_AF11_CAN3;       //����ΪCAN3
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);         //��ʼ��
    
#if CAN1_RX0_INT_ENABLE
    __HAL_CAN_ENABLE_IT(&CAN1_Handler,CAN_IT_FMP0);//FIFO0��Ϣ�Һ��ж�����.	  
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn,1,2);    //��ռ���ȼ�1�������ȼ�2
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);          //ʹ���ж�
#endif	

#if CAN3_RX0_INT_ENABLE
    __HAL_CAN_ENABLE_IT(&CAN3_Handler,CAN_IT_FMP0);//FIFO0��Ϣ�Һ��ж�����.	  
    HAL_NVIC_SetPriority(CAN3_RX0_IRQn,1,2);    //��ռ���ȼ�1�������ȼ�2
    HAL_NVIC_EnableIRQ(CAN3_RX0_IRQn);          //ʹ���ж�
#endif	
}

#if CAN3_RX0_INT_ENABLE                         //ʹ��RX0�ж�
//CAN�жϷ�����
void CAN3_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&CAN3_Handler);//�˺��������CAN_Receive_IT()��������
}
#endif

#if CAN1_RX0_INT_ENABLE                         //ʹ��RX0�ж�
//CAN�жϷ�����
void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&CAN1_Handler);//�˺��������CAN_Receive_IT()��������
}
#endif	


//CAN�жϴ������
//�˺����ᱻCAN_Receive_IT()����
//hcan:CAN���
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
		u8 i=0, j=0;
		u8 checkbit=0;
		//change by mch 20180531 ���߼���display.c��ʵ��
//		//���δ���յ�CAN3����չ֡���ݣ���ExtIdΪ0
//		if(CAN3_Handler.pRxMsg->ExtId == 0)
//		{
//			counter_fan++;
//		}
//		if(counter_fan==10)
//		{
//			counter_fan=0;
//			flag_fan = 0;
//		}
//		//add by mch 20180524
//		if(CAN1_Handler.pRxMsg->ExtId == 0)
//		{
//			counter_run++;
//		}
//		if(counter_run==10)
//		{
//			counter_run=0;
//			flag_run = 0;
//			flag_getmileage = 1;     //�޷����յ����ߵ��CAN���ݣ���������������ϵ磬��Ҫ���»�ȡ��׼��ʻ���
//		}
		
    //CAN_Receive_IT()������ر�FIFO0��Ϣ�Һ��жϣ����������Ҫ���´�
    __HAL_CAN_ENABLE_IT(&CAN1_Handler,CAN_IT_FMP0);//���¿���FIF00��Ϣ�Һ��ж�
		__HAL_CAN_ENABLE_IT(&CAN3_Handler,CAN_IT_FMP0);//���¿���FIF00��Ϣ�Һ��ж�

	//�����Ǳ�׼֡���ж�
	if(CAN1_Handler.pRxMsg->IDE == CAN_ID_STD)                    //ȷ�����յ���׼֡����
	{
		switch(CAN1_Handler.pRxMsg->StdId & 0X7FF)
		{			
			case 0X01E1:                                       //��ȡ��ص�ѹ������������
				 BatteryVoltage = (CAN1_Handler.pRxMsg->Data[0] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 BatteryCurrent = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[3];
				 BatteryQuantity = (CAN1_Handler.pRxMsg->Data[4] << 8) | CAN1_Handler.pRxMsg->Data[5];
			break;
				
			case 0x01E4:                                       //��ȡ������������������״̬������߱����ȼ�
				Charge_Info = CAN1_Handler.pRxMsg->Data[0];
				Highest_Alarm_Level = CAN1_Handler.pRxMsg->Data[6];
			break;
			
			case 0x01F5:                                       //��ȡ��ر�����ͱ����ȼ�
				for(i=0;i<8;i++)
			  {
					Alarm_Code[i] = CAN1_Handler.pRxMsg->Data[i];
				}
//				Alarm_Code = CAN1_Handler.pRxMsg->Data[0];
//				Alarm_Level = CAN1_Handler.pRxMsg->Data[1];
			break;
			
			case 0x0353:                                       //��ȡ�����ߡ�����¶�
				BatteryMaxTemp = CAN1_Handler.pRxMsg->Data[0];
			  BatteryMinTemp = CAN1_Handler.pRxMsg->Data[3];
			break;
			
			case 0x03F6:                                       //��ȡ����¶�
				BatteryTemp = CAN1_Handler.pRxMsg->Data[2];
			break;
			
			default: 
				break;
		}
	}
	error_count = 0;
	for(i=0;i<8;i++)
	{
		for(j=0;j<8;j++)
		{
			
			checkbit = Alarm_Code[i] & (1<<j);
			if(checkbit == (1<<j))
			{
				error_code[error_count] = 8*i+j;
				error_count++;
				checkbit=0;
			}
		}
	}
	
	//change by mch 20180524
	if(CAN1_Handler.pRxMsg->IDE == CAN_ID_EXT)                    //ȷ�����յ����ߵ����չ֡����
	{
		switch(CAN1_Handler.pRxMsg->ExtId & 0x1FFFFFFF)
		{
			case 0x10F8109A:
				 RunStatus = CAN1_Handler.pRxMsg->Data[0];
				 RunSpeed = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 RunErrorCode = CAN1_Handler.pRxMsg->Data[3];
			   RunLowPower = CAN1_Handler.pRxMsg->Data[4];
				 RunMileage = (CAN1_Handler.pRxMsg->Data[6] << 8) | CAN1_Handler.pRxMsg->Data[5];
			   flag_run = 1;
				 flag_run_item = 1;
				 flag_run_done = 0;
			   flag_init_run = 0;
			   counter_run = 0;
			break;
			
			case 0x10F8108D:
				 RunVoltage = (CAN1_Handler.pRxMsg->Data[1] << 8) | CAN1_Handler.pRxMsg->Data[0];
				 RunCurrent = (CAN1_Handler.pRxMsg->Data[3] << 8) | CAN1_Handler.pRxMsg->Data[2];
				 RunTemp = (CAN1_Handler.pRxMsg->Data[5] << 8) | CAN1_Handler.pRxMsg->Data[4];
	    	 RunCtlTemp = (CAN1_Handler.pRxMsg->Data[7] << 8) | CAN1_Handler.pRxMsg->Data[6];
				 flag_run = 1;
				 flag_run_item = 1;
				 flag_run_done = 0;
		     flag_init_run = 0;
			   counter_run = 0;
			break;
			
			//�����CAN��CAN3�ƶ���CAN1
			case 0x10088A5A:
				 
				 FanVoltage = (CAN1_Handler.pRxMsg->Data[1] << 8) | CAN1_Handler.pRxMsg->Data[0];
				 FanSpeed = (CAN1_Handler.pRxMsg->Data[3] << 8) | CAN1_Handler.pRxMsg->Data[2];
				 FanCurrent = CAN1_Handler.pRxMsg->Data[4];
				 FanTemp = CAN1_Handler.pRxMsg->Data[5];
				 FanErrorCode = CAN1_Handler.pRxMsg->Data[7];
					
				 counter_fan=0;
		     flag_fan = 1;
				 flag_fan_item = 1;
				 flag_fan_done = 0;
			   flag_init_fan = 0;
				
			break;
			
			//��ǰ
			case 0x10F80190:
				 SweeperStatus = CAN1_Handler.pRxMsg->Data[0];
				 SweeperSpeed = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 SweeperErrorCode = CAN1_Handler.pRxMsg->Data[4] | CAN1_Handler.pRxMsg->Data[3];
			   flag_sweeper = 1;
				 flag_sweeper_item = 1;
				 flag_sweeper_done = 0;
			   flag_init_sweeper=0;
			break;
			
			case 0x10F80290:
				 SweeperVoltage = (CAN1_Handler.pRxMsg->Data[1] << 8) | CAN1_Handler.pRxMsg->Data[0];
				 SweeperCurrent = (CAN1_Handler.pRxMsg->Data[3] << 8) | CAN1_Handler.pRxMsg->Data[2];
				 SweeperTemp = (CAN1_Handler.pRxMsg->Data[5] << 8) | CAN1_Handler.pRxMsg->Data[4];
	    	 SweeperCtlTemp = (CAN1_Handler.pRxMsg->Data[7] << 8) | CAN1_Handler.pRxMsg->Data[6];
			   flag_sweeper = 1;
				 flag_sweeper_item = 1;
				 flag_sweeper_done = 0;
			   flag_init_sweeper=0;
			break;
			
			//�Һ�
			case 0x10F80191:
				 SweeperStatus1 = CAN1_Handler.pRxMsg->Data[0];
				 SweeperSpeed1 = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 SweeperErrorCode1 = CAN1_Handler.pRxMsg->Data[4] | CAN1_Handler.pRxMsg->Data[3];
			   flag_sweeper1 = 1;
				 flag_sweeper_item1 = 1;
				 flag_sweeper_done1 = 0;
			   flag_init_sweeper1=0;
			break;
			
			case 0x10F80291:
				 SweeperVoltage1 = (CAN1_Handler.pRxMsg->Data[1] << 8) | CAN1_Handler.pRxMsg->Data[0];
				 SweeperCurrent1 = (CAN1_Handler.pRxMsg->Data[3] << 8) | CAN1_Handler.pRxMsg->Data[2];
				 SweeperTemp1 = (CAN1_Handler.pRxMsg->Data[5] << 8) | CAN1_Handler.pRxMsg->Data[4];
	    	 SweeperCtlTemp1 = (CAN1_Handler.pRxMsg->Data[7] << 8) | CAN1_Handler.pRxMsg->Data[6];
			   flag_sweeper1 = 1;
				 flag_sweeper_item1 = 1;
				 flag_sweeper_done1 = 0;
			   flag_init_sweeper1=0;
			break;
			
			//��ǰ
			case 0x10F80192:
				 SweeperStatus2 = CAN1_Handler.pRxMsg->Data[0];
				 SweeperSpeed2 = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 SweeperErrorCode2 = CAN1_Handler.pRxMsg->Data[4] | CAN1_Handler.pRxMsg->Data[3];
			   flag_sweeper2 = 1;
				 flag_sweeper_item2 = 1;
				 flag_sweeper_done2 = 0;
			   flag_init_sweeper2=0;
			break;
			
			case 0x10F80292:
				 SweeperVoltage2 = (CAN1_Handler.pRxMsg->Data[1] << 8) | CAN1_Handler.pRxMsg->Data[0];
				 SweeperCurrent2 = (CAN1_Handler.pRxMsg->Data[3] << 8) | CAN1_Handler.pRxMsg->Data[2];
				 SweeperTemp2 = (CAN1_Handler.pRxMsg->Data[5] << 8) | CAN1_Handler.pRxMsg->Data[4];
	    	 SweeperCtlTemp2 = (CAN1_Handler.pRxMsg->Data[7] << 8) | CAN1_Handler.pRxMsg->Data[6];
			   flag_sweeper2 = 1;
				 flag_sweeper_item2 = 1;
				 flag_sweeper_done2 = 0;
			   flag_init_sweeper2=0;
			break;
			
			//���
			case 0x10F80193:
				 SweeperStatus3 = CAN1_Handler.pRxMsg->Data[0];
				 SweeperSpeed3 = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 SweeperErrorCode3 = CAN1_Handler.pRxMsg->Data[4] | CAN1_Handler.pRxMsg->Data[3];
			   flag_sweeper3 = 1;
				 flag_sweeper_item3 = 1;
				 flag_sweeper_done3 = 0;
			   flag_init_sweeper3=0;
			break;
			
			case 0x10F80293:
				 SweeperVoltage3 = (CAN1_Handler.pRxMsg->Data[1] << 8) | CAN1_Handler.pRxMsg->Data[0];
				 SweeperCurrent3 = (CAN1_Handler.pRxMsg->Data[3] << 8) | CAN1_Handler.pRxMsg->Data[2];
				 SweeperTemp3 = (CAN1_Handler.pRxMsg->Data[5] << 8) | CAN1_Handler.pRxMsg->Data[4];
	    	 SweeperCtlTemp3 = (CAN1_Handler.pRxMsg->Data[7] << 8) | CAN1_Handler.pRxMsg->Data[6];
			   flag_sweeper3 = 1;
				 flag_sweeper_item3 = 1;
				 flag_sweeper_done3 = 0;
			   flag_init_sweeper3=0;
			break;
			
			//����
			case 0x10F80194:
				 SweeperStatus4 = CAN1_Handler.pRxMsg->Data[0];
				 SweeperSpeed4 = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 SweeperErrorCode4 = CAN1_Handler.pRxMsg->Data[4] | CAN1_Handler.pRxMsg->Data[3];
			   flag_sweeper4 = 1;
				 flag_sweeper_item4 = 1;
				 flag_sweeper_done4 = 0;
			   flag_init_sweeper4=0;
			break;
			
			case 0x10F80294:
				 SweeperVoltage4 = (CAN1_Handler.pRxMsg->Data[1] << 8) | CAN1_Handler.pRxMsg->Data[0];
				 SweeperCurrent4 = (CAN1_Handler.pRxMsg->Data[3] << 8) | CAN1_Handler.pRxMsg->Data[2];
				 SweeperTemp4 = (CAN1_Handler.pRxMsg->Data[5] << 8) | CAN1_Handler.pRxMsg->Data[4];
	    	 SweeperCtlTemp4 = (CAN1_Handler.pRxMsg->Data[7] << 8) | CAN1_Handler.pRxMsg->Data[6];
			   flag_sweeper4 = 1;
				 flag_sweeper_item4 = 1;
				 flag_sweeper_done4 = 0;
			   flag_init_sweeper4=0;
			break;
			
			default:
			break;
			
		}
	}
	
	//��������չ֡���ж�
//	if(CAN3_Handler.pRxMsg->IDE == CAN_ID_EXT)                    //ȷ�����յ���չ֡����
//	{
//		switch(CAN3_Handler.pRxMsg->ExtId & 0x1FFFFFFF)
//		{
//			case 0x10088A5A:
//				 
//				 FanVoltage = (CAN3_Handler.pRxMsg->Data[1] << 8) | CAN3_Handler.pRxMsg->Data[0];
//				 FanSpeed = (CAN3_Handler.pRxMsg->Data[3] << 8) | CAN3_Handler.pRxMsg->Data[2];
//				 FanCurrent = CAN3_Handler.pRxMsg->Data[4];
//				 FanTemp = CAN3_Handler.pRxMsg->Data[5];
//				 FanErrorCode = CAN3_Handler.pRxMsg->Data[7];
//					
//				 counter_fan=0;
//		     flag_fan = 1;
//				 //CAN1_Handler.pRxMsg->ExtId = 0;   //�ֶ�����չ֡ID��0����Ϊ��չ֡����ֻ��һ�����������0����ExtIdһֱ���ڣ�Ϊ0x10088A5A�����´��������ر�����Ϊ�������
//			break;
//			
//			default:
//			break;
//			
//		}
//	}
	
	for(i=0;i<8;i++)
	{
		CAN1_Handler.pRxMsg->Data[i] = 0;
		//CAN3_Handler.pRxMsg->Data[i] = 0;
	}
	//CAN3_Handler.pRxMsg->ExtId = 0;
	CAN1_Handler.pRxMsg->StdId = 0;
	CAN1_Handler.pRxMsg->ExtId = 0;
}

//can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)	
//len:���ݳ���(���Ϊ8)				     
//msg:����ָ��,���Ϊ8���ֽ�.
//����ֵ:0,�ɹ�;
//����,ʧ��;
u8 CAN1_Send_Msg(u8* msg,u8 len)
{	
    u16 i=0;
    CAN1_Handler.pTxMsg->StdId=0X01E1;        //��׼��ʶ��
    //CAN1_Handler.pTxMsg->ExtId=0x12;        //��չ��ʶ��(29λ)
    CAN1_Handler.pTxMsg->IDE=CAN_ID_STD;    //ʹ�ñ�׼֡
    CAN1_Handler.pTxMsg->RTR=CAN_RTR_DATA;  //����֡
    CAN1_Handler.pTxMsg->DLC=len;                
    for(i=0;i<len;i++)
    CAN1_Handler.pTxMsg->Data[i]=msg[i];
    if(HAL_CAN_Transmit(&CAN1_Handler,10)!=HAL_OK) return 1;     //����
    return 0;		
}

//can�ڽ������ݲ�ѯ
//buf:���ݻ�����;	 
//����ֵ:0,�����ݱ��յ�;
//����,���յ����ݳ���;
u8 CAN1_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
    if(HAL_CAN_Receive(&CAN1_Handler,CAN_FIFO0,0)!=HAL_OK) return 0;    //��������	
    for(i=0;i<CAN1_Handler.pRxMsg->DLC;i++)
		buf[i]=CAN1_Handler.pRxMsg->Data[i];
	return CAN1_Handler.pRxMsg->DLC;	
}
