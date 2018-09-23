#ifndef __HZFONTUPDATA_H
#define __HZFONTUPDATA_H
#include "sys.h"
#include "GUI.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//STemwin 字库升级界面    
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/3/30
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

u32 emwin_fupd_prog(u32 fsize,u32 pos); //EMWIN使用的显示更新进度
u8 emwin_updata_fontx(u8 *fxpath,u8 fx);//EMWIN使用的更新指定字库函数
u8 emwin_update_font(u8* src);    //EMWIN使用的字库更新函数
void HzFontupdataCreate(void);
#endif
