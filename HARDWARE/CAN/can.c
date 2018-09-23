#include "can.h"
#include "usart.h"
#include "delay.h"
#include "user_config.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F7开发板
//CAN驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/12/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									////////////////////////////////////////////////
  
////////////////  //////////////////
CAN_HandleTypeDef   CAN1_Handler;   //CAN1句柄
CAN_HandleTypeDef   CAN3_Handler;   //CAN3句柄

CanTxMsgTypeDef     TxMessage;      //发送消息
CanRxMsgTypeDef     RxMessage;      //接收消息

CanTxMsgTypeDef     Tx3Message;      //CAN3发送消息
CanRxMsgTypeDef     Rx3Message;      //CAN3接收消息

uint16_t FanVoltage=0,              //风机电压
				 FanSpeed=0,                //风机转速
				 FanCtlTemp=0;              //风机电控温度


uint8_t FanCurrent=0,               //风机电流
				FanTemp=50,                 //风机温度
				FanErrorCode=1;             //风机故障码

uint16_t RunSpeed=0,                //行走电机转速
         RunMileage=0,              //行驶里程
				 RunVoltage=0,              //行走电机电压
				 RunCurrent=0,              //行走电机电流
				 RunTemp=0,                 //行走电机温度
				 RunCtlTemp=0;              //行走电控温度

uint8_t RunStatus=0,                //行走电机模式
        RunErrorCode=0,             //行走电机故障码
				RunLowPower=0;              //行走电机低功耗
				
uint16_t SweeperSpeed=0,                //右前扫刷转速
				 SweeperVoltage=0,              //右前扫刷电压
				 SweeperCurrent=0,              //右前扫刷电流
				 SweeperTemp=0,                 //右前扫刷电机温度
				 SweeperCtlTemp=0,              //右前扫刷控制器温度
				 SweeperErrorCode=0;            //右前扫刷故障码

uint8_t SweeperStatus=0;                //右前扫刷工作模式

uint16_t SweeperSpeed1=0,                //右后扫刷转速
				 SweeperVoltage1=0,              //右后扫刷电压
				 SweeperCurrent1=0,              //右后扫刷电流
				 SweeperTemp1=0,                 //右后扫刷电机温度
				 SweeperCtlTemp1=0,              //右后扫刷控制器温度
				 SweeperErrorCode1=0;            //右后扫刷故障码

uint8_t SweeperStatus1=0;                //右后扫刷工作模式

uint16_t SweeperSpeed2=0,                //左前扫刷转速
				 SweeperVoltage2=0,              //左前扫刷电压
				 SweeperCurrent2=0,              //左前扫刷电流
				 SweeperTemp2=0,                 //左前扫刷电机温度
				 SweeperCtlTemp2=0,              //左前扫刷控制器温度
				 SweeperErrorCode2=0;            //左前扫刷故障码

uint8_t SweeperStatus2=0;                //左前扫刷工作模式

uint16_t SweeperSpeed3=0,                //左后扫刷转速
				 SweeperVoltage3=0,              //左后扫刷电压
				 SweeperCurrent3=0,              //左后扫刷电流
				 SweeperTemp3=0,                 //左后扫刷电机温度
				 SweeperCtlTemp3=0,              //左后扫刷控制器温度
				 SweeperErrorCode3=0;            //左后扫刷故障码

uint8_t SweeperStatus3=0;                //左后扫刷工作模式

uint16_t SweeperSpeed4=0,                //中枢扫刷转速
				 SweeperVoltage4=0,              //中枢扫刷电压
				 SweeperCurrent4=0,              //中枢扫刷电流
				 SweeperTemp4=0,                 //中枢扫刷电机温度
				 SweeperCtlTemp4=0,              //中枢扫刷控制器温度
				 SweeperErrorCode4=0;            //中枢扫刷故障码

uint8_t SweeperStatus4=0;                //中枢扫刷工作模式

uint16_t BatteryVoltage=0,          //电池电压
				 BatteryQuantity=0;         //电池电量
				 
uint16_t OBC_Voltage=0,             //电池充电电压
				 OBC_Current=0;             //电池充电电流
				 
int16_t BatteryCurrent=10000;        //电池电流，存在负数
uint8_t BatteryTemp=50;               //电池温度，存在负数	
uint8_t BatteryMaxTemp=50;            //电池最高温度，存在负数	
uint8_t BatteryMinTemp=50;            //电池最低温度，存在负数	

uint8_t Charge_Info=0,              //电池充电标志位
				Highest_Alarm_Level=0,      //电池最高报警等级
				Alarm_Level=0;              //电池故障等级
				//Alarm_Code=255,               //电池故障码
