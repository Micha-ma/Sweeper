#include "bsp.h"
#include "display.h"
#include "user_config.h"

int main(void)
{ 	
	bsp_init();    //板级初始化、引脚、中断、定时器等
	
	DisplayData(); //界面显示
}
