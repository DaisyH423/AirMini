
#include "ADC_Drv.h"
#include "board_version.h"


#include <stdio.h>
#include <stdarg.h>
#include "delay.h"


#if   DEBUG_ADC_EN
#define ADC_DEBUG(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#define ADC_DEBUG(...)
#endif


// �����ѹֵ
// ��ʵ��ֵ��1000��, ������3λС��תΪ����
#define  GetVoltValue(adc_val) \
	((uint16_t)((double)adc_val * 3300 / 4096))  

// �����ص�ѹ
// ����: uint16_t volt: ADC�����ĵ�ѹ, ��λ: mV
// ����ֵ: uint16_t ��ص�ѹ: ��λ: mV
#define  GetBatVolt(volt)   ((uint16_t)(( 635.0 / 470.0) * (volt)))




// �����ADC ģʽ��ʼ��
void ADCDrv_NormalModeInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	STM32_RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��ADC1ͨ��ʱ��
	STM32_RCC_ADCCLKConfig(RCC_PCLK2_Div4);   //����ADC��Ƶ����4, 48 M /4 = 12,ADC���ʱ�䲻�ܳ���14M

	//PA0 ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������

    STM32_GPIOInit(GPIOA, &GPIO_InitStructure);	
	STM32_ADC_DeInit_ADC1();
	
	STM32_ADC_Init(ADC1, 
		             ADC_Mode_Independent,        //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
		             DISABLE,   	                //ģ��ת�������ڵ�ͨ��ģʽ
		             DISABLE,	                    //ģ��ת�������ڵ���ת��ģʽ
		             ADC_ExternalTrigConv_None, 	//ת��������������ⲿ��������
		             ADC_DataAlign_Right,        //ADC�����Ҷ���
		             1);                            //˳����й���ת����ADCͨ������Ŀ

    /* ADC1 regular channel 0 configuration */ 
    // STM32_ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

	STM32_ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	delay_us(30);
	
	STM32_ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(STM32_ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	STM32_ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(STM32_ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����

	//STM32_NVICInit(ADC1_2_IRQn,NVIC_GROUP,1,1);	//����ADC1�ж����ȼ�
}



//���ADCֵ
//ch:ͨ��ֵ 0~3
uint16_t ADCDrv_GetVal(void)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	STM32_ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    

    STM32_ADC_SoftwareStartConvCmd(ADC1, ENABLE);         // ʹ��ָ����ADC1�����ת����������	
    while(! (READ_REG_32_BIT(ADC1->SR, ADC_FLAG_EOC)));  // �ȴ�ת������

	return (uint16_t)ADC1->DR;	//�������һ��ADC1�������ת�����
}

#include "os_timer.h"
#include "ExtiDrv.h"

static os_timer_t tADCTimer;
static os_timer_t tBatChargeCloseTimer;  
#define MAX_TIMES   40    // 40 ��ƽ��
static uint32_t total_sum = 0;
static uint16_t get_adc_count = 0;


// ÿ�λ�ȡ��ص�ѹ�����������ɴ˶�ʱ��������
#if DEBUG_ADC_EN
static uint8_t is_first_sequ = 0;
#endif

static void BatChargeCloseTimer_CallBack(void * arg)
{
   #if DEBUG_ADC_EN
   if(VIN_DETECT_Read())
    {
       //ADC_DEBUG("USB Power is pluged\r\n");
    }
   is_first_sequ = E_TRUE;
   #endif

   BAT_CE_Set(SW_CLOSE);  // �ȹرյ�س��, �ټ���ص�ѹ, ���������ĵ�ص�ѹ��׼ȷ
   STM32_ADC_SoftwareStartConvCmd(ADC1, ENABLE);   // ������� AD ת��
   os_timer_arm(&tADCTimer, 5, 0);  
}

static void ADCTimer_CallBack(void * arg)
{
	uint32_t tick = 2;

	//ADC_DEBUG("c=%d, %ld\n", get_adc_count, os_get_tick());
	if(READ_REG_32_BIT(ADC1->SR, ADC_SR_EOC)) //ת������  
	{
	    #if DEBUG_ADC_EN
	    uint16_t cur_volt = 0;  //��ѹ

		if(is_first_sequ)
		{
		    is_first_sequ = E_FALSE;
			
			cur_volt = (uint16_t)ADC1->DR;
			cur_volt = 3300 * cur_volt / 4096;
			cur_volt = GetBatVolt(cur_volt);
			ADC_DEBUG("1th=%d.%03d V\n", cur_volt / 1000, cur_volt % 1000);
		}
		#endif
		
	    total_sum += (uint16_t)ADC1->DR;
		get_adc_count++;
		if(get_adc_count >= MAX_TIMES)
		{
		   uint16_t aver_volt, aver_adc, bat_volt;
		   
		   aver_adc = total_sum / MAX_TIMES;

           aver_volt = GetVoltValue(aver_adc); // ƽ����ѹֵ
		   bat_volt  = GetBatVolt(aver_volt);
		   
		   ADC_DEBUG("adc=%d, v=%d.%03d V, bat=%d.%03d V, %ld\n", aver_adc, aver_volt / 1000, aver_volt % 1000, 
		   	           bat_volt / 1000, bat_volt % 1000, os_get_tick());
		   
		   total_sum = 0;
		   get_adc_count = 0;
		   
		   if(arg)
		   {
		       ((void (*)(uint16_t))(arg))(bat_volt);
		   }
		   BAT_CE_Set(SW_OPEN);  // �� TP4056 ���г��
		   os_timer_arm(&tBatChargeCloseTimer, SEC(30), 0);
		   return;
		}
	}
	else
	{
	  ADC_DEBUG("adc not end\n");
	  tick = 50;
	}
    STM32_ADC_SoftwareStartConvCmd(ADC1, ENABLE);   // ������� AD ת��
    os_timer_arm(&tADCTimer, tick, 0);  
}

// ADC ������ʼ��
void ADCDrv_Start(void)
{
    ADCDrv_NormalModeInit();
    STM32_ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );
	
	/* Start ADC1 Software Conversion */ 
    //STM32_ADC_SoftwareStartConvCmd(ADC1, ENABLE);  // �������ADC

	//os_timer_setfn(&tADCTimer, ADCTimer_CallBack, NULL);
	//os_timer_arm(&tADCTimer, 100, 0);
}	

/*****************************
����: ���� ADC �������ʣ�����
             ����������ִ��(*end_exe_func)(uint16_t ) �ص�����
����: end_exe_func: ������������Ҫִ�еĲ���
********************************/
void ADCDrv_StartBatteryMeasure(void  (*end_exe_func)(uint16_t arg))
{
    
    if(VIN_DETECT_Read())  
    {
       //ADC_DEBUG("USB Power is pluged\r\n");
       battery_is_charging |= USB_PLUGED_MASK;
    }
	#if DEBUG_ADC_EN
	is_first_sequ = E_TRUE;
    #endif
	
    ADCDrv_Start();

    os_timer_setfn(&tBatChargeCloseTimer, BatChargeCloseTimer_CallBack, NULL);
	os_timer_setfn(&tADCTimer, ADCTimer_CallBack, end_exe_func);

	BAT_CE_Set(SW_CLOSE);
	STM32_ADC_SoftwareStartConvCmd(ADC1, ENABLE);  // ������� AD ת��
    os_timer_arm(&tADCTimer, 5, 0);
}


