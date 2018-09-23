#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "TEXT.h"

//extern GUI_CONST_STORAGE GUI_BITMAP bmShowAll720480;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700jinguang;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700kongbai;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700shousha;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700youzhuan;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700zuozhuan;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700wenduji;

//extern GUI_CONST_STORAGE GUI_BITMAP bm700car;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700dianchi;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700fan;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700houtui;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700jiaosha;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700qianjin;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700saoshua;
extern GUI_CONST_STORAGE GUI_BITMAP bm700shandian;
extern GUI_CONST_STORAGE GUI_BITMAP bm700dianchi_error;
//extern GUI_CONST_STORAGE GUI_BITMAP bm700shuiweidi;
extern GUI_CONST_STORAGE GUI_BITMAP bmsheshidu14;
extern GUI_CONST_STORAGE GUI_BITMAP bmsheshidu18;

extern GUI_CONST_STORAGE GUI_FONT GUI_FontDSDigital30;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontDSDigital32;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontDSDigital36;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontDSDigital40;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontDSDigital96;

extern uint8_t BEEPN;
extern uint8_t flagbattery;

void ShowAll(void);
void InitMainInterface(void);
void DisplayData(void);
void ShowLeftLight(void);
void ShowRightLight(void);
void ShowForwardLight(void);
void ShowBackwardLight(void);
void ShowHandBrakeLight(void);
void ShowFootBrakeLight(void);
void ShowDippedLight(void);
void ShowLowWaterWarning(void);


#endif
