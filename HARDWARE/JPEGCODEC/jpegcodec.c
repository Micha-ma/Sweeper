#include "jpegcodec.h"
#include "usart.h"
#include "malloc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板 
//JPEG硬件编解码器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/7/22
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//JPEG规范(ISO/IEC 10918-1标准)的样本量化表
//获取JPEG图片质量时需要用到
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

//JPEG硬件解码输入&输出DMA配置
//meminaddr:JPEG输入DMA存储器地址. 
//memoutaddr:JPEG输出DMA存储器地址. 
//meminsize:输入DMA数据长度,0~262143,以字节为单位
//memoutsize:输出DMA数据长度,0~262143,以字节为单位 
void JPEG_IN_OUT_DMA_Init(u32 meminaddr,u32 memoutaddr,u32 meminsize,u32 memoutsize)
{ 
	u32 tempreg=0;
	if(meminsize%4)meminsize+=4-meminsize%4;	//扩展到4的倍数
	meminsize/=4;								//除以4
	if(memoutsize%4)memoutsize+=4-memoutsize%4;	//扩展到4的倍数
	memoutsize/=4;								//除以4 
	//开启中断服务函数
	MY_NVIC_Init(2,3,DMA2_Stream0_IRQn,2);		//抢占2，子优先级3，组2 
	MY_NVIC_Init(2,3,DMA2_Stream1_IRQn,2);		//抢占2，子优先级3，组2
	
	RCC->AHB1ENR|=1<<22;			//DMA2时钟使能 
	DMA2_Stream0->CR=0;				//开启DMA2,Stream0 
	DMA2_Stream1->CR=0;				//开启DMA2,Stream1 
 	while(DMA2_Stream0->CR&0X01);	//等待DMA2_Stream0可配置 
	DMA2->LIFCR|=0X3D<<6*0;			//清空通道0上所有中断标志
	DMA2_Stream0->FCR=0X0000021;	//设置为默认值
	DMA2_Stream0->PAR=(u32)&JPEG->DIR;//外设地址为:JPEG->DIR
	DMA2_Stream0->M0AR=meminaddr;	//meminaddr作为目标地址0 
	DMA2_Stream0->NDTR=meminsize;	//传输长度为meminsize
	tempreg=1<<2;					//使能FIFO
	tempreg|=3<<0;					//全FIFO
	DMA2_Stream0->FCR=tempreg;		//设置FIFO
	tempreg=1<<4;					//开启传输完成中断
 	tempreg|=1<<6;					//存储器到外设模式 
	tempreg|=0<<8;					//普通模式
	tempreg|=0<<9;					//外设非增量模式
	tempreg|=1<<10;					//存储器增量模式
	tempreg|=2<<11;					//外设数据长度:32位
	tempreg|=2<<13;					//存储器位宽:32bit
	tempreg|=2<<16;					//高优先级 
	tempreg|=1<<21;					//外设4节拍增量突发传输
	tempreg|=1<<23;					//存储4节拍增量突发传输
	tempreg|=9<<25;					//通道9 JPEG IN 通道  
	DMA2_Stream0->CR=tempreg;		//设置CR寄存器 
	
 	while(DMA2_Stream1->CR&0X01);	//等待DMA2_Stream1可配置 
	DMA2->LIFCR|=0X3D<<6*1;			//清空通道1上所有中断标志
	DMA2_Stream1->CR=tempreg;		//设置CR寄存器
	DMA2_Stream1->FCR=0X0000021;	//设置为默认值
	DMA2_Stream1->PAR=(u32)&JPEG->DOR;//外设地址为:JPEG->DOR
	DMA2_Stream1->M0AR=memoutaddr;	//memoutaddr作为目标地址0 
	DMA2_Stream1->NDTR=memoutsize;	//传输长度为memoutsize
	tempreg=1<<2;					//使能FIFO
	tempreg|=3<<0;					//全FIFO
	DMA2_Stream1->FCR=tempreg;		//设置FIFO 
	tempreg=1<<4;					//开启传输完成中断
 	tempreg|=0<<6;					//外设到存储器模式 
	tempreg|=0<<8;					//普通模式
	tempreg|=0<<9;					//外设非增量模式
	tempreg|=1<<10;					//存储器增量模式
	tempreg|=2<<11;					//外设数据长度:32位
	tempreg|=2<<13;					//存储器位宽:32bit
	tempreg|=3<<16;					//极高优先级 
	tempreg|=1<<21;					//外设4节拍增量突发传输
	tempreg|=1<<23;					//存储4节拍增量突发传输
	tempreg|=9<<25;					//通道9 JPEG OUT 通道  
	DMA2_Stream1->CR=tempreg;		//设置CR寄存器 
} 

void (*jpeg_in_callback)(void);		//JPEG DMA输入回调函数
void (*jpeg_out_callback)(void);	//JPEG DMA输出 回调函数
void (*jpeg_eoc_callback)(void);	//JPEG 解码完成 回调函数
void (*jpeg_hdp_callback)(void);	//JPEG Header解码完成 回调函数

