#include "Hzfontupdata.h"
#include "fontupd.h"
#include "ff.h"	  
#include "w25qxx.h"   
#include "lcd.h"  
#include "string.h"
#include "malloc.h"
#include "delay.h"
#include "usart.h"
#include "DIALOG.h"
//#include "includes.h"
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

//�ֿ�����ռ�õ�����������С(4���ֿ�+unigbk��+�ֿ���Ϣ=6302984�ֽ�,Լռ1539��W25QXX����,һ������4K�ֽ�)
#define FONTSECSIZE	 	1539
//�ֿ�����ʼ��ַ 
#define FONTINFOADDR 	1024*1024*25 					//Apollo STM32�������Ǵ�25M��ַ�Ժ�ʼ����ֿ�
														//ǰ��25M��fatfsռ����.
														//25M�Ժ����4���ֿ�+UNIGBK.BIN,�ܴ�С6.01M,���ֿ�ռ����,���ܶ�!
														//31.01M�Ժ�,�û���������ʹ��.
														
//���������ֿ������Ϣ����ַ����С��
extern _font_info ftinfo;
extern u8* const GBK_PATH[5];
extern u8* const UPDATE_REMIND_TBL[5];  

//�ؼ�ID
#define ID_WINDOW_0 (GUI_ID_USER + 0x100)
#define ID_TEXT_0 (GUI_ID_USER + 0x101)
#define ID_TEXT_1 (GUI_ID_USER + 0x102)
#define ID_TEXT_2 (GUI_ID_USER + 0x103)
    
WM_HWIN HzfontWin;  //������������
    
//�Ի�����Դ�б�
static const GUI_WIDGET_CREATE_INFO EmwinHzDialog[]=
{
    { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 800, 480, 0, 0x0, 0 },
    { TEXT_CreateIndirect, "Text", ID_TEXT_0, 20, 15, 320, 28, 0, 0x64, 0 },
    { TEXT_CreateIndirect, "Text", ID_TEXT_1, 20, 40, 150, 20, 0, 0x64, 0 },
    { TEXT_CreateIndirect, "Text", ID_TEXT_2, 180, 40, 80, 20, 0, 0x64, 0 },
};

