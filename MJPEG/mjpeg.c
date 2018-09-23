#include "mjpeg.h" 
#include "malloc.h"
#include "ff.h"  
#include "tftlcd.h" 
#include "ltdc.h" 
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

jpeg_codec_typedef mjpeg;  		//JPEG硬件解码结构体
u16 imgoffx,imgoffy;			//图像在LCD屏幕上x,y方向的偏移量
u16* rgb565buf;					//解码后的RGB图像显存地址

u16 *mjpeg_rgb_framebuf;		//RGB屏帧缓存地址
extern u32 *ltdc_framebuf[2];	//LTDC LCD帧缓存数组指针,在ltdc.c里面定义

	
//JPEG输入数据流,回调函数,用于获取JPEG文件原始数据
//每当JPEG DMA IN BUF为空的时候,调用该函数
void mjpeg_dma_in_callback(void)
{ 
	mjpeg.inbuf[mjpeg.inbuf_read_ptr].sta=0;	//此buf已经处理完了
	mjpeg.inbuf[mjpeg.inbuf_read_ptr].size=0;	//此buf已经处理完了 
	mjpeg.inbuf_read_ptr++;						//指向下一个buf
	if(mjpeg.inbuf_read_ptr>=JPEG_DMA_INBUF_NB)mjpeg.inbuf_read_ptr=0;//归零
	if(mjpeg.inbuf[mjpeg.inbuf_read_ptr].sta==0)//无有效buf
	{
		JPEG_IN_DMA_Pause();					//暂停读取数据
		mjpeg.indma_pause=1;					//暂停读取数据
	}else										//有效的buf
	{
		JPEG_IN_DMA_Resume((u32)mjpeg.inbuf[mjpeg.inbuf_read_ptr].buf,mjpeg.inbuf[mjpeg.inbuf_read_ptr].size);//继续下一次DMA传输
	}
}
//JPEG输出数据流(YCBCR)回调函数,用于输出YCbCr数据流
void mjpeg_dma_out_callback(void)
{	  
	u32 *pdata=0; 
	mjpeg.outbuf[mjpeg.outbuf_write_ptr].sta=1;	//此buf已满
	mjpeg.outbuf[mjpeg.outbuf_write_ptr].size=JPEG_DMA_OUTBUF_LEN-(DMA2_Stream1->NDTR<<2);//此buf里面数据的长度
	if(mjpeg.state==JPEG_STATE_FINISHED)		//如果文件已经解码完成,需要读取DOR最后的数据(<=32字节)
	{
		pdata=(u32*)(mjpeg.outbuf[mjpeg.outbuf_write_ptr].buf+mjpeg.outbuf[mjpeg.outbuf_write_ptr].size);
		while(JPEG->SR&(1<<4))
		{
			*pdata=JPEG->DOR;
			pdata++;
			mjpeg.outbuf[mjpeg.outbuf_write_ptr].size+=4; 
		}
	}  
	mjpeg.outbuf_write_ptr++;					//指向下一个buf
	if(mjpeg.outbuf_write_ptr>=JPEG_DMA_OUTBUF_NB)mjpeg.outbuf_write_ptr=0;//归零
	if(mjpeg.outbuf[mjpeg.outbuf_write_ptr].sta==1)//无有效buf
	{
		JPEG_OUT_DMA_Pause();					//暂停输出数据
		mjpeg.outdma_pause=1;					//暂停输出数据
	}else										//有效的buf
	{
		JPEG_OUT_DMA_Resume((u32)mjpeg.outbuf[mjpeg.outbuf_write_ptr].buf,JPEG_DMA_OUTBUF_LEN);//继续下一次DMA传输
	}
}
//JPEG整个文件解码完成回调函数
void mjpeg_endofcovert_callback(void)
{ 
	mjpeg.state=JPEG_STATE_FINISHED;			//标记JPEG解码完成
}
//JPEG header解析成功回调函数
void mjpeg_hdrover_callback(void)
{
	mjpeg.state=JPEG_STATE_HEADEROK;			//HEADER获取成功
	JPEG_Get_Info(&mjpeg);						//获取JPEG相关信息,包括大小,色彩空间,抽样等
	JPEG_GetDecodeColorConvertFunc(&mjpeg.Conf,&mjpeg.ycbcr2rgb,&mjpeg.total_blks);//获取JPEG色彩转换函数,以及总MCU数 
}

