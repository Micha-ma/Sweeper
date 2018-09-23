#include "pictureplay.h"
#include "malloc.h"
#include "string.h"  
#include "usart.h"   
#include "delay.h"
#include "exfuns.h"
#include "text.h"
#include "piclib.h"

//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO *tfileinfo;	//临时文件信息	    			     
	tfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//申请内存
    res=f_opendir(&tdir,(const TCHAR*)path); 	//打开目录 
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,tfileinfo);       		//读取目录下的一个文件  	 
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;//错误了/到末尾了,退出	 		 
			res=f_typetell((u8*)tfileinfo->fname);
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	}  
	myfree(SRAMIN,tfileinfo);//释放内存
	return rval;
} 

//图片播放
void picture_play(void)
{   
	u8 i=0;
	u8 led0sta=1;
	u8 res;
 	DIR picdir;	 		//图片目录
	FILINFO *picfileinfo;//文件信息 
	u8 *pname;			//带路径的文件名
	u16 totpicnum; 		//图片文件总数
	u16 curindex;		//图片当前索引
	u8 key;				//键值
	u8 pause=0;			//暂停标记
	u8 t;
	u16 temp;
	u32 *picoffsettbl;	//图片文件offset索引表 
	
	while(f_opendir(&picdir,"0:/PICTURE"))//打开图片文件夹
 	{	    
		Show_Str(60,170,240,16,"PICTURE文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,WHITE);//清除显示	     
		delay_ms(200);				  
	}  
	totpicnum=pic_get_tnum("0:/PICTURE"); //得到总有效文件数
  	while(totpicnum==NULL)//图片文件为0		
 	{	    
		Show_Str(60,170,240,16,"没有图片文件!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,WHITE);//清除显示	     
		delay_ms(200);
        i++;
        if(i==5) return ;			
	} 
	i=0;	
	picfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));		//申请内存
 	pname=mymalloc(SRAMIN,_MAX_LFN*2+1);						//为带路径的文件名分配内存
 	picoffsettbl=mymalloc(SRAMIN,4*totpicnum);					//申请4*totpicnum个字节的内存,用于存放图片索引
 	while(!picfileinfo||!pname||!picoffsettbl)					//内存分配出错
 	{	    	
		Show_Str(60,170,240,16,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,WHITE);//清除显示	     
		delay_ms(200);
        i++;
        if(i==5) return ;			
	}
    i=0;  	
	//记录索引
    res=f_opendir(&picdir,"0:/PICTURE"); //打开目录
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=picdir.dptr;								//记录当前dptr偏移
	        res=f_readdir(&picdir,picfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||picfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出	 	 
			res=f_typetell((u8*)picfileinfo->fname);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件	
			{
				picoffsettbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}
	Show_Str(60,100,240,16,"开始显示...",16,0); 
	delay_ms(1500);
	piclib_init();										//初始化画图	   	   
	curindex=0;											//从0开始显示
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
		dir_sdi(&picdir,picoffsettbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&picdir,picfileinfo);       			//读取目录下的一个文件
        if(res!=FR_OK||picfileinfo->fname[0]==0)break;		//错误了/到末尾了,退出
		strcpy((char*)pname,"0:/PICTURE/");					//复制路径(目录)
		strcat((char*)pname,(const char*)picfileinfo->fname);//将文件名接在后面
 		LCD_Clear(BLACK);
 		ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);//显示图片    
		Show_Str(2,2,lcddev.width,16,pname,16,1); 				//显示图片名字
	} 							    
	myfree(SRAMIN,picfileinfo);			//释放内存						   		    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,picoffsettbl);		//释放内存	
		
}
