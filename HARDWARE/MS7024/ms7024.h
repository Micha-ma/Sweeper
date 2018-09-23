#ifndef _MS7024_H
#define _MS7024_H_H
#include "sys.h"
#include "myiic.h"

typedef struct _MS_REGISTER8BI8BV_STREAM_
{
    u8        u8_index;
    u8        u8_val;
}MS7024_8BI8BV_T;
//extern MS7024_8BI8BV_T ms7024_config[];
extern MS7024_8BI8BV_T ms7024_config_1[];
extern MS7024_8BI8BV_T ms7024_config_2[];
extern MS7024_8BI8BV_T ms7024_config_3[];
extern MS7024_8BI8BV_T ms7024_config_4[];
extern MS7024_8BI8BV_T ms7024_config_5[];
void MS7024_WriteBytes(MS7024_8BI8BV_T *pbulkdata);
void MS7024_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//指定地址写入一个字节
#endif
