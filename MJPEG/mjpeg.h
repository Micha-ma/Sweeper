#ifndef __MJPEG_H
#define __MJPEG_H  
#include "sys.h"
#include "jpegcodec.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//MJPEG视频处理 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/8/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   


extern jpeg_codec_typedef mjpeg;  

 

void mjpeg_dma_in_callback(void);
void mjpeg_dma_out_callback(void);
void mjpeg_endofcovert_callback(void);
void mjpeg_hdrover_callback(void);
u8 mjpeg_init(u16 offx,u16 offy,u32 width,u32 height);
void mjpeg_free(void);
u8 mjpeg_decode(u8* buf,u32 bsize);
#endif
