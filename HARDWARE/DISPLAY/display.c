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
uint8_t displayflag = 0;                          //displayflag�����������ά��������л�
u8 flagbattery = 0;																//��ѹ���±�־
u8 flag_cellcharge=0,flag1=0;
u16 main_count=0,can_counter=0;
u16 main_counter2=0;
u8 flag_charge=0;                                 //0����磬1���
u8 flag_fault=0;                                  //ά������͹��Ͻ�������ֱ�־
u8 flag_fault_click=0;                            //���Ͻ����л���־
u8 flag_fault_init=0;                             //���Ͻ��������ݳ�ʼ����־��0Ϊδ��ʼ����1Ϊ��ɳ�ʼ��
u8 flag_weihu_click=0;                            //ά�������л���־
u8 flag_weihu_init=0;                             //ά�����������ݳ�ʼ����־��0Ϊδ��ʼ����1Ϊ��ɳ�ʼ��
u8 flag_batteryalarm=0;
u8 flag_lessthan12=0;
u8 flag_beep_close1=0;
u8 flag_beep_close2=0;
u8 flag_beep_close3=0;
u8 beep_count = 0;
u8 beep_count2 = 0;
u8 beep_count3 = 0;
//flag2���ڿ���Key2������50�κ󣬲������л�ҳ��
//flag3���ڿ����������治�ظ���ʼ�������½�������������
//flag21���ڿ���Key3������50�κ󣬲������л�ҳ��
//flag31���ڿ����������治�ظ���ʼ�������½�������������
uint8_t flag2=0, flag3=1;
uint8_t flag21=0, flag31=1;
u32 Base_OdoMeter;                  //��̻�׼ֵ
const char BMS_ErrorCode[64][32] = {
	                      "��絥���ѹ                   ",
												"�ŵ絥���ѹ                   ",
												"��絥���ѹ                   ",
												"�ŵ絥���ѹ                   ",
												"�����ѹ��                     ",
												"�ŵ���ѹ��                     ",
												"�����ѹ��                     ",
												"�ŵ���ѹ��                     ",
												"�����ѹ��                     ",
												"�ŵ���ѹ��                     ",
												"���ѹ��                       ",
												"�ŵ�ѹ��                       ",
												"������                       ",
												"�ŵ����                       ",
												"������                       ",
												"�ŵ����                       ",
												"����²�                       ",
												"�ŵ��²�                       ",
												"������                       ",
												"�������                       ",
												"��������                       ",
												"�����ŵ����                   ",
												"˲ʱ�ŵ����                   ",
												"SOC��                          ",
												"SOC��                          ",
												"©��                           ",
												"�����ȹ���                   ",
												"�ŵ���ȹ���                   ",
												"�������²����               ",
												"�ŵ�����²����               ",
												"�����ȳ�ʱ                   ",
												"�ŵ���ȳ�ʱ                   ",
												"�����ȹ���                   ",
												"�ŵ���ȹ���                   ",
												"SOC����                        ",
												"�����ѹ����                   ",
												"��缫������                   ",
												"�ŵ缫������                   ",
												"��缫���²����               ",
												"�ŵ缫���²����               ",
												"��ѹ����                       ",
												"�¸�����                       ",
												"����ͨ��                       ",
												"ֱ������������¶�             ",
												"ֱ����縺�����¶�             ",
												"���������A/L�����¶�          ",
												"���������B/N�����¶�          ",
												"���������C�����¶�            ",
												"����ͨ���ж�                 ",
												"����ͨ���ж�                   ",
												"�������                       ",
												"�������¸��쳣               ",
												"Ԥ��ʧ��                       ",
												"�����쳣                       ",
												"BMS��ʼ������                  ",
												"HVIL����                       ",
												"�̵�������                     ",
												"���ȹ���                       ",
												"CC2���ӹ���                    ",
												"CC���ӹ���                     ",
												"CP���ӹ���                     ",
												"�����¸��쳣                   ",
												"�����¸��쳣                   ",
												"Test                           "
											 };

const char Fan_ErrorCode[32][32] = {"",                     //��25��
																	"����                       ",
																	"��ص�ѹ��                 ",
																	"U���������������          ",
																	"V���������������          ",
																	"ϵͳ���Ź���               ",
																	"U�����                    ",
											            "V�����                    ",
																	"W�����                    ",
																	"����¶ȴ���������         ",
																	"��̬ģʽ��Ԥ����ѹ��     ",
																	"Ӳ����ȫ����               ",
																	"��������������             ",
	                                "��������Χ�豸��������     ",
																	"��ص�ѹ����               ",
																	"��ص�ѹ����               ",
																	"��̬ģʽ��Ԥ����ѹ����   ",
																	"��������������1            ",
																	"��������������2            ",
																	"�ƶ���ѹ����               ",
                                  "�ٶȴ���������1            ",
	                                "�ٶȴ���������2            ",
	                                "�ٶȴ���������3            ",
                                  "�ٶȴ���������4            ",
                                  "Ԥ��                       "
																	};											 
//
const char Run_ErrorCode[16][32] = {"����              ",                     //��25��
																	"��̤�����          ",
																	"Ԥ������          ",
																	"����                ",
																	"����������          ",
	                                "                    ",
																	"����������·����    ",
	                                "                    ",
																	"BMS����             ",
											            "�����Ƿѹ          ",
																	"������ѹ          ",
																	"�������            ",
	                                "                    ",
																	"����������          "
																	};	

const char Sweeper_ErrorCode[16][32] = {"����              ",                     //��25��
																	      "��̤�����          ",
																	      "Ԥ������          ",
																	      "����                ",
																	      "����������          ",
	                                      "                    ",
																	      "����������·����    ",
	                                      "                    ",
																	      "BMS����             ",
											                  "�����Ƿѹ          ",
																	      "������ѹ          ",
																	      "�������            ",
	                                      "                    ",
																	      "����������          "
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

//��ʾ�н��ٶ�
void ShowSpeed(void)
{
	//add by mch 20180524 �������ת������CAN
	if(flag_run==1)
	{
		ci_parameter.Speed=(RunSpeed*3.14*0.00054)*60/18.12;//��λkm/h
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
	
	//add by mch 20180531 ��������ٶ�һֱ���䣬����Ϊ���ߵ��δ������CAN������
//	if(ci_parameter.Speed == ci_parameter_old.Speed)
//	{
//		counter_run++;
//	}
//	if(counter_run==5)
//	{
//		counter_run=0;
//		flag_run = 0;
//		flag_getmileage = 1;     //�޷����յ����ߵ��CAN���ݣ���������������ϵ磬��Ҫ���»�ȡ��׼��ʻ���
//	}
}

//��ʾ���ߵ������
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
//��ʾɨˢת��
void ShowSweeperSpeed(void)
{
	ci_parameter.SweeperSpeed = SweeperSpeed;  //��ʾ��ǰɨˢת��
	if((display_init_OK==0) || (ci_parameter.SweeperSpeed!=ci_parameter_old.SweeperSpeed))
	{
		GUI_SetFont(&GUI_FontDSDigital32);	
		GUI_SetColor(GUI_BLACK);
		GUI_DispDecAt(ci_parameter.SweeperSpeed, 520, 380, 3);	
		ci_parameter_old.SweeperSpeed=ci_parameter.SweeperSpeed;
	}
}

//��ʾ���ת��
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
	//add by mch 20180531 ������ת��һֱ���䣬����Ϊ���δ������CAN������
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

//��ʾ�������
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

//��ʾ����
void ShowRoomTemp(int x, int y)//ע��ttΪʵ���¶ȡ�10��ȡ��
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
		GUI_DispDecAt(tempture/10, x, y, 2);	  //��ʾ������������ //
		GUI_DispDecAt(tempture%10, x+37, y, 1);	//��ʾ����С������ //

		ci_parameter_old.RoomTemp=ci_parameter.RoomTemp;
	}
}

//��ʾ����ʱ��
void ShowWorktime(void)//worktime�ļ�������ƣ����Գ���ֻ��ʾ���ο�������ʱ�䣬��λs
{
	uint32_t t;
	
	ci_parameter.WorkTime=minute;
	if((display_init_OK==0) || (display_init_OK==1 && ci_parameter.WorkTime!=ci_parameter_old.WorkTime))
		{
		t=ci_parameter.WorkTime;
			
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_FontDSDigital30);
		GUI_DispDecAt(t/60, 485, 432, 5);	        //��ʾ����ʱ���������� //
		GUI_DispDecAt((t%60)/6, 568, 432, 1);    	//��ʾ����ʱ��С������ //
		ci_parameter_old.WorkTime=ci_parameter.WorkTime;
		}
}

