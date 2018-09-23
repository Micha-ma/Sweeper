#include "mjpeg.h" 
#include "malloc.h"
#include "ff.h"  
#include "tftlcd.h" 
#include "ltdc.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//MJPEG��Ƶ���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/8/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   

jpeg_codec_typedef mjpeg;  		//JPEGӲ������ṹ��
u16 imgoffx,imgoffy;			//ͼ����LCD��Ļ��x,y�����ƫ����
u16* rgb565buf;					//������RGBͼ���Դ��ַ

u16 *mjpeg_rgb_framebuf;		//RGB��֡�����ַ
extern u32 *ltdc_framebuf[2];	//LTDC LCD֡��������ָ��,��ltdc.c���涨��

	
//JPEG����������,�ص�����,���ڻ�ȡJPEG�ļ�ԭʼ����
//ÿ��JPEG DMA IN BUFΪ�յ�ʱ��,���øú���
void mjpeg_dma_in_callback(void)
{ 
	mjpeg.inbuf[mjpeg.inbuf_read_ptr].sta=0;	//��buf�Ѿ���������
	mjpeg.inbuf[mjpeg.inbuf_read_ptr].size=0;	//��buf�Ѿ��������� 
	mjpeg.inbuf_read_ptr++;						//ָ����һ��buf
	if(mjpeg.inbuf_read_ptr>=JPEG_DMA_INBUF_NB)mjpeg.inbuf_read_ptr=0;//����
	if(mjpeg.inbuf[mjpeg.inbuf_read_ptr].sta==0)//����Чbuf
	{
		JPEG_IN_DMA_Pause();					//��ͣ��ȡ����
		mjpeg.indma_pause=1;					//��ͣ��ȡ����
	}else										//��Ч��buf
	{
		JPEG_IN_DMA_Resume((u32)mjpeg.inbuf[mjpeg.inbuf_read_ptr].buf,mjpeg.inbuf[mjpeg.inbuf_read_ptr].size);//������һ��DMA����
	}
}
//JPEG���������(YCBCR)�ص�����,�������YCbCr������
void mjpeg_dma_out_callback(void)
{	  
	u32 *pdata=0; 
	mjpeg.outbuf[mjpeg.outbuf_write_ptr].sta=1;	//��buf����
	mjpeg.outbuf[mjpeg.outbuf_write_ptr].size=JPEG_DMA_OUTBUF_LEN-(DMA2_Stream1->NDTR<<2);//��buf�������ݵĳ���
	if(mjpeg.state==JPEG_STATE_FINISHED)		//����ļ��Ѿ��������,��Ҫ��ȡDOR��������(<=32�ֽ�)
	{
		pdata=(u32*)(mjpeg.outbuf[mjpeg.outbuf_write_ptr].buf+mjpeg.outbuf[mjpeg.outbuf_write_ptr].size);
		while(JPEG->SR&(1<<4))
		{
			*pdata=JPEG->DOR;
			pdata++;
			mjpeg.outbuf[mjpeg.outbuf_write_ptr].size+=4; 
		}
	}  
	mjpeg.outbuf_write_ptr++;					//ָ����һ��buf
	if(mjpeg.outbuf_write_ptr>=JPEG_DMA_OUTBUF_NB)mjpeg.outbuf_write_ptr=0;//����
	if(mjpeg.outbuf[mjpeg.outbuf_write_ptr].sta==1)//����Чbuf
	{
		JPEG_OUT_DMA_Pause();					//��ͣ�������
		mjpeg.outdma_pause=1;					//��ͣ�������
	}else										//��Ч��buf
	{
		JPEG_OUT_DMA_Resume((u32)mjpeg.outbuf[mjpeg.outbuf_write_ptr].buf,JPEG_DMA_OUTBUF_LEN);//������һ��DMA����
	}
}
//JPEG�����ļ�������ɻص�����
void mjpeg_endofcovert_callback(void)
{ 
	mjpeg.state=JPEG_STATE_FINISHED;			//���JPEG�������
}
//JPEG header�����ɹ��ص�����
void mjpeg_hdrover_callback(void)
{
	mjpeg.state=JPEG_STATE_HEADEROK;			//HEADER��ȡ�ɹ�
	JPEG_Get_Info(&mjpeg);						//��ȡJPEG�����Ϣ,������С,ɫ�ʿռ�,������
	JPEG_GetDecodeColorConvertFunc(&mjpeg.Conf,&mjpeg.ycbcr2rgb,&mjpeg.total_blks);//��ȡJPEGɫ��ת������,�Լ���MCU�� 
}

