
#include "Uart_Drv.h"
#include "stm32f10x.h"
#include "GlobalDef.h"
#include "os_global.h"
#include "PM25Sensor.h"

#ifdef USE_STD_LIB
void USART1_Init(uint32_t baudrate)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
     //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
	
    //Usart1 NVIC ����
    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	

    // ���������ʼ��
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // �����ж�ʹ��
    //USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    USART_Cmd(USART1, ENABLE);     
}

void USART2_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
	/* config USART2 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* USART2 GPIO config */
    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//Usart NVIC ����
    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	
	
	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure); 

	
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);  
	//USART_ITConfig(USART2, USART_IT_TXE, ENABLE); 
    
	USART_Cmd(USART2, ENABLE);
}

void USART3_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	/* USART  GPIO config */
    /* Configure USART3  Tx  as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
    /* Configure USART3 Rx  as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	//Usart NVIC ����
    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	

	/* USART mode config */
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	
	USART_InitStructure.USART_Mode = USART_Mode_Rx; //| USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure); 
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  
	//USART_ITConfig(USART3, USART_IT_TXE, ENABLE); 

	USART_Cmd(USART3, ENABLE);
}
#else
// Ĭ��: һ����ʼλ��8������λ��n��ֹͣλ, ��У��λ, ��Ӳ��������
// USART2, 3, 4, 5: PCLK1, ����Ϊ 24MHz; USART1: PCLK2: 48MHz
void USART1_Init(uint32_t freq, uint32_t baudrate)
{
    RCC->APB2RSTR |=  RCC_APB2RSTR_USART1RST;		            //����1��λ
	RCC->APB2RSTR &=~ RCC_APB2RSTR_USART1RST;		            //����1ֹͣ��λ

	SET_REG_32_BIT(RCC->APB2ENR, RCC_APB2Periph_GPIOA);		// ʹ�� GPIOA ʱ��
	SET_REG_32_BIT(RCC->APB2ENR, RCC_APB2Periph_USART1);     // ����1ʱ��ʹ��
	GPIOA->CRH = (GPIOA->CRH & 0xFFFFF00F) + 0x000004B0;		// PA9: TX, �����������; PA10: RX, ��������					    

	//USART1->BRR  = FREQ_48MHz / baudrate;  //����������ΪBaudRate
	USART1->BRR  = freq / baudrate;  //����������ΪBaudRate
	
	USART1->CR1 |= (USART_CR1_UE | USART_Mode_Rx | USART_Mode_Tx);
	USART1->CR1 |= USART_CR1_RXNEIE;	    // �����ж�ʹ��
	//USART1->CR1 |= USART_CR1_TXEIE;     // ���Ϳ��ж�ʹ��
	
	STM32_NVICInit(USART1_IRQn, 3, 5, 0);  // ��3�����ȼ�, 3λ��ռ���ȼ�, 1λ��Ӧ���ȼ�
}

void USART2_Init(uint32_t freq, uint32_t baudrate)
{
    RCC->APB1RSTR |=  RCC_APB1RSTR_USART2RST;		            // ����2��λ
	RCC->APB1RSTR &=~ RCC_APB1RSTR_USART2RST;		            // ����2ֹͣ��λ
	
	SET_REG_32_BIT(RCC->APB2ENR, RCC_APB2Periph_GPIOA);		//ʹ�� GPIOA ʱ��
	SET_REG_32_BIT(RCC->APB1ENR, RCC_APB1Periph_USART2);		//����2ʱ��ʹ��
	GPIOA->CRL = (GPIOA->CRL & 0xFFFF00FF) + 0x00004B00;		//PA2: TX, �����������; PA3: RX, ��������

	USART2->BRR  = freq / baudrate;  //����������ΪBaudRate
	
	//USART2->CR1 |= (USART_CR1_UE | USART_Mode_Rx | USART_Mode_Tx);
	USART2->CR1 |= (USART_CR1_UE | USART_Mode_Tx);
	
	//USART2->CR1 |= USART_CR1_RXNEIE;	// �����ж�ʹ��
	//USART2->CR1 |= USART_CR1_TXEIE;     // ���Ϳ��ж�ʹ��
	
	STM32_NVICInit(USART2_IRQn, 3, 5, 1);   // ��3�����ȼ�, 3λ��ռ���ȼ�, 1λ��Ӧ���ȼ�
}

void USART3_Init(uint32_t freq, uint32_t baudrate)
{
    RCC->APB1RSTR |=  RCC_APB1RSTR_USART3RST;		            //����3��λ
	RCC->APB1RSTR &=~ RCC_APB1RSTR_USART3RST;		            //����3ֹͣ��λ
	
	SET_REG_32_BIT(RCC->APB2ENR, RCC_APB2Periph_GPIOB);       //ʹ�� GPIOB ʱ��
	SET_REG_32_BIT(RCC->APB1ENR, RCC_APB1Periph_USART3);		//����3ʱ��ʹ��
	GPIOB->CRH = (GPIOB->CRH & 0xFFFF00FF) + 0x00004B00;		//PB10: TX, �����������; PB11: RX, ��������	
	
	//USART3->BRR  = FREQ_24MHz / baudrate;  //����������ΪBaudRate
	USART3->BRR  = freq / baudrate;  //����������ΪBaudRate
	
	USART3->CR1 |= (USART_CR1_UE | USART_Mode_Rx | USART_Mode_Tx);
	USART3->CR1 |= USART_CR1_RXNEIE;	   // �����ж�ʹ��
	//USARTx->CR1 |= USART_CR1_TXEIE;     // ���Ϳ��ж�ʹ��
	
	STM32_NVICInit(USART3_IRQn, 3, 5, 1);  // ��3�����ȼ�, 3λ��ռ���ȼ�, 1λ��Ӧ���ȼ�
}
#endif


#if 1 //DEBUG_VERSION
#include <stdio.h>
#include "uart_queue.h"
//FILE __stdout;  

int fputc(int ch, FILE *f)
{
  // uint16_t count = 0xFFF;
   
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  #if (PRINTF_OUT_SEL == UART_BLOCK)
  QUEUE_UART->DR = (ch & (uint16_t)0x01FF);
  
  /* Loop until the end of transmission */
  //while (USART_GetFlagStatus(QUEUE_UART, USART_FLAG_TXE) == RESET);
  while (!(QUEUE_UART->SR & USART_FLAG_TXE));
  #else
  Uart_SendByte(ch);
  #endif
  
  return ch;
}

#else
void os_print(char * s)
{
     while(*s)
     {
        QUEUE_UART->DR = ((*s++) & (uint16_t)0x01FF);
        while (!(QUEUE_UART->SR & USART_FLAG_TXE));
     }
}
#endif


void USART1_IRQHandler(void)
{ 
   #if  (PM25_SEL == 1)
   PM25_UART_IRQHandler();
   #elif (HCHO_SEL == 1)
   HCHO_UART_IRQHandler();
   #elif (QUEUE_SEL == 1)
   Queue_UART_IRQHandler();
   #endif
}

void USART2_IRQHandler(void)
{
   #if  (PM25_SEL == 2)
   PM25_UART_IRQHandler();
   #elif (HCHO_SEL == 2)
   HCHO_UART_IRQHandler();
   #elif (QUEUE_SEL == 2)
   Queue_UART_IRQHandler();
   #endif

}

void USART3_IRQHandler(void)
{
   #if  (PM25_SEL == 3)
   PM25_UART_IRQHandler();
   #elif (HCHO_SEL == 3)
   HCHO_UART_IRQHandler();
   #elif (QUEUE_SEL == 3)
   Queue_UART_IRQHandler();
   #endif
}