//��ʾ��ʻ�����
void ShowOdometer(void)
{
	uint32_t t;
	//add by mch 20180524 �����ʻ�������CAN
	if(flag_run == 1)
	{
		ci_parameter.OdoMeter = Base_OdoMeter+RunMileage*100;
		if(RunMileage%2==0)                    //���С����̴ﵽ200�ף��ͽ������д��EEPROM
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
		//����1λС��
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

//��ʾ��ת���
void ShowLeftLight(void)
{
	U16 value;
	value=ci_parameter.LeftLight;
	//��ǰ��Ƶ�ָʾ��״̬��ͬ���ж������������displayflag == 0(������)����Ϊ��ת�Ƶ�״̬���ڶ�ʱ������ʾ�ģ����������������ά�����棬��Ҫ�����������
	if(((display_init_OK==0) || (display_init_OK==1 && value!=ci_parameter_old.LeftLight)) && (displayflag == 0))
	{	
		//ָʾ�����ź�����ʾͼƬ��������ʾ�հ�ͼƬ
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

//��ʾ��ת���
void ShowRightLight(void)
{
	U16 value;
	value=ci_parameter.RightLight;
	//��ǰ��Ƶ�ָʾ��״̬��ͬ���ж������������displayflag == 0(������)����Ϊ��ת�Ƶ�״̬���ڶ�ʱ������ʾ�ģ����������������ά�����棬��Ҫ�����������
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


//ָʾ��ͳһ��ʾ������ֻ����������ʾ
/*
@parameter lightstatus: ָʾ����״̬
@parameter lightstatus_old: ָʾ��ԭ״̬
@parameter picname: ͼƬ����
@parameter level: �жϵ�ƽ״̬
@parameter x: ������
@parameter y: ������
*/
void Show_Light(u16 lightstatus, u16 lightstatus_old, PICNAME picname, u16 level, u16 x, u16 y)
{
		if((displayflag == 0) && ((display_init_OK == 0 ) || (lightstatus!=lightstatus_old)))
		{	
			if (lightstatus == level)
			{
				bmptest(picname, (u8 *)BMPFilePath[picname], x, y);
				if(picname == 3)              //����Ǻ��˵�
				{
					if(P_DO2 == 0)              //ԭ����MP5����
					{
						DisOUT2(1);               //P_DO2�ø�
						flag_do2 = 1;             //DO2��־��1
					}
					P_VideoSwitch(1);           //��Ϊ����Ӱ����
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
					P_VideoSwitch(0);           //��Ϊ�Ǳ���
				}
			}
			lightstatus_old = lightstatus;
		}
}

//ָʾ����ʾ�ڶ��ַ�����ֱ��ͨ��ͼƬC�ļ����ƣ�ռ�ڴ�
/*
@parameter lightstatus: ָʾ����״̬
@parameter lightstatus_old: ָʾ��ԭ״̬
@parameter pic: ͼƬ����, C�ļ�
@parameter level: �жϵ�ƽ״̬
@parameter flag_back: �Ƿ��Ǻ���ָʾ�Ƶ��жϱ�־
@parameter x: ������
@parameter y: ������

void Show_Light2(u8 lightstatus, u8 lightstatus_old, GUI_CONST_STORAGE GUI_BITMAP pic, u8 level, u8 flag_back, u16 x, u16 y)
{
		if((display_init_OK == 0 ) || (lightstatus!=lightstatus_old))
		{	
			if (lightstatus == level)
			{
				GUI_DrawBitmap(&pic, x, y);	
				if(flag_back == 1)              //����Ǻ��˵�
				{
					if(P_DO2 == 0)              //ԭ����MP5����
					{
						DisOUT2(1);               //P_DO2�ø�
						flag_do2 = 1;             //DO2��־��1
					}
					P_VideoSwitch(1);           //��Ϊ����Ӱ����
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
					P_VideoSwitch(0);           //��Ϊ�Ǳ���
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
		//��ر��������ʾ
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
					GUI_DispStringAt("1��", 15, 160);			
				break;
				
				case 2:
					//GUI_DrawBivtmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("2��", 15, 160);					
				break;
				
				case 3:
					//GUI_DrawBitmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("3��", 15, 160);
				break;
				
				case 254:
					//GUI_DrawBitmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("�쳣", 10, 160);
					DisOUT1(0);
				break;
				
				case 255:
					//GUI_DrawBitmap(&bm700dianchi_error, 20, 180);
					GUI_DispStringAt("��Ч", 10, 160);
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
		GUI_DispDecAt((int)(BatteryQuantity*0.1), 99, 80, 3);        //��ʾ��ص���
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
		GUI_SetFont(&GUI_FontDSDigital32); 								   	  //��������
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
		GUI_SetFont(&GUI_FontDSDigital32); 								   	  //��������
		Itemp10=fabs(BatteryTemp-50);
		//GUI_DispDecAt(BatteryTemp, 75, 85, 3);                  //��ʾ����¶�
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
		GUI_SetFont(&GUI_FontDSDigital30); 								   	  //��������
		Itemp10=fabs(BatteryMaxTemp-50);
		//GUI_DispDecAt(BatteryTemp, 75, 85, 3);                  //��ʾ����¶�
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
		GUI_SetFont(&GUI_FontDSDigital30); 								   	  //��������
		Itemp10=fabs(BatteryMinTemp-50);
		//GUI_DispDecAt(BatteryTemp, 75, 85, 3);                  //��ʾ����¶�
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
	ci_parameter.CellCurrent = BatteryCurrent * 0.1;              //����Ϊ0.1
	
	if((display_init_OK==0) || (fabs(ci_parameter.CellCurrent - ci_parameter_old.CellCurrent) >= 0.1))
	{ 
//		Icell10=(int)(ci_parameter.CellCurrent*10);
		
		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(&GUI_Font32B_ASCII);
//		if(BatteryCurrent & 0x8000)        //��������Ǹ�������ʾ"-"
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
		Icell10=(u16)(fabs(BatteryCurrent-10000)*1.0);//ԭ����0.9 2018/08/09
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispDecAt(Icell10/10, 54, 335, 3);	//��ʾ��ص����������� //
		GUI_DispDecAt(Icell10%10, 110, 335, 1);	//��ʾ��ص���С������ //
		ci_parameter_old.CellCurrent=ci_parameter.CellCurrent;
	}
}

void ShowBatteryStatus(void)
{
	uint16_t y;
	GUI_SetFont(&GUI_Font32B_ASCII);
	y = 315-(int)(160*BatteryQuantity/1000);

	switch(Charge_Info & 192)       //�жϳ��״̬
	{
		case 0:                       //δ���
			flag_charge = 0;
		break;
		
		case 64:                      //�����
			flag_charge = 1;
		break;
		
		case 128:                     //������
		break;
		
		case 192:                     //����
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
	else//+�ǳ��״̬
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
		//�޸ĵ�ع��ϱ�������ص����������߼���ֻҪ����3�����ϡ���������25%������������12%���㣬������������
		//������3�����ϻ��ߵ�������12%���㣬���ضϿ����磬�ұ�֤���ظ��Ͽ�����
		if(Highest_Alarm_Level == 3)        //����3������
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
		
				if(flag_lessthan12 == 0) //�����ʱ��������12%����Ͽ����磬�������ѶϿ����磬�����ظ�����
				{	
					DisOUT1(1);      //D01����͵�ƽ����ضϿ�����
					
				}						
			}
		}
		else
		{
			flag_batteryalarm = 0;     //��3������
			if(flag_beep_close1 == 0)
			{
					BEEP_CTRL(0);
					flag_beep_close1 = 1;
			}
			//flag_beep_close1 = 0;
			if(flag_lessthan12 == 0)   //�����ʱ��������12%����ָ�����
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
		
		if(BatteryQuantity < 120)      //����С��12%
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
				if(flag_batteryalarm == 0)   //�����ʱ��3�����ϣ���Ͽ����磬���������ظ�����
				{
					DisOUT1(1);      //D01����͵�ƽ����ضϿ�����
					
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
			if(flag_batteryalarm == 0)     //�����ʱ��3�����ϣ���ָ����磬��������ִ��
			{
				DisOUT1(0);
			}
			beep_count3 = 0;
		}
	}
}

//��ʾ��س�����
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
		
//	if((ci_parameter.CellVoltage - ci_parameter_old.CellVoltage > 0.06 )&&(ci_parameter.Speed==0)&&(ci_parameter.SweeperSpeed==0)&&(FanSpeed==0))//+���״̬
//	{//��ת��ʱ������ʾ���ͼ�ꡣ ����ֻͨ����ѹ���������ж��Ƿ�Ϊ���״̬����ɨˢͣ����˲�䣬��ѹ���������
//		flag_charge=1;
//	}
//	if ((ci_parameter.CellVoltage - ci_parameter_old.CellVoltage < 0.01 )||(ci_parameter.Speed!=0)||(ci_parameter.SweeperSpeed!=0)||(FanSpeed!=0))//+�ǳ��״̬
//	{
//		flag_charge=0;
//	}
	switch(Charge_Info & 192)       //�жϳ��״̬
	{
		case 0:                       //δ���
			flag_charge = 0;
		break;
		
		case 64:                      //�����
			flag_charge = 1;
		break;
		
		case 128:                     //������
		break;
		
		case 192:                     //����
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
	else//+�ǳ��״̬
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
	ci_parameter.CellVoltage = BatteryVoltage * 0.1;          //����Ϊ0.1
	if((display_init_OK==0) || (main_count>=50) || (fabs(ci_parameter.CellVoltage - ci_parameter_old.CellVoltage) >= 0.2))
		{  //��ѹ�¾�ѹ�����0.2V�Ÿ��µ�ѹֵ
			Vcell10=(int)(ci_parameter.CellVoltage*10);
			GUI_SetColor(GUI_BLACK);
			GUI_SetFont(&GUI_FontDSDigital32);
			GUI_DispDecAt(Vcell10/10, 70, 370, 2);	//��ʾ��ص�ѹ�������� //
			GUI_DispDecAt(Vcell10%10, 110, 370, 1);	//��ʾ��ص�ѹС������ //
			
      ci_parameter_old.CellVoltage = ci_parameter.CellVoltage;		
			main_count=0;
	}		
}

//��ѹ��ʾ��ͳһ�ӿ�
/*
@parameter volatge: ���ڵĵ�ѹֵ
@parameter volatge_old: ��һ״̬�ĵ�ѹֵ
@parameter num: ��ʾλ��
@parameter x: ������
@parameter y: ������
*/
void Show_Voltage(u16 volatge, u16 volatge_old, u8 num, u16 x, u16 y)
{
	u8 size;
	
	if((display_init_OK == 0) || (fabs(volatge - volatge_old) > 2))
	{  //��ѹ�¾�ѹ�����0.2V�Ÿ��µ�ѹֵ
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
	
		GUI_DispDecAt(volatge/10, x, y, num-1);	//��ʾ��ص�ѹ�������� //
		GUI_DispDecAt(volatge%10, x+(num-1)*size, y, 1);	//��ʾ��ص�ѹС������ //
		
		volatge_old = volatge;
	}		
}

//������ʾ��ͳһ�ӿ�
/*
@parameter volatge: ���ڵĵ���ֵ
@parameter volatge_old: ��һ״̬�ĵ���ֵ
@parameter offset: ƫ��ֵ
@parameter num: ��ʾλ��
@parameter x: ������
@parameter y: ������
*/
void Show_Current(u16 current, u16 current_old, u16 offset, u8 num, u16 x, u16 y)
{
	u8 size;
	u16 temp;
	
	if((display_init_OK == 0) || (fabs(current - current_old) > 2))
	{  //��ѹ�¾�ѹ�����0.2V�Ÿ��µ�ѹֵ
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
		GUI_DispDecAt(temp/10, x, y, num-1);	                  //��ʾ��ص�ѹ�������� //
		GUI_DispDecAt(temp%10, x+(num-1)*size, y, 1);	          //��ʾ��ص�ѹС������ //
		current_old = current;
	}		
}

//�¶���ʾ��ͳһ�ӿ�
/*
@parameter temperture: ���ڵ��¶�ֵ
@parameter temperture_old: ��һ״̬���¶�ֵ
@parameter offset: ƫ��ֵ
@parameter num: ��ʾλ��
@parameter x: ������
@parameter y: ������
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
		GUI_DispDecAt(temp/10, x, y, num-1);	                  //��ʾ��������
		GUI_DispDecAt(temp%10, x+(num-1)*size, y, 1);	          //��ʾС������
		temperture_old = temperture;
	}		
}

//ת����ʾ��ͳһ�ӿ�
/*
@parameter speed: ���ڵ�ת��ֵ
@parameter speed_old: ��һ״̬��ת��ֵ
@parameter flag_device: ������ɨˢ������������ߵ��(1:ɨˢ��2�������4�����ߵ��)
@parameter num: ��ʾλ��
@parameter x: ������
@parameter y: ������
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
	
	if(flag_device == 4)    //���ߵ��
	{
		if(speed > 0)
		{
			flag_timing = 1; //��ʼ��ʱ
		}
		else
		{
			flag_timing = 0;
		}
		temp=(speed*3.14*0.00054)*60/18.12;//��λkm/h
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
		GUI_DispDecAt(temp, x, y, num);	                  //��ʾ��������
		speed_old = speed;
	}		
}


//���ϴ�����ʾ��ͳһ�ӿ�
/*
@parameter errorcode: ���ڵĹ��ϴ���ֵ
@parameter errorcode_old: ��һ״̬�Ĺ��ϴ���ֵ
@parameter num: ��ʾλ��
@parameter x: ������
@parameter y: ������
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



//CAN�����豸������ʼ����ͳһ�ӿ�
/*
@parameter target: ������豸����ṹ
@parameter volatge: ���õĵ�ѹֵ
@parameter current: ���õĵ���ֵ
@parameter speed: ���õ�ת��ֵ
@parameter temp: ���õĵ���¶�ֵ
@parameter ctltemp: ���õĿ������¶�ֵ
@parameter errorcode: ���õĹ��ϴ���ֵ
*/
void Set_InitData(CAN_SWEEPER *target, u16 volatge, u16 current, u16 speed, u16 temp, u16 ctltemp, u16 errorcode)
{
	//Ŀǰ�ݶ���CAN_SWEEPER�ṹ�壬����״̬�Ǹ������CAN�豸����
	target->Voltage = volatge;
	target->Current = current;
	target->Speed = speed;
	target->Temp = temp;
	target->CtlTemp = ctltemp;
	target->ErrorCode = errorcode;
}

//��ʾ���϶ȵ�ͼ��
void ShowSheShiDu(void)
{
	GUI_DrawBitmap(&bmsheshidu14,382,435);
	//GUI_DrawBitmap(&bmsheshidu14,125,90);
	GUI_DrawBitmap(&bmsheshidu14,175,117);
	GUI_DrawBitmap(&bmsheshidu14,190,225);
	GUI_DrawBitmap(&bmsheshidu14,190,275);
}	

//��ʾ���ͼ��
void ShowFan(void)
{
	//GUI_DrawBitmap(&bm700fan, 	510, 88);	
	bmptest(FAN, (u8 *)"1:/PICTURE/Diandong/700fan.bmp", 510, 88);
}

//��ʾɨˢͼ��
void ShowSweeper(void)
{

	//GUI_DrawBitmap(&bm700saoshua, 	480, 275);	
	//GUI_DrawBitmap(&bm700saoshua, 	570, 275);	
	bmptest(SAOSHUA, (u8 *)"1:/PICTURE/Diandong/700saoshua.bmp", 480, 275);
	bmptest(SAOSHUA, (u8 *)"1:/PICTURE/Diandong/700saoshua.bmp", 570, 275);
}


//��ʼ��������Ϣ�����������
void InitFaultInterface(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
		GUI_Clear();
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
		GUI_SetPenSize(2);

		GUI_DrawLine(0,50,695,50);
		GUI_DrawLine(0,100,695,100);
		//GUI_DrawLine(0,260,695,260);
	  GUI_DrawLine(0,230,695,230);
	
//	  GUI_DrawLine(0,310,695,310);
//		GUI_DrawLine(0,360,695,360);
//		GUI_DrawLine(0,460,695,460);	
		//�������
		GUI_DrawLine(0,275,695,275);
		GUI_DrawLine(0,325,695,325);
		GUI_DrawLine(0,375,695,375);
		
		//���ߵ������
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
		GUI_DispStringAt("��ع�����Ϣ����",250,20);
		GUI_DispStringAt("������",35,65);
		GUI_DispStringAt("��������",25,150);
		GUI_DispStringAt("��߹��ϵȼ�",400,65);
		
		GUI_DispStringAt("���������Ϣ����",250,245);
		GUI_DispStringAt("������",35,290);
		GUI_DispStringAt("��������",25,340);
		
		GUI_DispStringAt("���ߵ��������Ϣ����",230,387);
		GUI_DispStringAt("������",35,430);
		GUI_DispStringAt("��������",285,430);
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 		
}

//��ʼ��������Ϣ����1��������
void InitFaultInterface1(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
		GUI_Clear();
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
		GUI_SetPenSize(2);

		//����ɨˢ���� 50
		GUI_DrawLine(0,50,695,50);
		GUI_DrawLine(0,100,695,100);
		GUI_DrawLine(0,150,695,150);
	
		//��ǰɨˢ���� 275
		GUI_DrawLine(0,200,695,200);
		GUI_DrawLine(0,250,695,250);
		GUI_DrawLine(0,300,695,300);
		
		//��ǰɨˢ����
		GUI_DrawLine(0,350,695,350);
		GUI_DrawLine(0,400,695,400);
		GUI_DrawLine(0,450,695,450);
		
		GUI_DrawLine(145,50,145,150);
		GUI_DrawLine(145,200,145,300);
		GUI_DrawLine(145,350,145,450);
		GUI_DrawRoundedFrame(0, 5, 695, 475, 6, 3);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("����ɨˢ������Ϣ",250,20);
		GUI_DispStringAt("������",35,65);
		GUI_DispStringAt("��������",25,115);
		
		GUI_DispStringAt("��ǰɨˢ������Ϣ",250,170);
		GUI_DispStringAt("������",35,215);
		GUI_DispStringAt("��������",25,265);
		
		GUI_DispStringAt("��ǰɨˢ������Ϣ",250,320);
		GUI_DispStringAt("������",35,365);
		GUI_DispStringAt("��������",25,415);
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 		
}

//��ʼ��������Ϣ����2��������
void InitFaultInterface2(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
		GUI_Clear();
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
		GUI_SetPenSize(2);

		//���ɨˢ����
		GUI_DrawLine(0,50,695,50);
		GUI_DrawLine(0,100,695,100);
		GUI_DrawLine(0,150,695,150);
	
		//�Һ�ɨˢ����
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
		GUI_DispStringAt("���ɨˢ������Ϣ",250,20);
		GUI_DispStringAt("������",35,65);
		GUI_DispStringAt("��������",25,115);
		
		GUI_DispStringAt("�Һ�ɨˢ������Ϣ",250,170);
		GUI_DispStringAt("������",35,215);
		GUI_DispStringAt("��������",25,265);
		
		//GUI_DispStringAt("��ǰɨˢ������Ϣ",250,320);
		//GUI_DispStringAt("������",35,365);
		//GUI_DispStringAt("��������",25,415);
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 				
}


void Init_Fault(u8 num)
{
	  display_init_OK=0;
		switch(num)
		{
		  case 0:
				//���Ͻ���0, ���+���+����
				InitFaultInterface();
			  break;
			case 1:
				//���Ͻ���1������+��ǰ+��ǰɨˢ
				InitFaultInterface1();
			  break;
			case 2:
				//���Ͻ���2�����+�Һ�ɨˢ
				InitFaultInterface2();
			  break;
		}
		flag_fault_init = 1;
}

//��ʾ���Ͻ������ֵ
void ShowMsgInFaultInterface(void)
{
	u8 i;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�

	GUI_SetColor(GUI_BLACK);
	//GUI_SetFont(&GUI_Font16_ASCII);
	//��ع���������
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
	
	//��ع����������ʾ
	GUI_SetFont(&GUI_Font24_ASCII);
//	GUI_DispStringAt("59, 52, 51, 31, 17, 1", 340, 102);
	for(i=0;i<8;i++)
	{
		//GUI_DispDecAt(error_code[i], 157+28*i, 65, 2);
		GUI_DispHexAt(Alarm_Code[7-i], 157+28*i, 65, 2);
	}
//	GUI_DispStringAt("59, 52, 1", 165, 65);                 //����
	
	//�����߹��ϵȼ�
//	GUI_DispHexAt(3, 240, 255, 2);
	GUI_SetFont(&GUI_FontHZ24);
	//���Ͻ���Թ������˵��������������
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
				GUI_DispStringAt("�޹���", 580, 65);
			break;
			
			case 1:
				GUI_DispStringAt("1������", 575, 65);
			break;
			
			case 2:
				GUI_DispStringAt("2������", 575, 65);
			break;
			
			case 3:
				GUI_DispStringAt("3������", 575, 65);
			break;
			
			case 254:
				GUI_DispStringAt("�쳣", 590, 65);
			break;
			
			case 255:
				GUI_DispStringAt("��Ч", 590, 65);
			break;
			
			default:
				break;
		}

		//GUI_DispHexAt(Alarm_Level, 605, 65, 2);
		
		//��ع���������У�����
//		GUI_DispStringAt("59 CC���ӹ���", 220, 120);
//		GUI_DispStringAt("52 Ԥ��ʧ��", 220, 155);
//		GUI_DispStringAt("1 �ŵ絥���ѹ", 220, 190);
		
		//ֻ����3��������Ϣ
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
	
	//��ع�������
	//GUI_DispDecAt(6, 585, 255, 2);
	
	//�����������
	//GUI_DispHexAt(2, 585, 315, 2);
	
	//��������������ʾ
	
	//GUI_DispStringAt("23, 17", 220, 325);             //����
	GUI_DispDecAt(FanErrorCode, 220, 290, 3);
	
	
	//��������������
	GUI_SetFont(&GUI_FontHZ24);
	if(FanErrorCode>=24)
	{
		GUI_DispStringAt(Fan_ErrorCode[24], 220, 340);
	}
	else
	{
		GUI_DispStringAt(Fan_ErrorCode[FanErrorCode], 220, 340);
	}
//	GUI_DispStringAt("23 �ٶȴ���������4", 220, 340);
//	GUI_DispStringAt("17 ��������������1", 220, 420);
	
	//���ߵ��������
	GUI_DispDecAt(RunErrorCode, 182, 430, 2);
	//���ߵ�������������
	GUI_DispStringAt(Run_ErrorCode[RunErrorCode], 460, 430);
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

//��ʾ���Ͻ���1����ֵ
void ShowMsgInFaultInterface1(void)
{
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�

	GUI_SetColor(GUI_BLACK);
  GUI_SetFont(&GUI_Font24_ASCII);
	//����ɨˢ������
	GUI_DispDecAt(SweeperErrorCode4, 220, 65, 2);
	
	//��ǰɨˢ������
	GUI_DispDecAt(SweeperErrorCode2, 220, 215, 2);
	
	//��ǰɨˢ������
	GUI_DispDecAt(SweeperErrorCode, 220, 365, 2);
	
	GUI_SetFont(&GUI_FontHZ24);
	//����ɨˢ��������
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode4], 220, 115);
	
	//��ǰɨˢ��������
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode2], 220, 265);
	
	//��ǰɨˢ��������
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode], 220, 415);
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