uint8_t Alarm_Code[8];

u8 flag_fan=0;                      //风机开启标志，1代表启动
u16 counter_fan = 0;                //风机CAN接收计数

u8 flag_getmileage = 1;             //默认要读取基准行驶里程
u8 flag_run=0;                      //行走电机开启标志，1代表启动
u16 counter_run = 0;                //行走电机CAN接收计数

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

////CAN初始化
//tsjw:重新同步跳跃时间单元.范围:CAN_SJW_1TQ~CAN_SJW_4TQ
//tbs2:时间段2的时间单元.   范围:CAN_BS2_1TQ~CAN_BS2_8TQ;
//tbs1:时间段1的时间单元.   范围:CAN_BS1_1TQ~CAN_BS1_16TQ
//brp :波特率分频器.范围:1~1024; tq=(brp)*tpclk1
//波特率=Fpclk1/((tbs1+1+tbs2+1+1)*brp);
//mode:CAN_MODE_NORMAL,普通模式;CAN_MODE_LOOPBACK,回环模式;
//Fpclk1的时钟在初始化的时候设置为54M,如果设置CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_11tq,6,CAN_MODE_LOOPBACK);
//则波特率为:54M/((6+11+1)*6)=500Kbps
//返回值:0,初始化OK;
//    其他,初始化失败; 

u8 CAN1_Mode_Init(u32 tsjw,u32 tbs2,u32 tbs1,u16 brp,u32 mode)
{
    CAN_FilterConfTypeDef  CAN1_FilerConf;
    
    CAN1_Handler.Instance=CAN1; 
    CAN1_Handler.pTxMsg=&TxMessage;     //发送消息
    CAN1_Handler.pRxMsg=&RxMessage;     //接收消息
    CAN1_Handler.Init.Prescaler=brp;    //分频系数(Fdiv)为brp+1
    CAN1_Handler.Init.Mode=mode;        //模式设置 
    CAN1_Handler.Init.SJW=tsjw;         //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1TQ~CAN_SJW_4TQ
    CAN1_Handler.Init.BS1=tbs1;         //tbs1范围CAN_BS1_1TQ~CAN_BS1_16TQ
    CAN1_Handler.Init.BS2=tbs2;         //tbs2范围CAN_BS2_1TQ~CAN_BS2_8TQ
    CAN1_Handler.Init.TTCM=DISABLE;     //非时间触发通信模式 
    CAN1_Handler.Init.ABOM=DISABLE;     //软件自动离线管理
    CAN1_Handler.Init.AWUM=DISABLE;     //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
    CAN1_Handler.Init.NART=ENABLE;      //禁止报文自动传送 
    CAN1_Handler.Init.RFLM=DISABLE;     //报文不锁定,新的覆盖旧的 
    CAN1_Handler.Init.TXFP=DISABLE;     //优先级由报文标识符决定 
    if(HAL_CAN_Init(&CAN1_Handler)!=HAL_OK) return 1;   //初始化
    
    CAN1_FilerConf.FilterIdHigh=0X0000;     //32位ID
    CAN1_FilerConf.FilterIdLow=0X0000;
    CAN1_FilerConf.FilterMaskIdHigh=0X0000; //32位MASK
    CAN1_FilerConf.FilterMaskIdLow=0X0000;  
    CAN1_FilerConf.FilterFIFOAssignment=CAN_FILTER_FIFO0;//过滤器0关联到FIFO0
    CAN1_FilerConf.FilterNumber=0;          //过滤器0
    CAN1_FilerConf.FilterMode=CAN_FILTERMODE_IDMASK;
    CAN1_FilerConf.FilterScale=CAN_FILTERSCALE_32BIT;
    CAN1_FilerConf.FilterActivation=ENABLE; //激活滤波器0
    CAN1_FilerConf.BankNumber=14;
    if(HAL_CAN_ConfigFilter(&CAN1_Handler,&CAN1_FilerConf)!=HAL_OK) return 2;//滤波器初始化
    return 0;
}

