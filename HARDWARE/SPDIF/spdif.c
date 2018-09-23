#include "spdif.h"
#include "delay.h"
#include "wm8978.h"
#include "sai.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F7开发板
//SPDIF驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/3/6
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define AUDIODATA_SIZE 200
u32 spdif_audiobuff[2][AUDIODATA_SIZE]; //音频数据双缓冲区,200*4=800字节
u32 spdif_controlbuff[10];              //SPDIF传输通道状态和用户信息

spdif_struct spdif_dev;                 //SPDIF控制结构体

SPDIFRX_HandleTypeDef SPDIFIN1_Handle;  //SPDIF IN1句柄
DMA_HandleTypeDef SPDIF_DTDMA_Handler;  //SPDIF音频数据DMA
DMA_HandleTypeDef SPDIF_CSDMA_Handler;  //SPDIF传输通道状态和用户信息DMA

//初始化SPDIF
void SPDIFRX_Init(void)
{
    spdif_dev.spdif_clk=1580000;                                        //默认为158M，单位为100HZ
    SPDIFCLK_Config();                                                  //配置SPDIF时钟
    SPDIFIN1_Handle.Instance=SPDIFRX;
    SPDIFIN1_Handle.Init.InputSelection=SPDIFRX_INPUT_IN1;              //SPDIF输入1
    SPDIFIN1_Handle.Init.Retries=SPDIFRX_MAXRETRIES_15;                 //同步阶段允许重试次数
    SPDIFIN1_Handle.Init.WaitForActivity=SPDIFRX_WAITFORACTIVITY_ON;    //等待同步
    SPDIFIN1_Handle.Init.ChannelSelection=SPDIFRX_CHANNEL_A;            //控制流从通道A获取通道状态
    SPDIFIN1_Handle.Init.DataFormat=SPDIFRX_DATAFORMAT_32BITS;          //右对齐
    SPDIFIN1_Handle.Init.StereoMode=SPDIFRX_STEREOMODE_ENABLE;          //使能立体声模式
    SPDIFIN1_Handle.Init.PreambleTypeMask=SPDIFRX_PREAMBLETYPEMASK_OFF; //报头类型不复制到SPDIFRX_DR中
    SPDIFIN1_Handle.Init.ChannelStatusMask=SPDIFRX_CHANNELSTATUS_OFF;   //通道状态和用户位不复制到SPDIFRX_DR中
    SPDIFIN1_Handle.Init.ValidityBitMask=SPDIFRX_VALIDITYMASK_ON;      //有效性位不复制到SPDIFRX_DR中
    SPDIFIN1_Handle.Init.ParityErrorMask=SPDIFRX_PARITYERRORMASK_ON;   //奇偶校验错误位不复制到SPDIFRX_DR中
    HAL_SPDIFRX_Init(&SPDIFIN1_Handle);
    SPDIFIN1_Handle.Instance->CR|=SPDIFRX_CR_RXDMAEN;                   //SPDIF音频数据使用DMA来接收
    SPDIFIN1_Handle.Instance->CR|=SPDIFRX_CR_CBDMAEN;                   //SPDIF传输通道状态和用户信息使用DMA来接收
    
    HAL_NVIC_SetPriority(SPDIF_RX_IRQn,1,0);                            //SPDIF中断
    HAL_NVIC_EnableIRQ(SPDIF_RX_IRQn); 
    //使能SPDIF的串行接口错误中断、上溢错误和奇偶校验错误  
    __HAL_SPDIFRX_ENABLE_IT(&SPDIFIN1_Handle,SPDIFRX_IT_IFEIE|SPDIFRX_IT_PERRIE); 
    SPDIF_AUDIODATA_DMA_Init((u32*)&spdif_audiobuff[0],(u32*)&spdif_audiobuff[1],AUDIODATA_SIZE,2);  
    SPDIF_CONTROLDATA_DMA_Init(spdif_controlbuff,10);
}

