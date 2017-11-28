
#include "board_version.h"
#include "IIC_Drv.h"

#include "delay.h"
//#include "nos_api.h"

#if OS_SEM_EN
//static nos_sem_t *semIIC = NULL;
#endif

//��ʼ��IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	
	IIC_APBPeriphClockCmdEnable();  
	   
	GPIO_InitStructure.GPIO_Pin   = IIC_SDA_Pin | IIC_SCL_Pin;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	STM32_GPIOInit(IIC_BUS_PORT, &GPIO_InitStructure);

    IIC_SCL_H();
    IIC_SDA_H();

	//if(nos_sem_init(&semIIC, "semIIC", 1, NOS_IPC_FLAG_FIFO) != NOS_OK) // ��ʼ���ź���
	//{
	//   os_printf("create semIIC failed, %s, %d\r\n", __FILE__, __LINE__);
	//}
}

// �������ݿ�Ϊ��������
static void SDA_DIR(E_IO_DIR dir)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = IIC_SDA_Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	if(dir == INPUT)GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    else GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	STM32_GPIOInit(IIC_SDA_PORT, &GPIO_InitStructure);
}


//����IIC��ʼ�ź�
SYS_RESULT i2c_start(void)
{
	SDA_DIR(OUTPUT);  	 
	IIC_SDA_H();
	IIC_SCL_H();
	delay_us(1);  // 4
	if(! IIC_SDA_READ())return SYS_FAILED;	//SDA��Ϊ�͵�ƽ������æ,�˳�
	
 	IIC_SDA_L();  // START:when CLK is high,DATA change form high to low 
	delay_us(1);  // 4

	IIC_SCL_L();  //ǯסI2C���ߣ�׼�����ͻ�������� 
    
    return SYS_SUCCESS;
}	  


