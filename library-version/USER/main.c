#include "BSP.h"
#include <stdio.h>
#include "delay.h"
#include "queue_loop.h"

typedef void (*pFunction)(void);//函数指针
pFunction Jump_To_Application;//定义一个函数指针变量

void jump2app(void)
{
    Jump_To_Application = (pFunction)(*(vu32*)(IAPSTART + 4));
	//__MSR_MSP(*(vu32*) IAPSTART);
	__set_MSP(*(vu32*) IAPSTART);
	Jump_To_Application();
}


int main(void)
{
	char flag = 1,i;
	char head[3],tail[2];
	short data_seq,data_len;
	RCC_Configuration();
	GPIO_Configuration();
	USART_Configuration();
	delay_init(72);
	while(flag)
	{
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))//if PB0 is high
		{
			flag = 0;
		    //1 erase the flash
			FlashAllErase();
			while(!Queueisempty(&queue)){
				for(i = 0;i < 3; i++)
					head[i] = Queuegetc(&queue);
				if(head[0] == 0xff&&head[1] == 0xff&&head[2] == 0xff){
					data_seq = Queuegetc(&queue);
					data_len = Queuegetc(&queue);
					for(i = 0;i < BASESIZE; i++)
						uart_temp[i] = Queuegetc(&queue);
					for(i = 0;i < 2; i++)
						tail[i] = Queuegetc(&queue);
					if(tail[0] == 0xff&&tail[1] == 0xff){
						//2 program the flash
						FlashProgram();
					}
				}
				else
					break;
			}
		}

		if(flag == 0)
		{
		    jump2app();
		}
	}   
}

