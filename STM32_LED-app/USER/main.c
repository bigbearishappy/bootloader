
/*******************************************************************************
*	
* 软件功能:	 GPIO闪灯实验（软件延时方式）
* 
*******************************************************************************/
#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"

/*************************************************
函数: void RCC_Configuration(void)
功能: 复位和时钟控制 配置
参数: 无
返回: 无
**************************************************/
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
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);  //允许GPIOB、AFIO时钟
}

/*************************************************
函数: void GPIO_Configuration(void）
功能: GPIO配置
参数: 无
返回: 无
**************************************************/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;        //定义GPIO初始化结构体

  /* Configure PE.0,PE.1,PE.2,PE.3,PE.4,PE.5,PE.6,PE.7 as Output push-pull */ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13  ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
}



/*************************************************
函数: int main(void)
功能: main主函数
参数: 无
返回: 无
**************************************************/
int main(void)
{
  RCC_Configuration();
  GPIO_Configuration();
  delay_init(72);
  while(1)
  {
    GPIO_SetBits(GPIOC,GPIO_Pin_13);	  //设置为高电平
            
    delay_ms(1000);
    GPIO_ResetBits(GPIOC,GPIO_Pin_13); //设置为低电平
             
    delay_ms(1000);
	}   
}
