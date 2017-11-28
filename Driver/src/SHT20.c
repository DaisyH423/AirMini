
#include "SHT20.h"
#include "IIC_Drv.h"
#include "os_timer.h"
//#include "ucos_ii.h"
//#include "nos_api.h"
#include "delay.h"
#include "PM25Sensor.h"
#include "SDRR.h"


#define SHT_DEBUG_EN   1  // ����ʹ��: 1; ��ֹ: 0

#ifdef USING_NOS_TIMER 
#define SHT_USING_NOS_TIMER   1   // SHT20 �������ʱ�� ʹ�� ucos �Ķ�ʱ��
#endif

#define SHT20_I2C_ADDR  0x80    // SHT20 I2C�豸��ַ



#define SHT_T_Hold_CMD        0xE3   // �¶Ȳ���, ��������
#define SHT_RH_Hold_CMD       0xE5   // ʪ�Ȳ���, ��������

#define SHT_T_NotHold_CMD     0xF3   // �¶Ȳ���, �Ǳ�������
#define SHT_RH_NotHold_CMD    0xF5   // ʪ�Ȳ���, �Ǳ�������

#define SHT_WriteReg_CMD      0xE6   // д�û��Ĵ���
#define SHT_ReadReg_CMD       0xE7   // ���û��Ĵ���

#define SHT_SoftReset_CMD     0xFE   // ��λ

// ���ݼĴ���λ����
#define SHT_DATA_REG_STA_BitMask       0x02  // �����ֽڵĵͰ�λ��״̬λ(bit 1)����

#define SHT_DATA_REG_STA_Bit_IsTemp    0x00  // bit1: 0
#define SHT_DATA_REG_STA_Bit_IsHumi    0x02  // bit1: 1

// �û��Ĵ���λ����

//��ʪ�Ȳ����������õ�����λ
#define SHT_USER_REG_RH_T_BitMask       0x81   // �����������룺 bit7, bit0

#define SHT_USER_REG_RH12_T14   0x00   // ʪ�Ȳ��� 12 bit, �¶Ȳ��� 14bit
#define SHT_USER_REG_RH8_T12    0x01   // ʪ�Ȳ���8bit, �¶Ȳ��� 12bit
#define SHT_USER_REG_RH10_T13   0x80   // ʪ�Ȳ��� 10bit, �¶Ȳ��� 13bit
#define SHT_USER_REG_RH11_T11   0x81   // ʪ�Ȳ��� 11bit, �¶Ȳ��� 11bit 

// ���״̬����λ, bit6
#define SHT_USER_REG_EndOfBat_BitMask         0x40   

#define SHT_USER_REG_EndOfBat_MoreThan2P25V    0x00   // ��ص�ѹ���� 2.25 V
#define SHT_USER_REG_EndOfBat_LessThan2P25V    0x40   // ��ص�ѹС�� 2.25 V

// Ƭ�ϼ�����״̬����λ
#define SHT_USER_REG_Heat_BitMask   0x04             

#define SHT_USER_REG_Heat_OnLine    0x04             // �ڼ���
#define SHT_USER_REG_Heat_OffLine   0x00             // �޼���



#if SHT_DEBUG_EN
static uint32_t  read_sht20_err_count = 0;
#endif

// SHT 20 ��������
// SHT 20 �ļ������������ SHT20 �Ƿ���
//����: uint16_t * temp: ֮ǰ��ƽ���¶�ֵ, Ϊʵ���¶�ֵ��10��, �� 350 = 35.0 'C
//      uint16_t * humi: ֮ǰ��ƽ��ʪ��ֵ, �� 35 = 35 % RH
//static uint16_t last_temp;
//static uint16_t last_humi;

//static 
//os_timer_t tTimerHeating;

static os_timer_t tTimerSHT20;







#define TEMP_INDEX 0
#define HUMI_INDEX 1

#define READ_TIMES    4   // ��ȡ����
static uint16_t sht_buf[2][READ_TIMES];  
static uint8_t temp_read_count = 0;
static uint8_t humi_read_count = 0;
static uint8_t sht20_read_count = 0; 
static uint8_t next_read_which = 0; // ��һ�ζ��¶� 0, ����ʪ��: 1

// ��ʽ: T = - 46.85 + (175.72 * T) / (2 ^ 16)
// ����ֵ: -1 ˵���¶�ֵ < 0, 1: �¶�ֵ > 0
int8_t SHT20_CalculateTemp(uint16_t adc, uint16_t * out_temp)
{
   double real_temp = 0.0;
   int8_t sign = 0;  

   adc &= 0xFFFC;  // ���2λ�Ǳ�־λ, ����
   real_temp = ((175.72 * adc) / 65536.0) - 46.85;
   if(real_temp < 0)
   {
      sign = -1;
	  real_temp = -real_temp;  
   }
   *out_temp = (uint16_t)(real_temp * 100);
   return sign;
}

