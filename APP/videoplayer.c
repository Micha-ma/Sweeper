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
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//��Ƶ������ Ӧ�ô���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/1/12
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved	
//All rights reserved
//********************************************************************************
//V1.1 20160602
//1,�޸Ĳ��ִ��룬��֧�����µ�fatfs��R0.12��
//2,�޸ĺܶ����:i2s��Ϊsai
////////////////////////////////////////////////////////////////////////////////// 	
 
    
extern u16 frame;
extern vu8 frameup;//��Ƶ����ʱ϶���Ʊ���,������1��ʱ��,���Ը�����һ֡��Ƶ


volatile u8 saiplaybuf;	//�������ŵ���Ƶ֡������
u8* saibuf[4]; 			//��Ƶ����֡,��4֡,4*5K=20K
  
//��Ƶ����sai DMA����ص�����
void audio_sai_dma_callback(void) 
{      
	saiplaybuf++;
	if(saiplaybuf>3)saiplaybuf=0;
	if(DMA2_Stream3->CR&(1<<19))
	{	 
		DMA2_Stream3->M0AR=(u32)saibuf[saiplaybuf];//ָ����һ��buf
	}
	else 
	{   		
		DMA2_Stream3->M1AR=(u32)saibuf[saiplaybuf];//ָ����һ��buf
	} 
} 
//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 video_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO* tfileinfo;	//��ʱ�ļ���Ϣ		 
	tfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//�����ڴ�   			     
    res=f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;//������/��ĩβ��,�˳�	 		 
			res=f_typetell((u8*)tfileinfo->fname);	
			if((res&0XF0)==0X60)//ȡ����λ,�����ǲ�����Ƶ�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	myfree(SRAMIN,tfileinfo);//�ͷ��ڴ�
	return rval;
} 
//��ʾ��ǰ����ʱ��
//favi:��ǰ���ŵ���Ƶ�ļ�
//aviinfo;avi���ƽṹ��
void video_time_show(FIL *favi,AVI_INFO *aviinfo)
{	 
	static u32 oldsec;	//��һ�εĲ���ʱ��
	u8* buf;
	u32 totsec=0;		//video�ļ���ʱ�� 
	u32 cursec; 		//��ǰ����ʱ�� 
	totsec=(aviinfo->SecPerFrame/1000)*aviinfo->TotalFrame;	//�����ܳ���(��λ:ms) 
	totsec/=1000;		//������. 
  	cursec=((double)favi->fptr/favi->obj.objsize)*totsec;//��ǰ���ŵ���������?  
	if(oldsec!=cursec)	//��Ҫ������ʾʱ��
	{
		buf=mymalloc(SRAMIN,100);//����100�ֽ��ڴ�
		oldsec=cursec; 
		POINT_COLOR=BLUE; 
		sprintf((char*)buf,"����ʱ��:%02d:%02d:%02d/%02d:%02d:%02d",cursec/3600,(cursec%3600)/60,cursec%60,totsec/3600,(totsec%3600)/60,totsec%60);
 		Show_Str(10,90,lcddev.width-10,16,buf,16,0);	//��ʾ��������
		myfree(SRAMIN,buf);		
	} 		 
}
//��ʾ��ǰ��Ƶ�ļ��������Ϣ 
//aviinfo;avi���ƽṹ��
void video_info_show(AVI_INFO *aviinfo)
{	  
	u8 *buf;
	buf=mymalloc(SRAMIN,100);//����100�ֽ��ڴ� 
	POINT_COLOR=RED; 
	sprintf((char*)buf,"������:%d,������:%d",aviinfo->Channels,aviinfo->SampleRate*10); 
 	Show_Str(10,50,lcddev.width-10,16,buf,16,0);	//��ʾ��������
	sprintf((char*)buf,"֡��:%d֡",1000/(aviinfo->SecPerFrame/1000)); 
 	Show_Str(10,70,lcddev.width-10,16,buf,16,0);	//��ʾ��������
	myfree(SRAMIN,buf);	  
}
//��Ƶ������Ϣ��ʾ
//name:��Ƶ����
//index:��ǰ����
//total:���ļ���
void video_bmsg_show(u8* name,u16 index,u16 total)
{		
	u8* buf;
	buf=mymalloc(SRAMIN,100);//����100�ֽ��ڴ�
	POINT_COLOR=RED;
	sprintf((char*)buf,"�ļ���:%s",name);
	Show_Str(10,10,lcddev.width-10,16,buf,16,0);//��ʾ�ļ���
	sprintf((char*)buf,"����:%d/%d",index,total);	
	Show_Str(10,30,lcddev.width-10,16,buf,16,0);//��ʾ���� 		  	  
	myfree(SRAMIN,buf);		
}

