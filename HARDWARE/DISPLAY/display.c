#include "display.h"
#include "bsp.h"
#include "user_config.h"
#include "math.h"
#include "time.h"
#include "stdio.h"
#include "EmWinHZFont.h"
#include "WM.h"
#include "bmpdisplay.h"

GUI_BITMAP *bitmap;
CI_PARAMETER ci_parameter;
CI_PARAMETER ci_parameter_old;
CAN_SWEEPER rf_sweeper, lf_sweeper, rb_sweeper, lb_sweeper, mid_sweeper;
CAN_SWEEPER rf_sweeper_old, lf_sweeper_old, rb_sweeper_old, lb_sweeper_old, mid_sweeper_old;
CAN_FANMOTOR fan_motor, fan_motor_old;
CAN_RUNMOTOR run_motor, run_motor_old;

uint8_t BEEPN = 0;
u8 weihu_counter = 0;
u8 fault_counter = 0;
u8 flag_key3 = 0;
u8 flag_do2 = 0;
U16 display_init_OK = 0;
uint8_t displayflag = 0;                          //displayflag用于主界面和维护界面的切换
u8 flagbattery = 0;																//电压更新标志
u8 flag_cellcharge=0,flag1=0;
u16 main_count=0,can_counter=0;
u16 main_counter2=0;
u8 flag_charge=0;                                 //0不充电，1充电
u8 flag_fault=0;                                  //维护界面和故障界面的区分标志
u8 flag_fault_click=0;                            //故障界面切换标志
u8 flag_fault_init=0;                             //故障界面框架内容初始化标志，0为未初始化，1为完成初始化
u8 flag_weihu_click=0;                            //维护界面切换标志
u8 flag_weihu_init=0;                             //维护界面框架内容初始化标志，0为未初始化，1为完成初始化
u8 flag_batteryalarm=0;
u8 flag_lessthan12=0;
u8 flag_beep_close1=0;
u8 flag_beep_close2=0;
u8 flag_beep_close3=0;
u8 beep_count = 0;
u8 beep_count2 = 0;
u8 beep_count3 = 0;
//flag2用于控制Key2计数满50次后，不反复切换页面
//flag3用于控制两个界面不重复初始化，导致界面出现跳闪情况
//flag21用于控制Key3计数满50次后，不反复切换页面
//flag31用于控制两个界面不重复初始化，导致界面出现跳闪情况
uint8_t flag2=0, flag3=1;
uint8_t flag21=0, flag31=1;
u32 Base_OdoMeter;                  //里程基准值
const char BMS_ErrorCode[64][32] = {
	                      "充电单体高压                   ",
												"放电单体高压                   ",
												"充电单体低压                   ",
												"放电单体低压                   ",
												"充电总压高                     ",
												"放电总压高                     ",
												"充电总压低                     ",
												"放电总压低                     ",
												"充电总压差                     ",
												"放电总压差                     ",
												"充电压差                       ",
												"放电压差                       ",
												"充电高温                       ",
												"放电高温                       ",
												"充电低温                       ",
												"放电低温                       ",
												"充电温差                       ",
												"放电温差                       ",
												"快充过流                       ",
												"慢充过流                       ",
												"回馈过流                       ",
												"持续放电过流                   ",
												"瞬时放电过流                   ",
												"SOC高                          ",
												"SOC低                          ",
												"漏电                           ",
												"充电加热过温                   ",
												"放电加热过温                   ",
												"充电加热温差过大               ",
												"放电加热温差过大               ",
												"充电加热超时                   ",
												"放电加热超时                   ",
												"充电加热过流                   ",
												"放电加热过流                   ",
												"SOC跳变                        ",
												"供电电压过低                   ",
												"充电极柱过温                   ",
												"放电极柱过温                   ",
												"充电极柱温差过大               ",
												"放电极柱温差过大               ",
												"电压排线                       ",
												"温感排线                       ",
												"内网通信                       ",
												"直流充电正插座温度             ",
												"直流充电负插座温度             ",
												"交流充电相A/L插座温度          ",
												"交流充电相B/N插座温度          ",
												"交流充电相C插座温度            ",
												"充电机通信中断                 ",
												"整车通信中断                   ",
												"满充诊断                       ",
												"充电插座温感异常               ",
												"预充失败                       ",
												"电流异常                       ",
												"BMS初始化故障                  ",
												"HVIL故障                       ",
												"继电器故障                     ",
												"加热故障                       ",
												"CC2连接故障                    ",
												"CC连接故障                     ",
												"CP连接故障                     ",
												"加热温感异常                   ",
												"极柱温感异常                   ",
												"Test                           "
											 };

const char Fan_ErrorCode[32][32] = {"",                     //共25条
																	"正常                       ",
																	"电池电压低                 ",
																	"U相电流传感器故障          ",
																	"V相电流传感器故障          ",
																	"系统干扰故障               ",
																	"U相故障                    ",
											            "V相故障                    ",
																	"W相故障                    ",
																	"电机温度传感器故障         ",
																	"静态模式下预充电电压低     ",
																	"硬件安全故障               ",
																	"控制器温升故障             ",
	                                "控制器外围设备驱动过载     ",
																	"电池电压过高               ",
																	"电池电压过低               ",
																	"动态模式下预充电电压过低   ",
																	"电流传感器故障1            ",
																	"电流传感器故障2            ",
																	"制动电压过高               ",
                                  "速度传感器故障1            ",
	                                "速度传感器故障2            ",
	                                "速度传感器故障3            ",
                                  "速度传感器故障4            ",
                                  "预留                       "
																	};											 
//
const char Run_ErrorCode[16][32] = {"正常              ",                     //共25条
																	"高踏板故障          ",
																	"预充电故障          ",
																	"过流                ",
																	"控制器过热          ",
	                                "                    ",
																	"电流采样电路故障    ",
	                                "                    ",
																	"BMS故障             ",
											            "电池组欠压          ",
																	"电池组过压          ",
																	"电机过热            ",
	                                "                    ",
																	"加速器故障          "
																	};	

const char Sweeper_ErrorCode[16][32] = {"正常              ",                     //共25条
																	      "高踏板故障          ",
																	      "预充电故障          ",
																	      "过流                ",
																	      "控制器过热          ",
	                                      "                    ",
																	      "电流采样电路故障    ",
	                                      "                    ",
																	      "BMS故障             ",
											                  "电池组欠压          ",
																	      "电池组过压          ",
																	      "电机过热            ",
	                                      "                    ",
																	      "加速器故障          "
																	};	

const char BMPFilePath[16][64] = {
	"1:/PICTURE/Diandong/700car.bmp",
	"1:/PICTURE/Diandong/700dianchi.bmp",
	"1:/PICTURE/Diandong/700fan.bmp",
	"1:/PICTURE/Diandong/700houtui.bmp",
	"1:/PICTURE/Diandong/700jiaosha.bmp",
	"1:/PICTURE/Diandong/700jingguang.bmp",
	"1:/PICTURE/Diandong/700kongbai.bmp",
	"1:/PICTURE/Diandong/700qianjin.bmp",
	"1:/PICTURE/Diandong/700saoshua.bmp",
	"1:/PICTURE/Diandong/700shandian.bmp",
	"1:/PICTURE/Diandong/700shousha.bmp",
	"1:/PICTURE/Diandong/700shuiweidi.bmp",
	"1:/PICTURE/Diandong/700wenduji.bmp",
	"1:/PICTURE/Diandong/700youzhuan.bmp",
	"1:/PICTURE/Diandong/700zuozhuan.bmp"
};

//显示行进速度
void ShowSpeed(void)
{
	//add by mch 20180524 如果行走转速来自CAN
	if(flag_run==1)
	{
		ci_parameter.Speed=(RunSpeed*3.14*0.00054)*60/18.12;//单位km/h
	}
	//add by mch 20180613
	if(ci_parameter.Speed > 0)
	{
		flag_timing = 1;
	}
	else
	{
		flag_timing = 0;
	}
	
	if((display_init_OK==0) || (ci_parameter.Speed!=ci_parameter_old.Speed))
	{
		if(ci_parameter.Speed>99)
		{
			ci_parameter.Speed=99;
		}
		
		GUI_SetFont(&GUI_FontDSDigital96);	
		GUI_SetColor(GUI_WHITE);
		GUI_DispDecAt(0, 280, 240, 2);
		GUI_SetColor(GUI_BLACK);
		GUI_DispDecAt(ci_parameter.Speed, 280, 240, 2);
		ci_parameter_old.Speed=ci_parameter.Speed;
		
	}
	
	//add by mch 20180531 如果行走速度一直不变，则认为行走电机未工作，CAN无数据
//	if(ci_parameter.Speed == ci_parameter_old.Speed)
//	{
//		counter_run++;
//	}
//	if(counter_run==5)
//	{
//		counter_run=0;
//		flag_run = 0;
//		flag_getmileage = 1;     //无法接收到行走电机CAN数据，代表控制器重新上电，故要重新获取基准行驶里程
//	}
}

//显示行走电机故障
void ShowRunError(void)
{
	if(RunErrorCode > 0 && RunErrorCode < 14)
	{
		GUI_SetColor(GUI_RED);
		GUI_SetFont(&GUI_Font24_ASCII);
		GUI_DispDecAt(RunErrorCode, 400, 290, 2);
		GUI_DispStringAt("ERR", 400, 260);
	}
	else
	{
		GUI_SetColor(GUI_WHITE);
		GUI_FillRect(390,250,450,320);
	}
}
//显示扫刷转速
void ShowSweeperSpeed(void)
{
	ci_parameter.SweeperSpeed = SweeperSpeed;  //显示右前扫刷转速
	if((display_init_OK==0) || (ci_parameter.SweeperSpeed!=ci_parameter_old.SweeperSpeed))
	{
		GUI_SetFont(&GUI_FontDSDigital32);	
		GUI_SetColor(GUI_BLACK);
		GUI_DispDecAt(ci_parameter.SweeperSpeed, 520, 380, 3);	
		ci_parameter_old.SweeperSpeed=ci_parameter.SweeperSpeed;
	}
}

//显示风机转速
void ShowFanSpeed(void)
{
	ci_parameter.FanSpeed = FanSpeed;
	if((display_init_OK==0) || ci_parameter.FanSpeed!=ci_parameter_old.FanSpeed)
	{
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_FontDSDigital32);	
		GUI_DispDecAt(ci_parameter.FanSpeed, 510, 211, 4);
		ci_parameter_old.FanSpeed=ci_parameter.FanSpeed;
	}
	/*
	//add by mch 20180531 如果风机转速一直不变，则认为风机未工作，CAN无数据
	if(ci_parameter.FanSpeed == ci_parameter_old.FanSpeed)
	{
		counter_fan++;
	}
	if(counter_fan==5)
	{
		counter_fan=0;
		flag_fan = 0;
	}
	*/
}

//显示风机故障
void ShowFanError(void)
{
	if(FanErrorCode > 1 && FanErrorCode < 24)
	{
		GUI_SetColor(GUI_RED);
		GUI_SetFont(&GUI_Font24_ASCII);
		GUI_DispDecAt(FanErrorCode, 625, 150, 2);
		GUI_DispStringAt("ERR", 625, 120);
	}
	else
	{
		GUI_SetColor(GUI_WHITE);
		GUI_FillRect(622,110,670,180);
	}
}

//显示室温
void ShowRoomTemp(int x, int y)//注意tt为实际温度×10后取整
{
	short t;
	int tempture;

	if((display_init_OK==0) || (display_init_OK==1 && ci_parameter.RoomTemp!=ci_parameter_old.RoomTemp))
	{	
		t=ci_parameter.RoomTemp;
		if(t>850)
		{
			t=850;
		}

		if(t<0)
		{
				GUI_SetColor(GUI_BLACK);
				GUI_SetFont(&GUI_Font32_ASCII);
				GUI_DispStringAt("-", x-8, y);				
				t=fabs(t);
		}
		else
		{
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_FontDSDigital30);
			GUI_DispStringAt("-", x-8, y);			
		}
		
		tempture=(int)ci_parameter.RoomTemp;
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_FontDSDigital30);
		GUI_DispDecAt(tempture/10, x, y, 2);	  //显示室温整数部分 //
		GUI_DispDecAt(tempture%10, x+37, y, 1);	//显示室温小数部分 //

		ci_parameter_old.RoomTemp=ci_parameter.RoomTemp;
	}
}