u8 CAN3_Mode_Init(u32 tsjw,u32 tbs2,u32 tbs1,u16 brp,u32 mode)
{
    CAN_FilterConfTypeDef  CAN3_FilerConf;
    
    CAN3_Handler.Instance=CAN3; 
    CAN3_Handler.pTxMsg=&Tx3Message;     //发送消息
    CAN3_Handler.pRxMsg=&Rx3Message;     //接收消息
    CAN3_Handler.Init.Prescaler=brp;    //分频系数(Fdiv)为brp+1
    CAN3_Handler.Init.Mode=mode;        //模式设置 
    CAN3_Handler.Init.SJW=tsjw;         //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1TQ~CAN_SJW_4TQ
    CAN3_Handler.Init.BS1=tbs1;         //tbs1范围CAN_BS1_1TQ~CAN_BS1_16TQ
    CAN3_Handler.Init.BS2=tbs2;         //tbs2范围CAN_BS2_1TQ~CAN_BS2_8TQ
    CAN3_Handler.Init.TTCM=DISABLE;     //非时间触发通信模式 
    CAN3_Handler.Init.ABOM=DISABLE;     //软件自动离线管理
    CAN3_Handler.Init.AWUM=DISABLE;     //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
    CAN3_Handler.Init.NART=ENABLE;      //禁止报文自动传送 
    CAN3_Handler.Init.RFLM=DISABLE;     //报文不锁定,新的覆盖旧的 
    CAN3_Handler.Init.TXFP=DISABLE;     //优先级由报文标识符决定 
    if(HAL_CAN_Init(&CAN3_Handler)!=HAL_OK) return 1;   //初始化
    
    CAN3_FilerConf.FilterIdHigh=0X0000;     //32位ID
    CAN3_FilerConf.FilterIdLow=0X0000;
    CAN3_FilerConf.FilterMaskIdHigh=0X0000; //32位MASK
    CAN3_FilerConf.FilterMaskIdLow=0X0000;  
    CAN3_FilerConf.FilterFIFOAssignment=CAN_FILTER_FIFO0;//过滤器0关联到FIFO0
    CAN3_FilerConf.FilterNumber=0;          //过滤器0
    CAN3_FilerConf.FilterMode=CAN_FILTERMODE_IDMASK;
    CAN3_FilerConf.FilterScale=CAN_FILTERSCALE_32BIT;
    CAN3_FilerConf.FilterActivation=ENABLE; //激活滤波器0
    CAN3_FilerConf.BankNumber=14;
    if(HAL_CAN_ConfigFilter(&CAN3_Handler,&CAN3_FilerConf)!=HAL_OK) return 2;//滤波器初始化
    return 0;
}

//CAN底层驱动，引脚配置，时钟配置，中断配置
//此函数会被HAL_CAN_Init()调用
//hcan:CAN句柄
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_CAN1_CLK_ENABLE();                //使能CAN1时钟
		__HAL_RCC_CAN3_CLK_ENABLE();                //使能CAN3时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();			    //开启GPIOA时钟
		__HAL_RCC_GPIOB_CLK_ENABLE();			    //开启GPIOB时钟
	
    GPIO_Initure.Pin=GPIO_PIN_11|GPIO_PIN_12;   //PA11,12
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //推挽复用
    GPIO_Initure.Pull=GPIO_PULLUP;              //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;         //快速
    GPIO_Initure.Alternate=GPIO_AF9_CAN1;       //复用为CAN1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);         //初始化
	
		GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4;     //PB3,4
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //推挽复用
    GPIO_Initure.Pull=GPIO_PULLUP;              //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;         //快速
    GPIO_Initure.Alternate=GPIO_AF11_CAN3;       //复用为CAN3
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);         //初始化
    
#if CAN1_RX0_INT_ENABLE
    __HAL_CAN_ENABLE_IT(&CAN1_Handler,CAN_IT_FMP0);//FIFO0消息挂号中断允许.	  
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn,1,2);    //抢占优先级1，子优先级2
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);          //使能中断
#endif	

#if CAN3_RX0_INT_ENABLE
    __HAL_CAN_ENABLE_IT(&CAN3_Handler,CAN_IT_FMP0);//FIFO0消息挂号中断允许.	  
    HAL_NVIC_SetPriority(CAN3_RX0_IRQn,1,2);    //抢占优先级1，子优先级2
    HAL_NVIC_EnableIRQ(CAN3_RX0_IRQn);          //使能中断
#endif	
}

#if CAN3_RX0_INT_ENABLE                         //使能RX0中断
//CAN中断服务函数
void CAN3_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&CAN3_Handler);//此函数会调用CAN_Receive_IT()接收数据
}
#endif

#if CAN1_RX0_INT_ENABLE                         //使能RX0中断
//CAN中断服务函数
void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&CAN1_Handler);//此函数会调用CAN_Receive_IT()接收数据
}
#endif	


//CAN中断处理过程
//此函数会被CAN_Receive_IT()调用
//hcan:CAN句柄
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
		u8 i=0, j=0;
		u8 checkbit=0;
		//change by mch 20180531 将逻辑在display.c中实现
