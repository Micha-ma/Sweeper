#include "hjpgd.h"
#include "piclib.h"  
#include "usart.h"
#include "timer.h"
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
	

jpeg_codec_typedef hjpgd;  	//JPEGӲ������ṹ��

//JPEG����������,�ص�����,���ڻ�ȡJPEG�ļ�ԭʼ����
//ÿ��JPEG DMA IN BUFΪ�յ�ʱ��,���øú���
void jpeg_dma_in_callback(void)
{ 
	hjpgd.inbuf[hjpgd.inbuf_read_ptr].sta=0;	//��buf�Ѿ���������
	hjpgd.inbuf[hjpgd.inbuf_read_ptr].size=0;	//��buf�Ѿ��������� 
	hjpgd.inbuf_read_ptr++;						//ָ����һ��buf
	if(hjpgd.inbuf_read_ptr>=JPEG_DMA_INBUF_NB)hjpgd.inbuf_read_ptr=0;//����
	if(hjpgd.inbuf[hjpgd.inbuf_read_ptr].sta==0)//����Чbuf
	{
		JPEG_IN_DMA_Pause();					//��ͣ��ȡ����
		hjpgd.indma_pause=1;					//��ͣ��ȡ����
	}else										//��Ч��buf
	{
		JPEG_IN_DMA_Resume((u32)hjpgd.inbuf[hjpgd.inbuf_read_ptr].buf,hjpgd.inbuf[hjpgd.inbuf_read_ptr].size);//������һ��DMA����
	}
}
//JPEG���������(YCBCR)�ص�����,�������YCbCr������
void jpeg_dma_out_callback(void)
{	  
	u32 *pdata=0; 
	hjpgd.outbuf[hjpgd.outbuf_write_ptr].sta=1;	//��buf����
	hjpgd.outbuf[hjpgd.outbuf_write_ptr].size=JPEG_DMA_OUTBUF_LEN-(DMA2_Stream1->NDTR<<2);//��buf�������ݵĳ���
	if(hjpgd.state==JPEG_STATE_FINISHED)		//����ļ��Ѿ��������,��Ҫ��ȡDOR��������(<=32�ֽ�)
	{
		pdata=(u32*)(hjpgd.outbuf[hjpgd.outbuf_write_ptr].buf+hjpgd.outbuf[hjpgd.outbuf_write_ptr].size);
		while(JPEG->SR&(1<<4))
		{
			*pdata=JPEG->DOR;
			pdata++;
			hjpgd.outbuf[hjpgd.outbuf_write_ptr].size+=4; 
		}
	}  
	hjpgd.outbuf_write_ptr++;					//ָ����һ��buf
	if(hjpgd.outbuf_write_ptr>=JPEG_DMA_OUTBUF_NB)hjpgd.outbuf_write_ptr=0;//����
	if(hjpgd.outbuf[hjpgd.outbuf_write_ptr].sta==1)//����Чbuf
	{
		JPEG_OUT_DMA_Pause();					//��ͣ�������
		hjpgd.outdma_pause=1;					//��ͣ�������
	}else										//��Ч��buf
	{
		JPEG_OUT_DMA_Resume((u32)hjpgd.outbuf[hjpgd.outbuf_write_ptr].buf,JPEG_DMA_OUTBUF_LEN);//������һ��DMA����
	}
}
//JPEG�����ļ�������ɻص�����
void jpeg_endofcovert_callback(void)
{ 
	hjpgd.state=JPEG_STATE_FINISHED;			//���JPEG�������
}
//JPEG header�����ɹ��ص�����
void jpeg_hdrover_callback(void)
{
	hjpgd.state=JPEG_STATE_HEADEROK;			//HEADER��ȡ�ɹ�
	JPEG_Get_Info(&hjpgd);						//��ȡJPEG�����Ϣ,������С,ɫ�ʿռ�,������
	JPEG_GetDecodeColorConvertFunc(&hjpgd.Conf,&hjpgd.ycbcr2rgb,&hjpgd.total_blks);//��ȡJPEGɫ��ת������,�Լ���MCU��
	picinfo.ImgWidth=hjpgd.Conf.ImageWidth;
	picinfo.ImgHeight=hjpgd.Conf.ImageHeight; 
	ai_draw_init();
}
  