//显示工作时间
void ShowWorktime(void)//worktime的计算待完善，测试程序只显示本次开机运行时间，单位s
{
	uint32_t t;
	
	ci_parameter.WorkTime=minute;
	if((display_init_OK==0) || (display_init_OK==1 && ci_parameter.WorkTime!=ci_parameter_old.WorkTime))
		{
		t=ci_parameter.WorkTime;
			
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_FontDSDigital30);
		GUI_DispDecAt(t/60, 485, 432, 5);	        //显示工作时间整数部分 //
		GUI_DispDecAt((t%60)/6, 568, 432, 1);    	//显示工作时间小数部分 //
		ci_parameter_old.WorkTime=ci_parameter.WorkTime;
		}
}

//显示行驶里程数
void ShowOdometer(void)
{
	uint32_t t;
	//add by mch 20180524 如果行驶里程来自CAN
	if(flag_run == 1)
	{
		ci_parameter.OdoMeter = Base_OdoMeter+RunMileage*100;
		if(RunMileage%2==0)                    //如果小计里程达到200米，就将里程数写入EEPROM
		{
			AT24CXX_WriteLenByte(4,ci_parameter.OdoMeter,4);
		}
	}
	t=ci_parameter.OdoMeter;///1000;
	if((display_init_OK==0) || (display_init_OK==1 && t!=ci_parameter_old.OdoMeter))
	{	

		if(t>999999)
		{
			 t=999999;
		}
		//保留1位小数
		GUI_SetFont(&GUI_FontDSDigital30);	
		GUI_SetColor(GUI_BLACK);
//		GUI_DispDecAt((t*10)/10, 137, 432, 5);	
//		GUI_DispDecAt((t*10)%10, 221, 432, 1);
		//GUI_DispDecAt(RunMileage, 137, 402, 5);
		GUI_DispDecAt((t/100)/10, 137, 432, 5);	
		GUI_DispDecAt((t/100)%10, 221, 432, 1);
		ci_parameter_old.OdoMeter=t;
	}
}

//显示左转向灯
void ShowLeftLight(void)
{
	U16 value;
	value=ci_parameter.LeftLight;
	//与前向灯等指示灯状态不同，判断条件中添加了displayflag == 0(主界面)，因为左转灯的状态是在定时器中显示的，不会区分主界面和维护界面，故要对其进行限制
	if(((display_init_OK==0) || (display_init_OK==1 && value!=ci_parameter_old.LeftLight)) && (displayflag == 0))
	{	
		//指示灯有信号则显示图片，否则显示空白图片
		if (value==1)
		{
			//GUI_DrawBitmap(&bm700zuozhuan, 	10, 15);
			bmptest(ZUOZHUAN, (u8 *)"1:/PICTURE/Diandong/700zuozhuan.bmp", 10, 15);
		}
		else
		{
			//GUI_DrawBitmap(&bm700kongbai, 	10, 15);
			bmptest(KONGBAI, (u8 *)"1:/PICTURE/Diandong/700kongbai.bmp", 10, 15);
			//bmptest(ZUOZHUAN, (u8 *)"1:/PICTURE/Diandong/700zuozhuan.bmp", 10, 15);
			//GUI_DrawBitmap(&bm700zuozhuan, 	10, 15);
		}			
		ci_parameter_old.LeftLight=value;
	}
}

//显示右转向灯
void ShowRightLight(void)
{
	U16 value;
	value=ci_parameter.RightLight;
	//与前向灯等指示灯状态不同，判断条件中添加了displayflag == 0(主界面)，因为右转灯的状态是在定时器中显示的，不会区分主界面和维护界面，故要对其进行限制
	if(((display_init_OK==0) || (display_init_OK==1 && value!=ci_parameter_old.RightLight)) && (displayflag == 0))
	{
		if (value==1)
		{
			//GUI_DrawBitmap(&bm700youzhuan, 	591, 15);
			bmptest(YOUZHUAN, (u8 *)"1:/PICTURE/Diandong/700youzhuan.bmp", 591, 15);
		}
		else
		{
			bmptest(KONGBAI, (u8 *)"1:/PICTURE/Diandong/700kongbai.bmp", 591, 15);
			//GUI_DrawBitmap(&bm700kongbai, 	591, 15);
			//bmptest(YOUZHUAN, (u8 *)"1:/PICTURE/Diandong/700youzhuan.bmp", 591, 15);
			//GUI_DrawBitmap(&bm700youzhuan, 	591, 15);
		}
		ci_parameter_old.RightLight=value;
	}
}


//指示灯统一显示函数，只在主界面显示
/*
@parameter lightstatus: 指示灯现状态
@parameter lightstatus_old: 指示灯原状态
@parameter picname: 图片名称
@parameter level: 判断电平状态
@parameter x: 横坐标
@parameter y: 纵坐标
*/
void Show_Light(u16 lightstatus, u16 lightstatus_old, PICNAME picname, u16 level, u16 x, u16 y)
{
		if((displayflag == 0) && ((display_init_OK == 0 ) || (lightstatus!=lightstatus_old)))
		{	
			if (lightstatus == level)
			{
				bmptest(picname, (u8 *)BMPFilePath[picname], x, y);
				if(picname == 3)              //如果是后退灯
				{
					if(P_DO2 == 0)              //原先是MP5画面
					{
						DisOUT2(1);               //P_DO2置高
						flag_do2 = 1;             //DO2标志置1
					}
					P_VideoSwitch(1);           //切为倒车影像画面
				}
			}
			else
			{
				bmptest(KONGBAI, (u8 *)BMPFilePath[KONGBAI], x, y);
				if(picname == 3)
				{
					if(flag_do2)
					{
						DisOUT2(0);
						flag_do2 = 0;
						delay_ms(1000);
					}			
					P_VideoSwitch(0);           //切为仪表画面
				}
			}
			lightstatus_old = lightstatus;
		}
}

//指示灯显示第二种方法，直接通过图片C文件绘制，占内存
/*
@parameter lightstatus: 指示灯现状态
@parameter lightstatus_old: 指示灯原状态
@parameter pic: 图片名称, C文件
@parameter level: 判断电平状态
@parameter flag_back: 是否是后退指示灯的判断标志
@parameter x: 横坐标
@parameter y: 纵坐标

void Show_Light2(u8 lightstatus, u8 lightstatus_old, GUI_CONST_STORAGE GUI_BITMAP pic, u8 level, u8 flag_back, u16 x, u16 y)
{
		if((display_init_OK == 0 ) || (lightstatus!=lightstatus_old))
		{	
			if (lightstatus == level)
			{
				GUI_DrawBitmap(&pic, x, y);	
				if(flag_back == 1)              //如果是后退灯
				{
					if(P_DO2 == 0)              //原先是MP5画面
					{
						DisOUT2(1);               //P_DO2置高
						flag_do2 = 1;             //DO2标志置1
					}
					P_VideoSwitch(1);           //切为倒车影像画面
				}
			}
			else
			{
				GUI_DrawBitmap(&bm700kongbai, x, y);     
				if(flag_back == 1)
				{
					if(flag_do2)
					{
						DisOUT2(0);
						flag_do2 = 0;
						delay_ms(1000);
					}			
					P_VideoSwitch(0);           //切为仪表画面
				}
			}
			lightstatus_old = lightstatus;
		}
}
*/



void ShowCar(void)
{
	//GUI_DrawBitmap(&bm700car, 225, 115);	
	bmptest(CAR, (u8 *)"1:/PICTURE/Diandong/700car.bmp", 215, 115);
}

void ShowBatteryAlarm(void)
{
		int i;
		//电池报警情况显示
		if(error_count != error_count_old)
		{
			GUI_SetColor(GUI_WHITE);
			GUI_FillRect(8,150,55,300);
			error_count_old = error_count;
		}
		if(Highest_Alarm_Level)
		{
			
			GUI_SetColor(GUI_RED);
			GUI_SetFont(&GUI_Font24_ASCII);
			GUI_DispStringAt("ERR", 10, 190);
			
			GUI_SetFont(&GUI_FontHZ24);

			switch(Highest_Alarm_Level)
			{
				case 1:
					//GUI_DrawBitmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("1级", 15, 160);			
				break;
				
				case 2:
					//GUI_DrawBivtmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("2级", 15, 160);					
				break;
				
				case 3:
					//GUI_DrawBitmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("3级", 15, 160);
				break;
				
				case 254:
					//GUI_DrawBitmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("异常", 10, 160);
					DisOUT1(0);
				break;
				
				case 255:
					//GUI_DrawBitmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("无效", 10, 160);
				break;
				
				default:
					break;
			}
			
			if(error_count >= 3)
			{
				error_count = 3;
			}
			for(i=0;i<error_count;i++)
			{
				alarm_code = error_code[i];
				GUI_DispDecAt(alarm_code, 20, 215+25*i, 2);
			}
			
		}
		else
		{
			GUI_SetColor(GUI_WHITE);
			GUI_FillRect(8,150,55,300);
		}

}


void ShowBatteryQuantity(void)
{
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_FontDSDigital32);
		if(BatteryQuantity > 0x03E8)
		{
			BatteryQuantity = 1000;
		}
		GUI_DispDecAt((int)(BatteryQuantity*0.1), 99, 80, 3);        //显示电池电量
}

void ShowTemp(uint8_t temp, int x, int y, int num)
{
		int Itemp10;
		GUI_SetFont(&GUI_Font32B_ASCII);
		if(temp < 50)
		{
			GUI_DispStringAt("-", x-10, y);
		}
		else
		{
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringAt("-", x-10, y);	
		}
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_FontDSDigital32); 								   	  //设置字体
		Itemp10=fabs(temp-50);
	  if(temp > 175)
		{
		  GUI_DispStringAt("FF", x, y);
		}
		else
		{
		  GUI_DispDecAt(Itemp10, x, y, num);
		}
}

void ShowBatteryTemp(void)
{
		int Itemp10;
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_Font32B_ASCII);
		if(BatteryTemp < 50)
		{
			GUI_DispStringAt("-", 132, 112);
		}
		GUI_SetFont(&GUI_FontDSDigital32); 								   	  //设置字体
		Itemp10=fabs(BatteryTemp-50);
		//GUI_DispDecAt(BatteryTemp, 75, 85, 3);                  //显示电池温度
	  if(BatteryTemp > 175)
		{
		  GUI_DispStringAt("FF", 142, 112);
		}
		else
		{
		  GUI_DispDecAt(Itemp10, 142, 112, 2);
		}
}

//2018.08.10
void ShowMaxBatTemp(void)
{
		int Itemp10;
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_Font24B_ASCII);
		if(BatteryMaxTemp < 50)
		{
			GUI_DispStringAt("-", 152, 220);
		}
		GUI_SetFont(&GUI_FontDSDigital30); 								   	  //设置字体
		Itemp10=fabs(BatteryMaxTemp-50);
		//GUI_DispDecAt(BatteryTemp, 75, 85, 3);                  //显示电池温度
		if(BatteryMaxTemp > 175)
		{
		  GUI_DispStringAt("FF", 160, 220);
		}
		else
		{
		  GUI_DispDecAt(Itemp10, 160, 220, 2);
		}
}

void ShowMinBatTemp(void)
{
		int Itemp10;
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_Font24B_ASCII);
		if(BatteryMinTemp < 50)
		{
			GUI_DispStringAt("-", 152, 270);
		}
		GUI_SetFont(&GUI_FontDSDigital30); 								   	  //设置字体
		Itemp10=fabs(BatteryMinTemp-50);
		//GUI_DispDecAt(BatteryTemp, 75, 85, 3);                  //显示电池温度
		if(BatteryMinTemp > 175)
		{
		  GUI_DispStringAt("FF", 160, 270);
		}
		else
		{
		  GUI_DispDecAt(Itemp10, 160, 270, 2);
		}
}