//DMA2_Stream0中断服务函数
//处理硬件JPEG解码时输入的数据流
void DMA2_Stream0_IRQHandler(void)
{              
	if(DMA2->LISR&(1<<5))			//DMA2_Steam0,传输完成标志
	{ 
		DMA2->LIFCR|=1<<5;			//清除传输完成中断
		JPEG->CR&=~(1<<11);			//关闭JPEG的DMA IN
		JPEG->CR&=~(0X7E);			//关闭JPEG中断,防止被打断.
      	jpeg_in_callback();			//执行摄像头接收回调函数,读取数据等操作在这里面处理   
		JPEG->CR|=3<<5;				//使能EOC和HPD中断.
	}     		 											 
}  
//DMA2_Stream1中断服务函数
//处理硬件JPEG解码后输出的数据流
void DMA2_Stream1_IRQHandler(void)
{              
	if(DMA2->LISR&(1<<11))			//DMA2_Steam1,传输完成标志
	{ 
		DMA2->LIFCR|=1<<11;			//清除传输完成中断 
		JPEG->CR&=~(1<<12);			//关闭JPEG的DMA OUT
		JPEG->CR&=~(0X7E);			//关闭JPEG中断,防止被打断.
      	jpeg_out_callback();		//执行摄像头接收回调函数,读取数据等操作在这里面处理  
		JPEG->CR|=3<<5;				//使能EOC和HPD中断.
	}     		 											 
}   
//JPEG解码中断服务函数
void JPEG_IRQHandler(void)
{
	if(JPEG->SR&(1<<6))				//JPEG Header解码完成
	{ 
		jpeg_hdp_callback();
		JPEG->CR&=~(1<<6);			//禁止Jpeg Header解码完成中断
		JPEG->CFR|=1<<6;			//清除HPDF位(header解码完成位)
	}
	if(JPEG->SR&(1<<5))				//JPEG解码完成   
	{
		JPEG_DMA_Stop();
		jpeg_eoc_callback();
		JPEG->CFR|=1<<5;			//清除EOC位(解码完成位)
		DMA2_Stream0->CR&=~(1<<0);	//开启DMA2,Stream0 
		DMA2_Stream1->CR&=~(1<<0);	//开启DMA2,Stream1 
	}
}

//初始化硬件JPEG内核
//tjpeg:jpeg编解码控制结构体
//返回值:0,成功;
//    其他,失败
u8 JPEG_Core_Init(jpeg_codec_typedef *tjpeg)
{
	u8 i;
	RCC->AHB2ENR|=1<<1;				//使能硬件jpeg时钟
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
		tjpeg->outbuf[i].buf=mymalloc(SRAMIN,JPEG_DMA_OUTBUF_LEN+32);//有可能会多需要32字节内存
		if(tjpeg->outbuf[i].buf==NULL)		
		{
			JPEG_Core_Destroy(tjpeg);
			return 1;
		}   
	}
	JPEG->CR=0;						//先清零
	JPEG->CR|=1<<0;					//使能硬件JPEG
	JPEG->CONFR0&=~(1<<0);			//停止JPEG编解码进程
	JPEG->CR|=1<<13;				//清空输入fifo
	JPEG->CR|=1<<14;				//清空输出fifo
	JPEG->CFR=3<<5;					//清空标志 
	MY_NVIC_Init(1,3,JPEG_IRQn,2);	//抢占1，子优先级3，组2  
	JPEG->CONFR1|=1<<8;				//使能header处理
	return 0;
}

//关闭硬件JPEG内核,并释放内存
//tjpeg:jpeg编解码控制结构体
void JPEG_Core_Destroy(jpeg_codec_typedef *tjpeg)
{
	u8 i; 
	JPEG_DMA_Stop();//停止DMA传输
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)myfree(SRAMIN,tjpeg->inbuf[i].buf);		//释放内存
	for(i=0;i<JPEG_DMA_OUTBUF_NB;i++)myfree(SRAMIN,tjpeg->outbuf[i].buf);	//释放内存
}

