#include "BSP.h"
#include <stdio.h>
#include "delay.h"

int main(void)
{
	RCC_Configuration();
	GPIO_Configuration();
	delay_init(72);
	while(1)
	{
		GPIO_SetBits(GPIOC,GPIO_Pin_13);
		delay_ms(1000);
		GPIO_ResetBits(GPIOC,GPIO_Pin_13);
		delay_ms(1000);

	}   
}