void ShowCellCurrent(void)
{
	u16 Icell10;
	ci_parameter.CellCurrent = BatteryCurrent * 0.1;              //精度为0.1
	
	if((display_init_OK==0) || (fabs(ci_parameter.CellCurrent - ci_parameter_old.CellCurrent) >= 0.1))
	{ 
//		Icell10=(int)(ci_parameter.CellCurrent*10);
		
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_Font32B_ASCII);
//		if(BatteryCurrent & 0x8000)        //如果电流是负数就显示"-"
//		{
//			GUI_DispStringAt("-", 41, 333);
//			BatteryCurrent = 0xFFFF - BatteryCurrent + 1;
//		}
//		if(flag_charge==1)
//		{
//			Icell10=OBC_Current;
//		}
//		else
//		{
//			Icell10=(u16)((10000-BatteryCurrent)*0.9);
//			if(BatteryCurrent==0)
//			{
//				Icell10=0;
//			}
//		}
		Icell10=(u16)(fabs(BatteryCurrent-10000)*1.0);//原乘以0.9 2018/08/09
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispDecAt(Icell10/10, 54, 335, 3);	//显示电池电流整数部分 //
		GUI_DispDecAt(Icell10%10, 110, 335, 1);	//显示电池电流小数部分 //
		ci_parameter_old.CellCurrent=ci_parameter.CellCurrent;
	}
}

void ShowBatteryStatus(void)
{
	uint16_t y;
	GUI_SetFont(&GUI_Font32B_ASCII);
	y = 315-(int)(160*BatteryQuantity/1000);

	switch(Charge_Info & 192)       //判断充电状态
	{
		case 0:                       //未充电
			flag_charge = 0;
		break;
		
		case 64:                      //充电中
			flag_charge = 1;
		break;
		
		case 128:                     //充电完成
		break;
		
		case 192:                     //保留
		break;
		
		default:
		break;
	}
	if(flag_charge==1)              // && flag_poweron == 1
	{
		GUI_SetColor(0xFF66CC66);
		GUI_FillRect(72,y,136,315);
		GUI_DrawBitmap(&bm700shandian, 87, 185);	
	}
	else//+非充电状态
	{
		GUI_SetColor(GUI_WHITE);
		GUI_FillRect(72,155,136,y);
		
		if(BatteryQuantity <= 100)
		{
				GUI_SetColor(GUI_RED);
		    GUI_FillRect(72,y,136,315);
		}
		else if(BatteryQuantity <= 250)
		{
				GUI_SetColor(GUI_YELLOW);
		    GUI_FillRect(72,y,136,315);
		}
		else
		{
				GUI_SetColor(0xFF66CC66);
				GUI_FillRect(72,y,136,315);
		}

		//change by mch 20180809
		//修改电池故障报警及电池电量报警的逻辑。只要出现3级故障、电量低于25%零界点或电量低于12%零界点，蜂鸣器报警。
		//而出现3级故障或者电量低于12%零界点，则电池断开供电，且保证不重复断开供电
		if(Highest_Alarm_Level == 3)        //出现3级故障
		{
			flag_batteryalarm = 1;
			if(beep_count < 20)
			{			
				BEEP_CTRL(BEEPN);
				beep_count++;
			}
			else
			{
				if(flag_beep_close1 == 0)
				{
				    BEEP_CTRL(0);
					  flag_beep_close1 = 1;
				}
		
				if(flag_lessthan12 == 0) //如果此时电量大于12%，则断开供电，否则电池已断开供电，无需重复操作
				{	
					DisOUT1(1);      //D01输出低电平，电池断开供电
					
				}						
			}
		}
		else
		{
			flag_batteryalarm = 0;     //无3级故障
			if(flag_beep_close1 == 0)
			{
					BEEP_CTRL(0);
					flag_beep_close1 = 1;
			}
			//flag_beep_close1 = 0;
			if(flag_lessthan12 == 0)   //如果此时电量大于12%，则恢复供电
			{
				DisOUT1(0);
			}
			beep_count = 0;
		}
		
		if(BatteryQuantity < 250 && BatteryQuantity >= 120)
		{
			if(beep_count2 < 20)
			{			
				BEEP_CTRL(BEEPN);
				flag_beep_close2 = 0;
				beep_count2++;
			}
			else
			{		
        if(flag_beep_close2 == 0)
				{
				    BEEP_CTRL(0);
					  flag_beep_close2 = 1;
				}			
			}
		}
		else
		{
			beep_count2 = 0;
			if(flag_beep_close2 == 0)
			{
					BEEP_CTRL(0);
					flag_beep_close2 = 1;
			}
			//flag_beep_close2 = 0;
		}
		
		if(BatteryQuantity < 120)      //电量小于12%
		{
			flag_lessthan12 = 1;
			if(beep_count3 < 20)
			{			
				BEEP_CTRL(BEEPN);
				flag_beep_close3 = 0;
				beep_count3++;
			}
			else
			{		
				if(flag_beep_close3 == 0)
				{
				    BEEP_CTRL(0);
					  flag_beep_close3 = 1;
				}
				if(flag_batteryalarm == 0)   //如果此时无3级故障，则断开供电，否则无需重复操作
				{
					DisOUT1(1);      //D01输出低电平，电池断开供电
					
				}		
			}
		}
		else
		{
			//add by mch 20180823
			if(flag_beep_close3 == 0)
			{
				   BEEP_CTRL(0);
					 flag_beep_close3 = 1;
			}
			flag_lessthan12 = 0;
			//flag_beep_close3 = 0;
			if(flag_batteryalarm == 0)     //如果此时无3级故障，则恢复供电，否则跳过执行
			{
				DisOUT1(0);
			}
			beep_count3 = 0;
		}
	}
}

//显示电池充电情况
void ShowBattery(void)
{
	uint16_t y;
	GUI_SetFont(&GUI_Font32B_ASCII);

	if(ci_parameter.CellVoltage <60)
	{
	y = 315;
	}
	else if(ci_parameter.CellVoltage > 80)
	{
		y = 155;
	}
	else
	{
		y = 315 - ((ci_parameter.CellVoltage-60) * 160) / 20;
	}
		
//	if((ci_parameter.CellVoltage - ci_parameter_old.CellVoltage > 0.06 )&&(ci_parameter.Speed==0)&&(ci_parameter.SweeperSpeed==0)&&(FanSpeed==0))//+充电状态
//	{//有转速时，不显示充电图标。 不能只通过电压上升进行判断是否为充电状态。如扫刷停机的瞬间，电压会快速上升
//		flag_charge=1;
//	}
//	if ((ci_parameter.CellVoltage - ci_parameter_old.CellVoltage < 0.01 )||(ci_parameter.Speed!=0)||(ci_parameter.SweeperSpeed!=0)||(FanSpeed!=0))//+非充电状态
//	{
//		flag_charge=0;
//	}
	switch(Charge_Info & 192)       //判断充电状态
	{
		case 0:                       //未充电
			flag_charge = 0;
		break;
		
		case 64:                      //充电中
			flag_charge = 1;
		break;
		
		case 128:                     //充电完成
		break;
		
		case 192:                     //保留
		break;
		
		default:
		break;
	}
	if(flag_charge==1)              // && flag_poweron == 1
	{
		GUI_SetColor(0xFF66CC66);
		GUI_FillRect(72,y,136,315);
		GUI_DrawBitmap(&bm700shandian, 87, 185);	
	}
	else//+非充电状态
	{
		GUI_SetColor(GUI_WHITE);
		GUI_FillRect(72,155,136,y);
		
		if(ci_parameter.CellVoltage <= 62)
		{
				GUI_SetColor(GUI_RED);
		    GUI_FillRect(72,y,136,315);
		}
		else if(ci_parameter.CellVoltage <= 65)
		{
				GUI_SetColor(GUI_YELLOW);
		    GUI_FillRect(72,y,136,315);
		}
		else
		{
				GUI_SetColor(0xFF66CC66);
				GUI_FillRect(72,y,136,315);
		}
	}
}

void ShowCellVoltage(void)
{
	int Vcell10;
	ci_parameter.CellVoltage = BatteryVoltage * 0.1;          //精度为0.1
	if((display_init_OK==0) || (main_count>=50) || (fabs(ci_parameter.CellVoltage - ci_parameter_old.CellVoltage) >= 0.2))
		{  //电压新旧压差大于0.2V才更新电压值
			Vcell10=(int)(ci_parameter.CellVoltage*10);
			GUI_SetColor(GUI_BLACK);
			GUI_SetFont(&GUI_FontDSDigital32);
			GUI_DispDecAt(Vcell10/10, 70, 370, 2);	//显示电池电压整数部分 //
			GUI_DispDecAt(Vcell10%10, 110, 370, 1);	//显示电池电压小数部分 //
			
      ci_parameter_old.CellVoltage = ci_parameter.CellVoltage;		
			main_count=0;
	}		
}

//电压显示的统一接口
/*
@parameter volatge: 现在的电压值
@parameter volatge_old: 上一状态的电压值
@parameter num: 显示位数
@parameter x: 横坐标
@parameter y: 纵坐标
*/
void Show_Voltage(u16 volatge, u16 volatge_old, u8 num, u16 x, u16 y)
{
	u8 size;
	
	if((display_init_OK == 0) || (fabs(volatge - volatge_old) > 2))
	{  //电压新旧压差大于0.2V才更新电压值
		if(displayflag)
		{
			GUI_SetFont(&GUI_FontDSDigital30);
			size = 19;
		}
		else
		{
			GUI_SetFont(&GUI_FontDSDigital32);
			size = 20;
		}
	
		GUI_DispDecAt(volatge/10, x, y, num-1);	//显示电池电压整数部分 //
		GUI_DispDecAt(volatge%10, x+(num-1)*size, y, 1);	//显示电池电压小数部分 //
		
		volatge_old = volatge;
	}		
}

//电流显示的统一接口
/*
@parameter volatge: 现在的电流值
@parameter volatge_old: 上一状态的电流值
@parameter offset: 偏移值
@parameter num: 显示位数
@parameter x: 横坐标
@parameter y: 纵坐标
*/
void Show_Current(u16 current, u16 current_old, u16 offset, u8 num, u16 x, u16 y)
{
	u8 size;
	u16 temp;
	
	if((display_init_OK == 0) || (fabs(current - current_old) > 2))
	{  //电压新旧压差大于0.2V才更新电压值
		GUI_SetFont(&GUI_Font32B_ASCII);
		if(current < offset)
		{
			GUI_DispStringAt("-", x-10, y);
		}
		else
		{
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringAt("-", x-10, y);	
		}
		GUI_SetColor(GUI_BLACK);
		if(displayflag)
		{
			GUI_SetFont(&GUI_FontDSDigital30);
			size = 17;
		}
		else
		{
			GUI_SetFont(&GUI_FontDSDigital32);
			size = 19;
		}
		
		temp=fabs(current-offset);
		GUI_DispDecAt(temp/10, x, y, num-1);	                  //显示电池电压整数部分 //
		GUI_DispDecAt(temp%10, x+(num-1)*size, y, 1);	          //显示电池电压小数部分 //
		current_old = current;
	}		
}

//温度显示的统一接口
/*
@parameter temperture: 现在的温度值
@parameter temperture_old: 上一状态的温度值
@parameter offset: 偏移值
@parameter num: 显示位数
@parameter x: 横坐标
@parameter y: 纵坐标
*/
void Show_Temperature(u16 temperture, u16 temperture_old, u16 offset, u8 num, u16 x, u16 y)
{
	u8 size;
	u16 temp;
	
	if((display_init_OK == 0) || (fabs(temperture - temperture_old) > 2))
	{ 
		GUI_SetFont(&GUI_Font32B_ASCII);
		if(temperture < offset)
		{
			GUI_DispStringAt("-", x-10, y);
		}
		else
		{
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringAt("-", x-10, y);	
		}
		GUI_SetColor(GUI_BLACK);
		if(displayflag)
		{
			GUI_SetFont(&GUI_FontDSDigital30);
			size = 17;
		}
		else
		{
			GUI_SetFont(&GUI_FontDSDigital32);
			size = 19;
		}
		
		temp=fabs(temperture-offset);
		GUI_DispDecAt(temp/10, x, y, num-1);	                  //显示整数部分
		GUI_DispDecAt(temp%10, x+(num-1)*size, y, 1);	          //显示小数部分
		temperture_old = temperture;
	}		
}

