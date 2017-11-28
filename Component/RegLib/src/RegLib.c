
#include "RegLib.h"
#include <math.h>

//�������ã�����������ƫ�Ƶ�ַ
//����˵����NVIC_VectTab:��ַ��NVIC_Offset:ƫ����
//���Խ����NOT CHECK
//������ڣ�14.02.27
void STM32_NVICSetVectorTable(u32 NVIC_VectTab, u32 NVIC_Offset)
{
	//�������Ϸ���
	assert_param(IS_NVIC_VECTTAB(NVIC_VectTab));
	assert_param(IS_NVIC_OFFSET(NVIC_Offset));
	SCB->VTOR = NVIC_VectTab|(NVIC_Offset & (u32)0x1fffff80);	//����NVIC��������ƫ�ƼĴ���
}


//�������ã������жϷ���
//����˵����NVIC_Group:�жϷ���
//���Խ����CHECK OK
//������ڣ�14.02.22
//�������ڣ�15.10.19
void STM32_NVICPriorityGroupConfig(u8 NVIC_Group)													
{
	if(NVIC_Group<=4)		//�������
		SCB->AIRCR=((SCB->AIRCR)&0x0000f8ff)+0x05fa0000+((7-NVIC_Group)<<8);				
}

//�������ã������жϷ��顢���ȼ����ü��ж�ʹ��
//����˵����NVIC_ChannelΪ�ж�ͨ�����жϱ�ţ���NVIC_GroupΪ�жϷ��飬NVIC_PreemptionPriorityΪ��ռ���ȼ�
// NVIC_SubPriorityΪ��Ӧ���ȼ���
//���Խ����CHECK OK
//������ڣ�14.02.22
//�������ڣ�15.10.19
void STM32_NVICInit(u8 NVIC_Channel,u8 NVIC_Group,u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority)	
{
	u8 Priority = 0;
	STM32_NVICPriorityGroupConfig(NVIC_Group);																//�жϷ���	
	if((NVIC_PreemptionPriority<ldexp(1,NVIC_GROUP))&&(NVIC_SubPriority<ldexp(1,4-NVIC_GROUP)))		//�������
	{
		Priority=(NVIC_PreemptionPriority<<(4-NVIC_Group))+NVIC_SubPriority;		//�ж����ȼ�����
		NVIC->IP[NVIC_Channel/4]=(NVIC->IP[NVIC_Channel/4]&(~(0xf<<((NVIC_Channel%4)*8+4))))+(Priority<<((NVIC_Channel%4)*8+4));
	}
	NVIC->ISER[NVIC_Channel/32]|=1<<(NVIC_Channel%32);												//�ж�ʹ��
}

//�������ã���ʼ�����Ź���ģʽ
//����˵����GPIOx:Ҫ��ʼ������������GPIO�ڣ�GPIOInit_pst:���ڳ�ʼ�����ŵĽṹ�壨���stm32f10x_gpio.h��
//���Խ����NOT CHECK
//������ڣ�14.03.01
void STM32_GPIOInit(GPIO_TypeDef* GPIOx,GPIO_InitTypeDef* GPIOInit_pst)
{
	u8 pin=0,mode=0;								//pin���ڴ�0��15���������Ҳ������ţ�mode���ڵó����������ģʽ��ֵ
	for(pin=0;pin<16;pin++)
	{
		if(((GPIOInit_pst->GPIO_Pin>>pin)&0x1)==1)				//����pin����
		{
			if(((GPIOInit_pst->GPIO_Mode>>4)&0x1)==1)			//���
				mode=(GPIOInit_pst->GPIO_Mode&0xf)+GPIOInit_pst->GPIO_Speed;
			else												//����
			{
				mode=GPIOInit_pst->GPIO_Mode&0xf;
				if(((GPIOInit_pst->GPIO_Mode>>4)&0xf)==0x2)			//��������
					GPIOx->ODR&=~(1<<pin);
				else if(((GPIOInit_pst->GPIO_Mode>>4)&0xf)==0x4)	//��������
					GPIOx->ODR|=1<<pin;
			}
				
			if(pin<8)												
				GPIOx->CRL=(GPIOx->CRL&(~(0xf<<(pin*4))))+(mode<<(pin*4));			//�������Ź���ģʽ
			else
				GPIOx->CRH=(GPIOx->CRH&(~(0xf<<((pin-8)*4))))+(mode<<((pin-8)*4));	//�������Ź���ģʽ
		}
	}
}