//��ʼ��MJPEG
//offx,offy:��Ƶ��LCD����,X,Y�����ƫ����
//width,height:ͼ��Ŀ�Ⱥ͸߶�
//����ֵ:0,�ɹ�;����,ʧ��.
u8 mjpeg_init(u16 offx,u16 offy,u32 width,u32 height)
{
	u8 res;
	res=JPEG_Core_Init(&mjpeg);					//��ʼ��JPEG�ں�
	if(res)return 1;
	rgb565buf=mymalloc(SRAMEX,width*height*2);	//����RGB����
	if(rgb565buf==NULL)return  2;
	imgoffx=offx;
	imgoffy=offy;
	mjpeg_rgb_framebuf=(u16*)ltdc_framebuf[lcdltdc.activelayer];//ָ��RGBLCD��ǰ�Դ� 
	return 0; 
}
//�ͷ��ڴ�
void mjpeg_free(void)
{ 
	JPEG_Core_Destroy(&mjpeg); 
	myfree(SRAMEX,rgb565buf);	
}

//�����ɫ
//x,y:��ʼ����
//width��height����Ⱥ͸߶ȡ�
//*color����ɫ����
void mjpeg_fill_color(u16 x,u16 y,u16 width,u16 height,u16 *color)
{  
	u16 i,j;
	u32 param1;	
	u32 param2;
	u32 param3;
	u16* pdata; 
	if(lcdltdc.pwidth!=0&&lcddev.dir==0)//�����RGB��,������,����亯������ֱ����
	{ 
		param1=lcdltdc.pixsize*lcdltdc.pwidth*(lcdltdc.pheight-x-1)+lcdltdc.pixsize*y;	//������������,����ٶ�
		param2=lcdltdc.pixsize*lcdltdc.pwidth;
		for(i=0;i<height;i++)
		{
			param3=i*lcdltdc.pixsize+param1;
			pdata=color+i*width;
			for(j=0;j<width;j++)
			{ 
				*(u16*)((u32)mjpeg_rgb_framebuf+param3-param2*j)=pdata[j];  
			}
		}
	}else LCD_Color_Fill(x,y,x+width-1,y+height-1,color);//�������,ֱ�����	
}