int8_t SHT20_CalculateHumi(uint16_t adc, uint16_t *out_humi)
{
    int8_t sign = 0;
	double real_humi = 0.0;

	adc &= 0xFFFC;
	real_humi = (125.0 * adc) / 65536.0 - 6;
	if(real_humi < 0)
	{
	    sign = -1;
		real_humi = -real_humi;
	}
	*out_humi = (uint16_t)(real_humi * 100);
	return sign;
}

extern T_TempHumi tTempHumi;

static 
void TimerSHT20Sensor_CallBack(void * arg)
{
	SYS_RESULT res;
    uint8_t buf[3] = {0, 0, 0};
	uint8_t val_type = 0;  // ֵ����: �¶�: 0; ʪ��: 1
	uint8_t reg_val = 0;
	int8_t  sign = 0;
	uint16_t temp_humi = 0;

	res = IIC_ReadNByteDirectly(SHT20_I2C_ADDR, buf, 3);
    if(res)
    {
	   os_timer_arm(&tTimerSHT20, 200, 0);   // 2 S 

       #if SHT_DEBUG_EN
	   read_sht20_err_count++;
	   os_printf("read SHT20 failed, tick = %ld, err_count = %ld\r\n", os_get_tick(), read_sht20_err_count);
	   #endif
	   
       return;    // ��ȡʧ��
    }
	else
	{
	    val_type = ((buf[1] & SHT_DATA_REG_STA_BitMask) ? HUMI_INDEX : TEMP_INDEX);

		#if SHT_DEBUG_EN
		os_printf("read SHT20 %s success, tick = %ld, err_count = %ld\r\n", (val_type ? "humi" : "temp"), 
			        os_get_tick(), read_sht20_err_count);
        os_printf("buf[0] = 0x%x, buf[1] = 0x%x, buf[2] = 0x%x\r\n", buf[0], buf[1], buf[2]);
		#endif
		
        if(val_type)  // 1: Ϊʪ��ֵ
        {
            sht_buf[val_type][humi_read_count] = ((uint16_t)buf[0] << 8) + buf[1];   // ����ʪ��ֵ
			sign = SHT20_CalculateHumi(sht_buf[val_type][humi_read_count], &temp_humi);

			#if SHT_DEBUG_EN
            os_printf("humi = 0x%x, %c%d.%02d%% RH \r\n", sht_buf[val_type][humi_read_count], 
				        (sign == 0 ? ' ' : '-'), temp_humi / 100, temp_humi);
			os_printf("humi count = %d\r\n", humi_read_count);
			#endif
			
            tTempHumi.humi = temp_humi;

			os_printf("old humi = %d, humi_p = %d\r\n", temp_humi, tTempHumi.humi);
			
			temp_humi     = tTempHumi.humi % 10000 / 100;
			SDRR_SaveSensorPoint(SENSOR_HUMI, &temp_humi);  // ˢ��ʪ�����ݵ�

            if(++humi_read_count >= READ_TIMES)humi_read_count = 0;
			if(next_read_which == HUMI_INDEX)next_read_which = TEMP_INDEX;
        }
		else  // 0: Ϊ�¶�ֵ
		{
		    sht_buf[val_type][temp_read_count] = ((uint16_t)buf[0] << 8) + buf[1]; // �����¶�ֵ 
		    sign = SHT20_CalculateTemp(sht_buf[val_type][temp_read_count], &temp_humi);

            #if SHT_DEBUG_EN
            os_printf("temp = 0x%x, %c%d.%02d 'C \r\n", sht_buf[val_type][temp_read_count], 
				       (sign == 0 ? ' ' : '-'), temp_humi / 100, temp_humi % 100);
			os_printf("temp count = %d\r\n", temp_read_count);
			#endif

			tTempHumi.temp = temp_humi;
            if(sign == -1)
			{
				tTempHumi.temp += 10000;  // ��5λ Ϊ 1��ʾΪ��ֵ  
            }
			
			os_printf("old_temp = %d, temp_p: %d\r\n", temp_humi, tTempHumi.temp);
			temp_humi = tTempHumi.temp;
			SDRR_SaveSensorPoint(SENSOR_TEMP, &temp_humi);  // ˢ���¶����ݵ�

		  
            if(++temp_read_count >= READ_TIMES)temp_read_count = 0;
			if(next_read_which == TEMP_INDEX)next_read_which = HUMI_INDEX;
		}

        TempHumi_SetSensorExisted(E_TRUE);
		sht20_read_count++;
		if(sht20_read_count >= (READ_TIMES * 2))
		{
		   sht20_read_count = 0;
		}

		
		if(next_read_which == TEMP_INDEX)  // ��һ����Ҫ���¶�
		{
		   	IIC_WriteNByte(SHT20_I2C_ADDR, SHT_T_NotHold_CMD, &reg_val, 0);     // �����¶Ȳ���
		}
		else
		{
		   IIC_WriteNByte(SHT20_I2C_ADDR, SHT_RH_NotHold_CMD, &reg_val, 0);     // ����ʪ�Ȳ���
		}
	    os_timer_arm(&tTimerSHT20, 135, 0);  // 1.35 sec ����һ��, ����̫Ƶ��, ����оƬ���������
	}
}


