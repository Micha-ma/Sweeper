#ifndef _BSP_H
#define _BSP_H

#include "sys.h"
#include "delay.h"
#include "tftlcd.h"
#include "ltdc.h"
#include "timer.h"
#include "usart.h"
#include "sdram.h"
//#include "nand.h"  
#include "sdmmc_sdcard.h"
#include "malloc.h"
#include "ff.h"
#include "mpu.h"
#include "exfuns.h"
#include "24cxx.h"
#include "can.h"
#include "w25qxx.h"
#include "fontupd.h"
#include "text.h"
//#include "audioplay.h"
//#include "videoplayer.h"
//#include "ds18b20.h"

void bsp_init(void);
void DISIO_Init(void);
void LED_Init(void);

#endif