//����һ��JPEGͼƬ
//ע��:
//1,������ͼƬ�ķֱ���,����С�ڵ�����Ļ�ķֱ���!
//2,�뱣֤ͼƬ�Ŀ����16�ı���,���������ֻ���.
//buf:jpeg����������
//bsize:�����С
//����ֵ:0,�ɹ�
//    ����,����
u8 mjpeg_decode(u8* buf,u32 bsize) 
{ 
	vu32 timecnt=0;
	u8 fileover=0;
	u8 i=0;  
	u32 mcublkindex=0;    
	if(bsize==0)return 0;
	JPEG_Decode_Init(&mjpeg);						//��ʼ��Ӳ��JPEG������
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)
	{
		if(bsize>JPEG_DMA_INBUF_LEN)
		{
			mymemcpy(mjpeg.inbuf[i].buf,buf,JPEG_DMA_INBUF_LEN);
			mjpeg.inbuf[i].size=JPEG_DMA_INBUF_LEN;	//��ȡ�˵����ݳ���
			mjpeg.inbuf[i].sta=1;					//���buf�� 
			buf+=JPEG_DMA_INBUF_LEN;				//Դ��������ƫ��
			bsize-=JPEG_DMA_INBUF_LEN;				//�ļ���С����
		}else
		{
			mymemcpy(mjpeg.inbuf[i].buf,buf,bsize);
			mjpeg.inbuf[i].size=bsize;				//��ȡ�˵����ݳ���
			mjpeg.inbuf[i].sta=1;					//���buf��
			buf+=bsize;								//Դ��������ƫ��
			bsize=0;								//�ļ���СΪ0��.
			break;
		}
	}
	JPEG_IN_OUT_DMA_Init((u32)mjpeg.inbuf[0].buf,(u32)mjpeg.outbuf[0].buf,mjpeg.inbuf[0].size,JPEG_DMA_OUTBUF_LEN);//����DMA
	jpeg_in_callback=mjpeg_dma_in_callback;			//JPEG DMA��ȡ���ݻص�����
	jpeg_out_callback=mjpeg_dma_out_callback; 		//JPEG DMA������ݻص�����
	jpeg_eoc_callback=mjpeg_endofcovert_callback;	//JPEG ��������ص�����
	jpeg_hdp_callback=mjpeg_hdrover_callback; 		//JPEG Header������ɻص�����
	JPEG_DMA_Start();								//����DMA���� 
	while(1)
	{
        SCB_CleanInvalidateDCache();				//���D catch
		if(mjpeg.inbuf[mjpeg.inbuf_write_ptr].sta==0&&fileover==0)	//��bufΪ��
		{
			if(bsize>JPEG_DMA_INBUF_LEN)
			{
				mymemcpy(mjpeg.inbuf[mjpeg.inbuf_write_ptr].buf,buf,JPEG_DMA_INBUF_LEN);
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].size=JPEG_DMA_INBUF_LEN;	//��ȡ�˵����ݳ���
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].sta=1;					//���buf�� 
				buf+=JPEG_DMA_INBUF_LEN;									//Դ��������ƫ��
				bsize-=JPEG_DMA_INBUF_LEN;									//�ļ���С����
			}else
			{
				mymemcpy(mjpeg.inbuf[mjpeg.inbuf_write_ptr].buf,buf,bsize);
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].size=bsize;				//��ȡ�˵����ݳ���
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].sta=1;					//���buf��
				buf+=bsize;													//Դ��������ƫ��
				bsize=0;													//�ļ���СΪ0��.
				timecnt=0;	//�����ʱ��
				fileover=1;	//�ļ�������...
			} 
			if(mjpeg.indma_pause==1&&mjpeg.inbuf[mjpeg.inbuf_read_ptr].sta==1)//֮ǰ����ͣ����,��������
			{
				JPEG_IN_DMA_Resume((u32)mjpeg.inbuf[mjpeg.inbuf_read_ptr].buf,mjpeg.inbuf[mjpeg.inbuf_read_ptr].size);	//������һ��DMA����
				mjpeg.indma_pause=0;
			}
			mjpeg.inbuf_write_ptr++;
			if(mjpeg.inbuf_write_ptr>=JPEG_DMA_INBUF_NB)mjpeg.inbuf_write_ptr=0;
		}
		if(mjpeg.outbuf[mjpeg.outbuf_read_ptr].sta==1)	//buf����������Ҫ����
		{
			mcublkindex+=mjpeg.ycbcr2rgb(mjpeg.outbuf[mjpeg.outbuf_read_ptr].buf,(u8*)rgb565buf,mcublkindex,mjpeg.outbuf[mjpeg.outbuf_read_ptr].size);//YUV --> RGB565
			mjpeg.outbuf[mjpeg.outbuf_read_ptr].sta=0;	//���bufΪ��
			mjpeg.outbuf[mjpeg.outbuf_read_ptr].size=0;	//���������
			mjpeg.outbuf_read_ptr++;
			if(mjpeg.outbuf_read_ptr>=JPEG_DMA_OUTBUF_NB)mjpeg.outbuf_read_ptr=0;//���Ʒ�Χ
			if(mcublkindex==mjpeg.total_blks)
			{
				break;
			}
		}else if(mjpeg.outdma_pause==1&&mjpeg.outbuf[mjpeg.outbuf_write_ptr].sta==0)		//out��ͣ,�ҵ�ǰwritebuf�Ѿ�Ϊ����,��ָ�out���
		{
			JPEG_OUT_DMA_Resume((u32)mjpeg.outbuf[mjpeg.outbuf_write_ptr].buf,JPEG_DMA_OUTBUF_LEN);//������һ��DMA����
			mjpeg.outdma_pause=0;
		}
		timecnt++; 
		if(fileover)//�ļ�������,��ʱ�˳�,��ֹ��ѭ��
		{
			if(mjpeg.state==JPEG_STATE_NOHEADER)break;	//����ʧ����
			if(timecnt>0X3FFF)break;					//��ʱ�˳�
		}
	}    
	if(mjpeg.state==JPEG_STATE_FINISHED)	//���������
	{
		mjpeg_fill_color(imgoffx,imgoffy,mjpeg.Conf.ImageWidth,mjpeg.Conf.ImageHeight,rgb565buf);  
	} 
	return 0;
}

