//转速显示的统一接口
/*
@parameter speed: 现在的转速值
@parameter speed_old: 上一状态的转速值
@parameter flag_device: 决定是扫刷、风机还是行走电机(1:扫刷，2：风机，4：行走电机)
@parameter num: 显示位数
@parameter x: 横坐标
@parameter y: 纵坐标
*/
void Show_Speed(u16 speed, u16 speed_old, u8 flag_device, u8 num, u16 x, u16 y)
{
	u8 bias;
	u8 temp;
	
	temp = speed;
	
	if(flag_device == 1)
	{
		bias = 2;
	}
	else
  {
	  bias = 10;
	}
	
	if(flag_device == 4)    //行走电机
	{
		if(speed > 0)
		{
			flag_timing = 1; //开始计时
		}
		else
		{
			flag_timing = 0;
		}
		temp=(speed*3.14*0.00054)*60/18.12;//单位km/h
		if(temp > 99)
		{
		  temp = 99;
		}
	}
	
	if((display_init_OK == 0) || (fabs(speed - speed_old) > bias))
	{ 
		if(displayflag)
		{
			GUI_SetFont(&GUI_FontDSDigital30);
		}
		else
		{
			GUI_SetFont(&GUI_FontDSDigital32);
		}
		if(flag_device == 4)
		{
			GUI_SetFont(&GUI_FontDSDigital96);
			GUI_SetColor(GUI_WHITE);
			GUI_DispDecAt(0, 280, 240, 2);
		}
		GUI_SetColor(GUI_BLACK);
		GUI_DispDecAt(temp, x, y, num);	                  //显示整数部分
		speed_old = speed;
	}		
}


//故障代码显示的统一接口
/*
@parameter errorcode: 现在的故障代码值
@parameter errorcode_old: 上一状态的故障代码值
@parameter num: 显示位数
@parameter x: 横坐标
@parameter y: 纵坐标
*/
void Show_ErrorCode(u16 errorcode, u16 errorcode_old, u8 num, u16 x, u16 y)
{
	if((display_init_OK == 0) || (errorcode != errorcode_old))
	{
		if(displayflag)
		{
			GUI_SetFont(&GUI_FontDSDigital30);
		}
		else
		{
			GUI_SetFont(&GUI_Font24_ASCII);
		}
		GUI_DispDecAt(errorcode, x, y, num);	
		
		errorcode_old = errorcode;
	}		
}



//CAN总线设备参数初始化的统一接口
/*
@parameter target: 传入的设备对象结构
@parameter volatge: 设置的电压值
@parameter current: 设置的电流值
@parameter speed: 设置的转速值
@parameter temp: 设置的电机温度值
@parameter ctltemp: 设置的控制器温度值
@parameter errorcode: 设置的故障代码值
*/
void Set_InitData(CAN_SWEEPER *target, u16 volatge, u16 current, u16 speed, u16 temp, u16 ctltemp, u16 errorcode)
{
	//目前暂定用CAN_SWEEPER结构体，理想状态是更抽象的CAN设备对象
	target->Voltage = volatge;
	target->Current = current;
	target->Speed = speed;
	target->Temp = temp;
	target->CtlTemp = ctltemp;
	target->ErrorCode = errorcode;
}

//显示摄氏度的图标
void ShowSheShiDu(void)
{
	GUI_DrawBitmap(&bmsheshidu14,382,435);
	//GUI_DrawBitmap(&bmsheshidu14,125,90);
	GUI_DrawBitmap(&bmsheshidu14,175,117);
	GUI_DrawBitmap(&bmsheshidu14,190,225);
	GUI_DrawBitmap(&bmsheshidu14,190,275);
}	

//显示风机图标
void ShowFan(void)
{
	//GUI_DrawBitmap(&bm700fan, 	510, 88);	
	bmptest(FAN, (u8 *)"1:/PICTURE/Diandong/700fan.bmp", 510, 88);
}

//显示扫刷图标
void ShowSweeper(void)
{

	//GUI_DrawBitmap(&bm700saoshua, 	480, 275);	
	//GUI_DrawBitmap(&bm700saoshua, 	570, 275);	
	bmptest(SAOSHUA, (u8 *)"1:/PICTURE/Diandong/700saoshua.bmp", 480, 275);
	bmptest(SAOSHUA, (u8 *)"1:/PICTURE/Diandong/700saoshua.bmp", 570, 275);
}


//初始化故障信息界面的整体框架
void InitFaultInterface(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
		GUI_Clear();
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);

		GUI_DrawLine(0,50,695,50);
		GUI_DrawLine(0,100,695,100);
		//GUI_DrawLine(0,260,695,260);
	  GUI_DrawLine(0,230,695,230);
	
//	  GUI_DrawLine(0,310,695,310);
//		GUI_DrawLine(0,360,695,360);
//		GUI_DrawLine(0,460,695,460);	
		//风机故障
		GUI_DrawLine(0,275,695,275);
		GUI_DrawLine(0,325,695,325);
		GUI_DrawLine(0,375,695,375);
		
		//行走电机故障
		GUI_DrawLine(0,417,695,417);
		
		GUI_DrawLine(145,50,145,230);
		GUI_DrawLine(385,50,385,100);
		GUI_DrawLine(560,50,560,100);
		GUI_DrawLine(145,275,145,375);
		
		GUI_DrawLine(145,417,145,460);
		GUI_DrawLine(245,417,245,460);
		GUI_DrawLine(405,417,405,460);
		GUI_DrawRoundedFrame(0, 5, 695, 475, 6, 3);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("电池故障信息汇总",250,20);
		GUI_DispStringAt("故障码",35,65);
		GUI_DispStringAt("故障名称",25,150);
		GUI_DispStringAt("最高故障等级",400,65);
		
		GUI_DispStringAt("风机故障信息汇总",250,245);
		GUI_DispStringAt("故障码",35,290);
		GUI_DispStringAt("故障名称",25,340);
		
		GUI_DispStringAt("行走电机故障信息汇总",230,387);
		GUI_DispStringAt("故障码",35,430);
		GUI_DispStringAt("故障名称",285,430);
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 		
}

//初始化故障信息界面1的整体框架
void InitFaultInterface1(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
		GUI_Clear();
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);

		//中枢扫刷故障 50
		GUI_DrawLine(0,50,695,50);
		GUI_DrawLine(0,100,695,100);
		GUI_DrawLine(0,150,695,150);
	
		//左前扫刷故障 275
		GUI_DrawLine(0,200,695,200);
		GUI_DrawLine(0,250,695,250);
		GUI_DrawLine(0,300,695,300);
		
		//右前扫刷故障
		GUI_DrawLine(0,350,695,350);
		GUI_DrawLine(0,400,695,400);
		GUI_DrawLine(0,450,695,450);
		
		GUI_DrawLine(145,50,145,150);
		GUI_DrawLine(145,200,145,300);
		GUI_DrawLine(145,350,145,450);
		GUI_DrawRoundedFrame(0, 5, 695, 475, 6, 3);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("中枢扫刷故障信息",250,20);
		GUI_DispStringAt("故障码",35,65);
		GUI_DispStringAt("故障名称",25,115);
		
		GUI_DispStringAt("左前扫刷故障信息",250,170);
		GUI_DispStringAt("故障码",35,215);
		GUI_DispStringAt("故障名称",25,265);
		
		GUI_DispStringAt("右前扫刷故障信息",250,320);
		GUI_DispStringAt("故障码",35,365);
		GUI_DispStringAt("故障名称",25,415);
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 		
}

//初始化故障信息界面2的整体框架
void InitFaultInterface2(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
		GUI_Clear();
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);

		//左后扫刷故障
		GUI_DrawLine(0,50,695,50);
		GUI_DrawLine(0,100,695,100);
		GUI_DrawLine(0,150,695,150);
	
		//右后扫刷故障
		GUI_DrawLine(0,200,695,200);
		GUI_DrawLine(0,250,695,250);
		GUI_DrawLine(0,300,695,300);
		
		//
		//GUI_DrawLine(0,350,695,350);
		//GUI_DrawLine(0,400,695,400);
		//GUI_DrawLine(0,450,695,450);
		
		GUI_DrawLine(145,50,145,150);
		GUI_DrawLine(145,200,145,300);
		//GUI_DrawLine(145,350,145,450);
		GUI_DrawRoundedFrame(0, 5, 695, 475, 6, 3);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("左后扫刷故障信息",250,20);
		GUI_DispStringAt("故障码",35,65);
		GUI_DispStringAt("故障名称",25,115);
		
		GUI_DispStringAt("右后扫刷故障信息",250,170);
		GUI_DispStringAt("故障码",35,215);
		GUI_DispStringAt("故障名称",25,265);
		
		//GUI_DispStringAt("右前扫刷故障信息",250,320);
		//GUI_DispStringAt("故障码",35,365);
		//GUI_DispStringAt("故障名称",25,415);
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 				
}


void Init_Fault(u8 num)
{
	  display_init_OK=0;
		switch(num)
		{
		  case 0:
				//故障界面0, 电池+风机+行走
				InitFaultInterface();
			  break;
			case 1:
				//故障界面1，中枢+左前+右前扫刷
				InitFaultInterface1();
			  break;
			case 2:
				//故障界面2，左后+右后扫刷
				InitFaultInterface2();
			  break;
		}
		flag_fault_init = 1;
}

//显示故障界面的数值
void ShowMsgInFaultInterface(void)
{
	u8 i;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断

	GUI_SetColor(GUI_BLACK);
	//GUI_SetFont(&GUI_Font16_ASCII);
	//电池故障码序列
	//GUI_DispStringAt("00001000 00011000 00000000 00000000 10000000 00000010 00000000 00000001", 175, 70);
//	for(i=0;i<8;i++)
//	{
//		for(j=0;j<8;j++)
//		{
//			
//			checkbit = Alarm_Code[i] & (1<<j);
//			if(checkbit == (1<<j))
//			{
//				error_code[error_count] = 8*i+j;
//				error_count++;
//				checkbit=0;
//			}
//		}
//	}
	
	//电池故障码汇总显示
	GUI_SetFont(&GUI_Font24_ASCII);
//	GUI_DispStringAt("59, 52, 51, 31, 17, 1", 340, 102);
	for(i=0;i<8;i++)
	{
		//GUI_DispDecAt(error_code[i], 157+28*i, 65, 2);
		GUI_DispHexAt(Alarm_Code[7-i], 157+28*i, 65, 2);
	}
//	GUI_DispStringAt("59, 52, 1", 165, 65);                 //测试
	
	//电池最高故障等级
//	GUI_DispHexAt(3, 240, 255, 2);
	GUI_SetFont(&GUI_FontHZ24);
	//故障界面对故障情况说明进行清屏操作
	if(error_count != error_count_old)
	{
		GUI_SetColor(GUI_WHITE);
		GUI_FillRect(200,120,670,220);
		error_count_old = error_count;
	}
	if(Highest_Alarm_Level)
	{
		switch(Highest_Alarm_Level)
		{
			case 0:
				GUI_DispStringAt("无故障", 580, 65);
			break;
			
			case 1:
				GUI_DispStringAt("1级故障", 575, 65);
			break;
			
			case 2:
				GUI_DispStringAt("2级故障", 575, 65);
			break;
			
			case 3:
				GUI_DispStringAt("3级故障", 575, 65);
			break;
			
			case 254:
				GUI_DispStringAt("异常", 590, 65);
			break;
			
			case 255:
				GUI_DispStringAt("无效", 590, 65);
			break;
			
			default:
				break;
		}

		//GUI_DispHexAt(Alarm_Level, 605, 65, 2);
		
		//电池故障情况罗列，测试
//		GUI_DispStringAt("59 CC连接故障", 220, 120);
//		GUI_DispStringAt("52 预充失败", 220, 155);
//		GUI_DispStringAt("1 放电单体高压", 220, 190);
		
		//只罗列3条故障信息
		if(error_count >= 3)
		{
			error_count = 3;
		}
		for(i=0;i<error_count;i++)
		{
			alarm_code = error_code[i];
			GUI_DispDecAt(alarm_code, 220, 120+35*i, 2);
			GUI_DispStringAt(BMS_ErrorCode[alarm_code], 280, 120+35*i);
		}
	}
	
	//电池故障数量
	//GUI_DispDecAt(6, 585, 255, 2);
	
	//风机故障数量
	//GUI_DispHexAt(2, 585, 315, 2);
	
	//风机故障码汇总显示
	
	//GUI_DispStringAt("23, 17", 220, 325);             //测试
	GUI_DispDecAt(FanErrorCode, 220, 290, 3);
	
	
	//风机故障情况罗列
	GUI_SetFont(&GUI_FontHZ24);
	if(FanErrorCode>=24)
	{
		GUI_DispStringAt(Fan_ErrorCode[24], 220, 340);
	}
	else
	{
		GUI_DispStringAt(Fan_ErrorCode[FanErrorCode], 220, 340);
	}
//	GUI_DispStringAt("23 速度传感器故障4", 220, 340);
//	GUI_DispStringAt("17 电流传感器故障1", 220, 420);
	
	//行走电机故障码
	GUI_DispDecAt(RunErrorCode, 182, 430, 2);
	//行走电机故障情况解析
	GUI_DispStringAt(Run_ErrorCode[RunErrorCode], 460, 430);
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

//显示故障界面1的数值
void ShowMsgInFaultInterface1(void)
{
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断

	GUI_SetColor(GUI_BLACK);
  GUI_SetFont(&GUI_Font24_ASCII);
	//中枢扫刷故障码
	GUI_DispDecAt(SweeperErrorCode4, 220, 65, 2);
	
	//左前扫刷故障码
	GUI_DispDecAt(SweeperErrorCode2, 220, 215, 2);
	
	//右前扫刷故障码
	GUI_DispDecAt(SweeperErrorCode, 220, 365, 2);
	
	GUI_SetFont(&GUI_FontHZ24);
	//中枢扫刷故障名称
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode4], 220, 115);
	
	//左前扫刷故障名称
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode2], 220, 265);
	
	//右前扫刷故障名称
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode], 220, 415);
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

