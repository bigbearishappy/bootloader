#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"
#define IAPSTART	0x8002000//bootloader memory is 8KB			

#define MAXSIZE	1024
unsigned char uart_data[MAXSIZE] = {0};
char data_rdy_flag = 0x00;//0x00 is unready,0x01 is ready
int cnt = 0;

typedef void (*pFunction)(void);//函数指针
pFunction Jump_To_Application;//定义一个函数指针变量

void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;                    //定义外部高速晶体启动状态枚举变量
  RCC_DeInit();                                    //复位RCC外部设备寄存器到默认值
  RCC_HSEConfig(RCC_HSE_ON);                       //打开外部高速晶振
  HSEStartUpStatus = RCC_WaitForHSEStartUp();      //等待外部高速时钟准备好
  if(HSEStartUpStatus == SUCCESS)                  //外部高速时钟已经准别好
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); //开启FLASH预读缓冲功能，加速FLASH的读取。所有程序中必须的用法.位置：RCC初始化子函数里面，时钟起振之后
    FLASH_SetLatency(FLASH_Latency_2);                    //flash操作的延时
      	
    RCC_HCLKConfig(RCC_SYSCLK_Div1);               //配置AHB(HCLK)时钟等于==SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1);                //配置APB2(PCLK2)钟==AHB时钟
    RCC_PCLK1Config(RCC_HCLK_Div2);                //配置APB1(PCLK1)钟==AHB1/2时钟
         
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);  //配置PLL时钟 == 外部高速晶体时钟 * 9 = 72MHz
    RCC_PLLCmd(ENABLE);                                   //使能PLL时钟
   
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)    //等待PLL时钟就绪
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);            //配置系统时钟 = PLL时钟
    while(RCC_GetSYSCLKSource() != 0x08)                  //检查PLL时钟是否作为系统时钟
    {
    }
  }
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);  //允许GPIOB、AFIO时钟
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 				  	//优先组设置
	NVIC_InitStructure.NVIC_IRQChannel =TIM2_IRQn ; 		  			//TIM2中断选通
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	   		//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			 		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 					//使能中断控制
	NVIC_Init(&NVIC_InitStructure);
}

void TIM_Configuration(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;   
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);  				//Timer2 时钟使能
	TIM_DeInit(TIM2);							   						//复位TIM2定时器 	 
	TIM_TimeBaseStructure.TIM_Period=2;									//定时器周期  1ms 
	TIM_TimeBaseStructure.TIM_Prescaler=36000-1;	 					//预分频数
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;				//TIM2时钟分频,为1表示不分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;			//定时器计数为向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);								//清除定时器2的溢出标志位
	//TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	  						//使能定时器2溢出中断
	//TIM_Cmd(TIM2, ENABLE);						 						//定时器2使能

}

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;        //定义GPIO初始化结构体

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

	/*USART外设时钟的初始化*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/*USART 初始化*/
	USART_configStruct.USART_BaudRate = 115200;
	USART_configStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_configStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_configStruct.USART_Parity=USART_Parity_No;
	USART_configStruct.USART_StopBits=USART_StopBits_1;
	USART_configStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART2,&USART_configStruct);
	
	/*USART 的使能*/
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
    GPIO_SetBits(GPIOB,GPIO_Pin_0);	  //设置为高电平
            
    delay_ms(500);
    GPIO_ResetBits(GPIOB,GPIO_Pin_0); //设置为低电平
             
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
串口接收数据策略：
步骤
1 每次接收1024个字节
2 通过定时器检测接收完成标志位
3 如果接收完成则检测数据保重是否有起始标志或者结束标志，然后开始烧写FLASH
4 烧写完成后向发送方返回OK
5 等待发送方发送下一个数据包或者结束
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
if ( TIM_GetITStatus(TIM2 , TIM_IT_Update) != RESET ) 					//判断中断溢出标志为是否为1
    {
    TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update); 						//清除溢出中断标志位

	TIM_ITConfig(TIM2,TIM_IT_Update,DISABLE);	  						//使能定时器2溢出中断    close the TIM2
	TIM_Cmd(TIM2, DISABLE);						 					    //定时器2使能

	if(data_rdy_flag == 0x01){
		//check the data and program the flash
		data_rdy_flag = 0x00;
		//printf("ok");//send ok to the sender
	}
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	  						//使能定时器2溢出中断
	TIM_Cmd(TIM2, ENABLE);						 						//定时器2使能
    }
}
