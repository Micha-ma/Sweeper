#ifndef __HZFONTUPDATA_H
#define __HZFONTUPDATA_H
#include "sys.h"
#include "GUI.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//STemwin �ֿ���������    
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/3/30
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

u32 emwin_fupd_prog(u32 fsize,u32 pos); //EMWINʹ�õ���ʾ���½���
u8 emwin_updata_fontx(u8 *fxpath,u8 fx);//EMWINʹ�õĸ���ָ���ֿ⺯��
u8 emwin_update_font(u8* src);    //EMWINʹ�õ��ֿ���º���
void HzFontupdataCreate(void);
#endif