//显示故障界面2的数值
void ShowMsgInFaultInterface2(void)
{
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断

	GUI_SetColor(GUI_BLACK);
  GUI_SetFont(&GUI_Font24_ASCII);
	//左后扫刷故障码
	GUI_DispDecAt(SweeperErrorCode3, 220, 65, 2);
	
	//右后扫刷故障码
	GUI_DispDecAt(SweeperErrorCode1, 220, 215, 2);
	
	//右前扫刷故障码
	//GUI_DispDecAt(SweeperErrorCode, 220, 365, 2);
	
	GUI_SetFont(&GUI_FontHZ24);
	//左后扫刷故障名称
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode3], 220, 115);
	
	//右后扫刷故障名称
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode1], 220, 265);
	
	//右前扫刷故障名称
	//GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode], 220, 415);
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}


//初始化维护界面的整体框架
void InitCareInterface(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //设置字体
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);
	  //横线
		GUI_DrawLine(0,57,695,57);
		GUI_DrawLine(0,107,695,107);
		GUI_DrawLine(0,157,695,157);
		GUI_DrawLine(0,207,695,207);
		GUI_DrawLine(0,257,695,257);
		GUI_DrawLine(0,307,695,307);
		GUI_DrawLine(0,357,695,357);
		//竖线
		GUI_DrawLine(182,5,182,107);
	  GUI_DrawLine(182,157,182,407);
		GUI_DrawLine(352,5,352,107);
	  GUI_DrawLine(352,157,352,357);
		GUI_DrawLine(521,5,521,107);
		GUI_DrawLine(521,157,521,357);

		GUI_DrawRoundedFrame(0, 5, 695, 407, 6, 2);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("电 池 相 关 参 数",240,122);
		GUI_DispStringAt("工作电压",22,22);		
		GUI_DispStringAt("工作时间",22,72);			
		GUI_DispStringAt("电池电压",22,172);
		GUI_DispStringAt("电池电流",22,222);
    GUI_DispStringAt("电池电量",22,272);		
		GUI_DispStringAt("故障等级",22,322);
		GUI_DispStringAt("电池故障码",22,372);
		
		GUI_DispStringAt("室内温度",371,22);
		GUI_DispStringAt("行驶里程",371,72);
		//change by mch 20180524
		GUI_DispStringAt("电池状态",371,172);
		GUI_DispStringAt("平均温度",371,222);	
		GUI_DispStringAt("最低温度",371,272);
		GUI_DispStringAt("最高温度",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("软件版本：V 2.1",550,427);     //2.1代表EEPROM为C02，2.0代表型号为C256
	  GUI_DispStringAt("软件版本：V 6.1",550,427);//2018年8月24日
		GUI_DispStringAt("页 1",30,427);//2018年8月24日
		
		GUI_SetFont(&GUI_Font24_ASCII); //设置字体
		GUI_DispStringAt("V",305,22);                    //工作电压
		GUI_DispStringAt("h",305,72);                    //工作时间
		GUI_DispStringAt("V",305,172);                   //电池电压
		GUI_DispStringAt("A",305,222);                   //电池电流
		
		GUI_DrawBitmap(&bmsheshidu14,623,22);		         //室内温度
		GUI_DispStringAt("km",639,72);                   //行驶里程
		GUI_DrawBitmap(&bmsheshidu14,640,222);           //平均温度
		GUI_DrawBitmap(&bmsheshidu14,640,272);           //最低温度
		GUI_DrawBitmap(&bmsheshidu14,640,322);		       //最高温度
		
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,17);                    
		GUI_DispStringAt(".",583,17);                    
		GUI_DispStringAt(".",278,67);                   
		GUI_DispStringAt(".",613,67); 
    GUI_DispStringAt(".",258,167);		
		GUI_DispStringAt(".",262,217);
		//GUI_DispStringAt(".",593,217);   
    //GUI_DispStringAt(".",593,267); 	                 
		//GUI_DispStringAt(".",593,317);                  
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 		
}

