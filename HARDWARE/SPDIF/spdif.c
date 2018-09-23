#include "spdif.h"
#include "delay.h"
#include "wm8978.h"
#include "sai.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//SPDIF��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/3/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define AUDIODATA_SIZE 200
u32 spdif_audiobuff[2][AUDIODATA_SIZE]; //��Ƶ����˫������,200*4=800�ֽ�
u32 spdif_controlbuff[10];              //SPDIF����ͨ��״̬���û���Ϣ

spdif_struct spdif_dev;                 //SPDIF���ƽṹ��

SPDIFRX_HandleTypeDef SPDIFIN1_Handle;  //SPDIF IN1���
DMA_HandleTypeDef SPDIF_DTDMA_Handler;  //SPDIF��Ƶ����DMA
DMA_HandleTypeDef SPDIF_CSDMA_Handler;  //SPDIF����ͨ��״̬���û���ϢDMA

//��ʼ��SPDIF
void SPDIFRX_Init(void)
{
    spdif_dev.spdif_clk=1580000;                                        //Ĭ��Ϊ158M����λΪ100HZ
    SPDIFCLK_Config();                                                  //����SPDIFʱ��
    SPDIFIN1_Handle.Instance=SPDIFRX;
    SPDIFIN1_Handle.Init.InputSelection=SPDIFRX_INPUT_IN1;              //SPDIF����1
    SPDIFIN1_Handle.Init.Retries=SPDIFRX_MAXRETRIES_15;                 //ͬ���׶��������Դ���
    SPDIFIN1_Handle.Init.WaitForActivity=SPDIFRX_WAITFORACTIVITY_ON;    //�ȴ�ͬ��
    SPDIFIN1_Handle.Init.ChannelSelection=SPDIFRX_CHANNEL_A;            //��������ͨ��A��ȡͨ��״̬
    SPDIFIN1_Handle.Init.DataFormat=SPDIFRX_DATAFORMAT_32BITS;          //�Ҷ���
    SPDIFIN1_Handle.Init.StereoMode=SPDIFRX_STEREOMODE_ENABLE;          //ʹ��������ģʽ
    SPDIFIN1_Handle.Init.PreambleTypeMask=SPDIFRX_PREAMBLETYPEMASK_OFF; //��ͷ���Ͳ����Ƶ�SPDIFRX_DR��
    SPDIFIN1_Handle.Init.ChannelStatusMask=SPDIFRX_CHANNELSTATUS_OFF;   //ͨ��״̬���û�λ�����Ƶ�SPDIFRX_DR��
    SPDIFIN1_Handle.Init.ValidityBitMask=SPDIFRX_VALIDITYMASK_ON;      //��Ч��λ�����Ƶ�SPDIFRX_DR��
    SPDIFIN1_Handle.Init.ParityErrorMask=SPDIFRX_PARITYERRORMASK_ON;   //��żУ�����λ�����Ƶ�SPDIFRX_DR��
    HAL_SPDIFRX_Init(&SPDIFIN1_Handle);
    SPDIFIN1_Handle.Instance->CR|=SPDIFRX_CR_RXDMAEN;                   //SPDIF��Ƶ����ʹ��DMA������
    SPDIFIN1_Handle.Instance->CR|=SPDIFRX_CR_CBDMAEN;                   //SPDIF����ͨ��״̬���û���Ϣʹ��DMA������
    
    HAL_NVIC_SetPriority(SPDIF_RX_IRQn,1,0);                            //SPDIF�ж�
    HAL_NVIC_EnableIRQ(SPDIF_RX_IRQn); 
    //ʹ��SPDIF�Ĵ��нӿڴ����жϡ�����������żУ�����  
    __HAL_SPDIFRX_ENABLE_IT(&SPDIFIN1_Handle,SPDIFRX_IT_IFEIE|SPDIFRX_IT_PERRIE); 
    SPDIF_AUDIODATA_DMA_Init((u32*)&spdif_audiobuff[0],(u32*)&spdif_audiobuff[1],AUDIODATA_SIZE,2);  
    SPDIF_CONTROLDATA_DMA_Init(spdif_controlbuff,10);
}