//��ʾ���Ͻ���2����ֵ
void ShowMsgInFaultInterface2(void)
{
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�

	GUI_SetColor(GUI_BLACK);
  GUI_SetFont(&GUI_Font24_ASCII);
	//���ɨˢ������
	GUI_DispDecAt(SweeperErrorCode3, 220, 65, 2);
	
	//�Һ�ɨˢ������
	GUI_DispDecAt(SweeperErrorCode1, 220, 215, 2);
	
	//��ǰɨˢ������
	//GUI_DispDecAt(SweeperErrorCode, 220, 365, 2);
	
	GUI_SetFont(&GUI_FontHZ24);
	//���ɨˢ��������
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode3], 220, 115);
	
	//�Һ�ɨˢ��������
	GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode1], 220, 265);
	
	//��ǰɨˢ��������
	//GUI_DispStringAt(Sweeper_ErrorCode[SweeperErrorCode], 220, 415);
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}


//��ʼ��ά�������������
void InitCareInterface(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //��������
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
		GUI_SetPenSize(2);
	  //����
		GUI_DrawLine(0,57,695,57);
		GUI_DrawLine(0,107,695,107);
		GUI_DrawLine(0,157,695,157);
		GUI_DrawLine(0,207,695,207);
		GUI_DrawLine(0,257,695,257);
		GUI_DrawLine(0,307,695,307);
		GUI_DrawLine(0,357,695,357);
		//����
		GUI_DrawLine(182,5,182,107);
	  GUI_DrawLine(182,157,182,407);
		GUI_DrawLine(352,5,352,107);
	  GUI_DrawLine(352,157,352,357);
		GUI_DrawLine(521,5,521,107);
		GUI_DrawLine(521,157,521,357);

		GUI_DrawRoundedFrame(0, 5, 695, 407, 6, 2);
		
		GUI_SetFont(&GUI_FontHZ24);
		GUI_DispStringAt("�� �� �� �� �� ��",240,122);
		GUI_DispStringAt("������ѹ",22,22);		
		GUI_DispStringAt("����ʱ��",22,72);			
		GUI_DispStringAt("��ص�ѹ",22,172);
		GUI_DispStringAt("��ص���",22,222);
    GUI_DispStringAt("��ص���",22,272);		
		GUI_DispStringAt("���ϵȼ�",22,322);
		GUI_DispStringAt("��ع�����",22,372);
		
		GUI_DispStringAt("�����¶�",371,22);
		GUI_DispStringAt("��ʻ���",371,72);
		//change by mch 20180524
		GUI_DispStringAt("���״̬",371,172);
		GUI_DispStringAt("ƽ���¶�",371,222);	
		GUI_DispStringAt("����¶�",371,272);
		GUI_DispStringAt("����¶�",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("����汾��V 2.1",550,427);     //2.1����EEPROMΪC02��2.0�����ͺ�ΪC256
	  GUI_DispStringAt("����汾��V 6.1",550,427);//2018��8��24��
		GUI_DispStringAt("ҳ 1",30,427);//2018��8��24��
		
		GUI_SetFont(&GUI_Font24_ASCII); //��������
		GUI_DispStringAt("V",305,22);                    //������ѹ
		GUI_DispStringAt("h",305,72);                    //����ʱ��
		GUI_DispStringAt("V",305,172);                   //��ص�ѹ
		GUI_DispStringAt("A",305,222);                   //��ص���
		
		GUI_DrawBitmap(&bmsheshidu14,623,22);		         //�����¶�
		GUI_DispStringAt("km",639,72);                   //��ʻ���
		GUI_DrawBitmap(&bmsheshidu14,640,222);           //ƽ���¶�
		GUI_DrawBitmap(&bmsheshidu14,640,272);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,640,322);		       //����¶�
		
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
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 		
}