//初始化维护界面1的整体框架
void InitCareInterface1(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //设置字体
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);
		GUI_DrawLine(0,57,695,57);
		GUI_DrawLine(0,107,695,107);
		GUI_DrawLine(0,157,695,157);
		GUI_DrawLine(0,207,695,207);
		GUI_DrawLine(0,257,695,257);
		GUI_DrawLine(0,307,695,307);
		GUI_DrawLine(0,357,695,357);		
		
		GUI_DrawLine(182,57,182,407);
		GUI_DrawLine(352,5,352,407);
		GUI_DrawLine(521,57,521,407);
		
		GUI_DrawRoundedFrame(0, 5, 695, 407, 6, 2);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("风机相关参数",90,22);
		GUI_DispStringAt("风机电压",22,72);		
		GUI_DispStringAt("风机电流",22,122);		
		GUI_DispStringAt("风机转速",22,172);		
		GUI_DispStringAt("电机温度",22,222);
		GUI_DispStringAt("控制器温度",22,272);
    GUI_DispStringAt("风机故障码",22,322);	
		
		GUI_DispStringAt("行走相关参数",436,22);
		GUI_DispStringAt("行走电压",371,72);
		GUI_DispStringAt("行走电流",371,122);		
		GUI_DispStringAt("行走速度",371,172);		
		GUI_DispStringAt("电机温度",371,222);
		GUI_DispStringAt("控制器温度",371,272);			
		GUI_DispStringAt("行走故障码",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("软件版本：V 2.1",550,427);     //2.1代表EEPROM为C02，2.0代表型号为C256
	  GUI_DispStringAt("软件版本：V 6.1",550,427);//2018年6月24日
		GUI_DispStringAt("页 2",30,427);//2018年8月24日
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //设置字体
		GUI_DispStringAt("V",305,72);                    //风机电压
		GUI_DispStringAt("A",305,122);                    //风机电流
		GUI_DispStringAt("r/min",287,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //室内温度 --> 电池温度
		
		GUI_DispStringAt("V",640,72);                    //风机电压
		GUI_DispStringAt("A",640,122);                    //风机电流
		GUI_DispStringAt("km/h",627,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //室内温度 --> 电池温度
		
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		GUI_DispStringAt(".",579,67);                    //sweeper current
		//GUI_DispStringAt(".",262,117);
		GUI_DispStringAt(".",595,117);                   //working voltage
		//GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		GUI_DispStringAt(".",595,217);                   //cell current --> run current
		//GUI_DispStringAt(".",262,267);                   //行走电控温度
		GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 		
}

//初始化维护界面2的整体框架
void InitCareInterface2(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //设置字体
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);
		GUI_DrawLine(0,57,695,57);
		GUI_DrawLine(0,107,695,107);
		GUI_DrawLine(0,157,695,157);
		GUI_DrawLine(0,207,695,207);
		GUI_DrawLine(0,257,695,257);
		GUI_DrawLine(0,307,695,307);
		GUI_DrawLine(0,357,695,357);		
		
		GUI_DrawLine(182,57,182,407);
		GUI_DrawLine(352,5,352,407);
		GUI_DrawLine(521,57,521,407);
		
		GUI_DrawRoundedFrame(0, 5, 695, 407, 6, 2);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("左前扫刷参数",90,22);
		GUI_DispStringAt("扫刷电压",22,72);		
		GUI_DispStringAt("扫刷电流",22,122);		
		GUI_DispStringAt("扫刷转速",22,172);		
		GUI_DispStringAt("电机温度",22,222);
		GUI_DispStringAt("控制器温度",22,272);
    GUI_DispStringAt("扫刷故障码",22,322);	
		
		GUI_DispStringAt("右前扫刷参数",436,22);
		GUI_DispStringAt("扫刷电压",371,72);
		GUI_DispStringAt("扫刷电流",371,122);		
		GUI_DispStringAt("扫刷转速",371,172);		
		GUI_DispStringAt("电机温度",371,222);
		GUI_DispStringAt("控制器温度",371,272);			
		GUI_DispStringAt("扫刷故障码",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("软件版本：V 2.1",550,427);     //2.1代表EEPROM为C02，2.0代表型号为C256
	  GUI_DispStringAt("软件版本：V 6.1",550,427);//2018年6月24日
		GUI_DispStringAt("页 3",30,427);//2018年8月24日
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //设置字体
		GUI_DispStringAt("V",305,72);                    //风机电压
		GUI_DispStringAt("A",305,122);                    //风机电流
		GUI_DispStringAt("r/min",287,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //室内温度 --> 电池温度
		
		GUI_DispStringAt("V",640,72);                    //风机电压
		GUI_DispStringAt("A",640,122);                    //风机电流
		GUI_DispStringAt("r/min",627,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //室内温度 --> 电池温度
		
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		GUI_DispStringAt(".",579,67);                    //sweeper current
		GUI_DispStringAt(".",262,117);
		GUI_DispStringAt(".",595,117);                   //working voltage
		GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		GUI_DispStringAt(".",595,217);                   //cell current --> run current
		GUI_DispStringAt(".",262,267);                   //行走电控温度
		GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 			
}

//初始化维护界面3的整体框架
void InitCareInterface3(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //设置字体
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);
		GUI_DrawLine(0,57,695,57);
		GUI_DrawLine(0,107,695,107);
		GUI_DrawLine(0,157,695,157);
		GUI_DrawLine(0,207,695,207);
		GUI_DrawLine(0,257,695,257);
		GUI_DrawLine(0,307,695,307);
		GUI_DrawLine(0,357,695,357);		
		
		GUI_DrawLine(182,57,182,407);
		GUI_DrawLine(352,5,352,407);
		GUI_DrawLine(521,57,521,407);
		
		GUI_DrawRoundedFrame(0, 5, 695, 407, 6, 2);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("左后扫刷参数",90,22);
		GUI_DispStringAt("扫刷电压",22,72);		
		GUI_DispStringAt("扫刷电流",22,122);		
		GUI_DispStringAt("扫刷转速",22,172);		
		GUI_DispStringAt("电机温度",22,222);
		GUI_DispStringAt("控制器温度",22,272);
    GUI_DispStringAt("扫刷故障码",22,322);	
		
		GUI_DispStringAt("右后扫刷参数",436,22);
		GUI_DispStringAt("扫刷电压",371,72);
		GUI_DispStringAt("扫刷电流",371,122);		
		GUI_DispStringAt("扫刷转速",371,172);		
		GUI_DispStringAt("电机温度",371,222);
		GUI_DispStringAt("控制器温度",371,272);			
		GUI_DispStringAt("扫刷故障码",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("软件版本：V 2.1",550,427);     //2.1代表EEPROM为C02，2.0代表型号为C256
	  GUI_DispStringAt("软件版本：V 6.1",550,427);//2018年6月24日
		GUI_DispStringAt("页 4",30,427);//2018年8月24日
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //设置字体
		GUI_DispStringAt("V",305,72);                    //风机电压
		GUI_DispStringAt("A",305,122);                    //风机电流
		GUI_DispStringAt("r/min",287,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //室内温度 --> 电池温度
		
		GUI_DispStringAt("V",640,72);                    //风机电压
		GUI_DispStringAt("A",640,122);                    //风机电流
		GUI_DispStringAt("r/min",627,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //室内温度 --> 电池温度
		
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		GUI_DispStringAt(".",579,67);                    //sweeper current
		GUI_DispStringAt(".",262,117);
		GUI_DispStringAt(".",595,117);                   //working voltage
		GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		GUI_DispStringAt(".",595,217);                   //cell current --> run current
		GUI_DispStringAt(".",262,267);                   //行走电控温度
		GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 	
}

//初始化维护界面4的整体框架
void InitCareInterface4(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //设置字体
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)
		GUI_SetPenSize(2);
		GUI_DrawLine(0,57,695,57);
		GUI_DrawLine(0,107,695,107);
		GUI_DrawLine(0,157,695,157);
		GUI_DrawLine(0,207,695,207);
		GUI_DrawLine(0,257,695,257);
		GUI_DrawLine(0,307,695,307);
		GUI_DrawLine(0,357,695,357);		
		
		GUI_DrawLine(182,57,182,407);
		GUI_DrawLine(352,5,352,407);
		GUI_DrawLine(521,57,521,407);
		
		GUI_DrawRoundedFrame(0, 5, 695, 407, 6, 2);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("中枢扫刷参数",90,22);
		GUI_DispStringAt("扫刷电压",22,72);		
		GUI_DispStringAt("扫刷电流",22,122);		
		GUI_DispStringAt("扫刷转速",22,172);		
		GUI_DispStringAt("电机温度",22,222);
		GUI_DispStringAt("控制器温度",22,272);
    GUI_DispStringAt("扫刷故障码",22,322);	
		/*
		GUI_DispStringAt("右前扫刷参数",436,22);
		GUI_DispStringAt("扫刷电压",371,72);
		GUI_DispStringAt("扫刷电流",371,122);		
		GUI_DispStringAt("扫刷转速",371,172);		
		GUI_DispStringAt("电机温度",371,222);
		GUI_DispStringAt("控制器温度",371,272);			
		GUI_DispStringAt("扫刷故障码",371,322);
		*/
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("软件版本：V 2.1",550,427);     //2.1代表EEPROM为C02，2.0代表型号为C256
	  GUI_DispStringAt("软件版本：V 6.1",550,427);//2018年6月24日
		GUI_DispStringAt("页 5",30,427);//2018年8月24日
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //设置字体
		GUI_DispStringAt("V",305,72);                    //风机电压
		GUI_DispStringAt("A",305,122);                    //风机电流
		GUI_DispStringAt("r/min",287,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //室内温度 --> 电池温度
		/*
		GUI_DispStringAt("V",640,72);                    //风机电压
		GUI_DispStringAt("A",640,122);                    //风机电流
		GUI_DispStringAt("r/min",627,172);               //风机转速
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //风机温度
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //室内温度 --> 电池温度
		*/
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		//GUI_DispStringAt(".",579,67);                    //sweeper current
		GUI_DispStringAt(".",262,117);
		//GUI_DispStringAt(".",595,117);                   //working voltage
		GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		//GUI_DispStringAt(".",595,217);                   //cell current --> run current
		GUI_DispStringAt(".",262,267);                   //行走电控温度
		//GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}


void Init_Care(u8 num)
{
	  display_init_OK=0;
    switch(num)
		{
		  case 0:
				//维护界面0, 基本信息+电池
				InitCareInterface();
			  break;
			case 1:
				//维护界面1，行走电机+风机
				InitCareInterface1();
			  break;
			case 2:
				//维护界面2，左前+右前扫刷
			  InitCareInterface2();
			  break;
			case 3:
				//维护界面3，左后+右后扫刷
				InitCareInterface3();
			  break;
			case 4:
				//维护界面4，中枢扫刷
				InitCareInterface4();
			  break;
		}
		flag_weihu_init = 1;
}

//参数清零，并写入E2PROM中
void ProcessUserKey1(void)
{
	if( p_disio_in_calc.P_ODOReset_Counter>20)//长按4s以上 4秒改为2秒
	{		

		p_disio_in_calc.P_ODOReset_Counter=0;
		minute=0;//里程清零
		ci_parameter.OdoMeter=0;//reset worktime
		AT24CXX_WriteLenByte(0,minute,4);
		AT24CXX_WriteLenByte(4,ci_parameter.OdoMeter,4);
		
		minute = AT24CXX_ReadLenByte(0,4);   //获取开机时间的数值
		ci_parameter.OdoMeter = AT24CXX_ReadLenByte(4,4);  //获取里程值
		Base_OdoMeter = AT24CXX_ReadLenByte(4,4);  //获取基准里程值
		ci_parameter.WorkTime=minute;
		
	}
}

//跳转界面
void ProcessUserKey2(void)
{
//	if( p_disio_in_calc.P_ODOKey2_Counter>50)//长按5s以上
//			ShowMsgInCareInterface();
}


//
void ProcessUserKey3(void)
{
	if(flag_shortpress == 1)               //短按，P_DO2电平翻转，切换视频
	{
		if((P_UserKey3 == 1) && (flag_key3 == 0)&&(ci_parameter.BackWardLight==0) && (flag_longpress == 0))
		{
			if(displayflag == 0)
			{
				DisOUT2_Toggle;
		  	
			}
			if(displayflag == 1)
			{
				//add by mch 20180824
				weihu_counter++;
				flag_weihu_click = weihu_counter % 5;
				flag_weihu_init = 0;
				Init_Care(flag_weihu_click);
				if(weihu_counter == 100)
				{
				  weihu_counter = 0;
				}
			}
			if(displayflag == 2)
			{
			  //add by mch 20180824
				fault_counter++;
				flag_fault_click = fault_counter % 3;
				flag_fault_init = 0;
				Init_Fault(flag_fault_click);
				if(fault_counter == 100)
				{
				  fault_counter = 0;
				}
			}
			
			flag_key3 = 1;
			flag_shortpress = 0;
		}
	}

}

//显示维护界面的数值
void ShowMsgInCareInterface(void)
{
	char* str;
	int i;
	u16 Icell10;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//工作电压
	GUI_DispDecAt((int)ci_parameter.WorkVoltage, 217, 17, 2);	
	GUI_DispDecAt((int)((ci_parameter.WorkVoltage-(int)ci_parameter.WorkVoltage)*10), 254, 17, 1);
	
	//工作时间
	GUI_DispDecAt(ci_parameter.WorkTime/60, 203, 67, 5);	
	GUI_DispDecAt((ci_parameter.WorkTime%60)/6, 285, 67, 1);
	
	//电池电压
	GUI_DispDecAt(BatteryVoltage/10, 227, 167, 2);
	GUI_DispDecAt(BatteryVoltage%10, 264, 167, 1);

	//电池电流
	Icell10=(u16)(fabs(BatteryCurrent-10000)*1.0);
	GUI_DispDecAt(Icell10/10, 217, 217, 3);
	GUI_DispDecAt(Icell10%10, 268, 217, 1);
	
	//电池电量
	GUI_DispDecAt(BatteryQuantity/10, 240, 267, 3);	
	
	//故障等级
	GUI_SetFont(&GUI_FontHZ24);
	switch(Highest_Alarm_Level)
	{
		case 0:
			GUI_DispStringAt("无故障", 230, 320);			
		  break;
		case 1:
			GUI_DispStringAt("1级", 230, 320);			
		  break;
		case 2:
			GUI_DispStringAt("2级", 230, 320);
		  break;
		case 3:
			GUI_DispStringAt("3级", 230, 320);
		  break;
		case 254:
			GUI_DispStringAt("异常", 230, 320);
		  break;
		case 255:
			GUI_DispStringAt("无效", 230, 320);
		  break;			
		default:
			break;
	}
	
	//电池状态
	if(Charge_Info)
	{
		GUI_DispStringAt("充电", 575, 170);
	}
	else
	{
		GUI_DispStringAt("放电", 575, 170);
	}
	
	GUI_SetFont(&GUI_FontDSDigital30);
	//室内温度
	//GUI_DispDecAt(BatteryVoltage/10, 552, 17, 2);
	//GUI_DispDecAt(BatteryVoltage%10, 588, 17, 1);	
	ShowRoomTemp(552, 17);
	
	//行驶里程
	GUI_DispDecAt((ci_parameter.OdoMeter/100)/10, 537, 67, 5);	
	GUI_DispDecAt((ci_parameter.OdoMeter/100)%10, 619, 67, 1);

	//平均温度
	ShowTemp(BatteryTemp, 565, 217, 3);
	//GUI_DispDecAt(BatteryTemp/10, 547, 217, 3);
	//GUI_DispDecAt(BatteryTemp%10, 598, 217, 1);
	
	//最低温度
	ShowTemp(BatteryMinTemp, 565, 267, 3);
	//GUI_DispDecAt(BatteryMinTemp/10, 547, 267, 3);
	//GUI_DispDecAt(BatteryMinTemp%10, 598, 267, 1);

	//最高温度
	ShowTemp(BatteryMaxTemp, 565, 317, 3);
	//GUI_DispDecAt(BatteryMaxTemp/10, 547, 317, 3);
	//GUI_DispDecAt(BatteryMaxTemp%10, 598, 317, 1);
	
	//电池故障码
	GUI_SetFont(&GUI_Font32_ASCII);
	for(i=0;i<8;i++)
	{
		GUI_DispHexAt(Alarm_Code[7-i], 222+40*i, 3697, 2);
	}
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

//显示维护界面1的数值
void ShowMsgInCareInterface1(void)
{
	char* str;
	int i;
	u16 Icell10;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//风机电压
	GUI_DispDecAt(FanVoltage/10, 217, 67, 2);	
	GUI_DispDecAt(FanVoltage%10, 254, 67, 1);
	
	//风机电流
	GUI_DispDecAt(FanCurrent, 217, 117, 3);
	//GUI_DispDecAt(FanCurrent%10, 268, 267, 1);
	
	//风机转速
	//GUI_DispDecAt(FanSpeed, 210, 167, 4);
	Show_Speed(fan_motor.Speed, fan_motor_old.Speed, 2, 4, 210, 167);              //风机转速
	
	//风机温度
	ShowTemp(FanTemp, 217, 217, 3);
	//GUI_DispDecAt(FanTemp, 217, 217, 3);
	//GUI_DispDecAt(FanTemp%10, 268, 267, 1);
	
	//控制器温度
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_DispStringAt("None",217,270);
	GUI_SetFont(&GUI_FontDSDigital30);
	//GUI_DispDecAt(RunCtlTemp/10, 217, 267, 3);
	//GUI_DispDecAt(RunCtlTemp%10, 268, 267, 1);
	
	///风机故障代码
	if(FanErrorCode > 0 && FanErrorCode < 24)
	{
		GUI_DispDecAt(FanErrorCode, 230, 317, 2);
	}		
	
	//行走电压
	GUI_DispDecAt(RunVoltage/10, 547, 67, 2);	
	GUI_DispDecAt(RunVoltage%10, 584, 67, 1);
	
	//行走电流
	GUI_DispDecAt(RunCurrent/10, 547, 117, 3);
	GUI_DispDecAt(RunCurrent%10, 602, 117, 1);
	
	//行走速度
	GUI_DispDecAt(ci_parameter.Speed, 560, 167, 2);
	
	//电机温度
	GUI_DispDecAt(RunTemp/10, 547, 217, 3);
	GUI_DispDecAt(RunTemp%10, 602, 217, 1);
	
	//控制器温度
	GUI_DispDecAt(RunCtlTemp/10, 547, 267, 3);
	GUI_DispDecAt(RunCtlTemp%10, 602, 267, 1);
	
	///行走故障代码
	if(RunErrorCode < 14)
	{
		GUI_DispDecAt(RunErrorCode, 560, 317, 2);
	}		
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

//显示维护界面2的数值
void ShowMsgInCareInterface2(void)
{
	char* str;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//左前扫刷电压
	GUI_DispDecAt(SweeperVoltage2/10, 217, 67, 2);	
	GUI_DispDecAt(SweeperVoltage2%10, 254, 67, 1);
	
	//左前扫刷电流
	GUI_DispDecAt(SweeperCurrent2/10, 217, 117, 3);
	GUI_DispDecAt(SweeperCurrent2%10, 268, 117, 1);
	
	//左前扫刷转速
	GUI_DispDecAt(SweeperSpeed2, 217, 167, 3);
	
	//左前扫刷电机温度
	GUI_DispDecAt(SweeperTemp2/10, 217, 217, 3);
	GUI_DispDecAt(SweeperTemp2%10, 268, 217, 1);
	
	//控制器温度
	GUI_DispDecAt(SweeperCtlTemp2/10, 217, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp2%10, 268, 267, 1);
	
	//左前扫刷故障代码
	if(SweeperErrorCode2 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode2, 230, 317, 2);
	}		
	
	//右前扫刷电压
	GUI_DispDecAt(SweeperVoltage/10, 547, 67, 2);	
	GUI_DispDecAt(SweeperVoltage%10, 584, 67, 1);
	
	//右前扫刷电压
	GUI_DispDecAt(SweeperCurrent/10, 547, 117, 3);
	GUI_DispDecAt(SweeperCurrent%10, 602, 117, 1);
	
	//右前扫刷转速
	GUI_DispDecAt(SweeperSpeed, 547, 167, 3);
	
	//右前扫刷电机温度
	GUI_DispDecAt(SweeperTemp/10, 547, 217, 3);
	GUI_DispDecAt(SweeperTemp%10, 602, 217, 1);
	
	//控制器温度
	GUI_DispDecAt(SweeperCtlTemp/10, 547, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp%10, 602, 267, 1);
	
	///右前扫刷故障代码
	if(SweeperErrorCode < 24)
	{
		GUI_DispDecAt(SweeperErrorCode, 560, 317, 2);
	}		
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

//显示维护界面3的数值
void ShowMsgInCareInterface3(void)
{
	char* str;
	int i;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//左后扫刷电压
	GUI_DispDecAt(SweeperVoltage3/10, 217, 67, 2);	
	GUI_DispDecAt(SweeperVoltage3%10, 254, 67, 1);
	
	//左后扫刷电流
	GUI_DispDecAt(SweeperCurrent3/10, 217, 117, 3);
	GUI_DispDecAt(SweeperCurrent3%10, 268, 117, 1);
	
	//左后扫刷转速
	GUI_DispDecAt(SweeperSpeed3, 217, 167, 3);
	
	//左后扫刷电机温度
	GUI_DispDecAt(SweeperTemp3/10, 217, 217, 3);
	GUI_DispDecAt(SweeperTemp3%10, 268, 217, 1);
	
	//控制器温度
	GUI_DispDecAt(SweeperCtlTemp3/10, 217, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp3%10, 268, 267, 1);
	
	//左后扫刷故障代码
	if(SweeperErrorCode3 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode3, 230, 317, 2);
	}		
	
	//右后扫刷电压
	GUI_DispDecAt(SweeperVoltage1/10, 547, 67, 2);	
	GUI_DispDecAt(SweeperVoltage1%10, 584, 67, 1);
	
	//右后扫刷电压
	GUI_DispDecAt(SweeperCurrent1/10, 547, 117, 3);
	GUI_DispDecAt(SweeperCurrent1%10, 602, 117, 1);
	
	//右后扫刷电压
	GUI_DispDecAt(SweeperSpeed1, 547, 167, 3);
	
	//右后扫刷电机温度
	GUI_DispDecAt(SweeperTemp1/10, 547, 217, 3);
	GUI_DispDecAt(SweeperTemp1%10, 602, 217, 1);
	
	//控制器温度
	GUI_DispDecAt(SweeperCtlTemp1/10, 547, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp1%10, 602, 267, 1);
	
	//右后扫刷故障代码
	if(SweeperErrorCode1 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode1, 560, 317, 2);
	}		
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

//显示维护界面4的数值
void ShowMsgInCareInterface4(void)
{
	char* str;
	int i;
	u16 Icell10;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//中枢扫刷电压
	GUI_DispDecAt(SweeperVoltage4/10, 217, 67, 2);	
	GUI_DispDecAt(SweeperVoltage4%10, 254, 67, 1);
	
	//中枢扫刷电流
	GUI_DispDecAt(SweeperCurrent4/10, 217, 117, 3);
	GUI_DispDecAt(SweeperCurrent4%10, 268, 117, 1);
	
	//中枢扫刷转速
	GUI_DispDecAt(SweeperSpeed4, 217, 167, 3);
	
	//中枢扫刷电机温度
	GUI_DispDecAt(SweeperTemp4/10, 217, 217, 3);
	GUI_DispDecAt(SweeperTemp4%10, 268, 217, 1);
	
	//控制器温度
	GUI_DispDecAt(SweeperCtlTemp4/10, 217, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp4%10, 268, 267, 1);
	
	//中枢扫刷故障代码
	if(SweeperErrorCode4 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode4, 230, 317, 2);
	}		
	/*
	//右前扫刷电压
	GUI_DispDecAt(RunVoltage/10, 547, 67, 2);	
	GUI_DispDecAt(RunVoltage%10, 584, 67, 1);
	
	//右前扫刷电压
	GUI_DispDecAt(RunCurrent/10, 547, 117, 3);
	GUI_DispDecAt(RunCurrent%10, 602, 117, 1);
	
	//右前扫刷电压
	GUI_DispDecAt(ci_parameter.Speed, 547, 167, 3);
	
	//右前扫刷电机温度
	GUI_DispDecAt(RunTemp/10, 547, 217, 3);
	GUI_DispDecAt(RunTemp%10, 602, 217, 1);
	
	//控制器温度
	GUI_DispDecAt(RunCtlTemp/10, 547, 267, 3);
	GUI_DispDecAt(RunCtlTemp%10, 602, 267, 1);
	
	///右前扫刷故障代码
	if(RunErrorCode >= 0 && RunErrorCode < 14)
	{
		GUI_DispDecAt(RunErrorCode, 560, 317, 2);
	}		
	*/
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

//显示主界面各参数的单位
void ShowAll(void)
{
 	//GUI_DrawBitmap(&bm700wenduji, 295, 410);	 //显示温度计图标
	bmptest(WENDUJI, (u8 *)"1:/PICTURE/Diandong/700wenduji.bmp", 295, 410);
	bmptest(DIANCHI, (u8 *)"1:/PICTURE/Diandong/700dianchi.bmp", 60, 120);
	ShowCar();
	ShowFan();
	ShowSweeper();
	ShowSheShiDu();
	ShowBatteryTemp();
	ShowMaxBatTemp();
	ShowMinBatTemp();
	ShowBatteryQuantity();
	
	GUI_SetFont(&GUI_Font32B_ASCII);
	GUI_DispStringAt("Km/h",295,340);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringAt("r/min",580,216);
	GUI_DispStringAt("r/min",575,385);
	GUI_DispStringAt("A",130,340);
	GUI_DispStringAt("V",130,376);
	GUI_DispStringAt("SOC:",45,85);
	GUI_DispStringAt("%",150,85);
	
	GUI_SetFont(&GUI_Font20B_ASCII);		
	GUI_DispStringAt("ODO",90,440);
	GUI_DispStringAt("Km",240,440);

	GUI_DispStringAt("HOUR",425,440);
	GUI_DispStringAt("h",587,440);
	
	GUI_DispStringAt("MAX",164,202);
	GUI_DispStringAt("MIN",166,252);
	
	GUI_SetFont(&GUI_FontDSDigital32);
	GUI_DispStringAt(".",104,335);//cell current
	GUI_DispStringAt(".",104,370);//cell voltage
	GUI_DispStringAt(".",360,430);//room tempture
	GUI_DispStringAt(".",214,430);//odo
	GUI_DispStringAt(".",561,430);//workingtime
	
}


//主界面初始化
void InitMainInterface(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断

		if(display_init_OK==0)
		{
			minute = AT24CXX_ReadLenByte(0,4);   //获取开机时间的数值
			ci_parameter.OdoMeter = AT24CXX_ReadLenByte(4,4);  //获取里程值
		}

		GUI_Clear();
		GUI_SetFont(&GUI_Font24_ASCII); //设置字体
		GUI_SetColor(GUI_BLACK);       //设置前景色(如文本，画线等颜色)

		ci_parameter.CellVoltage=0;
		ci_parameter.CellCurrent=0;
		ci_parameter.SweeperCurrent=0;
		ci_parameter.ChargingLight=0;
		ci_parameter.DippedLight=1;
		ci_parameter.SweeperSpeed=0;
		ci_parameter.HandBrakeLight=0;
		ci_parameter.LeftLight=0;
		ci_parameter.LowWaterWarning=0;
		ci_parameter.RightLight=0;
		ci_parameter.ForwardLight=0;
		ci_parameter.BackWardLight=0;
		ci_parameter.FootBrakeLight=0;
		ci_parameter.FanSpeed=0;

		ci_parameter.RoomTemp=0;
		ci_parameter.Speed=0;
		ci_parameter.WorkVoltage=0;
		ci_parameter.UserKey1=0;
		ci_parameter.UserKey2Debug=0;
		//add by mch 20180823
		BatteryQuantity=1000;
//		Alarm_Code=0;
		
		ci_parameter_old.CellVoltage=1;
		ci_parameter_old.CellCurrent=0;
		ci_parameter_old.ChargingLight=0;
		ci_parameter_old.DippedLight=1;
		ci_parameter_old.SweeperSpeed=0;
		ci_parameter_old.HandBrakeLight=0;
		ci_parameter_old.LeftLight=0;
		ci_parameter_old.LowWaterWarning=0;
		ci_parameter_old.OdoMeter=0;
		ci_parameter_old.RightLight=0;
		ci_parameter_old.ForwardLight=0;
		ci_parameter_old.BackWardLight=0;
		ci_parameter_old.FootBrakeLight=0;
		ci_parameter_old.RoomTemp=0;
		ci_parameter_old.Speed=0;
		ci_parameter_old.WorkTime=0;
		ci_parameter_old.WorkVoltage=0;
		ci_parameter_old.UserKey1=0;
		ci_parameter_old.FanSpeed=0;

		ShowAll();

//		Show_Light(ci_parameter.LeftLight, ci_parameter_old.LeftLight, ZUOZHUAN, 1, 10, 15);                //左转
//		Show_Light(ci_parameter.RightLight, ci_parameter_old.RightLight, YOUZHUAN, 1, 591, 15);             //右转
//		Show_Light(ci_parameter.DippedLight, ci_parameter_old.DippedLight, JINGUANG, 1, 259, 15);           //近光
//		Show_Light(ci_parameter.HandBrakeLight, ci_parameter_old.HandBrakeLight, SHOUSHA, 1, 93, 15);       //手刹
//		Show_Light(ci_parameter.BackWardLight, ci_parameter_old.BackWardLight, HOUTUI, 1, 425, 15);         //后退
//		Show_Light(ci_parameter.ForwardLight, ci_parameter_old.ForwardLight, QIANJIN, 1, 342, 15);          //前进
//		Show_Light(ci_parameter.FootBrakeLight, ci_parameter_old.FootBrakeLight, JIAOSHA, 1, 176, 15);      //脚刹
//		Show_Light(ci_parameter.LowWaterWarning, ci_parameter_old.LowWaterWarning, SHUIWEIDI, 0, 508, 15);  //水位低
		
		Show_Speed(rf_sweeper.Speed, rf_sweeper_old.Speed, 1, 3, 520, 380);            //右前扫刷转速
		Show_Speed(fan_motor.Speed, fan_motor_old.Speed, 2, 4, 510, 211);              //风机转速
		Show_Speed(run_motor.Speed, run_motor_old.Speed, 4, 2, 280, 240);              //行走速度
		
//		ShowSpeed();
//		ShowSweeperSpeed();
//		ShowFanSpeed();
		
		ShowOdometer();
		ShowRoomTemp(328, 430);
		ShowWorktime();

		ShowCellCurrent();
		ShowCellVoltage();
		ShowBatteryAlarm();
		ShowBatteryQuantity();
		ShowBatteryStatus();
		
		main_count=0;
	
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 	
}


//显示主界面
void ShowMsgInMainInterface(void)
{
		//在屏幕绘制的时候，关闭定时器，绘制完成再打开，防止绘制过程中数据更新导致画面的抖动
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);                //关中断
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //关中断
		
		Show_Light(ci_parameter.DippedLight, ci_parameter_old.DippedLight, JINGUANG, 1, 259, 15);           //近光
		Show_Light(ci_parameter.HandBrakeLight, ci_parameter_old.HandBrakeLight, SHOUSHA, 1, 93, 15);       //手刹
		Show_Light(ci_parameter.BackWardLight, ci_parameter_old.BackWardLight, HOUTUI, 1, 425, 15);         //后退
		Show_Light(ci_parameter.ForwardLight, ci_parameter_old.ForwardLight, QIANJIN, 1, 342, 15);          //前进
		Show_Light(ci_parameter.FootBrakeLight, ci_parameter_old.FootBrakeLight, JIAOSHA, 1, 176, 15);      //脚刹
		Show_Light(ci_parameter.LowWaterWarning, ci_parameter_old.LowWaterWarning, SHUIWEIDI, 0, 508, 15);  //水位低

		if(main_count<50)
		{
			main_count++;
		}
		
		ShowBatteryTemp();                                 //显示电池温度
		ShowMaxBatTemp();                                  //显示电池温度
		ShowMinBatTemp();                                  //显示电池温度
		ShowCellVoltage();                                 //显示电池电压
		ShowCellCurrent();                                 //显示电池电流
		ShowBatteryAlarm();                                //显示电池报警情况
		ShowBatteryQuantity();                             //显示电池电量
		ShowBatteryStatus();                               //显示电池充放电情况
		
		Show_Speed(rf_sweeper.Speed, rf_sweeper_old.Speed, 1, 3, 520, 380);            //右前扫刷转速
		Show_Speed(fan_motor.Speed, fan_motor_old.Speed, 2, 4, 510, 211);              //风机转速
		Show_Speed(run_motor.Speed, run_motor_old.Speed, 4, 2, 280, 240);              //行走速度
		
	  ShowRunError();                       //显示行走故障码
		ShowOdometer();                       //显示里程数
		ShowRoomTemp(328, 430);               //显示室内温度
		ShowWorktime();                       //显示工作时间
		ShowFanError();                       //显示风机故障
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //开启ITM6中断 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //开启ITM3中断 
}

void DisplayData(void)
{
	
	uint16_t count = 600, item1 = 0;
	
	u8 buf[8];
	u8 len, res, key, item=0;
	//开机上电时延迟5秒跳转监控界面，让MP5加载完成，而正常软重启则直接跳转监控界面
	item = AT24CXX_ReadLenByte(20,1);
	if(item == 0)
	{
		delay_ms(5000);
		DisOUT2(1);
	}
	else
	{
		DisOUT2(1);
	}
	AT24CXX_WriteLenByte(20,0,1);
	while(1)
	{ 
    rf_sweeper.Speed = 100;
    fan_motor.Speed = 1234;
    run_motor.Speed = 2451;	
		
		if(0)
		{	
			BatteryVoltage=685;
			BatteryCurrent=9354;
			BatteryTemp=74;
			BatteryMaxTemp = 88;
			BatteryMinTemp = 85;
		
			FanSpeed=3784;
			FanVoltage=374;
			FanCurrent=26;
			FanTemp=67;
			FanErrorCode=1;
			
			RunSpeed=1356;
			RunVoltage=236;
			RunCurrent=96;
			RunErrorCode=0;
			RunTemp=345;
			RunCtlTemp=236;
			
			SweeperSpeed=88;
			SweeperVoltage=239;
			SweeperCurrent=68;
			SweeperErrorCode=0;
			SweeperTemp=268;
			SweeperCtlTemp=138;
		
			SweeperSpeed3=78;
			SweeperVoltage3=225;
			SweeperCurrent3=67;
			SweeperErrorCode3=1;
			SweeperTemp3=337;
			SweeperCtlTemp3=269;
			
			ci_parameter.OdoMeter=2386;
			ci_parameter.RoomTemp=278;
			ci_parameter.Speed=18;
		}
		//一旦未接收到FAN的CAN数据,则直接初始化FAN的相关参数
		if(flag_init_fan == 0 && flag_fan == 0)
		{
			FanSpeed=0;
			FanVoltage=0;
			FanCurrent=0;
			FanTemp=50;
			FanErrorCode=1;
			flag_init_fan=1;
		}
		//add by mch 20180524
		//一旦未接收到行走点击的CAN数据,则直接初始化行走电机的相关参数
		if(flag_init_run == 0 && flag_run == 0)
		{
			RunSpeed=0;
			RunVoltage=0;
			RunCurrent=0;
			RunMileage=0;
			RunErrorCode=0;
			RunTemp=0;
			RunCtlTemp=0;
			flag_init_run=1;
		}	
		//add by mch 20180824
		if(flag_init_sweeper == 0 && flag_sweeper == 0)
		{
			SweeperSpeed=0;
			SweeperVoltage=0;
			SweeperCurrent=0;
			SweeperErrorCode=0;
			SweeperTemp=0;
			SweeperCtlTemp=0;
			flag_init_sweeper=1;
		}	
		
		if(flag_init_sweeper1 == 0 && flag_sweeper1 == 0)
		{
			SweeperSpeed1=0;
			SweeperVoltage1=0;
			SweeperCurrent1=0;
			SweeperErrorCode1=0;
			SweeperTemp1=0;
			SweeperCtlTemp1=0;
			flag_init_sweeper1=1;
		}
		
		if(flag_init_sweeper2 == 0 && flag_sweeper2 == 0)
		{
			SweeperSpeed2=0;
			SweeperVoltage2=0;
			SweeperCurrent2=0;
			SweeperErrorCode2=0;
			SweeperTemp2=0;
			SweeperCtlTemp2=0;
			flag_init_sweeper2=1;
		}
		
		if(flag_init_sweeper3 == 0 && flag_sweeper3 == 0)
		{
			SweeperSpeed3=0;
			SweeperVoltage3=0;
			SweeperCurrent3=0;
			SweeperErrorCode3=0;
			SweeperTemp3=0;
			SweeperCtlTemp3=0;
			flag_init_sweeper3=1;
		}
		
		if(flag_init_sweeper4 == 0 && flag_sweeper4 == 0)
		{
			SweeperSpeed4=0;
			SweeperVoltage4=0;
			SweeperCurrent4=0;
			SweeperErrorCode4=0;
			SweeperTemp4=0;
			SweeperCtlTemp4=0;
			flag_init_sweeper4=1;
		}
		
		//里程值偏大很大的可能是定时器中判断出行走CAN已断开，故在这重新获取了基准值。而实际上行走未断开！！
		//判断是否中途更新基准值的方法，在定时器中行走断开后增加一个flag，如果置1，就在主界面输出一个字符串！！！
		//解决的办法：1、增加计数值；2、直接不去判断行走CAN是否断开，或者默认行走电机和仪表是同时上下电，不需要中途获取基准值；
		if(flag_getmileage == 1)
		{
			Base_OdoMeter = AT24CXX_ReadLenByte(4,4);  //获取基准里程值
			flag_getmileage = 0;
		}
		
		ProcessUserKey1();                                             //界面参数清零
		ProcessUserKey3();
		
		//判断初始化哪个界面
		//注销掉下面代码可以去掉故障界面和主界面与维护界面的切换关系
		if(flag21 && flag31 && displayflag)   //&displayflag，则无法直接从主界面跳转到故障界面
		{
			if(flag_fault)
			{
				//InitFaultInterface();   //故障信息界面的初始化
				Init_Fault(flag_fault_click);
				displayflag=2;
			}
			else
			{
				//InitCareInterface();                                         //维护界面的初始化
				Init_Care(flag_weihu_click);
				displayflag=1;
			}
		}
		/*
		if(flag_fault == 1 && displayflag==2)
		{
			ShowMsgInFaultInterface();                                     //显示故障信息界面
			display_init_OK=1;
		}
		
		else if(flag_fault == 0 && displayflag==1)
		{
			ShowMsgInCareInterface();                                     //显示维护界面
		}
		*/
		
		if(flag2 && flag3)
		{
			display_init_OK=0;
			if(displayflag)
			{
				InitCareInterface();                                        //维护界面的初始化
				flag_weihu_click = 0;
			}
			else
			{ 
				InitMainInterface();                                    //主界面的初始化
			}
		}
		if(displayflag==0)
		{		
			ShowMsgInMainInterface();                                 //显示主界面
			display_init_OK=1;
		}
		/*
		else if(flag_fault == 0 && displayflag==1)
		{
			ShowMsgInCareInterface();                          //显示维护界面
			
		}
		*/
		//add by mch 20180824 增加维护界面显示的翻页逻辑
		if(flag_fault == 0 && displayflag==1)
		{
			switch(flag_weihu_click)
			{
			  case 0:
					ShowMsgInCareInterface();                          //显示维护界面0
					break;
				case 1:
					ShowMsgInCareInterface1();                          //显示维护界面1
					break;
				case 2:
					ShowMsgInCareInterface2();                          //显示维护界面2
					break;
				case 3:
					ShowMsgInCareInterface3();                          //显示维护界面3
					break;
				case 4:
					ShowMsgInCareInterface4();                          //显示维护界面4
					break;
			}
			display_init_OK=1;
		}

		if(flag_fault == 1 && displayflag==2)
		{
			switch(flag_fault_click)
			{
			  case 0:
					ShowMsgInFaultInterface();                           //显示故障界面0
					break;
				case 1:
					ShowMsgInFaultInterface1();                          //显示故障界面1
					break;
				case 2:
					ShowMsgInFaultInterface2();                          //显示故障界面2
					break;
			}
			display_init_OK=1;
		}
		
		//change by mch 20180624 延迟时间从4秒改为2秒
		if(p_disio_in_calc.P_ODOKey2_Counter>20)             //如果按键2按下超过4秒，则切换界面（主界面和维护界面间）
		{
			//change by mch 20180624
			//故障界面长按key2，不会切回主界面
			if(flag_fault == 0)
			{
					if(flag2==0)
					{
						displayflag = !displayflag;
						flag2 = 1;
						flag3 = 1;
					}	
					else
					{
						flag3=0;
					}
			}	
			
		}
		else
		{
			flag2 = 0;
		}
		
		if(p_disio_in_calc.P_ODOKey3_Counter>30)             //按键3按下超过3秒
		{
			flag_shortpress = 0;
			if(displayflag == 0)               //主界面则软重启
			{
				GUI_Clear();
				GUI_SetFont(&GUI_Font32_ASCII); //设置字体
				GUI_SetColor(GUI_BLACK);
				GUI_DispStringAt("Soft reseting ......", 250, 220);
				AT24CXX_WriteLenByte(20,1,1);
			  delay_ms(1000);
				NVIC_SystemReset();                    //软重启
			}
			else                                //否则切换界面
			{
				if(flag21==0)
				{
					flag_fault = !flag_fault;
					flag21 = 1;
					flag31 = 1;
				}	
				else
				{
					flag31=0;
				}
			}
		}
		else
		{
			flag21 = 0;
		}
		
		delay_ms(200); 

	}

}