//�������ã���IO�ڽ��а�λд����
//����˵����GPIOx:Ҫ����λ��������������GPIO�ڣ�GPIO_Pin:Ҫ���������ţ�֧�ֻ��������
//			BitVal:Ҫ����������Ϊ��ֵ��Bit_SET��Bit_RESET��
//���Խ����NOT CHECK
//������ڣ�14.03.01
void STM32_GPIOWriteBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, BitAction BitVal)
{
	if(Bit_SET==BitVal)			//pin�ø�
		GPIOx->BSRR=GPIO_Pin;
	else if(Bit_RESET==BitVal)	//pin�õ�
		GPIOx->BRR=GPIO_Pin;
}

//�������ã���IO�ڽ��а�λ������
//����˵����GPIOx:Ҫ����λ��������������GPIO�ڣ�GPIO_Pin:Ҫ����������
//����ֵ��Ҫ��ȡ���ŵĵ�ƽֵ��Bit_SET��Bit_RESET��
//���Խ����NOT CHECK
//������ڣ�14.03.01
BitAction STM32_GPIOReadBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	if(GPIOx->IDR&GPIO_Pin)		//�ߵ�ƽ
		return Bit_SET;
	else						//�͵�ƽ
		return Bit_RESET;
}

//�������ã���IO�ڽ�������д����
//����˵����GPIOx:Ҫ��������д������GPIO�ڣ�Data:Ҫд��IO�ڵ�ֵ
//���Խ����NOT CHECK
//������ڣ�14.03.01
void STM32_GPIOWriteData(GPIO_TypeDef* GPIOx,u16 Data)
{
	GPIOx->ODR=Data;
}

//�������ã���IO�ڽ������������
//����˵����GPIOx:Ҫ���������������GPIO��
//����ֵ��Ҫ��ȡ��GPIO�ڵ����ŵ�ƽֵ��16λ����
//���Խ����NOT CHECK
//������ڣ�14.03.01
u16 STM32_GPIOReadData(GPIO_TypeDef* GPIOx)
{
	return (GPIOx->IDR&0xffff);
}


//�������ã�SPIx�����ʷ�Ƶ����
//����˵����SPIxΪSPI�ڣ�SPIBaudFreqDivΪ�����ʷ�Ƶ���ͱ���
//���Խ����SPI2,CHECK OK
//������ڣ�15.02.05
//�����ʷ�Ƶ����:
//SPIBaudFreqDiv_2   2��Ƶ   (SPI 36M@sys 72M)
//SPIBaudFreqDiv_8   8��Ƶ   (SPI 9M@sys 72M)
//SPIBaudFreqDiv_16  16��Ƶ  (SPI 4.5M@sys 72M)
//SPIBaudFreqDiv_256 256��Ƶ (SPI 281.25K@sys 72M)
void STM32_SPIxSetBaudFreqDiv(SPI_TypeDef *SPIx,SPIBaudFreqDiv_TypeDef SPIBaudFreqDiv) 	//����SPI�����ʷ�Ƶ  
{
	SPIx->CR1&=0XFFC7;
	SPIx->CR1|=SPIBaudFreqDiv<<3;	
	SPIx->CR1|=1<<6; //SPI�豸ʹ��	  
} 
//�������ã�SPIx ��дһ���ֽ�
//����˵����SPIxΪSPI�ڣ�TxDataΪҪд����ֽڡ��������ض�ȡ�����ֽ�
//���Խ����SPI2,CHECK OK
//������ڣ�15.02.05
u8 STM32_SPIxReadWriteByte(SPI_TypeDef *SPIx,u8 TxData)
{		
	u8 retry=0;				 
	while((SPIx->SR&1<<1)==0)	//�ȴ���������	
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPIx->DR=TxData;	 	  		//����һ��byte 
	retry=0;
	while((SPIx->SR&1<<0)==0) //�ȴ�������һ��byte  
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return SPIx->DR;          //�����յ�������				    
}

void STM32_SPI_I2S_DeInit(SPI_TypeDef * SPIx)
{
    switch((uint32_t)SPIx)
    {
        case  (uint32_t)SPI1:
		{
			SPI1_I2S_DEINIT();
		}break;
		case  (uint32_t)SPI2:
		{
			SPI2_I2S_DEINIT();
		}break;
		case  (uint32_t)SPI3:
		{
			SPI3_I2C_DEINIT();
		}break;
	}
}


#define READ_OLD_ADC_SMPR1(ADCx, ADC_Channel) \
	(ADCx->SMPR1 & (~(ADC_SMPR1_SMP10 << (3 * (ADC_Channel - 10)))))

