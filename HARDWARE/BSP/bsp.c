#include "bsp.h"
#include "user_config.h"
#include "ms7024.h"

u16 ds18b20_init_err;

void bsp_init()
{
	u8 i;
	
	Cache_Enable();                 //��L1-Cache
	MPU_Memory_Protection();        //������ش洢����
	HAL_Init();                     //HAL���ʼ��
	Stm32_Clock_Init(432,25,2,9);   //216Mʱ��
	delay_init(216);    
	SDRAM_Init();                   //SDRAM��ʼ��
	delay_ms(250);       
	LED_Init();                     //LED��ʼ��
	while(AT24CXX_Check())
	AT24CXX_Init();				    			//��ʼ��I2C��EEPROM
	
	//7024оƬ��λ
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
	delay_ms(20);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
	delay_ms(20); 
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
//	//����MS7024,����Ĵ���ID��ָ�������	
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

	LTDC_Init();	 						  		//��ʼ��LCD��ʾ
	W25QXX_Init();                  //��ʼ��W25Q256(SPIFLASH)
	my_mem_init(SRAMIN);            //��ʼ���ڲ�SRAM���ڴ��
	my_mem_init(SRAMEX);            //��ʼ���ⲿSDRAM���ڴ��
	my_mem_init(SRAMDTCM);          //��ʼ���ڲ�DTCM���ڴ��
	exfuns_init();		              //Ϊfatfs��ر��������ڴ� 
	f_mount(fs[0],"0:",1); 		    	//����SD�� 
  f_mount(fs[1],"1:",1); 	        //����FLASH.	
  f_mount(fs[2],"2:",1); 		    	//����NAND FLASH
	
	//add by mch 20180530
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
	delay_ms(100);
	//����MS7024,����Ĵ���ID��ָ�������	
	
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
	
  while(font_init())							//��ʼ���ֿ�
	{
		LCD_ShowString(30,70,200,16,16,"Font Error!");
		while(W25QXX_ReadID()!=W25Q256)							//ԭΪ���SD��SD_Init()���ּ��SPI FLASH
		{
			LCD_ShowString(30,90,200,16,16,"SPI FLASH Failed!");
			delay_ms(200);
			LCD_Fill(30,90,200+30,70+16,WHITE);
			delay_ms(200);		    
		}
		update_font(30,90,16,"1:");		//����ֿⲻ���ھ͸����ֿ�
		delay_ms(2000);
		LCD_Clear(WHITE);							//����
		break;
	}  
	
	DISIO_Init();   							  //Ӳ��IO��ʼ��
	for(i=0;i<2;i++)
	{
		if(DS18B20_Init()==0)					//��ʼ��DS18B20
		{
			break;
		}
		else
		{
			delay_ms(200);
		}
	}
	//change by mch 20180508, ��������������
	BEEP_CTRL(1);//�򿪷����������1��
	delay_ms(200);
	BEEP_CTRL(0);
	
	MY_ADC_Init();	           			//��ʼ��ADC
	CAN1_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_11TQ, 12, CAN_MODE_NORMAL);  //CAN��ʼ����������Ϊ250K������ģʽ
	CAN3_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_11TQ, 12, CAN_MODE_NORMAL);  //CAN��ʼ����������Ϊ250K������ģʽ
	
	WM_SetCreateFlags(WM_CF_MEMDEV);//֧�ִ��ڴ洢�豸
	GUI_Init();                     //GUI��ʼ��
	WM_MULTIBUF_Enable(1);          //�����༶����
	GUI_SetBkColor(GUI_WHITE);      //�������洰�ڱ���Ϊ��ɫ
	InitMainInterface();						//�������ܳ�ʼ��
	
	TIM3_Init(100-1,10800-1); 		  //��ʱ��3��ʼ����100*100ns 10ms
	TIM6_Init(10-1,1080-1);    	    //��ʱ��6��ʼ��10*10ns   100ns
}

//��ʼ��PB0,PB1Ϊ���.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��
	__HAL_RCC_GPIOA_CLK_ENABLE();			//����GPIOAʱ��

	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1; //PB0,1
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull=GPIO_PULLUP;          //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_15; //PA15
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull=GPIO_PULLUP;          //����
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_0;            //PA0
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);	//PB1��0
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);	//PB1��1 
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);	//PB1��0
}

////////////////////////////////////////////////////////////////////////////////// 	 
//IO�ڳ�ʼ��
//////////////////////////////////////////////////////////////////////////////////
void DISIO_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();			//����GPIOA��ʱ��
	__HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOB��ʱ��
	__HAL_RCC_GPIOC_CLK_ENABLE();			//����GPIOC��ʱ��
	__HAL_RCC_GPIOD_CLK_ENABLE();			//����GPIOD��ʱ��
	__HAL_RCC_GPIOE_CLK_ENABLE();			//����GPIOE��ʱ��
	__HAL_RCC_GPIOG_CLK_ENABLE();			//����GPIOG��ʱ��
	__HAL_RCC_GPIOH_CLK_ENABLE();			//����GPIOH��ʱ��

	GPIO_Initure.Pin=GPIO_PIN_1;            //PA1 P_VS
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //��?��?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_8;            //PA8 key2
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLUP;          //pull up
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_11;           //PB11 key1
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLUP;          //pull up
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	

	GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;            //PB7/8/9/
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	//PC8��PC9��SD���и����ˣ�����SD����Ϊ�������ʹܽŵĳ�ʼ��DISIO_Init()Ҫ��SD_Init()֮��
	//�������������ű�������3.3V�����½������ɲ�������ǽ��⣩ָʾ���������
	GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7;            //PC6/7
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_8|GPIO_PIN_9;            //PC/8/9
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLUP;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;            //PC/10/11/12/13/high speed
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLUP;        //pull UP
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_3;            //PD3/high speed  KEY3
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLUP;        //pull UP
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);
	
	GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;            //PE2/3/4/5/6
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;            //PG12/13/14
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //��?������?3?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOG,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3;//|GPIO_PIN_4|GPIO_PIN_5;            //PH2/3/4/5
	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //��?��?
	GPIO_Initure.Pull=GPIO_PULLDOWN;        //pull down
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //???��
	HAL_GPIO_Init(GPIOH,&GPIO_Initure);

	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,GPIO_PIN_RESET);		//ԭUSB POWER ON���ߵ�ƽ
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_12,GPIO_PIN_RESET);	//PG12 BEEPCTRL���͵�ƽ
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);	//PG13 DO1���͵�ƽ
	HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);	//PG14 DO2���ߵ�ƽ
}