//SPDIF 音频数据接收DMA配置
//设置为双缓冲模式,并开启DMA传输完成中断
//buf0:M0AR地址.
//buf1:M1AR地址.
//num:每次传输数据量
//width:位宽(存储器和外设,同时设置),0,8位;1,16位;2,32位;
void SPDIF_AUDIODATA_DMA_Init(u32* buf0,u32 *buf1,u16 num,u8 width)
{ 
    u32 memwidth=0,perwidth=0;      //外设和存储器位宽
    switch(width)
    {
        case 0:         //8位
            memwidth=DMA_MDATAALIGN_BYTE;
            perwidth=DMA_PDATAALIGN_BYTE;
            break;
        case 1:         //16位
            memwidth=DMA_MDATAALIGN_HALFWORD;
            perwidth=DMA_PDATAALIGN_HALFWORD;
            break;
        case 2:         //32位
            memwidth=DMA_MDATAALIGN_WORD;
            perwidth=DMA_PDATAALIGN_WORD;
            break;
            
    }
    __HAL_RCC_DMA1_CLK_ENABLE();                                        //使能DMA1时钟
    __HAL_LINKDMA(&SPDIFIN1_Handle,hdmaDrRx,SPDIF_DTDMA_Handler);       //将DMA与SPDIF联系起来

    SPDIF_DTDMA_Handler.Instance=DMA1_Stream1;                          //DMA1数据流1                   
    SPDIF_DTDMA_Handler.Init.Channel=DMA_CHANNEL_0;                     //通道0
    SPDIF_DTDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;            //外设到存储器模式
    SPDIF_DTDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;                //外设非增量模式
    SPDIF_DTDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                    //存储器增量模式
    SPDIF_DTDMA_Handler.Init.PeriphDataAlignment=perwidth;              //外设数据长度:16/32位
    SPDIF_DTDMA_Handler.Init.MemDataAlignment=memwidth;                 //存储器数据长度:16/32位
    SPDIF_DTDMA_Handler.Init.Mode=DMA_CIRCULAR;                         //使用循环模式 
    SPDIF_DTDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;                //最高优先级
    SPDIF_DTDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;             //不使用FIFO
    SPDIF_DTDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;                //存储器单次突发传输
    SPDIF_DTDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;             //外设突发单次传输 
    HAL_DMA_DeInit(&SPDIF_DTDMA_Handler);                               //先清除以前的设置
    HAL_DMA_Init(&SPDIF_DTDMA_Handler);	                                //初始化DMA
    
    HAL_DMAEx_MultiBufferStart(&SPDIF_DTDMA_Handler,(u32)&SPDIFRX->DR,(u32)buf0,(u32)buf1,num);//开启双缓冲	
    __HAL_DMA_ENABLE_IT(&SPDIF_DTDMA_Handler,DMA_IT_TC);                //开启传输完成中断
    
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn,2,0);                        //DMA中断优先级
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

//SPDIF 传输通道状态和用户信息DMA配置
//设置为单缓冲模式,并开启DMA传输完成中断
//buf0:M0AR地址.
//num:每次传输数据量
//width:位宽(存储器和外设,同时设置),0,8位;1,16位;2,32位;
void SPDIF_CONTROLDATA_DMA_Init(u32* buf0,u16 num)
{ 
    __HAL_RCC_DMA1_CLK_ENABLE();                                        //使能DMA1时钟
    __HAL_LINKDMA(&SPDIFIN1_Handle,hdmaCsRx,SPDIF_CSDMA_Handler);       //将DMA与SPDIF联系起来

    SPDIF_CSDMA_Handler.Instance=DMA1_Stream6;                          //DMA1数据流6                   
    SPDIF_CSDMA_Handler.Init.Channel=DMA_CHANNEL_0;                     //通道0
    SPDIF_CSDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;            //外设到存储器模式
    SPDIF_CSDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;                //外设非增量模式
    SPDIF_CSDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                    //存储器增量模式
    SPDIF_CSDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_WORD;   //外设数据长度:32位
    SPDIF_CSDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_WORD;      //存储器数据长度:32位
    SPDIF_CSDMA_Handler.Init.Mode=DMA_CIRCULAR;                         //使用循环模式 
    SPDIF_CSDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;                //最高优先级
    SPDIF_CSDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;             //不使用FIFO
    SPDIF_CSDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;                //存储器单次突发传输
    SPDIF_CSDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;             //外设突发单次传输 
    HAL_DMA_DeInit(&SPDIF_CSDMA_Handler);                               //先清除以前的设置
    HAL_DMA_Init(&SPDIF_CSDMA_Handler);	                                //初始化DMA
    
    HAL_DMA_Start(&SPDIF_CSDMA_Handler,(u32)&SPDIFRX->CSR,(u32)buf0,num);//开启DMA   
    __HAL_DMA_ENABLE_IT(&SPDIF_CSDMA_Handler,DMA_IT_TC);                //开启传输完成中断
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn,3,0);                        //DMA中断优先级
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}


