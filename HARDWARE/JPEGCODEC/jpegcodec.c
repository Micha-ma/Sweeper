#include "jpegcodec.h"
#include "usart.h"
#include "malloc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������ 
//JPEGӲ��������� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/7/22
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//JPEG�淶(ISO/IEC 10918-1��׼)������������
//��ȡJPEGͼƬ����ʱ��Ҫ�õ�
const u8 JPEG_LUM_QuantTable[JPEG_QUANT_TABLE_SIZE] = 
{
	16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,
	14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,
	18,22,37,56,68,109,103,77,24,35,55,64,81,104,113,92,
	49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99
};
const u8 JPEG_ZIGZAG_ORDER[JPEG_QUANT_TABLE_SIZE]=
{
	0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
	12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
	35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
	58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
}; 

//JPEGӲ����������&���DMA����
//meminaddr:JPEG����DMA�洢����ַ. 
//memoutaddr:JPEG���DMA�洢����ַ. 
//meminsize:����DMA���ݳ���,0~262143,���ֽ�Ϊ��λ
//memoutsize:���DMA���ݳ���,0~262143,���ֽ�Ϊ��λ 
void JPEG_IN_OUT_DMA_Init(u32 meminaddr,u32 memoutaddr,u32 meminsize,u32 memoutsize)
{ 
	u32 tempreg=0;
	if(meminsize%4)meminsize+=4-meminsize%4;	//��չ��4�ı���
	meminsize/=4;								//����4
	if(memoutsize%4)memoutsize+=4-memoutsize%4;	//��չ��4�ı���
	memoutsize/=4;								//����4 
	//�����жϷ�����
	MY_NVIC_Init(2,3,DMA2_Stream0_IRQn,2);		//��ռ2�������ȼ�3����2 
	MY_NVIC_Init(2,3,DMA2_Stream1_IRQn,2);		//��ռ2�������ȼ�3����2
	
	RCC->AHB1ENR|=1<<22;			//DMA2ʱ��ʹ�� 
	DMA2_Stream0->CR=0;				//����DMA2,Stream0 
	DMA2_Stream1->CR=0;				//����DMA2,Stream1 
 	while(DMA2_Stream0->CR&0X01);	//�ȴ�DMA2_Stream0������ 
	DMA2->LIFCR|=0X3D<<6*0;			//���ͨ��0�������жϱ�־
	DMA2_Stream0->FCR=0X0000021;	//����ΪĬ��ֵ
	DMA2_Stream0->PAR=(u32)&JPEG->DIR;//�����ַΪ:JPEG->DIR
	DMA2_Stream0->M0AR=meminaddr;	//meminaddr��ΪĿ���ַ0 
	DMA2_Stream0->NDTR=meminsize;	//���䳤��Ϊmeminsize
	tempreg=1<<2;					//ʹ��FIFO
	tempreg|=3<<0;					//ȫFIFO
	DMA2_Stream0->FCR=tempreg;		//����FIFO
	tempreg=1<<4;					//������������ж�
 	tempreg|=1<<6;					//�洢��������ģʽ 
	tempreg|=0<<8;					//��ͨģʽ
	tempreg|=0<<9;					//���������ģʽ
	tempreg|=1<<10;					//�洢������ģʽ
	tempreg|=2<<11;					//�������ݳ���:32λ
	tempreg|=2<<13;					//�洢��λ��:32bit
	tempreg|=2<<16;					//�����ȼ� 
	tempreg|=1<<21;					//����4��������ͻ������
	tempreg|=1<<23;					//�洢4��������ͻ������
	tempreg|=9<<25;					//ͨ��9 JPEG IN ͨ��  
	DMA2_Stream0->CR=tempreg;		//����CR�Ĵ��� 
	
 	while(DMA2_Stream1->CR&0X01);	//�ȴ�DMA2_Stream1������ 
	DMA2->LIFCR|=0X3D<<6*1;			//���ͨ��1�������жϱ�־
	DMA2_Stream1->CR=tempreg;		//����CR�Ĵ���
	DMA2_Stream1->FCR=0X0000021;	//����ΪĬ��ֵ
	DMA2_Stream1->PAR=(u32)&JPEG->DOR;//�����ַΪ:JPEG->DOR
	DMA2_Stream1->M0AR=memoutaddr;	//memoutaddr��ΪĿ���ַ0 
	DMA2_Stream1->NDTR=memoutsize;	//���䳤��Ϊmemoutsize
	tempreg=1<<2;					//ʹ��FIFO
	tempreg|=3<<0;					//ȫFIFO
	DMA2_Stream1->FCR=tempreg;		//����FIFO 
	tempreg=1<<4;					//������������ж�
 	tempreg|=0<<6;					//���赽�洢��ģʽ 
	tempreg|=0<<8;					//��ͨģʽ
	tempreg|=0<<9;					//���������ģʽ
	tempreg|=1<<10;					//�洢������ģʽ
	tempreg|=2<<11;					//�������ݳ���:32λ
	tempreg|=2<<13;					//�洢��λ��:32bit
	tempreg|=3<<16;					//�������ȼ� 
	tempreg|=1<<21;					//����4��������ͻ������
	tempreg|=1<<23;					//�洢4��������ͻ������
	tempreg|=9<<25;					//ͨ��9 JPEG OUT ͨ��  
	DMA2_Stream1->CR=tempreg;		//����CR�Ĵ��� 
} 