//初始化MJPEG
//offx,offy:视频在LCD上面,X,Y方向的偏移量
//width,height:图像的宽度和高度
//返回值:0,成功;其他,失败.
u8 mjpeg_init(u16 offx,u16 offy,u32 width,u32 height)
{
	u8 res;
	res=JPEG_Core_Init(&mjpeg);					//初始化JPEG内核
	if(res)return 1;
	rgb565buf=mymalloc(SRAMEX,width*height*2);	//申请RGB缓存
	if(rgb565buf==NULL)return  2;
	imgoffx=offx;
	imgoffy=offy;
	mjpeg_rgb_framebuf=(u16*)ltdc_framebuf[lcdltdc.activelayer];//指向RGBLCD当前显存 
	return 0; 
}
//释放内存
void mjpeg_free(void)
{ 
	JPEG_Core_Destroy(&mjpeg); 
	myfree(SRAMEX,rgb565buf);	
}

//填充颜色
//x,y:起始坐标
//width，height：宽度和高度。
//*color：颜色数组
void mjpeg_fill_color(u16 x,u16 y,u16 width,u16 height,u16 *color)
{  
	u16 i,j;
	u32 param1;	
	u32 param2;
	u32 param3;
	u16* pdata; 
	if(lcdltdc.pwidth!=0&&lcddev.dir==0)//如果是RGB屏,且竖屏,则填充函数不可直接用
	{ 
		param1=lcdltdc.pixsize*lcdltdc.pwidth*(lcdltdc.pheight-x-1)+lcdltdc.pixsize*y;	//将运算先做完,提高速度
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
	}else LCD_Color_Fill(x,y,x+width-1,y+height-1,color);//其他情况,直接填充	
}