/********************************
����: SHT20 ��ʪ�ȼĴ�������
����: uint8_t precision_mask: ��ʪ�Ⱦ�������, ֵΪ:
                         SHT_USER_REG_RH12_T14
                         SHT_USER_REG_RH8_T12
                         SHT_USER_REG_RH10_T13
                         SHT_USER_REG_RH11_T11
             uint8_t is_heated: �Ƿ����

*********************************/
void SHT20_RegConfig(uint8_t precision_mask, uint8_t is_heated)
{
	uint8_t reg_val = 0;
    uint8_t res = 0;
    uint8_t old_config = 0; // ԭ����

    if(   precision_mask != SHT_USER_REG_RH12_T14
	   && precision_mask != SHT_USER_REG_RH8_T12
	   && precision_mask != SHT_USER_REG_RH10_T13
	   && precision_mask != SHT_USER_REG_RH11_T11)
    {
       INSERT_ERROR_INFO(1);
    }
	
    res = IIC_ReadNByteExt(SHT20_I2C_ADDR, SHT_ReadReg_CMD, &reg_val, 1, 1);
    if(res){ INSERT_ERROR_INFO(0); return; }
	
	#if SHT_DEBUG_EN
    os_printf("first read user_reg = 0x%x, line = %d\n", reg_val, __LINE__);
    #endif
	
	old_config = reg_val & SHT_USER_REG_RH_T_BitMask;
	if(old_config != precision_mask)
	{
	    #if SHT_DEBUG_EN
	    os_printf("sht precision change: old = 0x%x, new = 0x%x\r\n", old_config, precision_mask);
		#endif
		
        reg_val &= ~SHT_USER_REG_RH_T_BitMask;  // ����Ϊ RH 12bit, T 14 bit
	    if(precision_mask)
	    {
	       reg_val |= precision_mask;
	    }
	}
    
	old_config  = reg_val & SHT_USER_REG_Heat_OnLine;
	if(old_config != is_heated)
	{
	   #if SHT_DEBUG_EN
	   os_printf("sht heat config change: old = %d, new = %d\r\n", old_config, is_heated);
	   #endif
	   
	   if(is_heated)
           reg_val |= SHT_USER_REG_Heat_OnLine;   // ����Ƭ�ϼ�����
       else
	       reg_val &= ~SHT_USER_REG_Heat_OnLine;  // ֹͣƬ�ϼ�����
	}
	   
    res = IIC_WriteNByte(SHT20_I2C_ADDR, SHT_WriteReg_CMD, &reg_val, 1);
    if(res){ INSERT_ERROR_INFO(0); return; }
    
    reg_val = 0;
    res = IIC_ReadNByteExt(SHT20_I2C_ADDR, SHT_ReadReg_CMD, &reg_val, 1, 1);
    if(res){ INSERT_ERROR_INFO(0); return; }
	
	#if SHT_DEBUG_EN
    os_printf("reread user_reg = 0x%x, line = %d\n", reg_val, __LINE__);
    #endif
	
    if((reg_val & SHT_USER_REG_RH_T_BitMask) == precision_mask)  
    {
 	  IIC_WriteNByte(SHT20_I2C_ADDR, SHT_T_NotHold_CMD, &reg_val, 0);  // �����¶Ȳ���
    }
	#if SHT_DEBUG_EN
    else
    {
 	  os_printf("SHT RegConfig precision mask Failed, user_reg = 0x%x, tick = %ld, %s, %d\r\n", 
 				  reg_val, os_get_tick(), __FILE__, __LINE__);
    }
	
	if(is_heated)
	{
	    if((reg_val & SHT_USER_REG_Heat_BitMask) == 0)  // ��Ҫ����, ���ض�ֵȴΪ������, �Ĵ������ô���
	    {
	       os_printf("SHT RegConfig Heat Failed, user_reg = 0x%x, tick = %ld, %s, %d\r\n", 
 				  reg_val, os_get_tick(), __FILE__, __LINE__);
	    }
	}
	else  // ����Ҫ����
	{
	    if((reg_val & SHT_USER_REG_Heat_BitMask))  
	    {
	       os_printf("SHT RegConfig Cancel Heat Failed, user_reg = 0x%x, tick = %ld, %s, %d\r\n", 
 				  reg_val, os_get_tick(), __FILE__, __LINE__);
	    }
	}
	#endif
}

void SHT20_Init(void)
{
    SHT20_RegConfig(SHT_USER_REG_RH12_T14, 0);
    os_timer_setfn(&tTimerSHT20, TimerSHT20Sensor_CallBack, NULL);
	os_timer_arm(&tTimerSHT20,   500, 0);
}

