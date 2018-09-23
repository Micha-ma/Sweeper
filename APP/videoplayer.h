#ifndef __VIDEOPLAYER_H
#define __VIDEOPLAYER_H 
#include <stm32f7xx.h> 
#include "avi.h"
#include "ff.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK STM32������
//��Ƶ������ Ӧ�ô���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/12
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2014-2024
//All rights reserved	
//All rights reserved
//********************************************************************************
//V1.1 20160602
//1,�޸Ĳ��ִ��룬��֧�����µ�fatfs��R0.12��
//2,�޸ĺܶ����:i2s��Ϊsai
////////////////////////////////////////////////////////////////////////////////// 	
 
 
#define AVI_AUDIO_BUF_SIZE    1024*5		//����avi����ʱ,��Ƶbuf��С.
#define AVI_VIDEO_BUF_SIZE    1024*120		//����avi����ʱ,��Ƶbuf��С.


void video_play(void);
u8 video_play_mjpeg(u8 *pname); 
void video_time_show(FIL *favi,AVI_INFO *aviinfo);
void video_info_show(AVI_INFO *aviinfo);
void video_bmsg_show(u8* name,u16 index,u16 total);
u16 video_get_tnum(u8 *path);
u8 video_seek(FIL *favi,AVI_INFO *aviinfo,u8 *mbuf);

#endif
