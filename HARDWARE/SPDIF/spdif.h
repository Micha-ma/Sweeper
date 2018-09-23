#ifndef __SPDIF_H
#define __SPDIF_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F7开发板
//SPDIF驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/3/6
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//SPDIF控制结构体
typedef struct
{
    u8 connsta;     //连接状态，0 未连接;1 连接上
    u32 spdifrate;  //SPDIF采样率
    u32 spdif_clk;  //SPDIF时钟，默认为158M
}spdif_struct;

extern spdif_struct spdif_dev;                 //SPDIF控制结构体

void SPDIFCLK_Config(void);
void SPDIFRX_Init(void);
u8 WaitSync_TORecv(void);
void SPDIF_GetRate(void);
void SPDIF_AUDIODATA_DMA_Init(u32* buf0,u32 *buf1,u16 num,u8 width);
void SPDIF_CONTROLDATA_DMA_Init(u32* buf0,u16 num);
uint32_t SPDIF_AUDIO_Init(uint32_t AudioFreq);
void SPDIF_Play_Start(void);
void SPDIF_Play_Stop(void);
#endif