//SPDIF ��Ƶ���ݽ���DMA����
//����Ϊ˫����ģʽ,������DMA��������ж�
//buf0:M0AR��ַ.
//buf1:M1AR��ַ.
//num:ÿ�δ���������
//width:λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
void SPDIF_AUDIODATA_DMA_Init(u32* buf0,u32 *buf1,u16 num,u8 width)
{ 
    u32 memwidth=0,perwidth=0;      //����ʹ洢��λ��
    switch(width)
    {
        case 0:         //8λ
            memwidth=DMA_MDATAALIGN_BYTE;
            perwidth=DMA_PDATAALIGN_BYTE;
            break;
        case 1:         //16λ
            memwidth=DMA_MDATAALIGN_HALFWORD;
            perwidth=DMA_PDATAALIGN_HALFWORD;
            break;
        case 2:         //32λ
            memwidth=DMA_MDATAALIGN_WORD;
            perwidth=DMA_PDATAALIGN_WORD;
            break;
            
    }
    __HAL_RCC_DMA1_CLK_ENABLE();                                        //ʹ��DMA1ʱ��
    __HAL_LINKDMA(&SPDIFIN1_Handle,hdmaDrRx,SPDIF_DTDMA_Handler);       //��DMA��SPDIF��ϵ����

    SPDIF_DTDMA_Handler.Instance=DMA1_Stream1;                          //DMA1������1                   
    SPDIF_DTDMA_Handler.Init.Channel=DMA_CHANNEL_0;                     //ͨ��0
    SPDIF_DTDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;            //���赽�洢��ģʽ
    SPDIF_DTDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;                //���������ģʽ
    SPDIF_DTDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                    //�洢������ģʽ
    SPDIF_DTDMA_Handler.Init.PeriphDataAlignment=perwidth;              //�������ݳ���:16/32λ
    SPDIF_DTDMA_Handler.Init.MemDataAlignment=memwidth;                 //�洢�����ݳ���:16/32λ
    SPDIF_DTDMA_Handler.Init.Mode=DMA_CIRCULAR;                         //ʹ��ѭ��ģʽ 
    SPDIF_DTDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;                //������ȼ�
    SPDIF_DTDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;             //��ʹ��FIFO
    SPDIF_DTDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;                //�洢������ͻ������
    SPDIF_DTDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;             //����ͻ�����δ��� 
    HAL_DMA_DeInit(&SPDIF_DTDMA_Handler);                               //�������ǰ������
    HAL_DMA_Init(&SPDIF_DTDMA_Handler);	                                //��ʼ��DMA
    
    HAL_DMAEx_MultiBufferStart(&SPDIF_DTDMA_Handler,(u32)&SPDIFRX->DR,(u32)buf0,(u32)buf1,num);//����˫����	
    __HAL_DMA_ENABLE_IT(&SPDIF_DTDMA_Handler,DMA_IT_TC);                //������������ж�
    
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn,2,0);                        //DMA�ж����ȼ�
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

//SPDIF ����ͨ��״̬���û���ϢDMA����
//����Ϊ������ģʽ,������DMA��������ж�
//buf0:M0AR��ַ.
//num:ÿ�δ���������
//width:λ��(�洢��������,ͬʱ����),0,8λ;1,16λ;2,32λ;
void SPDIF_CONTROLDATA_DMA_Init(u32* buf0,u16 num)
{ 
    __HAL_RCC_DMA1_CLK_ENABLE();                                        //ʹ��DMA1ʱ��
    __HAL_LINKDMA(&SPDIFIN1_Handle,hdmaCsRx,SPDIF_CSDMA_Handler);       //��DMA��SPDIF��ϵ����

    SPDIF_CSDMA_Handler.Instance=DMA1_Stream6;                          //DMA1������6                   
    SPDIF_CSDMA_Handler.Init.Channel=DMA_CHANNEL_0;                     //ͨ��0
    SPDIF_CSDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;            //���赽�洢��ģʽ
    SPDIF_CSDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;                //���������ģʽ
    SPDIF_CSDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;                    //�洢������ģʽ
    SPDIF_CSDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_WORD;   //�������ݳ���:32λ
    SPDIF_CSDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_WORD;      //�洢�����ݳ���:32λ
    SPDIF_CSDMA_Handler.Init.Mode=DMA_CIRCULAR;                         //ʹ��ѭ��ģʽ 
    SPDIF_CSDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;                //������ȼ�
    SPDIF_CSDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;             //��ʹ��FIFO
    SPDIF_CSDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;                //�洢������ͻ������
    SPDIF_CSDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;             //����ͻ�����δ��� 
    HAL_DMA_DeInit(&SPDIF_CSDMA_Handler);                               //�������ǰ������
    HAL_DMA_Init(&SPDIF_CSDMA_Handler);	                                //��ʼ��DMA
    
    HAL_DMA_Start(&SPDIF_CSDMA_Handler,(u32)&SPDIFRX->CSR,(u32)buf0,num);//����DMA   
    __HAL_DMA_ENABLE_IT(&SPDIF_CSDMA_Handler,DMA_IT_TC);                //������������ж�
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn,3,0);                        //DMA�ж����ȼ�
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}


