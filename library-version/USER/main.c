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
	RCC_Configuration();
	GPIO_Configuration();
	USART_Configuration();
	delay_init(72);
	//while(1)
	//{
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))//if PB0 is high
		{
		    //1 erase the flash
			//2 program the flash
		}
		else
		{
		    jump2app();
		}
	//}   
}