//��ʾ��ǰ������½���
//fsize:�����ļ���С
//pos:��ǰ�ļ�ָ��λ��
u32 emwin_fupd_prog(u32 fsize,u32 pos)
{
    WM_HWIN hItem;
	float prog;
    char buffer[10];
	u8 t=0XFF;
	prog=(float)pos/fsize;
	prog*=100;
    hItem=WM_GetDialogItem(HzfontWin,ID_TEXT_2);
	if(t!=prog)
	{
		t=prog;
		if(t>100)t=100;
        sprintf(buffer,"%d%%",t);
        TEXT_SetText(hItem,buffer);
        GUI_Exec();
	}
	return 0;					    
} 
//����ĳһ��
//fxpath:·��
//fx:���µ����� 0,ungbk;1,gbk12;2,gbk16;3,gbk24;4,gbk32;
//����ֵ:0,�ɹ�;����,ʧ��.
u8 emwin_updata_fontx(u8 *fxpath,u8 fx)
{
	u32 flashaddr=0;								    
	FIL * fftemp;
	u8 *tempbuf;
 	u8 res;	
	u16 bread;
	u32 offx=0;
	u8 rval=0;	     
	fftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�	
	if(fftemp==NULL)rval=1;
	tempbuf=mymalloc(SRAMIN,4096);				//����4096���ֽڿռ�
	if(tempbuf==NULL)rval=1;
 	res=f_open(fftemp,(const TCHAR*)fxpath,FA_READ); 
 	if(res)rval=2;//���ļ�ʧ��  
 	if(rval==0)	 
	{
		switch(fx)
		{
			case 0:												//����UNIGBK.BIN
				ftinfo.ugbkaddr=FONTINFOADDR+sizeof(ftinfo);	//��Ϣͷ֮�󣬽���UNIGBKת�����
				ftinfo.ugbksize=fftemp->obj.objsize;					//UNIGBK��С
				flashaddr=ftinfo.ugbkaddr;
				break;
			case 1:
				ftinfo.f12addr=ftinfo.ugbkaddr+ftinfo.ugbksize;	//UNIGBK֮�󣬽���GBK12�ֿ�
				ftinfo.gbk12size=fftemp->obj.objsize;					//GBK12�ֿ��С
				flashaddr=ftinfo.f12addr;						//GBK12����ʼ��ַ
				break;
			case 2:
				ftinfo.f16addr=ftinfo.f12addr+ftinfo.gbk12size;	//GBK12֮�󣬽���GBK16�ֿ�
				ftinfo.gbk16size=fftemp->obj.objsize;					//GBK16�ֿ��С
				flashaddr=ftinfo.f16addr;						//GBK16����ʼ��ַ
				break;
			case 3:
				ftinfo.f24addr=ftinfo.f16addr+ftinfo.gbk16size;	//GBK16֮�󣬽���GBK24�ֿ�
				ftinfo.gbk24size=fftemp->obj.objsize;					//GBK24�ֿ��С
				flashaddr=ftinfo.f24addr;						//GBK24����ʼ��ַ
				break;
			case 4:
				ftinfo.f32addr=ftinfo.f24addr+ftinfo.gbk24size;	//GBK24֮�󣬽���GBK32�ֿ�
				ftinfo.gbk32size=fftemp->obj.objsize;					//GBK32�ֿ��С
				flashaddr=ftinfo.f32addr;						//GBK32����ʼ��ַ
				break;
		} 
			
		while(res==FR_OK)//��ѭ��ִ��
		{
	 		res=f_read(fftemp,tempbuf,4096,(UINT *)&bread);		//��ȡ����	 
			if(res!=FR_OK)break;								//ִ�д���
			W25QXX_Write(tempbuf,offx+flashaddr,4096);		    //��0��ʼд��4096������  
	  		offx+=bread;	  
			emwin_fupd_prog(fftemp->obj.objsize,offx);	    //������ʾ
			if(bread!=4096)break;								//������.
	 	} 	
		f_close(fftemp);		
	}			 
	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	myfree(SRAMIN,tempbuf);	//�ͷ��ڴ�
	return res;
} 
//���������ļ�,UNIGBK,GBK12,GBK16,GBK24,GBK32һ�����
//src:�ֿ���Դ����."0:",SD��;"1:",FLASH��,"2:",U��.
//��ʾ��Ϣ�����С										  
//����ֵ:0,���³ɹ�;
//		 ����,�������.	  
u8 emwin_update_font(u8* src)
{	
    WM_HWIN hItem;
	u8 *pname;
	u32 *buf;
	u8 res=0;		   
 	u16 i,j;
	FIL *fftemp;
	u8 rval=0; 
	res=0XFF;		
	ftinfo.fontok=0XFF;
	pname=mymalloc(SRAMIN,100);	//����100�ֽ��ڴ�  
	buf=mymalloc(SRAMIN,4096);	//����4K�ֽ��ڴ�  
	fftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));	//�����ڴ�	
	if(buf==NULL||pname==NULL||fftemp==NULL)
	{
		myfree(SRAMIN,fftemp);
		myfree(SRAMIN,pname);
		myfree(SRAMIN,buf);
		return 5;		//�ڴ�����ʧ��
	}
	for(i=0;i<5;i++)	//�Ȳ����ļ�UNIGBK,GBK12,GBK16,GBK24,GBK32�Ƿ����� 
	{ 
		strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
		strcat((char*)pname,(char*)GBK_PATH[i]);		//׷�Ӿ����ļ�·�� 
		res=f_open(fftemp,(const TCHAR*)pname,FA_READ);	//���Դ�
		if(res)
		{
			rval|=1<<7;	//��Ǵ��ļ�ʧ��  
			break;		//������,ֱ���˳�
		}
	} 

	myfree(SRAMIN,fftemp);	//�ͷ��ڴ�
	if(rval==0)				//�ֿ��ļ�������.
	{  
        hItem=WM_GetDialogItem(HzfontWin, ID_TEXT_1);
        TEXT_SetText(hItem,"Erasing sectors...");
        GUI_Exec();
		for(i=0;i<FONTSECSIZE;i++)			//�Ȳ����ֿ�����,���д���ٶ�
		{
			emwin_fupd_prog(FONTSECSIZE,i);//������ʾ
			W25QXX_Read((u8*)buf,((FONTINFOADDR/4096)+i)*4096,4096);//������������������
			for(j=0;j<1024;j++)//У������
			{
				if(buf[j]!=0XFFFFFFFF)break;//��Ҫ����  	  
			}
			if(j!=1024)W25QXX_Erase_Sector((FONTINFOADDR/4096)+i);	//��Ҫ����������
		}
		for(i=0;i<5;i++)	//���θ���UNIGBK,GBK12,GBK16,GBK24,GBK32
		{
            TEXT_SetText(hItem,(const char*)UPDATE_REMIND_TBL[i]);  //�޸�TEXT1�ؼ�����
            GUI_Exec();
			strcpy((char*)pname,(char*)src);				//copy src���ݵ�pname
			strcat((char*)pname,(char*)GBK_PATH[i]); 		//׷�Ӿ����ļ�·�� 
            res=emwin_updata_fontx(pname,i);	//�����ֿ�
			if(res)
			{
				myfree(SRAMIN,buf);
				myfree(SRAMIN,pname);
				return 1+i;
			} 
		} 
		//ȫ�����º���
		ftinfo.fontok=0XAA;
		W25QXX_Write((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//�����ֿ���Ϣ
	}
	myfree(SRAMIN,pname);//�ͷ��ڴ� 
	myfree(SRAMIN,buf);
    hItem=WM_GetDialogItem(HzfontWin, ID_TEXT_0);
    TEXT_SetTextColor(hItem,GUI_RED);
    TEXT_SetText(hItem,"Font Updata finshed,Please Restart!");
    GUI_Exec();
	return rval;//�޴���.			 
} 

//�Ի���ص�����
static void EmWinHzDialog(WM_MESSAGE * pMsg) 
{
    WM_HWIN hItem;
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            GUI_SetColor(GUI_WHITE);
			GUI_Clear();
            break;
        case WM_INIT_DIALOG:
            //��ʼ��TEXT
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
            TEXT_SetFont(hItem, &GUI_Font24B_ASCII);
            TEXT_SetText(hItem, "Font Updataing,Please Wait...");
        
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
            TEXT_SetFont(hItem, GUI_FONT_8X15B_ASCII);
            TEXT_SetText(hItem, "erase chip......");
            TEXT_SetTextColor(hItem,GUI_RED);
        
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
            TEXT_SetFont(hItem, GUI_FONT_8X15B_ASCII);
            TEXT_SetText(hItem, "0%");
            TEXT_SetTextColor(hItem,GUI_RED);
            break;
        default:
            WM_DefaultProc(pMsg);
            break;
    }
}

//�����ֿ���½���
void HzFontupdataCreate(void) 
{
//    OS_ERR err;
//    HzfontWin=GUI_CreateDialogBox(EmwinHzDialog, GUI_COUNTOF(EmwinHzDialog),EmWinHzDialog,WM_HBKWIN, 0, 0);
//    GUI_Exec();
//    while(1)
//    {
//        OSSchedLock(&err);		        //����������
//        emwin_update_font("0:");        //�����ֿ� 
//        OSSchedUnlock(&err);	        //����������
//        WM_DeleteWindow(HzfontWin);     //ɾ������
//        GUI_Delay(100);
//        break;                          //������ɣ�����ѭ��
//    }
}