//SPDIFʱ������,����Ϊ158MHZ
void SPDIFCLK_Config(void)
{
    RCC_PeriphCLKInitTypeDef SPDIFCLK_Sture;
    
    SPDIFCLK_Sture.PeriphClockSelection=RCC_PERIPHCLK_SPDIFRX;      //SPDIF RXʱ�� 
    SPDIFCLK_Sture.PLLI2S.PLLI2SN=316;                              //����PLLI2SN
    SPDIFCLK_Sture.PLLI2S.PLLI2SP=0;                                //����PLLI2SP 
    HAL_RCCEx_PeriphCLKConfig(&SPDIFCLK_Sture);                     //����ʱ��
}

//����ͬ��״̬��ͬ������Ժ�������״̬
//����ֵ:0 δͬ��;1 ͬ��
u8 WaitSync_TORecv(void)
{
    u8 flag=0;
    u8 timeout=0;
    __HAL_SPDIFRX_SYNC(&SPDIFIN1_Handle);
    while(__HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_SYNCD)==0)//�ȴ�ͬ�����
    {
        timeout++;
        delay_ms(5);
        if (timeout>100) break;//��ʱ������
    }
    if(timeout>100) flag=0;//δͬ��
    else //ͬ�����
    {
        flag=1;
        __HAL_SPDIFRX_RCV(&SPDIFIN1_Handle);//ͬ����ɣ�������ս׶�  
    }
    return flag;
}

//��ȡSPDIF�յ�����Ƶ������
void SPDIF_GetRate(void)
{
    u16 spdif_w5;
    u32 spdif_rate; 
    
    spdif_w5=(SPDIFIN1_Handle.Instance->SR)>>16;
    spdif_rate=(spdif_dev.spdif_clk*5)/(spdif_w5&0X7FFF);
    spdif_rate>>=6;     //����64
    spdif_rate*=100;    //����100���õ����յ�ʵ�ʲ�����

    if((44100-1500<=spdif_rate)&&(spdif_rate<=44100+1500)) spdif_dev.spdifrate=44100; //44.1K�Ĳ�����
    else if((48000-1500<=spdif_rate)&&(spdif_rate<=48000+1500)) spdif_dev.spdifrate=48000; //48K�Ĳ�����
    else if((88200-1500<=spdif_rate)&&(spdif_rate<=88200+1500)) spdif_dev.spdifrate=88200; //88.2K�Ĳ�����
    else if((96000-1500<=spdif_rate)&&(spdif_rate<=96000+1500)) spdif_dev.spdifrate=96000; //96K�Ĳ�����
    else if((176400-1500<=spdif_rate)&&(spdif_rate<=176400+1500)) spdif_dev.spdifrate=176400; //176.4K�Ĳ�
    else if((192000-1500<=spdif_rate)&&(spdif_rate<=192000+1500)) spdif_dev.spdifrate=192000; //192K�Ĳ�
    else spdif_dev.spdifrate=0;
}

