
#include "ExtiDrv.h"
#include "os_global.h"
#include "os_timer.h"
#include "board_version.h"
#include "BatteryLevel.h"
#include "PowerCtrl.h"
#include "FatFs_Demo.h"
#include "Application.h"

#if EXTI_DEBUG_EN 
#define EXT_DEBUG(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#define EXT_DEBUG(...)
#endif

uint8_t battery_is_charging = 0; // ����Ƿ����ڳ��, ��4λ��ʾ: USB�Ƿ����; �� 4λ��ʾ: ����Ƿ��ڳ��

static os_timer_t tExtiCheckTimer;   // �ⲿ�ж���ʱ��ⶨʱ��
static void ExtiCheckTimer_CallBack(void * arg)
{
    if(VIN_DETECT_Read())		// ��ǰΪ�ߵ�ƽ, ˵����������
	{
		EXT_DEBUG("usb pluged\n");
		battery_is_charging |= USB_PLUGED_MASK;
		if(is_5v_power_close)
		{
		        JumpToBootloader();
				is_5v_power_close = E_FALSE;
		}
		else 
			FILE_SearchUpdateBinFile(); // ����bin�ļ�, ��������������
	}
	else  // ��ǰ�ǵ͵�ƽ, ˵�����½���
	{
	   EXT_DEBUG("usb unpluged\n");
	   battery_is_charging = 0;
	}
}


#define STM32_EXTI_ClearITPendingBit(EXTI_Line) \
	(EXTI->PR = EXTI_Line)

void ExtiDrv_Init(void)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    GPIO_InitTypeDef   GPIO_InitStructure;


	 os_timer_setfn(&tExtiCheckTimer, ExtiCheckTimer_CallBack, NULL);
	 
	 // ʹ�� IO ʱ��
	 VIN_DETECT_RCC_APBPeriphClockCmdEnable();
	 
	 GPIO_InitStructure.GPIO_Pin  = VIN_DETECT_Pin;
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	 STM32_GPIOInit(VIN_DETECT_PORT, &GPIO_InitStructure);
	 
	 /* Enable AFIO clock */
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
   
	 /* Connect EXTIn �ⲿ�ж��ߵ� IO �ܽ�  */
	 GPIO_EXTILineConfig(VIN_DETECT_PortSource, VIN_DETECT_PinSource);
     STM32_EXTI_ClearITPendingBit(EXTI_Line_VinDetect);  // ����жϱ�־λ

	 
	 /* Configure EXTI0 line */
	 EXTI_InitStructure.EXTI_Line = EXTI_Line_VinDetect;
	 EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
	 EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	 EXTI_Init(&EXTI_InitStructure);
   
	 /* Enable and set EXTI0 Interrupt to the lowest priority */
	 STM32_NVICInit(EXTI_VinDetect_IRQn,3, 6, 1);	 // ��3�����ȼ�, 3λ��ռ���ȼ�, 1λ��Ӧ���ȼ�
}

//�ⲿ�ж�7������� 
void EXTI9_5_IRQHandler(void)
{
    CLEAR_REG_32_BIT(EXTI->IMR, EXTI_Line_VinDetect);  // ��ֹ�ⲿ�ж�
    
    if(READ_REG_32_BIT(EXTI->PR, EXTI_Line_VinDetect))  // PA7 �ܽŵ��ж�
    {

		if(VIN_DETECT_Read())		// ��ǰΪ�ߵ�ƽ, ˵����������
		{
		    EXT_DEBUG("VD Rise\r\n");
		}
		else  // ��ǰ�ǵ͵�ƽ, ˵�����½���
		{
		    EXT_DEBUG("VD Fall\r\n");
		}
		os_timer_arm(&tExtiCheckTimer, 1, 0);  // ��ʱ 10 ms ���

		#if 0
		EXTI_ClearITPendingBit(EXTI_Line_VinDetect); //���LINE0�ϵ��жϱ�־λ  
		#else
        EXTI->PR = EXTI_Line_VinDetect; // ������λд 1 ���жϱ�־
		#endif
    }
	SET_REG_32_BIT(EXTI->IMR, EXTI_Line_VinDetect);  // ʹ���ⲿ�ж�
}