//����IICֹͣ�ź�
void i2c_stop(void)
{
	SDA_DIR(OUTPUT);   // sda�����
	IIC_SCL_L();
	IIC_SDA_L();    // STOP:when CLK is high DATA change form low to high
 	delay_us(1);    //  4 
	IIC_SCL_H();
	IIC_SDA_H();    // ����I2C���߽����ź�
	delay_us(1);	 //  4						   	
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
SYS_RESULT i2c_wait_ack(void)
{
	uint16_t errCount = 0;     

	IIC_SCL_L();
	SDA_DIR(INPUT);  //SDA����Ϊ����  
	IIC_SDA_H(); delay_us(1);	   // 5 
	IIC_SCL_H(); delay_us(1);	   // 5
	while(IIC_SDA_READ())
	{
		errCount++;
		if(errCount > 250)  // 250
		{
			i2c_stop();
			return SYS_FAILED;
		}
	}
	IIC_SCL_L();   // ʱ�����0 	   
	return SYS_SUCCESS;  
}

//����ACKӦ��
void i2c_ack(void)
{
	IIC_SCL_L();
	SDA_DIR(OUTPUT);
	IIC_SDA_L();
	delay_us(1);  // 2
	IIC_SCL_H();
	delay_us(1);  // 2
	IIC_SCL_L();
}

//������ACKӦ��		    
void i2c_nack(void)
{
	IIC_SCL_L();
	SDA_DIR(OUTPUT);
	IIC_SDA_H();
	delay_us(1);  // 2
	IIC_SCL_H();
	delay_us(1);  // 2
	IIC_SCL_L();
}			

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void i2c_send_byte(uint8_t txd)
{                        
    uint8_t i;    	   
	
	SDA_DIR(OUTPUT);
    IIC_SCL_L(); //����ʱ�ӿ�ʼ���ݴ���
    for(i = 0; i < 8; i++)
    {              
		if(txd & 0x80)IIC_SDA_H();
		else { IIC_SDA_L(); }
        txd <<= 1; 	  
		delay_us(1);   // 1
		IIC_SCL_H();
		delay_us(1);  //  1
		IIC_SCL_L();
		delay_us(1);  // 1
    }	 
} 	    

//��1���ֽڣ�ack = 1ʱ������ACK��ack = 0������nACK   
uint8_t i2c_read_byte(uint8_t ack)
{
	uint8_t  i, receive = 0;
	
	SDA_DIR(INPUT); //SDA����Ϊ����
    for(i = 0; i < 8; i++ )
	{
		IIC_SCL_L();
        delay_us(1);  // 2
		IIC_SCL_H();
        receive <<= 1;
        if(IIC_SDA_READ())receive++;   
		delay_us(1); 
    }	
	IIC_SCL_L();
    if (! ack)
        i2c_nack(); //����nACK
    else
        i2c_ack();  //����ACK   
    return receive;
}

/***********************************************
˵��: I2C ����д�Ĵ�������
����: uint8_t sla_addr     I2C �Ի���ַ
             uint8_t data_addr   �Ĵ�����ַ
����ֵ: �����ɹ�: SYS_SUCCESS;   ʧ��: SYS_FAILED
************************************************/
SYS_RESULT i2c_start_write(uint8_t sla_addr, uint16_t data_addr)
{
    if(i2c_start())return SYS_FAILED;
		
    i2c_send_byte(sla_addr | IIC_WRITE);
	if(i2c_wait_ack()){ INSERT_ERROR_INFO(0); }

    // д�Ĵ�����ַ
    #if 0
	i2c_send_byte((data_addr >> 8)); 
	if(i2c_wait_ack()){ INSERT_ERROR_INFO(0); return SYS_FAILED; }
	#endif
	
	i2c_send_byte((uint8_t)data_addr);
	if(i2c_wait_ack()){ INSERT_ERROR_INFO(0); return SYS_FAILED; }
    
	
    return SYS_SUCCESS;
}

SYS_RESULT IIC_WriteNByte_Raw(uint8_t sla_addr, uint16_t data_addr, uint8_t * pdata, uint8_t size)
{  	
    uint8_t *p = (uint8_t *)pdata;
    register uint8_t i;
	
    if(i2c_start_write(sla_addr, data_addr)){ INSERT_ERROR_INFO(0); return SYS_FAILED; }  	

	for(i = 0; i<size; i++, p++)
    {
        i2c_send_byte(*p);			//д����
        if(i2c_wait_ack()){ INSERT_ERROR_INFO(0); return SYS_FAILED; }
    }
    i2c_stop();						//����STOP �ź�
    return SYS_SUCCESS;
}

SYS_RESULT IIC_WriteNByte(uint8_t sla_addr, uint16_t data_addr, uint8_t * pdata, uint8_t size)
{
   SYS_RESULT res;

  // nos_sem_take_wait_forever(semIIC);
   //OSSchedLock();
   res = IIC_WriteNByte_Raw(sla_addr, data_addr, pdata, size);
   //OSSchedUnlock();
   //nos_sem_release(semIIC);
    
   return res;
}

/*******************************
����˵��: ��ȡN���ֽ�
����: uint8_t sla_addr: I2C�豸�ӻ���ַ
             uint16_t data_addr: �Ĵ�����ַ
             uint8_t * pdata: ��ȡ��������ʼָ��
             uint8_t   size: ��ȡ�����ݳ���, ����ʱ��Ҫ��ȡ�����ݳ���. 
********************************/
SYS_RESULT IIC_ReadNByte_Raw(uint8_t sla_addr, uint16_t data_addr, uint8_t * pdata, uint8_t size, uint8_t restart_iic)
{
    uint8_t *p = pdata;

    if(i2c_start_write(sla_addr, data_addr)){ INSERT_ERROR_INFO(0); return SYS_FAILED; }

    if(restart_iic){ i2c_start(); }
	
    i2c_send_byte(sla_addr  | IIC_READ); //��ģʽ
    if(i2c_wait_ack()){ INSERT_ERROR_INFO(0); return SYS_FAILED; }

    for(; size > 1; size--, p++)	//ע�����������size > 1,��Ϊ���滹���ٶ�ȡһ��
    {
        *p = i2c_read_byte(1);  //��ȡ����
    }
    *p = i2c_read_byte(0);
    i2c_stop();
	return SYS_SUCCESS;
}

SYS_RESULT IIC_ReadNByteExt(uint8_t sla_addr, uint16_t data_addr, uint8_t * pdata, uint8_t size, uint8_t restart_iic)
{
   SYS_RESULT res;
   
   //nos_sem_take_wait_forever(semIIC);
   //OSSchedLock();
   res = IIC_ReadNByte_Raw(sla_addr, data_addr, pdata, size, restart_iic);
   //OSSchedUnlock();
  // nos_sem_release(semIIC);

   return res;
}

// ��׼IIC
SYS_RESULT IIC_ReadNByte(uint8_t sla_addr, uint16_t data_addr, uint8_t * pdata, uint8_t size)
{
    return IIC_ReadNByteExt(sla_addr, data_addr, pdata, size, 0);
}

/************************************************
����: ֱ�Ӷ�N���ֽ�, ����Ҫָ���Ĵ�����ַ
����: 
*************************************************/
SYS_RESULT IIC_ReadNByteDirectly_Raw(uint8_t sla_addr, uint8_t * pdata, uint8_t size)
{
    uint8_t *p = pdata;

    if(i2c_start()){ INSERT_ERROR_INFO(0); return SYS_FAILED; }
	
    i2c_send_byte(sla_addr | IIC_READ); //��ģʽ
    if(i2c_wait_ack()){ INSERT_ERROR_INFO(0); return SYS_FAILED; }

    for(; size > 1; size--, p++)	//ע�����������size > 1,��Ϊ���滹���ٶ�ȡһ��
    {
        *p = i2c_read_byte(1);  //��ȡ����
    }
    *p = i2c_read_byte(0);
    i2c_stop();

	return SYS_SUCCESS;
}

SYS_RESULT IIC_ReadNByteDirectly(uint8_t sla_addr, uint8_t * pdata, uint8_t size)
{
   SYS_RESULT res;

   //nos_sem_take_wait_forever(semIIC);
   //OSSchedLock();
   res = IIC_ReadNByteDirectly_Raw(sla_addr, pdata, size);
   //OSSchedUnlock();
   //nos_sem_release(semIIC);

   return res;
}

