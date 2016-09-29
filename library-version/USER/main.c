#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"
#define IAPSTART	0x8002000//bootloader memory is 8KB			

#define MAXSIZE	1024
unsigned char uart_data[MAXSIZE] = {0};
char data_rdy_flag = 0x00;//0x00 is unready,0x01 is ready
int cnt = 0;

typedef void (*pFunction)(void);//����ָ��
pFunction Jump_To_Application;//����һ������ָ�����

void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;                    //�����ⲿ���پ�������״̬ö�ٱ���
  RCC_DeInit();                                    //��λRCC�ⲿ�豸�Ĵ�����Ĭ��ֵ
  RCC_HSEConfig(RCC_HSE_ON);                       //���ⲿ���پ���
  HSEStartUpStatus = RCC_WaitForHSEStartUp();      //�ȴ��ⲿ����ʱ��׼����
  if(HSEStartUpStatus == SUCCESS)                  //�ⲿ����ʱ���Ѿ�׼���
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); //����FLASHԤ�����幦�ܣ�����FLASH�Ķ�ȡ�����г����б�����÷�.λ�ã�RCC��ʼ���Ӻ������棬ʱ������֮��
    FLASH_SetLatency(FLASH_Latency_2);                    //flash��������ʱ
      	
    RCC_HCLKConfig(RCC_SYSCLK_Div1);               //����AHB(HCLK)ʱ�ӵ���==SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1);                //����APB2(PCLK2)��==AHBʱ��
    RCC_PCLK1Config(RCC_HCLK_Div2);                //����APB1(PCLK1)��==AHB1/2ʱ��
         
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);  //����PLLʱ�� == �ⲿ���پ���ʱ�� * 9 = 72MHz
    RCC_PLLCmd(ENABLE);                                   //ʹ��PLLʱ��
   
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)    //�ȴ�PLLʱ�Ӿ���
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);            //����ϵͳʱ�� = PLLʱ��
    while(RCC_GetSYSCLKSource() != 0x08)                  //���PLLʱ���Ƿ���Ϊϵͳʱ��
    {
    }
  }
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);  //����GPIOB��AFIOʱ��
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 				  	//����������
	NVIC_InitStructure.NVIC_IRQChannel =TIM2_IRQn ; 		  			//TIM2�ж�ѡͨ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	   		//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			 		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 					//ʹ���жϿ���
	NVIC_Init(&NVIC_InitStructure);
}

void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;   
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);  				//Timer2 ʱ��ʹ��
	TIM_DeInit(TIM2);							   						//��λTIM2��ʱ�� 	 
	TIM_TimeBaseStructure.TIM_Period=2;									//��ʱ������  1ms 
	TIM_TimeBaseStructure.TIM_Prescaler=36000-1;	 					//Ԥ��Ƶ��
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;				//TIM2ʱ�ӷ�Ƶ,Ϊ1��ʾ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;			//��ʱ������Ϊ���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);								//�����ʱ��2�������־λ
	//TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	  						//ʹ�ܶ�ʱ��2����ж�
	//TIM_Cmd(TIM2, ENABLE);						 						//��ʱ��2ʹ��

}

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;        //����GPIO��ʼ���ṹ��

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
}

void GPIO_NVIC_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
//configurate the GPIO PA5
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//GPIO_Pin_5;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   
    GPIO_Init(GPIOA, &GPIO_InitStructure);  /*PA5 */ 

//configurate the NVIC 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);           
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

//configurate the EXTI
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    EXTI->IMR = 0X20;//opent the interrupt on the PA5
    EXTI->RTSR = 0X20;//configurate the interrupt to happen with rasing
    EXTI->FTSR = 0X20;//configurate the interrupt to happen with falling
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5);
}

void USART_Configuration(void)
{
	GPIO_InitTypeDef GPIO_initconfiguration;
	USART_InitTypeDef USART_configStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	
	GPIO_initconfiguration.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_initconfiguration.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_initconfiguration.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_initconfiguration);

	GPIO_initconfiguration.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_initconfiguration.GPIO_Pin = GPIO_Pin_3;
	GPIO_initconfiguration.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_initconfiguration);

	/*USART����ʱ�ӵĳ�ʼ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/*USART ��ʼ��*/
	USART_configStruct.USART_BaudRate = 115200;
	USART_configStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_configStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_configStruct.USART_Parity=USART_Parity_No;
	USART_configStruct.USART_StopBits=USART_StopBits_1;
	USART_configStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART2,&USART_configStruct);
	
	/*USART ��ʹ��*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//open the receive interrupt of USART1
	USART_Cmd(USART2,ENABLE);
}

void Program_Flash_HalfWord(unsigned int address,unsigned short data)
{
    unsigned int provalue = 0x00;
	//unsigned short result = 0x00;
	FLASH_Status status = FLASH_COMPLETE;

	provalue = (FLASH->CR & 0x80)>>7;
	if(provalue == 1){
        FLASH_Unlock();
	}
	FLASH->CR |= 0X01;

	status = FLASH_ProgramHalfWord(address, data);

	while(FLASH->SR & 0X01);

	//result = *((volatile unsigned int*)(0x0801FC00));

	//printf("res=0x%x\r\n",result);
}

unsigned short Program_Flash_ReadHalfWord(unsigned int address)
{
    unsigned short result = 0x00;
	result = *((volatile unsigned int*)(address));
	return result;
}

void jump2app(void)
{
    Jump_To_Application = (pFunction)(*(vu32*)(IAPSTART + 4));
	//__MSR_MSP(*(vu32*) IAPSTART);
	__set_MSP(*(vu32*) IAPSTART);
	Jump_To_Application();
}


int main(void)
{
  RCC_Configuration();
  GPIO_Configuration();
  delay_init(72);
  while(1)
  {
    GPIO_SetBits(GPIOB,GPIO_Pin_0);	  //����Ϊ�ߵ�ƽ
            
    delay_ms(500);
    GPIO_ResetBits(GPIOB,GPIO_Pin_0); //����Ϊ�͵�ƽ
             
    delay_ms(500);
	}   
}

void EXTI9_5_IRQHandler(void)
{
    if(EXTI->PR == 0X20)
    {
		//add the interrupt code
        EXTI->PR = 0X20;
    }

}

/*
���ڽ������ݲ��ԣ�
����
1 ÿ�ν���1024���ֽ�
2 ͨ����ʱ����������ɱ�־λ
3 �����������������ݱ����Ƿ�����ʼ��־���߽�����־��Ȼ��ʼ��дFLASH
4 ��д��ɺ����ͷ�����OK
5 �ȴ����ͷ�������һ�����ݰ����߽���
*/
void USART1_IRQHandler (void)
{	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
	    //add the interrupt code
		uart_data[cnt++] = USART_ReceiveData(USART1);
		if(cnt == 1024)
		    data_rdy_flag = 0x01;
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

void TIM2_IRQHandler(void)
{      
if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 					//�ж��ж������־Ϊ�Ƿ�Ϊ1
    {
    TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update); 						//�������жϱ�־λ

	TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE);	  						//ʹ�ܶ�ʱ��2����ж�    close the TIM2
	TIM_Cmd(TIM2, DISABLE);						 					    //��ʱ��2ʹ��

	if(data_rdy_flag == 0x01){
		//check the data and program the flash
		data_rdy_flag = 0x00;
		//printf("ok");//send ok to the sender
	}
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	  						//ʹ�ܶ�ʱ��2����ж�
	TIM_Cmd(TIM2, ENABLE);						 						//��ʱ��2ʹ��
    }
}