//��ʼ��ά������1��������
void InitCareInterface1(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //��������
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
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
		GUI_DispStringAt("�����ز���",90,22);
		GUI_DispStringAt("�����ѹ",22,72);		
		GUI_DispStringAt("�������",22,122);		
		GUI_DispStringAt("���ת��",22,172);		
		GUI_DispStringAt("����¶�",22,222);
		GUI_DispStringAt("�������¶�",22,272);
    GUI_DispStringAt("���������",22,322);	
		
		GUI_DispStringAt("������ز���",436,22);
		GUI_DispStringAt("���ߵ�ѹ",371,72);
		GUI_DispStringAt("���ߵ���",371,122);		
		GUI_DispStringAt("�����ٶ�",371,172);		
		GUI_DispStringAt("����¶�",371,222);
		GUI_DispStringAt("�������¶�",371,272);			
		GUI_DispStringAt("���߹�����",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("����汾��V 2.1",550,427);     //2.1����EEPROMΪC02��2.0�����ͺ�ΪC256
	  GUI_DispStringAt("����汾��V 6.1",550,427);//2018��6��24��
		GUI_DispStringAt("ҳ 2",30,427);//2018��8��24��
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //��������
		GUI_DispStringAt("V",305,72);                    //�����ѹ
		GUI_DispStringAt("A",305,122);                    //�������
		GUI_DispStringAt("r/min",287,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //�����¶� --> ����¶�
		
		GUI_DispStringAt("V",640,72);                    //�����ѹ
		GUI_DispStringAt("A",640,122);                    //�������
		GUI_DispStringAt("km/h",627,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //�����¶� --> ����¶�
		
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		GUI_DispStringAt(".",579,67);                    //sweeper current
		//GUI_DispStringAt(".",262,117);
		GUI_DispStringAt(".",595,117);                   //working voltage
		//GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		GUI_DispStringAt(".",595,217);                   //cell current --> run current
		//GUI_DispStringAt(".",262,267);                   //���ߵ���¶�
		GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 		
}

//��ʼ��ά������2��������
void InitCareInterface2(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //��������
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
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
		GUI_DispStringAt("��ǰɨˢ����",90,22);
		GUI_DispStringAt("ɨˢ��ѹ",22,72);		
		GUI_DispStringAt("ɨˢ����",22,122);		
		GUI_DispStringAt("ɨˢת��",22,172);		
		GUI_DispStringAt("����¶�",22,222);
		GUI_DispStringAt("�������¶�",22,272);
    GUI_DispStringAt("ɨˢ������",22,322);	
		
		GUI_DispStringAt("��ǰɨˢ����",436,22);
		GUI_DispStringAt("ɨˢ��ѹ",371,72);
		GUI_DispStringAt("ɨˢ����",371,122);		
		GUI_DispStringAt("ɨˢת��",371,172);		
		GUI_DispStringAt("����¶�",371,222);
		GUI_DispStringAt("�������¶�",371,272);			
		GUI_DispStringAt("ɨˢ������",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("����汾��V 2.1",550,427);     //2.1����EEPROMΪC02��2.0�����ͺ�ΪC256
	  GUI_DispStringAt("����汾��V 6.1",550,427);//2018��6��24��
		GUI_DispStringAt("ҳ 3",30,427);//2018��8��24��
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //��������
		GUI_DispStringAt("V",305,72);                    //�����ѹ
		GUI_DispStringAt("A",305,122);                    //�������
		GUI_DispStringAt("r/min",287,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //�����¶� --> ����¶�
		
		GUI_DispStringAt("V",640,72);                    //�����ѹ
		GUI_DispStringAt("A",640,122);                    //�������
		GUI_DispStringAt("r/min",627,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //�����¶� --> ����¶�
		
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		GUI_DispStringAt(".",579,67);                    //sweeper current
		GUI_DispStringAt(".",262,117);
		GUI_DispStringAt(".",595,117);                   //working voltage
		GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		GUI_DispStringAt(".",595,217);                   //cell current --> run current
		GUI_DispStringAt(".",262,267);                   //���ߵ���¶�
		GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 			
}

//��ʼ��ά������3��������
void InitCareInterface3(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //��������
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
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
		GUI_DispStringAt("���ɨˢ����",90,22);
		GUI_DispStringAt("ɨˢ��ѹ",22,72);		
		GUI_DispStringAt("ɨˢ����",22,122);		
		GUI_DispStringAt("ɨˢת��",22,172);		
		GUI_DispStringAt("����¶�",22,222);
		GUI_DispStringAt("�������¶�",22,272);
    GUI_DispStringAt("ɨˢ������",22,322);	
		
		GUI_DispStringAt("�Һ�ɨˢ����",436,22);
		GUI_DispStringAt("ɨˢ��ѹ",371,72);
		GUI_DispStringAt("ɨˢ����",371,122);		
		GUI_DispStringAt("ɨˢת��",371,172);		
		GUI_DispStringAt("����¶�",371,222);
		GUI_DispStringAt("�������¶�",371,272);			
		GUI_DispStringAt("ɨˢ������",371,322);
		
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("����汾��V 2.1",550,427);     //2.1����EEPROMΪC02��2.0�����ͺ�ΪC256
	  GUI_DispStringAt("����汾��V 6.1",550,427);//2018��6��24��
		GUI_DispStringAt("ҳ 4",30,427);//2018��8��24��
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //��������
		GUI_DispStringAt("V",305,72);                    //�����ѹ
		GUI_DispStringAt("A",305,122);                    //�������
		GUI_DispStringAt("r/min",287,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //�����¶� --> ����¶�
		
		GUI_DispStringAt("V",640,72);                    //�����ѹ
		GUI_DispStringAt("A",640,122);                    //�������
		GUI_DispStringAt("r/min",627,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //�����¶� --> ����¶�
		
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		GUI_DispStringAt(".",579,67);                    //sweeper current
		GUI_DispStringAt(".",262,117);
		GUI_DispStringAt(".",595,117);                   //working voltage
		GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		GUI_DispStringAt(".",595,217);                   //cell current --> run current
		GUI_DispStringAt(".",262,267);                   //���ߵ���¶�
		GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 	
}

//��ʼ��ά������4��������
void InitCareInterface4(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�	
	
		GUI_Clear();
		GUI_SetFont(&GUI_Font32_ASCII); //��������
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)
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
		GUI_DispStringAt("����ɨˢ����",90,22);
		GUI_DispStringAt("ɨˢ��ѹ",22,72);		
		GUI_DispStringAt("ɨˢ����",22,122);		
		GUI_DispStringAt("ɨˢת��",22,172);		
		GUI_DispStringAt("����¶�",22,222);
		GUI_DispStringAt("�������¶�",22,272);
    GUI_DispStringAt("ɨˢ������",22,322);	
		/*
		GUI_DispStringAt("��ǰɨˢ����",436,22);
		GUI_DispStringAt("ɨˢ��ѹ",371,72);
		GUI_DispStringAt("ɨˢ����",371,122);		
		GUI_DispStringAt("ɨˢת��",371,172);		
		GUI_DispStringAt("����¶�",371,222);
		GUI_DispStringAt("�������¶�",371,272);			
		GUI_DispStringAt("ɨˢ������",371,322);
		*/
		GUI_SetFont(&GUI_FontHZ16);
		//GUI_DispStringAt("����汾��V 2.1",550,427);     //2.1����EEPROMΪC02��2.0�����ͺ�ΪC256
	  GUI_DispStringAt("����汾��V 6.1",550,427);//2018��6��24��
		GUI_DispStringAt("ҳ 5",30,427);//2018��8��24��
		
		GUI_SetFont(&GUI_Font24_ASCII);                  //��������
		GUI_DispStringAt("V",305,72);                    //�����ѹ
		GUI_DispStringAt("A",305,122);                    //�������
		GUI_DispStringAt("r/min",287,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,295,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,295,272);           //�����¶� --> ����¶�
		/*
		GUI_DispStringAt("V",640,72);                    //�����ѹ
		GUI_DispStringAt("A",640,122);                    //�������
		GUI_DispStringAt("r/min",627,172);               //���ת��
		GUI_DrawBitmap(&bmsheshidu14,630,222);           //����¶�
		GUI_DrawBitmap(&bmsheshidu14,630,272);           //�����¶� --> ����¶�
		*/
		GUI_SetFont(&GUI_FontDSDigital32);
		GUI_DispStringAt(".",248,67);                    //fan voltage
		//GUI_DispStringAt(".",579,67);                    //sweeper current
		GUI_DispStringAt(".",262,117);
		//GUI_DispStringAt(".",595,117);                   //working voltage
		GUI_DispStringAt(".",262,217);                   //cell voltage --> run voltage
		//GUI_DispStringAt(".",595,217);                   //cell current --> run current
		GUI_DispStringAt(".",262,267);                   //���ߵ���¶�
		//GUI_DispStringAt(".",595,267);                   //cell current
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}


void Init_Care(u8 num)
{
	  display_init_OK=0;
    switch(num)
		{
		  case 0:
				//ά������0, ������Ϣ+���
				InitCareInterface();
			  break;
			case 1:
				//ά������1�����ߵ��+���
				InitCareInterface1();
			  break;
			case 2:
				//ά������2����ǰ+��ǰɨˢ
			  InitCareInterface2();
			  break;
			case 3:
				//ά������3�����+�Һ�ɨˢ
				InitCareInterface3();
			  break;
			case 4:
				//ά������4������ɨˢ
				InitCareInterface4();
			  break;
		}
		flag_weihu_init = 1;
}

//�������㣬��д��E2PROM��
void ProcessUserKey1(void)
{
	if( p_disio_in_calc.P_ODOReset_Counter>20)//����4s���� 4���Ϊ2��
	{		

		p_disio_in_calc.P_ODOReset_Counter=0;
		minute=0;//�������
		ci_parameter.OdoMeter=0;//reset worktime
		AT24CXX_WriteLenByte(0,minute,4);
		AT24CXX_WriteLenByte(4,ci_parameter.OdoMeter,4);
		
		minute = AT24CXX_ReadLenByte(0,4);   //��ȡ����ʱ�����ֵ
		ci_parameter.OdoMeter = AT24CXX_ReadLenByte(4,4);  //��ȡ���ֵ
		Base_OdoMeter = AT24CXX_ReadLenByte(4,4);  //��ȡ��׼���ֵ
		ci_parameter.WorkTime=minute;
		
	}
}

//��ת����
void ProcessUserKey2(void)
{
//	if( p_disio_in_calc.P_ODOKey2_Counter>50)//����5s����
//			ShowMsgInCareInterface();
}


//
void ProcessUserKey3(void)
{
	if(flag_shortpress == 1)               //�̰���P_DO2��ƽ��ת���л���Ƶ
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

//��ʾά���������ֵ
void ShowMsgInCareInterface(void)
{
	char* str;
	int i;
	u16 Icell10;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//������ѹ
	GUI_DispDecAt((int)ci_parameter.WorkVoltage, 217, 17, 2);	
	GUI_DispDecAt((int)((ci_parameter.WorkVoltage-(int)ci_parameter.WorkVoltage)*10), 254, 17, 1);
	
	//����ʱ��
	GUI_DispDecAt(ci_parameter.WorkTime/60, 203, 67, 5);	
	GUI_DispDecAt((ci_parameter.WorkTime%60)/6, 285, 67, 1);
	
	//��ص�ѹ
	GUI_DispDecAt(BatteryVoltage/10, 227, 167, 2);
	GUI_DispDecAt(BatteryVoltage%10, 264, 167, 1);

	//��ص���
	Icell10=(u16)(fabs(BatteryCurrent-10000)*1.0);
	GUI_DispDecAt(Icell10/10, 217, 217, 3);
	GUI_DispDecAt(Icell10%10, 268, 217, 1);
	
	//��ص���
	GUI_DispDecAt(BatteryQuantity/10, 240, 267, 3);	
	
	//���ϵȼ�
	GUI_SetFont(&GUI_FontHZ24);
	switch(Highest_Alarm_Level)
	{
		case 0:
			GUI_DispStringAt("�޹���", 230, 320);			
		  break;
		case 1:
			GUI_DispStringAt("1��", 230, 320);			
		  break;
		case 2:
			GUI_DispStringAt("2��", 230, 320);
		  break;
		case 3:
			GUI_DispStringAt("3��", 230, 320);
		  break;
		case 254:
			GUI_DispStringAt("�쳣", 230, 320);
		  break;
		case 255:
			GUI_DispStringAt("��Ч", 230, 320);
		  break;			
		default:
			break;
	}
	
	//���״̬
	if(Charge_Info)
	{
		GUI_DispStringAt("���", 575, 170);
	}
	else
	{
		GUI_DispStringAt("�ŵ�", 575, 170);
	}
	
	GUI_SetFont(&GUI_FontDSDigital30);
	//�����¶�
	//GUI_DispDecAt(BatteryVoltage/10, 552, 17, 2);
	//GUI_DispDecAt(BatteryVoltage%10, 588, 17, 1);	
	ShowRoomTemp(552, 17);
	
	//��ʻ���
	GUI_DispDecAt((ci_parameter.OdoMeter/100)/10, 537, 67, 5);	
	GUI_DispDecAt((ci_parameter.OdoMeter/100)%10, 619, 67, 1);

	//ƽ���¶�
	ShowTemp(BatteryTemp, 565, 217, 3);
	//GUI_DispDecAt(BatteryTemp/10, 547, 217, 3);
	//GUI_DispDecAt(BatteryTemp%10, 598, 217, 1);
	
	//����¶�
	ShowTemp(BatteryMinTemp, 565, 267, 3);
	//GUI_DispDecAt(BatteryMinTemp/10, 547, 267, 3);
	//GUI_DispDecAt(BatteryMinTemp%10, 598, 267, 1);

	//����¶�
	ShowTemp(BatteryMaxTemp, 565, 317, 3);
	//GUI_DispDecAt(BatteryMaxTemp/10, 547, 317, 3);
	//GUI_DispDecAt(BatteryMaxTemp%10, 598, 317, 1);
	
	//��ع�����
	GUI_SetFont(&GUI_Font32_ASCII);
	for(i=0;i<8;i++)
	{
		GUI_DispHexAt(Alarm_Code[7-i], 222+40*i, 3697, 2);
	}
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

//��ʾά������1����ֵ
void ShowMsgInCareInterface1(void)
{
	char* str;
	int i;
	u16 Icell10;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//�����ѹ
	GUI_DispDecAt(FanVoltage/10, 217, 67, 2);	
	GUI_DispDecAt(FanVoltage%10, 254, 67, 1);
	
	//�������
	GUI_DispDecAt(FanCurrent, 217, 117, 3);
	//GUI_DispDecAt(FanCurrent%10, 268, 267, 1);
	
	//���ת��
	//GUI_DispDecAt(FanSpeed, 210, 167, 4);
	Show_Speed(fan_motor.Speed, fan_motor_old.Speed, 2, 4, 210, 167);              //���ת��
	
	//����¶�
	ShowTemp(FanTemp, 217, 217, 3);
	//GUI_DispDecAt(FanTemp, 217, 217, 3);
	//GUI_DispDecAt(FanTemp%10, 268, 267, 1);
	
	//�������¶�
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_DispStringAt("None",217,270);
	GUI_SetFont(&GUI_FontDSDigital30);
	//GUI_DispDecAt(RunCtlTemp/10, 217, 267, 3);
	//GUI_DispDecAt(RunCtlTemp%10, 268, 267, 1);
	
	///������ϴ���
	if(FanErrorCode > 0 && FanErrorCode < 24)
	{
		GUI_DispDecAt(FanErrorCode, 230, 317, 2);
	}		
	
	//���ߵ�ѹ
	GUI_DispDecAt(RunVoltage/10, 547, 67, 2);	
	GUI_DispDecAt(RunVoltage%10, 584, 67, 1);
	
	//���ߵ���
	GUI_DispDecAt(RunCurrent/10, 547, 117, 3);
	GUI_DispDecAt(RunCurrent%10, 602, 117, 1);
	
	//�����ٶ�
	GUI_DispDecAt(ci_parameter.Speed, 560, 167, 2);
	
	//����¶�
	GUI_DispDecAt(RunTemp/10, 547, 217, 3);
	GUI_DispDecAt(RunTemp%10, 602, 217, 1);
	
	//�������¶�
	GUI_DispDecAt(RunCtlTemp/10, 547, 267, 3);
	GUI_DispDecAt(RunCtlTemp%10, 602, 267, 1);
	
	///���߹��ϴ���
	if(RunErrorCode < 14)
	{
		GUI_DispDecAt(RunErrorCode, 560, 317, 2);
	}		
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

//��ʾά������2����ֵ
void ShowMsgInCareInterface2(void)
{
	char* str;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//��ǰɨˢ��ѹ
	GUI_DispDecAt(SweeperVoltage2/10, 217, 67, 2);	
	GUI_DispDecAt(SweeperVoltage2%10, 254, 67, 1);
	
	//��ǰɨˢ����
	GUI_DispDecAt(SweeperCurrent2/10, 217, 117, 3);
	GUI_DispDecAt(SweeperCurrent2%10, 268, 117, 1);
	
	//��ǰɨˢת��
	GUI_DispDecAt(SweeperSpeed2, 217, 167, 3);
	
	//��ǰɨˢ����¶�
	GUI_DispDecAt(SweeperTemp2/10, 217, 217, 3);
	GUI_DispDecAt(SweeperTemp2%10, 268, 217, 1);
	
	//�������¶�
	GUI_DispDecAt(SweeperCtlTemp2/10, 217, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp2%10, 268, 267, 1);
	
	//��ǰɨˢ���ϴ���
	if(SweeperErrorCode2 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode2, 230, 317, 2);
	}		
	
	//��ǰɨˢ��ѹ
	GUI_DispDecAt(SweeperVoltage/10, 547, 67, 2);	
	GUI_DispDecAt(SweeperVoltage%10, 584, 67, 1);
	
	//��ǰɨˢ��ѹ
	GUI_DispDecAt(SweeperCurrent/10, 547, 117, 3);
	GUI_DispDecAt(SweeperCurrent%10, 602, 117, 1);
	
	//��ǰɨˢת��
	GUI_DispDecAt(SweeperSpeed, 547, 167, 3);
	
	//��ǰɨˢ����¶�
	GUI_DispDecAt(SweeperTemp/10, 547, 217, 3);
	GUI_DispDecAt(SweeperTemp%10, 602, 217, 1);
	
	//�������¶�
	GUI_DispDecAt(SweeperCtlTemp/10, 547, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp%10, 602, 267, 1);
	
	///��ǰɨˢ���ϴ���
	if(SweeperErrorCode < 24)
	{
		GUI_DispDecAt(SweeperErrorCode, 560, 317, 2);
	}		
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

//��ʾά������3����ֵ
void ShowMsgInCareInterface3(void)
{
	char* str;
	int i;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//���ɨˢ��ѹ
	GUI_DispDecAt(SweeperVoltage3/10, 217, 67, 2);	
	GUI_DispDecAt(SweeperVoltage3%10, 254, 67, 1);
	
	//���ɨˢ����
	GUI_DispDecAt(SweeperCurrent3/10, 217, 117, 3);
	GUI_DispDecAt(SweeperCurrent3%10, 268, 117, 1);
	
	//���ɨˢת��
	GUI_DispDecAt(SweeperSpeed3, 217, 167, 3);
	
	//���ɨˢ����¶�
	GUI_DispDecAt(SweeperTemp3/10, 217, 217, 3);
	GUI_DispDecAt(SweeperTemp3%10, 268, 217, 1);
	
	//�������¶�
	GUI_DispDecAt(SweeperCtlTemp3/10, 217, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp3%10, 268, 267, 1);
	
	//���ɨˢ���ϴ���
	if(SweeperErrorCode3 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode3, 230, 317, 2);
	}		
	
	//�Һ�ɨˢ��ѹ
	GUI_DispDecAt(SweeperVoltage1/10, 547, 67, 2);	
	GUI_DispDecAt(SweeperVoltage1%10, 584, 67, 1);
	
	//�Һ�ɨˢ��ѹ
	GUI_DispDecAt(SweeperCurrent1/10, 547, 117, 3);
	GUI_DispDecAt(SweeperCurrent1%10, 602, 117, 1);
	
	//�Һ�ɨˢ��ѹ
	GUI_DispDecAt(SweeperSpeed1, 547, 167, 3);
	
	//�Һ�ɨˢ����¶�
	GUI_DispDecAt(SweeperTemp1/10, 547, 217, 3);
	GUI_DispDecAt(SweeperTemp1%10, 602, 217, 1);
	
	//�������¶�
	GUI_DispDecAt(SweeperCtlTemp1/10, 547, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp1%10, 602, 267, 1);
	
	//�Һ�ɨˢ���ϴ���
	if(SweeperErrorCode1 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode1, 560, 317, 2);
	}		
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

//��ʾά������4����ֵ
void ShowMsgInCareInterface4(void)
{
	char* str;
	int i;
	u16 Icell10;
	HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
	HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�
	GUI_SetColor(GUI_BLACK);
	GUI_SetFont(&GUI_FontDSDigital30);
	
	//����ɨˢ��ѹ
	GUI_DispDecAt(SweeperVoltage4/10, 217, 67, 2);	
	GUI_DispDecAt(SweeperVoltage4%10, 254, 67, 1);
	
	//����ɨˢ����
	GUI_DispDecAt(SweeperCurrent4/10, 217, 117, 3);
	GUI_DispDecAt(SweeperCurrent4%10, 268, 117, 1);
	
	//����ɨˢת��
	GUI_DispDecAt(SweeperSpeed4, 217, 167, 3);
	
	//����ɨˢ����¶�
	GUI_DispDecAt(SweeperTemp4/10, 217, 217, 3);
	GUI_DispDecAt(SweeperTemp4%10, 268, 217, 1);
	
	//�������¶�
	GUI_DispDecAt(SweeperCtlTemp4/10, 217, 267, 3);
	GUI_DispDecAt(SweeperCtlTemp4%10, 268, 267, 1);
	
	//����ɨˢ���ϴ���
	if(SweeperErrorCode4 < 24)
	{
		GUI_DispDecAt(SweeperErrorCode4, 230, 317, 2);
	}		
	/*
	//��ǰɨˢ��ѹ
	GUI_DispDecAt(RunVoltage/10, 547, 67, 2);	
	GUI_DispDecAt(RunVoltage%10, 584, 67, 1);
	
	//��ǰɨˢ��ѹ
	GUI_DispDecAt(RunCurrent/10, 547, 117, 3);
	GUI_DispDecAt(RunCurrent%10, 602, 117, 1);
	
	//��ǰɨˢ��ѹ
	GUI_DispDecAt(ci_parameter.Speed, 547, 167, 3);
	
	//��ǰɨˢ����¶�
	GUI_DispDecAt(RunTemp/10, 547, 217, 3);
	GUI_DispDecAt(RunTemp%10, 602, 217, 1);
	
	//�������¶�
	GUI_DispDecAt(RunCtlTemp/10, 547, 267, 3);
	GUI_DispDecAt(RunCtlTemp%10, 602, 267, 1);
	
	///��ǰɨˢ���ϴ���
	if(RunErrorCode >= 0 && RunErrorCode < 14)
	{
		GUI_DispDecAt(RunErrorCode, 560, 317, 2);
	}		
	*/
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

//��ʾ������������ĵ�λ
void ShowAll(void)
{
 	//GUI_DrawBitmap(&bm700wenduji, 295, 410);	 //��ʾ�¶ȼ�ͼ��
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


//�������ʼ��
void InitMainInterface(void)
{
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);   //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�

		if(display_init_OK==0)
		{
			minute = AT24CXX_ReadLenByte(0,4);   //��ȡ����ʱ�����ֵ
			ci_parameter.OdoMeter = AT24CXX_ReadLenByte(4,4);  //��ȡ���ֵ
		}

		GUI_Clear();
		GUI_SetFont(&GUI_Font24_ASCII); //��������
		GUI_SetColor(GUI_BLACK);       //����ǰ��ɫ(���ı������ߵ���ɫ)

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

//		Show_Light(ci_parameter.LeftLight, ci_parameter_old.LeftLight, ZUOZHUAN, 1, 10, 15);                //��ת
//		Show_Light(ci_parameter.RightLight, ci_parameter_old.RightLight, YOUZHUAN, 1, 591, 15);             //��ת
//		Show_Light(ci_parameter.DippedLight, ci_parameter_old.DippedLight, JINGUANG, 1, 259, 15);           //����
//		Show_Light(ci_parameter.HandBrakeLight, ci_parameter_old.HandBrakeLight, SHOUSHA, 1, 93, 15);       //��ɲ
//		Show_Light(ci_parameter.BackWardLight, ci_parameter_old.BackWardLight, HOUTUI, 1, 425, 15);         //����
//		Show_Light(ci_parameter.ForwardLight, ci_parameter_old.ForwardLight, QIANJIN, 1, 342, 15);          //ǰ��
//		Show_Light(ci_parameter.FootBrakeLight, ci_parameter_old.FootBrakeLight, JIAOSHA, 1, 176, 15);      //��ɲ
//		Show_Light(ci_parameter.LowWaterWarning, ci_parameter_old.LowWaterWarning, SHUIWEIDI, 0, 508, 15);  //ˮλ��
		
		Show_Speed(rf_sweeper.Speed, rf_sweeper_old.Speed, 1, 3, 520, 380);            //��ǰɨˢת��
		Show_Speed(fan_motor.Speed, fan_motor_old.Speed, 2, 4, 510, 211);              //���ת��
		Show_Speed(run_motor.Speed, run_motor_old.Speed, 4, 2, 280, 240);              //�����ٶ�
		
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
	
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 	
}


//��ʾ������
void ShowMsgInMainInterface(void)
{
		//����Ļ���Ƶ�ʱ�򣬹رն�ʱ������������ٴ򿪣���ֹ���ƹ��������ݸ��µ��»���Ķ���
		HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);                //���ж�
		HAL_NVIC_DisableIRQ(TIM3_IRQn);       //���ж�
		
		Show_Light(ci_parameter.DippedLight, ci_parameter_old.DippedLight, JINGUANG, 1, 259, 15);           //����
		Show_Light(ci_parameter.HandBrakeLight, ci_parameter_old.HandBrakeLight, SHOUSHA, 1, 93, 15);       //��ɲ
		Show_Light(ci_parameter.BackWardLight, ci_parameter_old.BackWardLight, HOUTUI, 1, 425, 15);         //����
		Show_Light(ci_parameter.ForwardLight, ci_parameter_old.ForwardLight, QIANJIN, 1, 342, 15);          //ǰ��
		Show_Light(ci_parameter.FootBrakeLight, ci_parameter_old.FootBrakeLight, JIAOSHA, 1, 176, 15);      //��ɲ
		Show_Light(ci_parameter.LowWaterWarning, ci_parameter_old.LowWaterWarning, SHUIWEIDI, 0, 508, 15);  //ˮλ��

		if(main_count<50)
		{
			main_count++;
		}
		
		ShowBatteryTemp();                                 //��ʾ����¶�
		ShowMaxBatTemp();                                  //��ʾ����¶�
		ShowMinBatTemp();                                  //��ʾ����¶�
		ShowCellVoltage();                                 //��ʾ��ص�ѹ
		ShowCellCurrent();                                 //��ʾ��ص���
		ShowBatteryAlarm();                                //��ʾ��ر������
		ShowBatteryQuantity();                             //��ʾ��ص���
		ShowBatteryStatus();                               //��ʾ��س�ŵ����
		
		Show_Speed(rf_sweeper.Speed, rf_sweeper_old.Speed, 1, 3, 520, 380);            //��ǰɨˢת��
		Show_Speed(fan_motor.Speed, fan_motor_old.Speed, 2, 4, 510, 211);              //���ת��
		Show_Speed(run_motor.Speed, run_motor_old.Speed, 4, 2, 280, 240);              //�����ٶ�
		
	  ShowRunError();                       //��ʾ���߹�����
		ShowOdometer();                       //��ʾ�����
		ShowRoomTemp(328, 430);               //��ʾ�����¶�
		ShowWorktime();                       //��ʾ����ʱ��
		ShowFanError();                       //��ʾ�������
		
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);  							 //����ITM6�ж� 
		HAL_NVIC_EnableIRQ(TIM3_IRQn);     								 //����ITM3�ж� 
}

void DisplayData(void)
{
	
	uint16_t count = 600, item1 = 0;
	
	u8 buf[8];
	u8 len, res, key, item=0;
	//�����ϵ�ʱ�ӳ�5����ת��ؽ��棬��MP5������ɣ���������������ֱ����ת��ؽ���
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
		//һ��δ���յ�FAN��CAN����,��ֱ�ӳ�ʼ��FAN����ز���
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
		//һ��δ���յ����ߵ����CAN����,��ֱ�ӳ�ʼ�����ߵ������ز���
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
		
		//���ֵƫ��ܴ�Ŀ����Ƕ�ʱ�����жϳ�����CAN�ѶϿ������������»�ȡ�˻�׼ֵ����ʵ��������δ�Ͽ�����
		//�ж��Ƿ���;���»�׼ֵ�ķ������ڶ�ʱ�������߶Ͽ�������һ��flag�������1���������������һ���ַ���������
		//����İ취��1�����Ӽ���ֵ��2��ֱ�Ӳ�ȥ�ж�����CAN�Ƿ�Ͽ�������Ĭ�����ߵ�����Ǳ���ͬʱ���µ磬����Ҫ��;��ȡ��׼ֵ��
		if(flag_getmileage == 1)
		{
			Base_OdoMeter = AT24CXX_ReadLenByte(4,4);  //��ȡ��׼���ֵ
			flag_getmileage = 0;
		}
		
		ProcessUserKey1();                                             //�����������
		ProcessUserKey3();
		
		//�жϳ�ʼ���ĸ�����
		//ע��������������ȥ�����Ͻ������������ά��������л���ϵ
		if(flag21 && flag31 && displayflag)   //&displayflag�����޷�ֱ�Ӵ���������ת�����Ͻ���
		{
			if(flag_fault)
			{
				//InitFaultInterface();   //������Ϣ����ĳ�ʼ��
				Init_Fault(flag_fault_click);
				displayflag=2;
			}
			else
			{
				//InitCareInterface();                                         //ά������ĳ�ʼ��
				Init_Care(flag_weihu_click);
				displayflag=1;
			}
		}
		/*
		if(flag_fault == 1 && displayflag==2)
		{
			ShowMsgInFaultInterface();                                     //��ʾ������Ϣ����
			display_init_OK=1;
		}
		
		else if(flag_fault == 0 && displayflag==1)
		{
			ShowMsgInCareInterface();                                     //��ʾά������
		}
		*/
		
		if(flag2 && flag3)
		{
			display_init_OK=0;
			if(displayflag)
			{
				InitCareInterface();                                        //ά������ĳ�ʼ��
				flag_weihu_click = 0;
			}
			else
			{ 
				InitMainInterface();                                    //������ĳ�ʼ��
			}
		}
		if(displayflag==0)
		{		
			ShowMsgInMainInterface();                                 //��ʾ������
			display_init_OK=1;
		}
		/*
		else if(flag_fault == 0 && displayflag==1)
		{
			ShowMsgInCareInterface();                          //��ʾά������
			
		}
		*/
		//add by mch 20180824 ����ά��������ʾ�ķ�ҳ�߼�
		if(flag_fault == 0 && displayflag==1)
		{
			switch(flag_weihu_click)
			{
			  case 0:
					ShowMsgInCareInterface();                          //��ʾά������0
					break;
				case 1:
					ShowMsgInCareInterface1();                          //��ʾά������1
					break;
				case 2:
					ShowMsgInCareInterface2();                          //��ʾά������2
					break;
				case 3:
					ShowMsgInCareInterface3();                          //��ʾά������3
					break;
				case 4:
					ShowMsgInCareInterface4();                          //��ʾά������4
					break;
			}
			display_init_OK=1;
		}

		if(flag_fault == 1 && displayflag==2)
		{
			switch(flag_fault_click)
			{
			  case 0:
					ShowMsgInFaultInterface();                           //��ʾ���Ͻ���0
					break;
				case 1:
					ShowMsgInFaultInterface1();                          //��ʾ���Ͻ���1
					break;
				case 2:
					ShowMsgInFaultInterface2();                          //��ʾ���Ͻ���2
					break;
			}
			display_init_OK=1;
		}
		
		//change by mch 20180624 �ӳ�ʱ���4���Ϊ2��
		if(p_disio_in_calc.P_ODOKey2_Counter>20)             //�������2���³���4�룬���л����棨�������ά������䣩
		{
			//change by mch 20180624
			//���Ͻ��泤��key2�������л�������
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
		
		if(p_disio_in_calc.P_ODOKey3_Counter>30)             //����3���³���3��
		{
			flag_shortpress = 0;
			if(displayflag == 0)               //��������������
			{
				GUI_Clear();
				GUI_SetFont(&GUI_Font32_ASCII); //��������
				GUI_SetColor(GUI_BLACK);
				GUI_DispStringAt("Soft reseting ......", 250, 220);
				AT24CXX_WriteLenByte(20,1,1);
			  delay_ms(1000);
				NVIC_SystemReset();                    //������
			}
			else                                //�����л�����
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