//������Ƶ
void video_play(void)
{
	u8 i=0;
	u8 res;
 	DIR vdir;	 		//Ŀ¼
	FILINFO *vfileinfo;	//�ļ���Ϣ
	u8 *pname;			//��·�����ļ���
	u16 totavinum; 		//��Ƶ�ļ�����
	u16 curindex;		//��Ƶ�ļ���ǰ����
	u8 key;				//��ֵ		  
 	u32 temp;
	u32 *voffsettbl;	//��Ƶ�ļ�off block������
	Show_Str(60,50,200,16,"������STM32F4/F7������",16,0);				    	 
	Show_Str(60,70,200,16,"��Ƶ���ų���",16,0);	
	
 	while(f_opendir(&vdir,"0:/VIDEO"))//����Ƶ�ļ���
 	{	    
		Show_Str(60,190,240,16,"VIDEO�ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,206,WHITE);//�����ʾ	     
		delay_ms(200);
        i++;
        if(i==5) return ;			
	}
   	i=0;
	totavinum=video_get_tnum("0:/VIDEO"); //�õ�����Ч�ļ���
  	while(totavinum==NULL)//��Ƶ�ļ�����Ϊ0		
 	{	    
		Show_Str(60,190,240,16,"û����Ƶ�ļ�!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,146,WHITE);//�����ʾ	     
		delay_ms(200);
        i++;
        if(i==5) return ;				
	}
    i=0;		
	vfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//Ϊ���ļ������������ڴ�
 	pname=mymalloc(SRAMIN,2*_MAX_LFN+1);				//Ϊ��·�����ļ��������ڴ�
 	voffsettbl=mymalloc(SRAMIN,4*totavinum);			//����4*totavinum���ֽڵ��ڴ�,���ڴ����Ƶ�ļ�����
 	while(vfileinfo==NULL||pname==NULL||voffsettbl==NULL)//�ڴ�������
 	{	    
		Show_Str(60,190,240,16,"�ڴ����ʧ��!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,190,240,146,WHITE);//�����ʾ	     
		delay_ms(200);
        i++;
        if(i==5) return ;		
	} 
     i=0;		
 	//��¼����
    res=f_opendir(&vdir,"0:/VIDEO"); //��Ŀ¼
	if(res==FR_OK)
	{
		curindex=0;//��ǰ����Ϊ0
		while(1)//ȫ����ѯһ��
		{
			temp=vdir.dptr;								//��¼��ǰoffset
	        res=f_readdir(&vdir,vfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||vfileinfo->fname[0]==0)break;	//������/��ĩβ��,�˳�		  
			res=f_typetell((u8*)vfileinfo->fname);	
			if((res&0XF0)==0X60)//ȡ����λ,�����ǲ��������ļ�	
			{
				voffsettbl[curindex]=temp;//��¼����
				curindex++;
			}	    
		} 
	}   
   	curindex=0;										//��0��ʼ��ʾ
   	res=f_opendir(&vdir,(const TCHAR*)"0:/VIDEO"); 	//��Ŀ¼
	while(res==FR_OK)//�򿪳ɹ�
	{	
		dir_sdi(&vdir,voffsettbl[curindex]);			//�ı䵱ǰĿ¼����	   
        res=f_readdir(&vdir,vfileinfo);       			//��ȡĿ¼�µ�һ���ļ�
        if(res!=FR_OK||vfileinfo->fname[0]==0)break;	//������/��ĩβ��,�˳�
		strcpy((char*)pname,"0:/VIDEO/");			//����·��(Ŀ¼)
		strcat((char*)pname,(const char*)vfileinfo->fname); //���ļ������ں��� 
		LCD_Clear(WHITE);							//������
		video_bmsg_show((u8*)vfileinfo->fname,curindex+1,totavinum);//��ʾ����,��������Ϣ
		Show_Str(10,130,lcddev.width-10,16,"KEY2:��һ��",16,0); 	
		Show_Str(10,150,lcddev.width-10,16,"KEY0:��һ��",16,0);
		Show_Str(10,170,lcddev.width-10,16,"KEY_UP:���",16,0);
		Show_Str(10,190,lcddev.width-10,16,"KEY_1:�������˵�",16,0); 	 
	} 											  		    
	myfree(SRAMIN,vfileinfo);			//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�			    
	myfree(SRAMIN,voffsettbl);			//�ͷ��ڴ�	 
}
//����һ��mjpeg�ļ�
//pname:�ļ���
//����ֵ:
//KEY0_PRES:��һ��
//KEY2_PRES:��һ��
//WKUP_PRES:���
//KEY1_PRES:�������˵�
//����:����
u8 video_play_mjpeg(u8 *pname)
{   
	u8* framebuf;	//��Ƶ����buf	 
	u8* pbuf;		//bufָ��  
	FIL *favi;
	u8  res=0;
	u32 offset=0; 
	u32	nr; 
	u8 key;   
    u8 saisavebuf;  
	saibuf[0]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//������Ƶ�ڴ�
	saibuf[1]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//������Ƶ�ڴ�
	saibuf[2]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//������Ƶ�ڴ�
	saibuf[3]=mymalloc(SRAMIN,AVI_AUDIO_BUF_SIZE);	//������Ƶ�ڴ� 
	framebuf=mymalloc(SRAMIN,AVI_VIDEO_BUF_SIZE);	//������Ƶbuf
	favi=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//����favi�ڴ� 
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
			res=f_read(favi,pbuf,AVI_VIDEO_BUF_SIZE,&nr);//��ʼ��ȡ	
			if(res)
			{
				printf("fread error:%d\r\n",res);
				break;
			} 	 
			//��ʼavi����
			res=avi_init(pbuf,AVI_VIDEO_BUF_SIZE);	//avi����
			if(res)
			{
				printf("avi err:%d\r\n",res);
				break;
			} 	
			video_info_show(&avix); 
			TIM6_Init(avix.SecPerFrame/100-1,10800-1);//10Khz����Ƶ��,��1��100us 
			offset=avi_srarch_id(pbuf,AVI_VIDEO_BUF_SIZE,"movi");//Ѱ��movi ID	 
			avi_get_streaminfo(pbuf+offset+4);			//��ȡ����Ϣ 
			f_lseek(favi,offset+12);					//������־ID,����ַƫ�Ƶ������ݿ�ʼ��	 
//			res=mjpeg_init((lcddev.width-avix.Width)/2,110+(lcddev.height-110-avix.Height)/2,avix.Width,avix.Height);//JPG�����ʼ�� 
         
			if((avix.Width>lcddev.width)||(avix.Height>lcddev.height))	//�ֱ��ʳ������޷���������
			{
				break;	
			}else if(avix.Height>(lcddev.height-110))
			{				
				res=mjpeg_init((lcddev.width-avix.Width)/2,(lcddev.height-avix.Height)/2,avix.Width,avix.Height);			//JPG�����ʼ��(����ƫ��110)
			}else res=mjpeg_init((lcddev.width-avix.Width)/2,110+(lcddev.height-110-avix.Height)/2,avix.Width,avix.Height);	//JPG�����ʼ��(����ƫ��110)
		
			if(avix.SampleRate)							//����Ƶ��Ϣ,�ų�ʼ��
			{
				WM8978_I2S_Cfg(2,0);	//�����ֱ�׼,16λ���ݳ���
				SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);//����SAI,������,16λ���� 		
				SAIA_SampleRate_Set(avix.SampleRate);	//���ò�����
				SAIA_TX_DMA_Init(saibuf[1],saibuf[2],avix.AudioBufSize/2,1);//����DMA
				sai_tx_callback=audio_sai_dma_callback;	//�ص�����ָ��SAI_DMA_Callback
				saiplaybuf=0;
				saisavebuf=0; 
				SAI_Play_Start(); //����sai���� 
			}
 			while(1)//����ѭ��
			{					
				if(avix.StreamID==AVI_VIDS_FLAG)	//��Ƶ��
				{
					pbuf=framebuf;
					f_read(favi,pbuf,avix.StreamSize+8,&nr);		//������֡+��һ������ID��Ϣ  
					res=mjpeg_decode(pbuf,avix.StreamSize);
					if(res)
					{
						printf("decode error!\r\n");
					} 
					while(frameup==0);	//�ȴ�ʱ�䵽��(��TIM6���ж���������Ϊ1)
					frameup=0;			//��־����
					frame++; 
				}else 	//��Ƶ��
				{		  
					video_time_show(favi,&avix); 	//��ʾ��ǰ����ʱ��			
					saisavebuf++;
					if(saisavebuf>3)saisavebuf=0;
					do
					{
						nr=saiplaybuf;
						if(nr)nr--;
						else nr=3; 
					}while(saisavebuf==nr);//��ײ�ȴ�. 
					f_read(favi,saibuf[saisavebuf],avix.StreamSize+8,&nr);//���saibuf	 
					pbuf=saibuf[saisavebuf];  
				} 	
			}
			SAI_Play_Stop();
			TIM6->CR1&=~(1<<0); //�رն�ʱ��6
			LCD_Set_Window(0,0,lcddev.width,lcddev.height);//�ָ�����
			mjpeg_free();		//�ͷ��ڴ�
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
//avi�ļ�����
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
