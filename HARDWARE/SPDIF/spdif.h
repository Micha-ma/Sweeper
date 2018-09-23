#ifndef __SPDIF_H
#define __SPDIF_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//SPDIF��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/3/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//SPDIF���ƽṹ��
typedef struct
{
    u8 connsta;     //����״̬��0 δ����;1 ������
    u32 spdifrate;  //SPDIF������
    u32 spdif_clk;  //SPDIFʱ�ӣ�Ĭ��Ϊ158M
}spdif_struct;

extern spdif_struct spdif_dev;                 //SPDIF���ƽṹ��

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