void (*jpeg_in_callback)(void);		//JPEG DMA����ص�����
void (*jpeg_out_callback)(void);	//JPEG DMA��� �ص�����
void (*jpeg_eoc_callback)(void);	//JPEG ������� �ص�����
void (*jpeg_hdp_callback)(void);	//JPEG Header������� �ص�����

//DMA2_Stream0�жϷ�����
//����Ӳ��JPEG����ʱ�����������
void DMA2_Stream0_IRQHandler(void)
{              
	if(DMA2->LISR&(1<<5))			//DMA2_Steam0,������ɱ�־
	{ 
		DMA2->LIFCR|=1<<5;			//�����������ж�
		JPEG->CR&=~(1<<11);			//�ر�JPEG��DMA IN
		JPEG->CR&=~(0X7E);			//�ر�JPEG�ж�,��ֹ�����.
      	jpeg_in_callback();			//ִ������ͷ���ջص�����,��ȡ���ݵȲ����������洦��   
		JPEG->CR|=3<<5;				//ʹ��EOC��HPD�ж�.
	}     		 											 
}  
//DMA2_Stream1�жϷ�����
//����Ӳ��JPEG����������������
void DMA2_Stream1_IRQHandler(void)
{              
	if(DMA2->LISR&(1<<11))			//DMA2_Steam1,������ɱ�־
	{ 
		DMA2->LIFCR|=1<<11;			//�����������ж� 
		JPEG->CR&=~(1<<12);			//�ر�JPEG��DMA OUT
		JPEG->CR&=~(0X7E);			//�ر�JPEG�ж�,��ֹ�����.
      	jpeg_out_callback();		//ִ������ͷ���ջص�����,��ȡ���ݵȲ����������洦��  
		JPEG->CR|=3<<5;				//ʹ��EOC��HPD�ж�.
	}     		 											 
}   
//JPEG�����жϷ�����
void JPEG_IRQHandler(void)
{
	if(JPEG->SR&(1<<6))				//JPEG Header�������
	{ 
		jpeg_hdp_callback();
		JPEG->CR&=~(1<<6);			//��ֹJpeg Header��������ж�
		JPEG->CFR|=1<<6;			//���HPDFλ(header�������λ)
	}
	if(JPEG->SR&(1<<5))				//JPEG�������   
	{
		JPEG_DMA_Stop();
		jpeg_eoc_callback();
		JPEG->CFR|=1<<5;			//���EOCλ(�������λ)
		DMA2_Stream0->CR&=~(1<<0);	//����DMA2,Stream0 
		DMA2_Stream1->CR&=~(1<<0);	//����DMA2,Stream1 
	}
}

//��ʼ��Ӳ��JPEG�ں�
//tjpeg:jpeg�������ƽṹ��
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 JPEG_Core_Init(jpeg_codec_typedef *tjpeg)
{
	u8 i;
	RCC->AHB2ENR|=1<<1;				//ʹ��Ӳ��jpegʱ��
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)
	{
		tjpeg->inbuf[i].buf=mymalloc(SRAMIN,JPEG_DMA_INBUF_LEN);
		if(tjpeg->inbuf[i].buf==NULL)
		{
			JPEG_Core_Destroy(tjpeg);
			return 1;
		}   
	} 
	for(i=0;i<JPEG_DMA_OUTBUF_NB;i++)
	{
		tjpeg->outbuf[i].buf=mymalloc(SRAMIN,JPEG_DMA_OUTBUF_LEN+32);//�п��ܻ����Ҫ32�ֽ��ڴ�
		if(tjpeg->outbuf[i].buf==NULL)		
		{
			JPEG_Core_Destroy(tjpeg);
			return 1;
		}   
	}
	JPEG->CR=0;						//������
	JPEG->CR|=1<<0;					//ʹ��Ӳ��JPEG
	JPEG->CONFR0&=~(1<<0);			//ֹͣJPEG��������
	JPEG->CR|=1<<13;				//�������fifo
	JPEG->CR|=1<<14;				//������fifo
	JPEG->CFR=3<<5;					//��ձ�־ 
	MY_NVIC_Init(1,3,JPEG_IRQn,2);	//��ռ1�������ȼ�3����2  
	JPEG->CONFR1|=1<<8;				//ʹ��header����
	return 0;
}

