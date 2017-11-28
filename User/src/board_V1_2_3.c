
#include "board_V1_2_3.h"
#include "os_timer.h"
#include "delay.h"
#include "RegLib.h"


/*****************************************
����: ����LCD ��Դ���
����: CTRL_OPEN: ��;  CTRL_CLOSE: �رմ˿���

****************************************/
void LCD_Ctrl_Set(E_SW_STATE sta)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin   = LCD_Power_Ctrl_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

	if(SW_OPEN == sta)
	{
	    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		STM32_GPIOInit(LCD_Power_Ctrl_PORT, &GPIO_InitStructure);
		
		LCD_Power_Ctrl_L();  // PMOS�� �͵�ƽ��ͨ
	}
	else // ����Ϊ��������, ��Դ����Ϊ�ߵ�ƽ, PMOS�ܹر�
	{
	   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	   STM32_GPIOInit(LCD_Power_Ctrl_PORT, &GPIO_InitStructure);
	}
}

void BAT_CE_Set(E_SW_STATE sta)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = BAT_CE_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

	if(SW_OPEN == sta)
	{
	   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
       STM32_GPIOInit(BAT_CE_PORT, &GPIO_InitStructure);
	}
	else 
	{
	    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        STM32_GPIOInit(BAT_CE_PORT, &GPIO_InitStructure);
		
		BAT_CE_L();  // ��ֹ��س��
	}
}

// �������ݿ�Ϊ��������
// �弶�ܽų�ʼ��
void Board_GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // ʱ��ʹ��
	STM32_RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	
    // XR1151 5V EN  
	GPIO_InitStructure.GPIO_Pin   = XR1151_EN_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    STM32_GPIOInit(XR1151_EN_PORT, &GPIO_InitStructure);
	
	//XR1151_EN_Close();
	XR1151_EN_Open();

	// PWR_SW, 3.3V��ѹ������
	GPIO_InitStructure.GPIO_Pin   = PWR_SW_Pin;
    STM32_GPIOInit(PWR_SW_PORT, &GPIO_InitStructure);
	
	//PWR_SW_Close();
	PWR_SW_Open();

	// LCD_Power, ��Ļ��Դ����
	LCD_Ctrl_Set(SW_OPEN);

    // ��س����ƹܽ�
	BAT_CE_Set(SW_OPEN);  // Ĭ�ϴ򿪳��
    
    // ��س����ܽ�
    GPIO_InitStructure.GPIO_Pin   = CHRG_Indicate_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	STM32_GPIOInit(CHRG_Indicate_PORT, &GPIO_InitStructure);
	
	// VIN_DETECT 
	GPIO_InitStructure.GPIO_Pin   = VIN_DETECT_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
	STM32_GPIOInit(VIN_DETECT_PORT, &GPIO_InitStructure);
}