//SPDIF�ײ�IO��ʼ����ʱ��ʹ��
//�˺����ᱻHAL_SPDIF_Init()����
//hltdc:SPDIF���
void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef *hspdif)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_SPDIFRX_CLK_ENABLE();             //ʹ��SPDIF RXʱ��
    __HAL_RCC_GPIOG_CLK_ENABLE();               //ʹ��GPIOGʱ��

    //��ʼ��PG12��SPDIF IN����
    GPIO_Initure.Pin=GPIO_PIN_12;               //PG12��SPDIF IN����
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //����
    GPIO_Initure.Pull=GPIO_NOPULL;              //��������      
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;         //����
    GPIO_Initure.Alternate=GPIO_AF7_SPDIFRX;    //����ΪSPDIF RX
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);
}

//SPDIF�����жϷ�����
void SPDIF_RX_IRQHandler(void)
{
    //������ʱ��ͬ����֡�����ж�,�������ж�һ��Ҫ����
    if( __HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_FERR)||\
        __HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_SERR)||\
        __HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_TERR))
    {
        SPDIF_Play_Stop();//�������󣬹ر�SPDIF����
        __HAL_SPDIFRX_IDLE(&SPDIFIN1_Handle);   //��������ʱ��ͬ����֡�����ʱ��Ҫ��SPDIFRXENд0������ж�
        __HAL_SPDIFRX_RCV(&SPDIFIN1_Handle);    //������ж��Ժ���Ҫ���½�SPDIF����Ϊ����ģʽ
    }else if(__HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_OVR))     //�������
    {
        __HAL_SPDIFRX_CLEAR_IT(&SPDIFIN1_Handle, SPDIFRX_FLAG_OVR);         //�����������ж�
    }else if(__HAL_SPDIFRX_GET_FLAG(&SPDIFIN1_Handle,SPDIFRX_FLAG_PERR))    //��żУ�����
    {
        __HAL_SPDIFRX_CLEAR_IT(&SPDIFIN1_Handle, SPDIFRX_FLAG_PERR);        //�����żУ�����
    }
}

//������Ƶ�ӿ�
//AudioFreq:��Ƶ������
uint32_t SPDIF_AUDIO_Init(uint32_t AudioFreq)
{   
    SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);//����SAI,������,16λ����				
	SAIA_SampleRate_Set(AudioFreq);	//���ò�����  
	SAIA_TX_DMA_Init((u8*)&spdif_audiobuff[0],(u8*)&spdif_audiobuff[1],AUDIODATA_SIZE*2,1); //����TX DMA,16λ 
	SAI_Play_Start();							//����DMA  
	return 0; 
}

//DMA1������1(���ڴ�����Ƶ����)
void DMA1_Stream1_IRQHandler(void)
{
    if(__HAL_DMA_GET_FLAG(&SPDIF_DTDMA_Handler,DMA_FLAG_TCIF1_5)!=RESET)//DMA�������
    {
        __HAL_DMA_CLEAR_FLAG(&SPDIF_DTDMA_Handler,DMA_FLAG_TCIF1_5);//���DMA��������жϱ�־λ
    }  
}

//DMA1������6(���ڴ���ͨ��״̬���û���Ϣ)
void DMA1_Stream6_IRQHandler(void)
{
    if(__HAL_DMA_GET_FLAG(&SPDIF_CSDMA_Handler,DMA_FLAG_TCIF2_6)!=RESET)//DMA�������
    {
        __HAL_DMA_CLEAR_FLAG(&SPDIF_CSDMA_Handler,DMA_FLAG_TCIF2_6);//���DMA��������жϱ�־λ
    }  
}


//SPDIF��ʼ����
void SPDIF_Play_Start(void)
{ 
    spdif_dev.connsta=1;    //����Ѿ���SPDIF
    __HAL_DMA_ENABLE(&SPDIF_DTDMA_Handler); //����SPDIF DMA���� 
}
//SPDIF�ر�
void SPDIF_Play_Stop(void)
{
    spdif_dev.connsta=0;    //����Ѿ��ر�SPDIF
    __HAL_DMA_DISABLE(&SPDIF_DTDMA_Handler);//�ر�SPDIF DMA���� 	 
    //����������һ��Ҫ���㣬�����ڶϿ���ʱ����кܴ�����������
    memset((u8*)&spdif_audiobuff[0],0,AUDIODATA_SIZE*4);   
    memset((u8*)&spdif_audiobuff[1],0,AUDIODATA_SIZE*4);
} 


