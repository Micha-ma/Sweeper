#ifndef __HJPGD_H
#define __HJPGD_H
#include "sys.h"
#include "jpegcodec.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������ 
//ͼƬ���� ��������-jpegӲ�����벿��	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/7/31
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//����˵��
//��
//////////////////////////////////////////////////////////////////////////////////
	

extern jpeg_codec_typedef hjpgd;  

 

void jpeg_dma_in_callback(void);
void jpeg_dma_out_callback(void);
void jpeg_endofcovert_callback(void);
void jpeg_hdrover_callback(void);
u8 hjpgd_decode(u8* pname);

#endif




