//JPEGӲ������ͼƬ
//ע��:
//1,������ͼƬ�ķֱ���,����С�ڵ�����Ļ�ķֱ���!
//2,�뱣֤ͼƬ�Ŀ����16�ı���,���������ֻ���.
//pname:ͼƬ����(��·��)
//����ֵ:0,�ɹ�
//    ����,ʧ��
u8 hjpgd_decode(u8* pname)
{
	FIL* ftemp; 
	u16* rgb565buf;
	vu32 timecnt=0;
	u8 fileover=0;
	u8 i=0;
	u8 res;
	u32 mcublkindex=0;  
	res=JPEG_Core_Init(&hjpgd);						//��ʼ��JPEG�ں�
	if(res)return 1;
	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//�����ڴ�  
	if(f_open(ftemp,(char*)pname,FA_READ)!=FR_OK)	//��ͼƬʧ��
    {
		JPEG_Core_Destroy(&hjpgd);
		myfree(SRAMIN,ftemp);						//�ͷ��ڴ�
		return 2;
	} 
	rgb565buf=mymalloc(SRAMEX,800*480*2);			//������֡�ڴ�
	JPEG_Decode_Init(&hjpgd);						//��ʼ��Ӳ��JPEG������
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)
	{
		res=f_read(ftemp,hjpgd.inbuf[i].buf,JPEG_DMA_INBUF_LEN,&br);//���������������ݻ�����
		if(res==FR_OK&&br)
		{
			hjpgd.inbuf[i].size=br;					//��ȡ
			hjpgd.inbuf[i].sta=1;					//���buf�� 
		}
		if(br==0)break; 
	}
	JPEG_IN_OUT_DMA_Init((u32)hjpgd.inbuf[0].buf,(u32)hjpgd.outbuf[0].buf,hjpgd.inbuf[0].size,JPEG_DMA_OUTBUF_LEN);//����DMA
	jpeg_in_callback=jpeg_dma_in_callback;			//JPEG DMA��ȡ���ݻص�����
	jpeg_out_callback=jpeg_dma_out_callback; 		//JPEG DMA������ݻص�����
	jpeg_eoc_callback=jpeg_endofcovert_callback;	//JPEG ��������ص�����
	jpeg_hdp_callback=jpeg_hdrover_callback; 		//JPEG Header������ɻص�����
	JPEG_DMA_Start();								//����DMA���� 
	while(1)
	{
        SCB_CleanInvalidateDCache();				//���D catch
		if(hjpgd.inbuf[hjpgd.inbuf_write_ptr].sta==0&&fileover==0)	//��bufΪ��
		{
			res=f_read(ftemp,hjpgd.inbuf[hjpgd.inbuf_write_ptr].buf,JPEG_DMA_INBUF_LEN,&br);//����һ��������
 			if(res==FR_OK&&br)
			{
				hjpgd.inbuf[hjpgd.inbuf_write_ptr].size=br;	//��ȡ
				hjpgd.inbuf[hjpgd.inbuf_write_ptr].sta=1;	//buf�� 
			}else if(br==0)
			{
				timecnt=0;	//�����ʱ��
				fileover=1;	//�ļ�������...
			}
			if(hjpgd.indma_pause==1&&hjpgd.inbuf[hjpgd.inbuf_read_ptr].sta==1)//֮ǰ����ͣ����,��������
			{
				JPEG_IN_DMA_Resume((u32)hjpgd.inbuf[hjpgd.inbuf_read_ptr].buf,hjpgd.inbuf[hjpgd.inbuf_read_ptr].size);	//������һ��DMA����
				hjpgd.indma_pause=0;
			}
			hjpgd.inbuf_write_ptr++;
			if(hjpgd.inbuf_write_ptr>=JPEG_DMA_INBUF_NB)hjpgd.inbuf_write_ptr=0;
		}
		if(hjpgd.outbuf[hjpgd.outbuf_read_ptr].sta==1)	//buf����������Ҫ����
		{
			mcublkindex+=hjpgd.ycbcr2rgb(hjpgd.outbuf[hjpgd.outbuf_read_ptr].buf,(u8*)rgb565buf,mcublkindex,hjpgd.outbuf[hjpgd.outbuf_read_ptr].size); 
			hjpgd.outbuf[hjpgd.outbuf_read_ptr].sta=0;	//���bufΪ��
			hjpgd.outbuf[hjpgd.outbuf_read_ptr].size=0;	//���������
			hjpgd.outbuf_read_ptr++;
			if(hjpgd.outbuf_read_ptr>=JPEG_DMA_OUTBUF_NB)hjpgd.outbuf_read_ptr=0;//���Ʒ�Χ
			if(mcublkindex==hjpgd.total_blks)
			{
				break;
			}
		}else if(hjpgd.outdma_pause==1&&hjpgd.outbuf[hjpgd.outbuf_write_ptr].sta==0)		//out��ͣ,�ҵ�ǰwritebuf�Ѿ�Ϊ����,��ָ�out���
		{
			JPEG_OUT_DMA_Resume((u32)hjpgd.outbuf[hjpgd.outbuf_write_ptr].buf,JPEG_DMA_OUTBUF_LEN);//������һ��DMA����
			hjpgd.outdma_pause=0;
		}
		timecnt++; 
		if(fileover)//�ļ�������,��ʱ�˳�,��ֹ��ѭ��
		{
			if(hjpgd.state==JPEG_STATE_NOHEADER)break;	//����ʧ����
			if(timecnt>0X3FFF)break;					//��ʱ�˳�
		}
	}    
	if(hjpgd.state==JPEG_STATE_FINISHED)	//���������
	{
		piclib_fill_color(picinfo.S_XOFF,picinfo.S_YOFF,hjpgd.Conf.ImageWidth,hjpgd.Conf.ImageHeight,rgb565buf);  
	} 
	myfree(SRAMIN,ftemp);
	myfree(SRAMEX,rgb565buf);	
	JPEG_Core_Destroy(&hjpgd); 
	return 0;
}