//SPDIF时钟配置,设置为158MHZ
void SPDIFCLK_Config(void)
{
    RCC_PeriphCLKInitTypeDef SPDIFCLK_Sture;
    
    SPDIFCLK_Sture.PeriphClockSelection=RCC_PERIPHCLK_SPDIFRX;      //SPDIF RX时钟 
    SPDIFCLK_Sture.PLLI2S.PLLI2SN=316;                              //设置PLLI2SN
    SPDIFCLK_Sture.PLLI2S.PLLI2SP=0;                                //设置PLLI2SP 
    HAL_RCCEx_PeriphCLKConfig(&SPDIFCLK_Sture);                     //设置时钟
}

//进入同步状态，同步完成以后进入接收状态
//返回值:0 未同步;1 同步
u8 WaitSync_TORecv(void)
{
    u8 flag=0;
    u8 timeout=0;
    __HAL_SPDIFRX_SYNC(&SPDIFIN1_Handle);
    while(__HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_SYNCD)==0)//等待同步完成
    {
        timeout++;
        delay_ms(5);
        if (timeout>100) break;//超时，跳出
    }
    if(timeout>100) flag=0;//未同步
    else //同步完成
    {
        flag=1;
        __HAL_SPDIFRX_RCV(&SPDIFIN1_Handle);//同步完成，进入接收阶段  
    }
    return flag;
}

//获取SPDIF收到的音频采样率
void SPDIF_GetRate(void)
{
    u16 spdif_w5;
    u32 spdif_rate; 
    
    spdif_w5=(SPDIFIN1_Handle.Instance->SR)>>16;
    spdif_rate=(spdif_dev.spdif_clk*5)/(spdif_w5&0X7FFF);
    spdif_rate>>=6;     //除以64
    spdif_rate*=100;    //乘以100，得到最终的实际采样率

    if((44100-1500<=spdif_rate)&&(spdif_rate<=44100+1500)) spdif_dev.spdifrate=44100; //44.1K的采样率
    else if((48000-1500<=spdif_rate)&&(spdif_rate<=48000+1500)) spdif_dev.spdifrate=48000; //48K的采样率
    else if((88200-1500<=spdif_rate)&&(spdif_rate<=88200+1500)) spdif_dev.spdifrate=88200; //88.2K的采样率
    else if((96000-1500<=spdif_rate)&&(spdif_rate<=96000+1500)) spdif_dev.spdifrate=96000; //96K的采样率
    else if((176400-1500<=spdif_rate)&&(spdif_rate<=176400+1500)) spdif_dev.spdifrate=176400; //176.4K的采
    else if((192000-1500<=spdif_rate)&&(spdif_rate<=192000+1500)) spdif_dev.spdifrate=192000; //192K的采
    else spdif_dev.spdifrate=0;
}