//�ر�Ӳ��JPEG�ں�,���ͷ��ڴ�
//tjpeg:jpeg�������ƽṹ��
void JPEG_Core_Destroy(jpeg_codec_typedef *tjpeg)
{
	u8 i; 
	JPEG_DMA_Stop();//ֹͣDMA����
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)myfree(SRAMIN,tjpeg->inbuf[i].buf);		//�ͷ��ڴ�
	for(i=0;i<JPEG_DMA_OUTBUF_NB;i++)myfree(SRAMIN,tjpeg->outbuf[i].buf);	//�ͷ��ڴ�
}

//��ʼ��Ӳ��JPEG������
//tjpeg:jpeg�������ƽṹ��
void JPEG_Decode_Init(jpeg_codec_typedef *tjpeg)
{ 
	u8 i;
	tjpeg->inbuf_read_ptr=0;
	tjpeg->inbuf_write_ptr=0;
	tjpeg->indma_pause=0;
	tjpeg->outbuf_read_ptr=0;
	tjpeg->outbuf_write_ptr=0;	
	tjpeg->outdma_pause=0;	
	tjpeg->state=JPEG_STATE_NOHEADER;	//ͼƬ���������־
	tjpeg->blkindex=0;					//��ǰMCU���
	tjpeg->total_blks=0;				//��MCU��Ŀ
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)
	{
		tjpeg->inbuf[i].sta=0;
		tjpeg->inbuf[i].size=0;
	}
	for(i=0;i<JPEG_DMA_OUTBUF_NB;i++)
	{
		tjpeg->outbuf[i].sta=0;
		tjpeg->outbuf[i].size=0;
	}	
	JPEG->CONFR1|=1<<3;			//Ӳ��JPEG����ģʽ
	JPEG->CONFR0&=~(1<<0);		//ֹͣJPEG�������� 
	JPEG->CR&=~(3<<11);			//�ر�DMA
	JPEG->CR&=~(0X3F<<1);		//�ر������ж� 
	JPEG->CR|=1<<13;			//�������fifo
	JPEG->CR|=1<<14;			//������fifo
	JPEG->CR|=1<<6;				//ʹ��Jpeg Header��������ж�
	JPEG->CR|=1<<5;				//ʹ�ܽ�������ж�
	JPEG->CFR=3<<5;				//��ձ�־   
	JPEG->CONFR0|=1<<0;			//ʹ��JPEG�������� 
}
//����JPEG DMA�������
void JPEG_DMA_Start(void)
{ 
	DMA2_Stream0->CR|=1<<0;		//����DMA2,Stream0 
	DMA2_Stream1->CR|=1<<0;		//����DMA2,Stream1 
	JPEG->CR|=3<<11; 			//JPEG IN&OUT DMAʹ��	
}
//ֹͣJPEG DMA�������
void JPEG_DMA_Stop(void)
{
	JPEG->CR&=~(3<<11); 		//JPEG IN&OUT DMA��ֹ 
	JPEG->CONFR0&=~(1<<0);		//ֹͣJPEG�������� 
	JPEG->CR&=~(0X3F<<1);		//�ر������ж�  
	JPEG->CFR=3<<5;				//��ձ�־  
}
//��ͣDMA IN����
void JPEG_IN_DMA_Pause(void)
{  
	JPEG->CR&=~(1<<11);			//��ͣJPEG��DMA IN
}
//�ָ�DMA IN����
//memaddr:�洢���׵�ַ
//memlen:Ҫ�������ݳ���(���ֽ�Ϊ��λ)
void JPEG_IN_DMA_Resume(u32 memaddr,u32 memlen)
{  
	if(memlen%4)memlen+=4-memlen%4;//��չ��4�ı���
	memlen/=4;					//����4
	DMA2->LIFCR|=0X3D<<6*0;		//���ͨ��0�������жϱ�־
	DMA2_Stream0->M0AR=memaddr;	//���ô洢����ַ
	DMA2_Stream0->NDTR=memlen;	//���䳤��Ϊmemlen
	DMA2_Stream0->CR|=1<<0;		//����DMA2,Stream0 
	JPEG->CR|=1<<11; 			//�ָ�JPEG DMA IN 
}
//��ͣDMA OUT����
void JPEG_OUT_DMA_Pause(void)
{  
	JPEG->CR&=~(1<<12);			//��ͣJPEG��DMA OUT
}
//�ָ�DMA OUT����
//memaddr:�洢���׵�ַ
//memlen:Ҫ�������ݳ���(���ֽ�Ϊ��λ)
void JPEG_OUT_DMA_Resume(u32 memaddr,u32 memlen)
{  
	if(memlen%4)memlen+=4-memlen%4;//��չ��4�ı���
	memlen/=4;					//����4
	DMA2->LIFCR|=0X3D<<6*1;		//���ͨ��1�������жϱ�־
	DMA2_Stream1->M0AR=memaddr;	//���ô洢����ַ
	DMA2_Stream1->NDTR=memlen;	//���䳤��Ϊmemlen
	DMA2_Stream1->CR|=1<<0;		//����DMA2,Stream1 
	JPEG->CR|=1<<12; 			//�ָ�JPEG DMA OUT 
}
//��ȡͼ����Ϣ
//tjpeg:jpeg����ṹ��
void JPEG_Get_Info(jpeg_codec_typedef *tjpeg)
{ 
	u32 yblockNb,cBblockNb,cRblockNb; 
	switch(JPEG->CONFR1&0X03)
	{
		case 0://grayscale,1 color component
			tjpeg->Conf.ColorSpace=JPEG_GRAYSCALE_COLORSPACE;
			break;
		case 2://YUV/RGB,3 color component
			tjpeg->Conf.ColorSpace=JPEG_YCBCR_COLORSPACE;
			break;	
		case 3://CMYK,4 color component
			tjpeg->Conf.ColorSpace=JPEG_CMYK_COLORSPACE;
			break;			
	}
	tjpeg->Conf.ImageHeight=(JPEG->CONFR1&0XFFFF0000)>>16;	//���ͼ��߶�
	tjpeg->Conf.ImageWidth=(JPEG->CONFR3&0XFFFF0000)>>16;	//���ͼ����
	if((tjpeg->Conf.ColorSpace==JPEG_YCBCR_COLORSPACE)||(tjpeg->Conf.ColorSpace==JPEG_CMYK_COLORSPACE))
	{
		yblockNb  =(JPEG->CONFR4&(0XF<<4))>>4;
		cBblockNb =(JPEG->CONFR5&(0XF<<4))>>4;
		cRblockNb =(JPEG->CONFR6&(0XF<<4))>>4;
		if((yblockNb==1)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling=JPEG_422_SUBSAMPLING; //16x8 block
		else if((yblockNb==0)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING;
		else if((yblockNb==3)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling = JPEG_420_SUBSAMPLING;
		else tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING; 
	}else tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING;		//Ĭ����4:4:4 
	tjpeg->Conf.ImageQuality=0;	//ͼ����������������ͼƬ����ĩβ,�տ�ʼ��ʱ��,���޷���ȡ��,����ֱ������Ϊ0
}
//�õ�jpegͼ������
//�ڽ�����ɺ�,���Ե��ò������ȷ�Ľ��.
//����ֵ:ͼ������,0~100.
u8 JPEG_Get_Quality(void)
{
	u32 quality=0;
	u32 quantRow,quantVal,scale,i,j;
	u32 *tableAddress=(u32*)JPEG->QMEM0; 
	i=0;
	while(i<JPEG_QUANT_TABLE_SIZE)
	{
		quantRow=*tableAddress;
		for(j=0;j<4;j++)
		{
			quantVal=(quantRow>>(8*j))&0xFF;
			if(quantVal==1)quality+=100;	//100% 
			else
			{
				scale=(quantVal*100)/((u32)JPEG_LUM_QuantTable[JPEG_ZIGZAG_ORDER[i+j]]);
				if(scale<=100)quality+=(200-scale)/2;  
				else quality+=5000/scale;      
			}      
		} 
		i+=4;
		tableAddress++;    
	} 
	return (quality/((u32)64));   
}


































