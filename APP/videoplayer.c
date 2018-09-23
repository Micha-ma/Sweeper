#include "videoplayer.h" 
#include "string.h"  
#include "usart.h"   
#include "delay.h"
#include "timer.h"
#include "tftlcd.h"
#include "malloc.h"
#include "sai.h" 
#include "wm8978.h" 
#include "mjpeg.h" 
#include "avi.h"
#include "exfuns.h"
#include "text.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//视频播放器 应用代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/1/12
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved	
//All rights reserved
//********************************************************************************
//V1.1 20160602
//1,修改部分代码，以支持最新的fatfs（R0.12）
//2,修改很多参数:i2s改为sai
////////////////////////////////////////////////////////////////////////////////// 	
 
    
extern u16 frame;
extern vu8 frameup;//视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频


volatile u8 saiplaybuf;	//即将播放的音频帧缓冲编号
u8* saibuf[4]; 			//音频缓冲帧,共4帧,4*5K=20K
  
//音频数据sai DMA传输回调函数
void audio_sai_dma_callback(void) 
{      
	saiplaybuf++;
	if(saiplaybuf>3)saiplaybuf=0;
	if(DMA2_Stream3->CR&(1<<19))
	{	 
		DMA2_Stream3->M0AR=(u32)saibuf[saiplaybuf];//指向下一个buf
	}
	else 
	{   		
		DMA2_Stream3->M1AR=(u32)saibuf[saiplaybuf];//指向下一个buf
	} 
} 
//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 video_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO* tfileinfo;	//临时文件信息		 
	tfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//申请内存   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;//错误了/到末尾了,退出	 		 
			res=f_typetell((u8*)tfileinfo->fname);	
			if((res&0XF0)==0X60)//取高四位,看看是不是视频文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo);//释放内存
	return rval;
} 
//显示当前播放时间
//favi:当前播放的视频文件
//aviinfo;avi控制结构体
void video_time_show(FIL *favi,AVI_INFO *aviinfo)
{	 
	static u32 oldsec;	//上一次的播放时间
	u8* buf;
	u32 totsec=0;		//video文件总时间 
	u32 cursec; 		//当前播放时间 
	totsec=(aviinfo->SecPerFrame/1000)*aviinfo->TotalFrame;	//歌曲总长度(单位:ms) 
	totsec/=1000;		//秒钟数. 
  	cursec=((double)favi->fptr/favi->obj.objsize)*totsec;//当前播放到多少秒了?  
	if(oldsec!=cursec)	//需要更新显示时间
	{
		buf=mymalloc(SRAMIN,100);//申请100字节内存
		oldsec=cursec; 
		POINT_COLOR=BLUE; 
		sprintf((char*)buf,"播放时间:%02d:%02d:%02d/%02d:%02d:%02d",cursec/3600,(cursec%3600)/60,cursec%60,totsec/3600,(totsec%3600)/60,totsec%60);
 		Show_Str(10,90,lcddev.width-10,16,buf,16,0);	//显示歌曲名字
		myfree(SRAMIN,buf);		
	} 		 
}
//显示当前视频文件的相关信息 
//aviinfo;avi控制结构体
void video_info_show(AVI_INFO *aviinfo)
{	  
	u8 *buf;
	buf=mymalloc(SRAMIN,100);//申请100字节内存 
	POINT_COLOR=RED; 
	sprintf((char*)buf,"声道数:%d,采样率:%d",aviinfo->Channels,aviinfo->SampleRate*10); 
 	Show_Str(10,50,lcddev.width-10,16,buf,16,0);	//显示歌曲名字
	sprintf((char*)buf,"帧率:%d帧",1000/(aviinfo->SecPerFrame/1000)); 
 	Show_Str(10,70,lcddev.width-10,16,buf,16,0);	//显示歌曲名字
	myfree(SRAMIN,buf);	  
}
//视频基本信息显示
//name:视频名字
//index:当前索引
//total:总文件数
void video_bmsg_show(u8* name,u16 index,u16 total)
{		
	u8* buf;
	buf=mymalloc(SRAMIN,100);//申请100字节内存
	POINT_COLOR=RED;
	sprintf((char*)buf,"文件名:%s",name);
	Show_Str(10,10,lcddev.width-10,16,buf,16,0);//显示文件名
	sprintf((char*)buf,"索引:%d/%d",index,total);	
	Show_Str(10,30,lcddev.width-10,16,buf,16,0);//显示索引 		  	  
	myfree(SRAMIN,buf);		
}

