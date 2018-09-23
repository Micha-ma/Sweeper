#include "bmpdisplay.h"
#include "GUI.h"

static FIL BMPFile;

int bmptest(PICNAME name, u8 *BMPFileName, u32 x,u32 y)
{
	char result;
	char *bmpbuffer;
	u16 bread;
	
	switch(name)
	{
		case CAR:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);   //(const TCHAR*)"1:/PICTURE/Diandong/700car.bmp"
			break;
		case DIANCHI:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);  //(const TCHAR*)"1:/PICTURE/Diandong/700dianchi.bmp"
			break;
		case FAN:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ); //(const TCHAR*)"1:/PICTURE/Diandong/700fan.bmp"
			break;
		case HOUTUI:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);  //(const TCHAR*)"1:/PICTURE/Diandong/700houtui.bmp"
			break;
		case JIAOSHA:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);  //(const TCHAR*)"1:/PICTURE/Diandong/700jiaosha.bmp"
			break;
		case JINGUANG:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ); //(const TCHAR*)"1:/PICTURE/Diandong/700jinguang.bmp"
			break;
		case KONGBAI:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);  //(const TCHAR*)"1:/PICTURE/Diandong/700kongbai.bmp"
			break;
		case QIANJIN:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);   //(const TCHAR*)"1:/PICTURE/Diandong/700qianjin.bmp"
			break;
		case SAOSHUA:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);   //(const TCHAR*)"1:/PICTURE/Diandong/700saoshua.bmp"
			break;
		case SHANDIAN:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);  //(const TCHAR*)"1:/PICTURE/Diandong/700shandian.bmp"
			break;
		case SHOUSHA:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);   //(const TCHAR*)"1:/PICTURE/Diandong/700shousha.bmp"
			break;
		case SHUIWEIDI:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ); //(const TCHAR*)"1:/PICTURE/Diandong/700shuiweidi.bmp"
			break;
		case WENDUJI:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);   //(const TCHAR*)"1:/PICTURE/Diandong/700wenduji.bmp"
			break;
		case YOUZHUAN:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);  //(const TCHAR*)"1:/PICTURE/Diandong/700youzhuan.bmp"
			break;
		case ZUOZHUAN:
			result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);  //(const TCHAR*)"1:/PICTURE/Diandong/700zuozhuan.bmp"
			break;
		default:
			break;
	}

	if((result != FR_OK) || (BMPFile.obj.objsize>BMPMEMORYSIZE)) 	
		return 1;
	bmpbuffer = mymalloc(SRAMEX,BMPFile.obj.objsize);//申请内存
	if(bmpbuffer == NULL) 
		return 2;
	result = f_read(&BMPFile,bmpbuffer,BMPFile.obj.objsize,(UINT *)&bread); //读取数据
	if(result != FR_OK) 
		return 3;
	GUI_BMP_Draw(bmpbuffer,x,y);
	f_close(&BMPFile);
	myfree(SRAMEX,bmpbuffer);
	return 0;
}
