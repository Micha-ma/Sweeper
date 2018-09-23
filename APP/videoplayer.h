#ifndef __VIDEOPLAYER_H
#define __VIDEOPLAYER_H 
#include <stm32f7xx.h> 
#include "avi.h"
#include "ff.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//视频播放器 应用代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/1/12
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved	
//All rights reserved
//********************************************************************************
//V1.1 20160602
//1,修改部分代码，以支持最新的fatfs（R0.12）
//2,修改很多参数:i2s改为sai
////////////////////////////////////////////////////////////////////////////////// 	
 
 
#define AVI_AUDIO_BUF_SIZE    1024*5		//定义avi解码时,音频buf大小.
#define AVI_VIDEO_BUF_SIZE    1024*120		//定义avi解码时,视频buf大小.


void video_play(void);
u8 video_play_mjpeg(u8 *pname); 
void video_time_show(FIL *favi,AVI_INFO *aviinfo);
void video_info_show(AVI_INFO *aviinfo);
void video_bmsg_show(u8* name,u16 index,u16 total);
u16 video_get_tnum(u8 *path);
u8 video_seek(FIL *favi,AVI_INFO *aviinfo,u8 *mbuf);

#endif

