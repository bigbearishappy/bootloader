#include "BSP.h"
#include <stdio.h>
#include "delay.h"
#include "queue_loop.h"

typedef void (*pFunction)(void);//����ָ��
pFunction Jump_To_Application;//����һ������ָ�����

void jump2app(void)
{
    Jump_To_Application = (pFunction)(*(vu32*)(IAPSTART + 4));
	//__MSR_MSP(*(vu32*) IAPSTART);
	__set_MSP(*(vu32*) IAPSTART);
	Jump_To_Application();
}


int main(void)
{
	char flag = 1;
	RCC_Configuration();
	NVIC_Configuration();
	GPIO_Configuration();
	USART_Configuration();
	delay_init(72);
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
	while(flag)
	{
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))//if PB0 is high
		{
			flag = 0;
		    //1 erase the flash
			FlashAllErase();
			//2 program the flash
			FlashProgram();
			GPIO_SetBits(GPIOC,GPIO_Pin_13);
			delay_ms(500);
			GPIO_ResetBits(GPIOC,GPIO_Pin_13);
			delay_ms(500);
			GPIO_SetBits(GPIOC,GPIO_Pin_13);
			delay_ms(500);
		}

		if(flag == 0)
		{
		    jump2app();
		}
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		delay_ms(100);
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		delay_ms(100);
	}   
}