//播放视频
void video_play(void)
{
	u8 i=0;
	u8 res;
 	DIR vdir;	 		//目录
	FILINFO *vfileinfo;	//文件信息
	u8 *pname;			//带路径的文件名
	u16 totavinum; 		//视频文件总数
	u16 curindex;		//视频文件当前索引
	u8 key;				//键值		  
 	u32 temp;
	u32 *voffsettbl;	//视频文件off block索引表
	Show_Str(60,50,200,16,"阿波罗STM32F4/F7开发板",16,0);				    	 
	Show_Str(60,70,200,16,"视频播放程序",16,0);	
	
 	while(f_opendir(&vdir,"0:/VIDEO"))//打开视频文件夹
 	{	    
		Show_Str(60,190,240,16,"VIDEO文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,206,WHITE);//清除显示	     
		delay_ms(200);
        i++;
        if(i==5) return ;			
	}
   	i=0;
	totavinum=video_get_tnum("0:/VIDEO"); //得到总有效文件数
  	while(totavinum==NULL)//视频文件总数为0		
 	{	    
		Show_Str(60,190,240,16,"没有视频文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,146,WHITE);//清除显示	     
		delay_ms(200);
        i++;
        if(i==5) return ;				
	}
    i=0;		
	vfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//为长文件缓存区分配内存
 	pname=mymalloc(SRAMIN,2*_MAX_LFN+1);				//为带路径的文件名分配内存
 	voffsettbl=mymalloc(SRAMIN,4*totavinum);			//申请4*totavinum个字节的内存,用于存放视频文件索引
 	while(vfileinfo==NULL||pname==NULL||voffsettbl==NULL)//内存分配出错
 	{	    
		Show_Str(60,190,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,146,WHITE);//清除显示	     
		delay_ms(200);
        i++;
        if(i==5) return ;		
	} 
     i=0;		
 	//记录索引
    res=f_opendir(&vdir,"0:/VIDEO"); //打开目录
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=vdir.dptr;								//记录当前offset
	        res=f_readdir(&vdir,vfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||vfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出		  
			res=f_typetell((u8*)vfileinfo->fname);	
			if((res&0XF0)==0X60)//取高四位,看看是不是音乐文件	
			{
				voffsettbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
   	curindex=0;										//从0开始显示
   	res=f_opendir(&vdir,(const TCHAR*)"0:/VIDEO"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
		dir_sdi(&vdir,voffsettbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&vdir,vfileinfo);       			//读取目录下的一个文件
        if(res!=FR_OK||vfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出
		strcpy((char*)pname,"0:/VIDEO/");			//复制路径(目录)
		strcat((char*)pname,(const char*)vfileinfo->fname); //将文件名接在后面 
		LCD_Clear(WHITE);							//先清屏
		video_bmsg_show((u8*)vfileinfo->fname,curindex+1,totavinum);//显示名字,索引等信息
		Show_Str(10,130,lcddev.width-10,16,"KEY2:上一曲",16,0); 	
		Show_Str(10,150,lcddev.width-10,16,"KEY0:下一曲",16,0);
		Show_Str(10,170,lcddev.width-10,16,"KEY_UP:快进",16,0);
		Show_Str(10,190,lcddev.width-10,16,"KEY_1:返回主菜单",16,0); 	 
	} 											  		    
	myfree(SRAMIN,vfileinfo);			//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,voffsettbl);			//释放内存	 
}
//播放一个mjpeg文件
//pname:文件名
//返回值:
//KEY0_PRES:下一曲
//KEY2_PRES:上一曲
//WKUP_PRES:快进
//KEY1_PRES:返回主菜单
//其他:错误
u8 video_play_mjpeg(u8 *pname)
{   
	u8* framebuf;	//视频解码buf	 
	u8* pbuf;		//buf指针  
	FIL *favi;
	u8  res=0;
	u32 offset=0; 
	u32	nr; 
	u8 key;   
    u8 saisavebuf;  
	saibuf[0]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//申请音频内存
	saibuf[1]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//申请音频内存
	saibuf[2]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//申请音频内存
	saibuf[3]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//申请音频内存 
	framebuf=mymalloc(SRAMIN,AVI_VIDEO_BUF_SIZE);	//申请视频buf
	favi=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//申请favi内存 
	memset(saibuf[0],0,AVI_AUDIO_BUF_SIZE);
	memset(saibuf[1],0,AVI_AUDIO_BUF_SIZE); 
	memset(saibuf[2],0,AVI_AUDIO_BUF_SIZE);
	memset(saibuf[3],0,AVI_AUDIO_BUF_SIZE); 
	if(!saibuf[3]||!framebuf||!favi)
	{
		printf("memory error!\r\n");
		res=0XFF;
	}   
	while(res==0)
	{ 
		res=f_open(favi,(char *)pname,FA_READ);
		if(res==0)
		{
			pbuf=framebuf;			
			res=f_read(favi,pbuf,AVI_VIDEO_BUF_SIZE,&nr);//开始读取	
			if(res)
			{
				printf("fread error:%d\r\n",res);
				break;
			} 	 
			//开始avi解析
			res=avi_init(pbuf,AVI_VIDEO_BUF_SIZE);	//avi解析
			if(res)
			{
				printf("avi err:%d\r\n",res);
				break;
			} 	
			video_info_show(&avix); 
			TIM6_Init(avix.SecPerFrame/100-1,10800-1);//10Khz计数频率,加1是100us 
			offset=avi_srarch_id(pbuf,AVI_VIDEO_BUF_SIZE,"movi");//寻找movi ID	 
			avi_get_streaminfo(pbuf+offset+4);			//获取流信息 
			f_lseek(favi,offset+12);					//跳过标志ID,读地址偏移到流数据开始处	 
//			res=mjpeg_init((lcddev.width-avix.Width)/2,110+(lcddev.height-110-avix.Height)/2,avix.Width,avix.Height);//JPG解码初始化 
         
			if((avix.Width>lcddev.width)||(avix.Height>lcddev.height))	//分辨率超区域无法正常播放
			{
				break;	
			}else if(avix.Height>(lcddev.height-110))
			{				
				res=mjpeg_init((lcddev.width-avix.Width)/2,(lcddev.height-avix.Height)/2,avix.Width,avix.Height);			//JPG解码初始化(纵向不偏移110)
			}else res=mjpeg_init((lcddev.width-avix.Width)/2,110+(lcddev.height-110-avix.Height)/2,avix.Width,avix.Height);	//JPG解码初始化(纵向偏移110)
		
			if(avix.SampleRate)							//有音频信息,才初始化
			{
				WM8978_I2S_Cfg(2,0);	//飞利浦标准,16位数据长度
				SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);//设置SAI,主发送,16位数据 		
				SAIA_SampleRate_Set(avix.SampleRate);	//设置采样率
				SAIA_TX_DMA_Init(saibuf[1],saibuf[2],avix.AudioBufSize/2,1);//配置DMA
				sai_tx_callback=audio_sai_dma_callback;	//回调函数指向SAI_DMA_Callback
				saiplaybuf=0;
				saisavebuf=0; 
				SAI_Play_Start(); //开启sai播放 
			}
 			while(1)//播放循环
			{					
				if(avix.StreamID==AVI_VIDS_FLAG)	//视频流
				{
					pbuf=framebuf;
					f_read(favi,pbuf,avix.StreamSize+8,&nr);		//读入整帧+下一数据流ID信息  
					res=mjpeg_decode(pbuf,avix.StreamSize);
					if(res)
					{
						printf("decode error!\r\n");
					} 
					while(frameup==0);	//等待时间到达(在TIM6的中断里面设置为1)
					frameup=0;			//标志清零
					frame++; 
				}else 	//音频流
				{		  
					video_time_show(favi,&avix); 	//显示当前播放时间			
					saisavebuf++;
					if(saisavebuf>3)saisavebuf=0;
					do
					{
						nr=saiplaybuf;
						if(nr)nr--;
						else nr=3; 
					}while(saisavebuf==nr);//碰撞等待. 
					f_read(favi,saibuf[saisavebuf],avix.StreamSize+8,&nr);//填充saibuf	 
					pbuf=saibuf[saisavebuf];  
				} 	
			}
			SAI_Play_Stop();
			TIM6->CR1&=~(1<<0); //关闭定时器6
			LCD_Set_Window(0,0,lcddev.width,lcddev.height);//恢复窗口
			mjpeg_free();		//释放内存
			f_close(favi); 
		}
	}
	myfree(SRAMIN,saibuf[0]);
	myfree(SRAMIN,saibuf[1]);
	myfree(SRAMIN,saibuf[2]);
	myfree(SRAMIN,saibuf[3]);
	myfree(SRAMIN,framebuf);
	myfree(SRAMIN,favi);
	return res;
}
//avi文件查找
u8 video_seek(FIL *favi,AVI_INFO *aviinfo,u8 *mbuf)
{
	u32 fpos=favi->fptr;
	u8 *pbuf;
	u32 offset;
	u32 br;
	u32 totsec;
	u8 key; 
	totsec=(aviinfo->SecPerFrame/1000)*aviinfo->TotalFrame;	
	totsec/=1000;
	return 0;
}