#define READ_OLD_ADC_SMPR2(ADCx, ADC_Channel) \
	(ADCx->SMPR2 & (~(ADC_SMPR2_SMP0 << (3 * ADC_Channel))))

#define ADC_SMPR_NewSampleTime(ADC_SampleTime, ADC_Channel, index) \
	((uint32_t)ADC_SampleTime << (3 * (ADC_Channel - index)))

#define READ_OLD_ADC_SQR3(ADCx, Rank) \
	(ADCx->SQR3 & (~(ADC_SQR3_SQ1 << (5 * (Rank - 1)))))

#define READ_OLD_ADC_SQR2(ADCx, Rank) \
	(ADCx->SQR2 & (~(ADC_SQR2_SQ7 << (5 * (Rank - 7)))))

#define READ_OLD_ADC_SQR1(ADCx, Rank) \
	(ADCx->SQR1 & (~(ADC_SQR1_SQ13 << (5 * (Rank - 13)))))

#define ADC_SQR_NewRank(ADC_Channel, Rank, index) \
	((uint32_t)ADC_Channel << (5 * (Rank - index)))



void STM32_ADC_RegularChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t Rank, uint8_t ADC_SampleTime)
{
  /* if ADC_Channel_10 ... ADC_Channel_17 is selected */
  if (ADC_Channel > ADC_Channel_9)
  {

	ADCx->SMPR1 =  (READ_OLD_ADC_SMPR1(ADCx, ADC_Channel)) | ADC_SMPR_NewSampleTime(ADC_SampleTime, ADC_Channel, 10);
  }
  else /* ADC_Channel include in ADC_Channel_[0..9] */
  {	
	ADCx->SMPR2 =  (READ_OLD_ADC_SMPR2(ADCx, ADC_Channel)) | ADC_SMPR_NewSampleTime(ADC_SampleTime, ADC_Channel, 0);
  }


  	
  /* For Rank 1 to 6 */
  if (Rank < 7)
  {
	ADCx->SQR3 = READ_OLD_ADC_SQR3(ADCx, Rank) |  ADC_SQR_NewRank(ADC_Channel, Rank, 1);
  }
  /* For Rank 7 to 12 */
  else if (Rank < 13)
  {
    ADCx->SQR2 = READ_OLD_ADC_SQR2(ADCx, Rank) | ADC_SQR_NewRank(ADC_Channel, Rank, 7);
	
  }
  /* For Rank 13 to 16 */
  else
  {
    ADCx->SQR1 = READ_OLD_ADC_SQR1(ADCx, Rank) | ADC_SQR_NewRank(ADC_Channel, Rank, 13);
  }
}

/**********************
����: ADC ��ʼ��, ��Ӧ�ڿ⺯����ADC_Init()
����:  uint32_t ADC_Mode:   ֵΪ ADC_Mode_Independent ... ADC_Mode_AlterTrig
              uint8_t   ADC_ScanConvMode: DISABLE, or ENABLE
              uint8_t   ContinuousConvMode: ֵΪ DISABLE, or ENABLE
              uint32_t ExternalTrigConv:   ֵΪADC_ExternalTrigConv_None ... ADC_ExternalTrigConv_T1_CC1 ��
              uint32_t DataAlign: ADC_DataAlign_Right, ADC_DataAlign_Left
              uint8_t  NbrOfChannel: ��Ҫת����ͨ����, 1 - 16
*************************/
void STM32_ADC_Init(ADC_TypeDef* ADCx, 
                          uint32_t ADC_Mode, 
                          uint8_t  ScanConvMode, 
                          uint8_t  ContinuousConvMode, 
                          uint32_t ExternalTrigConv,
                          uint32_t DataAlign,
                          uint8_t  NbrOfChannel)
{
    ADCx->CR1 = (ADCx->CR1 & (~ADC_CR1_DUALMOD)) | ADC_Mode;   //ADC����ģʽ
	ADCx->CR1 = (ADCx->CR1 & (~ADC_CR1_SCAN)) | ((uint8_t)ScanConvMode << 8);    
	
	ADCx->CR2 = (ADCx->CR2 & (~ADC_CR2_CONT)) | ((uint8_t)ContinuousConvMode << 1);
	ADCx->CR2 = (ADCx->CR2 & (~ADC_ExternalTrigConv_None)) | ExternalTrigConv;
	ADCx->CR2 = (ADCx->CR2 & (~ADC_CR2_ALIGN)) | DataAlign;
	ADCx->SQR1  &=~ ADC_SQR1_L;								      //�������Ҫת����ͨ����
	ADCx->SQR1  |= (NbrOfChannel -1) << 20;  
}