//初始化硬件JPEG解码器
//tjpeg:jpeg编解码控制结构体
void JPEG_Decode_Init(jpeg_codec_typedef *tjpeg)
{ 
	u8 i;
	tjpeg->inbuf_read_ptr=0;
	tjpeg->inbuf_write_ptr=0;
	tjpeg->indma_pause=0;
	tjpeg->outbuf_read_ptr=0;
	tjpeg->outbuf_write_ptr=0;	
	tjpeg->outdma_pause=0;	
	tjpeg->state=JPEG_STATE_NOHEADER;	//图片解码结束标志
	tjpeg->blkindex=0;					//当前MCU编号
	tjpeg->total_blks=0;				//总MCU数目
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
	JPEG->CONFR1|=1<<3;			//硬件JPEG解码模式
	JPEG->CONFR0&=~(1<<0);		//停止JPEG编解码进程 
	JPEG->CR&=~(3<<11);			//关闭DMA
	JPEG->CR&=~(0X3F<<1);		//关闭所有中断 
	JPEG->CR|=1<<13;			//清空输入fifo
	JPEG->CR|=1<<14;			//清空输出fifo
	JPEG->CR|=1<<6;				//使能Jpeg Header解码完成中断
	JPEG->CR|=1<<5;				//使能解码完成中断
	JPEG->CFR=3<<5;				//清空标志   
	JPEG->CONFR0|=1<<0;			//使能JPEG编解码进程 
}
//启动JPEG DMA解码过程
void JPEG_DMA_Start(void)
{ 
	DMA2_Stream0->CR|=1<<0;		//开启DMA2,Stream0 
	DMA2_Stream1->CR|=1<<0;		//开启DMA2,Stream1 
	JPEG->CR|=3<<11; 			//JPEG IN&OUT DMA使能	
}
//停止JPEG DMA解码过程
void JPEG_DMA_Stop(void)
{
	JPEG->CR&=~(3<<11); 		//JPEG IN&OUT DMA禁止 
	JPEG->CONFR0&=~(1<<0);		//停止JPEG编解码进程 
	JPEG->CR&=~(0X3F<<1);		//关闭所有中断  
	JPEG->CFR=3<<5;				//清空标志  
}
//暂停DMA IN过程
void JPEG_IN_DMA_Pause(void)
{  
	JPEG->CR&=~(1<<11);			//暂停JPEG的DMA IN
}
//恢复DMA IN过程
//memaddr:存储区首地址
//memlen:要传输数据长度(以字节为单位)
void JPEG_IN_DMA_Resume(u32 memaddr,u32 memlen)
{  
	if(memlen%4)memlen+=4-memlen%4;//扩展到4的倍数
	memlen/=4;					//除以4
	DMA2->LIFCR|=0X3D<<6*0;		//清空通道0上所有中断标志
	DMA2_Stream0->M0AR=memaddr;	//设置存储器地址
	DMA2_Stream0->NDTR=memlen;	//传输长度为memlen
	DMA2_Stream0->CR|=1<<0;		//开启DMA2,Stream0 
	JPEG->CR|=1<<11; 			//恢复JPEG DMA IN 
}
//暂停DMA OUT过程
void JPEG_OUT_DMA_Pause(void)
{  
	JPEG->CR&=~(1<<12);			//暂停JPEG的DMA OUT
}
//恢复DMA OUT过程
//memaddr:存储区首地址
//memlen:要传输数据长度(以字节为单位)
void JPEG_OUT_DMA_Resume(u32 memaddr,u32 memlen)
{  
	if(memlen%4)memlen+=4-memlen%4;//扩展到4的倍数
	memlen/=4;					//除以4
	DMA2->LIFCR|=0X3D<<6*1;		//清空通道1上所有中断标志
	DMA2_Stream1->M0AR=memaddr;	//设置存储器地址
	DMA2_Stream1->NDTR=memlen;	//传输长度为memlen
	DMA2_Stream1->CR|=1<<0;		//开启DMA2,Stream1 
	JPEG->CR|=1<<12; 			//恢复JPEG DMA OUT 
}
//获取图像信息
//tjpeg:jpeg解码结构体
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
	tjpeg->Conf.ImageHeight=(JPEG->CONFR1&0XFFFF0000)>>16;	//获得图像高度
	tjpeg->Conf.ImageWidth=(JPEG->CONFR3&0XFFFF0000)>>16;	//获得图像宽度
	if((tjpeg->Conf.ColorSpace==JPEG_YCBCR_COLORSPACE)||(tjpeg->Conf.ColorSpace==JPEG_CMYK_COLORSPACE))
	{
		yblockNb  =(JPEG->CONFR4&(0XF<<4))>>4;
		cBblockNb =(JPEG->CONFR5&(0XF<<4))>>4;
		cRblockNb =(JPEG->CONFR6&(0XF<<4))>>4;
		if((yblockNb==1)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling=JPEG_422_SUBSAMPLING; //16x8 block
		else if((yblockNb==0)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING;
		else if((yblockNb==3)&&(cBblockNb==0)&&(cRblockNb==0))tjpeg->Conf.ChromaSubsampling = JPEG_420_SUBSAMPLING;
		else tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING; 
	}else tjpeg->Conf.ChromaSubsampling=JPEG_444_SUBSAMPLING;		//默认用4:4:4 
	tjpeg->Conf.ImageQuality=0;	//图像质量参数在整个图片的最末尾,刚开始的时候,是无法获取的,所以直接设置为0
}
//得到jpeg图像质量
//在解码完成后,可以调用并获得正确的结果.
//返回值:图像质量,0~100.
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


