//		//如果未接收到CAN3的扩展帧数据，即ExtId为0
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
//			flag_getmileage = 1;     //无法接收到行走电机CAN数据，代表控制器重新上电，故要重新获取基准行驶里程
//		}
		
    //CAN_Receive_IT()函数会关闭FIFO0消息挂号中断，因此我们需要重新打开
    __HAL_CAN_ENABLE_IT(&CAN1_Handler,CAN_IT_FMP0);//重新开启FIF00消息挂号中断
		__HAL_CAN_ENABLE_IT(&CAN3_Handler,CAN_IT_FMP0);//重新开启FIF00消息挂号中断

	//下面是标准帧的判断
	if(CAN1_Handler.pRxMsg->IDE == CAN_ID_STD)                    //确保接收到标准帧数据
	{
		switch(CAN1_Handler.pRxMsg->StdId & 0X7FF)
		{			
			case 0X01E1:                                       //获取电池电压、电流、电量
				 BatteryVoltage = (CAN1_Handler.pRxMsg->Data[0] << 8) | CAN1_Handler.pRxMsg->Data[1];
				 BatteryCurrent = (CAN1_Handler.pRxMsg->Data[2] << 8) | CAN1_Handler.pRxMsg->Data[3];
				 BatteryQuantity = (CAN1_Handler.pRxMsg->Data[4] << 8) | CAN1_Handler.pRxMsg->Data[5];
			break;
				
			case 0x01E4:                                       //获取充电情况（需解析出充电状态）和最高报警等级
				Charge_Info = CAN1_Handler.pRxMsg->Data[0];
				Highest_Alarm_Level = CAN1_Handler.pRxMsg->Data[6];
			break;
			
			case 0x01F5:                                       //获取电池报警码和报警等级
				for(i=0;i<8;i++)
			  {
					Alarm_Code[i] = CAN1_Handler.pRxMsg->Data[i];
				}
//				Alarm_Code = CAN1_Handler.pRxMsg->Data[0];
//				Alarm_Level = CAN1_Handler.pRxMsg->Data[1];
			break;
			
			case 0x0353:                                       //获取电池最高、最低温度
				BatteryMaxTemp = CAN1_Handler.pRxMsg->Data[0];
			  BatteryMinTemp = CAN1_Handler.pRxMsg->Data[3];
			break;
			
			case 0x03F6:                                       //获取电池温度
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
	if(CAN1_Handler.pRxMsg->IDE == CAN_ID_EXT)                    //确保接收到行走电机扩展帧数据
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
			
			//将风机CAN从CAN3移动到CAN1
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
			
			//右前
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
			
			//右后
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
			
			//左前
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
			
			//左后
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
			
			//中枢
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
	
	//下面是扩展帧的判断
//	if(CAN3_Handler.pRxMsg->IDE == CAN_ID_EXT)                    //确保接收到扩展帧数据
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
//				 //CAN1_Handler.pRxMsg->ExtId = 0;   //手动将扩展帧ID置0，因为扩展帧报文只有一条，如果不清0，则ExtId一直存在，为0x10088A5A，导致错误解析电池报文作为风机参数
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

//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//其他,失败;
u8 CAN1_Send_Msg(u8* msg,u8 len)
{	
    u16 i=0;
    CAN1_Handler.pTxMsg->StdId=0X01E1;        //标准标识符
    //CAN1_Handler.pTxMsg->ExtId=0x12;        //扩展标识符(29位)
    CAN1_Handler.pTxMsg->IDE=CAN_ID_STD;    //使用标准帧
    CAN1_Handler.pTxMsg->RTR=CAN_RTR_DATA;  //数据帧
    CAN1_Handler.pTxMsg->DLC=len;                
    for(i=0;i<len;i++)
    CAN1_Handler.pTxMsg->Data[i]=msg[i];
    if(HAL_CAN_Transmit(&CAN1_Handler,10)!=HAL_OK) return 1;     //发送
    return 0;		
}

//can口接收数据查询
//buf:数据缓存区;	 
//返回值:0,无数据被收到;
//其他,接收的数据长度;
u8 CAN1_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
    if(HAL_CAN_Receive(&CAN1_Handler,CAN_FIFO0,0)!=HAL_OK) return 0;    //接收数据	
    for(i=0;i<CAN1_Handler.pRxMsg->DLC;i++)
		buf[i]=CAN1_Handler.pRxMsg->Data[i];
	return CAN1_Handler.pRxMsg->DLC;	
}