//解码一副JPEG图片
//注意:
//1,待解吗图片的分辨率,必须小于等于屏幕的分辨率!
//2,请保证图片的宽度是16的倍数,以免左侧出现花纹.
//buf:jpeg数据流数组
//bsize:数组大小
//返回值:0,成功
//    其他,错误
u8 mjpeg_decode(u8* buf,u32 bsize) 
{ 
	vu32 timecnt=0;
	u8 fileover=0;
	u8 i=0;  
	u32 mcublkindex=0;    
	if(bsize==0)return 0;
	JPEG_Decode_Init(&mjpeg);						//初始化硬件JPEG解码器
	for(i=0;i<JPEG_DMA_INBUF_NB;i++)
	{
		if(bsize>JPEG_DMA_INBUF_LEN)
		{
			mymemcpy(mjpeg.inbuf[i].buf,buf,JPEG_DMA_INBUF_LEN);
			mjpeg.inbuf[i].size=JPEG_DMA_INBUF_LEN;	//读取了的数据长度
			mjpeg.inbuf[i].sta=1;					//标记buf满 
			buf+=JPEG_DMA_INBUF_LEN;				//源数组往后偏移
			bsize-=JPEG_DMA_INBUF_LEN;				//文件大小减少
		}else
		{
			mymemcpy(mjpeg.inbuf[i].buf,buf,bsize);
			mjpeg.inbuf[i].size=bsize;				//读取了的数据长度
			mjpeg.inbuf[i].sta=1;					//标记buf满
			buf+=bsize;								//源数组往后偏移
			bsize=0;								//文件大小为0了.
			break;
		}
	}
	JPEG_IN_OUT_DMA_Init((u32)mjpeg.inbuf[0].buf,(u32)mjpeg.outbuf[0].buf,mjpeg.inbuf[0].size,JPEG_DMA_OUTBUF_LEN);//配置DMA
	jpeg_in_callback=mjpeg_dma_in_callback;			//JPEG DMA读取数据回调函数
	jpeg_out_callback=mjpeg_dma_out_callback; 		//JPEG DMA输出数据回调函数
	jpeg_eoc_callback=mjpeg_endofcovert_callback;	//JPEG 解码结束回调函数
	jpeg_hdp_callback=mjpeg_hdrover_callback; 		//JPEG Header解码完成回调函数
	JPEG_DMA_Start();								//启动DMA传输 
	while(1)
	{
        SCB_CleanInvalidateDCache();				//清空D catch
		if(mjpeg.inbuf[mjpeg.inbuf_write_ptr].sta==0&&fileover==0)	//有buf为空
		{
			if(bsize>JPEG_DMA_INBUF_LEN)
			{
				mymemcpy(mjpeg.inbuf[mjpeg.inbuf_write_ptr].buf,buf,JPEG_DMA_INBUF_LEN);
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].size=JPEG_DMA_INBUF_LEN;	//读取了的数据长度
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].sta=1;					//标记buf满 
				buf+=JPEG_DMA_INBUF_LEN;									//源数组往后偏移
				bsize-=JPEG_DMA_INBUF_LEN;									//文件大小减少
			}else
			{
				mymemcpy(mjpeg.inbuf[mjpeg.inbuf_write_ptr].buf,buf,bsize);
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].size=bsize;				//读取了的数据长度
				mjpeg.inbuf[mjpeg.inbuf_write_ptr].sta=1;					//标记buf满
				buf+=bsize;													//源数组往后偏移
				bsize=0;													//文件大小为0了.
				timecnt=0;	//清零计时器
				fileover=1;	//文件结束了...
			} 
			if(mjpeg.indma_pause==1&&mjpeg.inbuf[mjpeg.inbuf_read_ptr].sta==1)//之前是暂停的了,继续传输
			{
				JPEG_IN_DMA_Resume((u32)mjpeg.inbuf[mjpeg.inbuf_read_ptr].buf,mjpeg.inbuf[mjpeg.inbuf_read_ptr].size);	//继续下一次DMA传输
				mjpeg.indma_pause=0;
			}
			mjpeg.inbuf_write_ptr++;
			if(mjpeg.inbuf_write_ptr>=JPEG_DMA_INBUF_NB)mjpeg.inbuf_write_ptr=0;
		}
		if(mjpeg.outbuf[mjpeg.outbuf_read_ptr].sta==1)	//buf里面有数据要处理
		{
			mcublkindex+=mjpeg.ycbcr2rgb(mjpeg.outbuf[mjpeg.outbuf_read_ptr].buf,(u8*)rgb565buf,mcublkindex,mjpeg.outbuf[mjpeg.outbuf_read_ptr].size);//YUV --> RGB565
			mjpeg.outbuf[mjpeg.outbuf_read_ptr].sta=0;	//标记buf为空
			mjpeg.outbuf[mjpeg.outbuf_read_ptr].size=0;	//数据量清空
			mjpeg.outbuf_read_ptr++;
			if(mjpeg.outbuf_read_ptr>=JPEG_DMA_OUTBUF_NB)mjpeg.outbuf_read_ptr=0;//限制范围
			if(mcublkindex==mjpeg.total_blks)
			{
				break;
			}
		}else if(mjpeg.outdma_pause==1&&mjpeg.outbuf[mjpeg.outbuf_write_ptr].sta==0)		//out暂停,且当前writebuf已经为空了,则恢复out输出
		{
			JPEG_OUT_DMA_Resume((u32)mjpeg.outbuf[mjpeg.outbuf_write_ptr].buf,JPEG_DMA_OUTBUF_LEN);//继续下一次DMA传输
			mjpeg.outdma_pause=0;
		}
		timecnt++; 
		if(fileover)//文件结束后,及时退出,防止死循环
		{
			if(mjpeg.state==JPEG_STATE_NOHEADER)break;	//解码失败了
			if(timecnt>0X3FFF)break;					//超时退出
		}
	}    
	if(mjpeg.state==JPEG_STATE_FINISHED)	//解码完成了
	{
		mjpeg_fill_color(imgoffx,imgoffy,mjpeg.Conf.ImageWidth,mjpeg.Conf.ImageHeight,rgb565buf);  
	} 
	return 0;
}

























