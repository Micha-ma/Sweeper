#include "bsp.h"
#include "user_config.h"
#include "ms7024.h"

u16 ds18b20_init_err;

void bsp_init()
{
	u8 i;
	
	Cache_Enable();                 //打开L1-Cache
	MPU_Memory_Protection();        //保护相关存储区域
	HAL_Init();                     //HAL库初始化
	Stm32_Clock_Init(432,25,2,9);   //216M时钟
	delay_init(216);    
	SDRAM_Init();                   //SDRAM初始化
	delay_ms(250);       
	LED_Init();                     //LED初始化
	while(AT24CXX_Check())
	AT24CXX_Init();				    			//初始化I2C和EEPROM
	
	//7024芯片复位
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
	delay_ms(20);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
	delay_ms(20); 
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
//	//配置MS7024,具体寄存器ID和指令还需设置	
//	
////	MS7024_WriteBytes(ms7024_config);
////	delay_ms(250);
//	//add by mch 20180528
//	MS7024_WriteBytes(ms7024_config_1);
//	delay_ms(10);
//	MS7024_WriteBytes(ms7024_config_2);
//	delay_ms(10);
//	MS7024_WriteBytes(ms7024_config_3);
//	delay_ms(10);
//	MS7024_WriteBytes(ms7024_config_4);
//	delay_ms(100);
//	MS7024_WriteBytes(ms7024_config_5);
//	delay_ms(100);

	LTDC_Init();	 						  		//初始化LCD显示
	W25QXX_Init();                  //初始化W25Q256(SPIFLASH)
	my_mem_init(SRAMIN);            //初始化内部SRAM的内存池
	my_mem_init(SRAMEX);            //初始化外部SDRAM的内存池
	my_mem_init(SRAMDTCM);          //初始化内部DTCM的内存池
	exfuns_init();		              //为fatfs相关变量申请内存 
	f_mount(fs[0],"0:",1); 		    	//挂载SD卡 
  f_mount(fs[1],"1:",1); 	        //挂载FLASH.	
  f_mount(fs[2],"2:",1); 		    	//挂载NAND FLASH
	
	//add by mch 20180530
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
	delay_ms(100);
	//配置MS7024,具体寄存器ID和指令还需设置	
	
	MS7024_WriteBytes(ms7024_config_1);
	delay_ms(10);
	MS7024_WriteBytes(ms7024_config_2);
	delay_ms(10);
	MS7024_WriteBytes(ms7024_config_3);
	delay_ms(10);
	MS7024_WriteBytes(ms7024_config_4);
	delay_ms(100);
	MS7024_WriteBytes(ms7024_config_5);
	delay_ms(100);
	
  while(font_init())							//初始化字库
	{
		LCD_ShowString(30,70,200,16,16,"Font Error!");
		while(W25QXX_ReadID()!=W25Q256)							//原为检测SD卡SD_Init()，现检查SPI FLASH
		{
			LCD_ShowString(30,90,200,16,16,"SPI FLASH Failed!");
			delay_ms(200);
			LCD_Fill(30,90,200+30,70+16,WHITE);
			delay_ms(200);		    
		}
		update_font(30,90,16,"1:");		//如果字库不存在就更新字库
		delay_ms(2000);
		LCD_Clear(WHITE);							//清屏
		break;
	}  
	
	DISIO_Init();   							  //硬件IO初始化
	for(i=0;i<2;i++)
	{
		if(DS18B20_Init()==0)					//初始化DS18B20
		{
			break;
		}
		else
		{
			delay_ms(200);
		}
	}
	//change by mch 20180508, 开机蜂鸣器提醒
	BEEP_CTRL(1);//打开蜂鸣器，间隔1秒
	delay_ms(200);
	BEEP_CTRL(0);
	
	MY_ADC_Init();	           			//初始化ADC
	CAN1_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_11TQ, 12, CAN_MODE_NORMAL);  //CAN初始化，波特率为250K，正常模式
	CAN3_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_11TQ, 12, CAN_MODE_NORMAL);  //CAN初始化，波特率为250K，正常模式
	
	WM_SetCreateFlags(WM_CF_MEMDEV);//支持窗口存储设备
	GUI_Init();                     //GUI初始化
	WM_MULTIBUF_Enable(1);          //开启多级缓存
	GUI_SetBkColor(GUI_WHITE);      //设置桌面窗口背景为白色
	InitMainInterface();						//主界面框架初始化
	
	TIM3_Init(100-1,10800-1); 		  //定时器3初始化，100*100ns 10ms
	TIM6_Init(10-1,1080-1);    	    //定时器6初始化10*10ns   100ns
}

//初始化PB0,PB1为输出.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();			//开启GPIOB时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();			//开启GPIOA时钟

	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1; //PB0,1
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_15; //PA15
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_0;            //PA0
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);	//PB1置0
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);	//PB1置1 
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);	//PB1置0
}

////////////////////////////////////////////////////////////////////////////////// 	 
//IO口初始化
//////////////////////////////////////////////////////////////////////////////////
void DISIO_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();			//开启GPIOA的时钟
	__HAL_RCC_GPIOB_CLK_ENABLE();			//开启GPIOB的时钟
	__HAL_RCC_GPIOC_CLK_ENABLE();			//开启GPIOC的时钟
	__HAL_RCC_GPIOD_CLK_ENABLE();			//开启GPIOD的时钟
	__HAL_RCC_GPIOE_CLK_ENABLE();			//开启GPIOE的时钟
	__HAL_RCC_GPIOG_CLK_ENABLE();			//开启GPIOG的时钟
	__HAL_RCC_GPIOH_CLK_ENABLE();			//开启GPIOH的时钟

	GPIO_Initure.Pin=GPIO_PIN_1;            //PA1 P_VS
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //ê?è?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_8;            //PA8 key2
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLUP;          //pull up
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_11;           //PB11 key1
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLUP;          //pull up
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	

	GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;            //PB7/8/9/
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	//PC8和PC9在SD卡中复用了，其在SD设置为上拉，故管脚的初始化DISIO_Init()要在SD_Init()之后
	//否则这两个引脚本身会输出3.3V，导致近光和手刹（尤其是近光）指示会随机出现
	GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7;            //PC6/7
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_8|GPIO_PIN_9;            //PC/8/9
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLUP;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;            //PC/10/11/12/13/high speed
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLUP;        //pull UP
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_3;            //PD3/high speed  KEY3
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLUP;        //pull UP
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;            //PE2/3/4/5/6
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;            //PG12/13/14
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //í?íìê?3?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOG,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3;//|GPIO_PIN_4|GPIO_PIN_5;            //PH2/3/4/5
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //ê?è?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???ù
	HAL_GPIO_Init(GPIOH,&GPIO_Initure);

	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,GPIO_PIN_RESET);		//原USB POWER ON，高电平
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_12,GPIO_PIN_RESET);	//PG12 BEEPCTRL，低电平
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);	//PG13 DO1，低电平
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);	//PG14 DO2，高电平
}