//SPDIF底层IO初始化和时钟使能
//此函数会被HAL_SPDIF_Init()调用
//hltdc:SPDIF句柄
void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef *hspdif)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_SPDIFRX_CLK_ENABLE();             //使能SPDIF RX时钟
    __HAL_RCC_GPIOG_CLK_ENABLE();               //使能GPIOG时钟

    //初始化PG12，SPDIF IN引脚
    GPIO_Initure.Pin=GPIO_PIN_12;               //PG12，SPDIF IN引脚
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //复用
    GPIO_Initure.Pull=GPIO_NOPULL;              //无上下拉      
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;         //高速
    GPIO_Initure.Alternate=GPIO_AF7_SPDIFRX;    //复用为SPDIF RX
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);
}

//SPDIF接收中断服务函数
void SPDIF_RX_IRQHandler(void)
{
    //发生超时、同步和帧错误中断,这三个中断一定要处理！
    if( __HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_FERR)||\
        __HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_SERR)||\
        __HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_TERR))
    {
        SPDIF_Play_Stop();//发生错误，关闭SPDIF播放
        __HAL_SPDIFRX_IDLE(&SPDIFIN1_Handle);   //当发生超时、同步和帧错误的时候要将SPDIFRXEN写0来清除中断
        __HAL_SPDIFRX_RCV(&SPDIFIN1_Handle);    //当清除中断以后需要重新将SPDIF设置为接收模式
    }else if(__HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_OVR))     //上溢错误
    {
        __HAL_SPDIFRX_CLEAR_IT(&SPDIFIN1_Handle, SPDIFRX_FLAG_OVR);         //清除上溢错误中断
    }else if(__HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_PERR))    //奇偶校验错误
    {
        __HAL_SPDIFRX_CLEAR_IT(&SPDIFIN1_Handle, SPDIFRX_FLAG_PERR);        //清除奇偶校验错误
    }
}

//配置音频接口
//AudioFreq:音频采样率
uint32_t SPDIF_AUDIO_Init(uint32_t AudioFreq)
{   
    SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);//设置SAI,主发送,16位数据				
	SAIA_SampleRate_Set(AudioFreq);	//设置采样率  
	SAIA_TX_DMA_Init((u8*)&spdif_audiobuff[0],(u8*)&spdif_audiobuff[1],AUDIODATA_SIZE*2,1); //配置TX DMA,16位 
	SAI_Play_Start();							//开启DMA  
	return 0; 
}

//DMA1数据流1(用于传输音频数据)
void DMA1_Stream1_IRQHandler(void)
{
    if(__HAL_DMA_GET_FLAG(&SPDIF_DTDMA_Handler,DMA_FLAG_TCIF1_5)!=RESET)//DMA传输完成
    {
        __HAL_DMA_CLEAR_FLAG(&SPDIF_DTDMA_Handler,DMA_FLAG_TCIF1_5);//清除DMA传输完成中断标志位
    }  
}

//DMA1数据流6(用于传输通道状态和用户信息)
void DMA1_Stream6_IRQHandler(void)
{
    if(__HAL_DMA_GET_FLAG(&SPDIF_CSDMA_Handler,DMA_FLAG_TCIF2_6)!=RESET)//DMA传输完成
    {
        __HAL_DMA_CLEAR_FLAG(&SPDIF_CSDMA_Handler,DMA_FLAG_TCIF2_6);//清除DMA传输完成中断标志位
    }  
}


//SPDIF开始播放
void SPDIF_Play_Start(void)
{ 
    spdif_dev.connsta=1;    //标记已经打开SPDIF
    __HAL_DMA_ENABLE(&SPDIF_DTDMA_Handler); //开启SPDIF DMA传输 
}
//SPDIF关闭
void SPDIF_Play_Stop(void)
{
    spdif_dev.connsta=0;    //标记已经关闭SPDIF
    __HAL_DMA_DISABLE(&SPDIF_DTDMA_Handler);//关闭SPDIF DMA传输 	 
    //两个缓冲区一定要清零，否则在断开的时候会有很大杂音！！！
    memset((u8*)&spdif_audiobuff[0],0,AUDIODATA_SIZE*4);   
    memset((u8*)&spdif_audiobuff[1],0,AUDIODATA_SIZE*4);
} 


