#ifndef BSP_H
#define BSP_H
#include "stm32f10x.h"
#include "queue_loop.h"

#define IAPSTART	0x8003000//bootloader memory is 8KB			

#define MAXSIZE	1024*5
#define PAGESIZE 1024

extern unsigned char buffer[MAXSIZE];
extern Queue_l queue;

void RCC_Configuration(void);
void NVIC_Configuration(void);
void TIM_Configuration(void);
void GPIO_Configuration(void);
void GPIO_NVIC_Init(void);
void USART_Configuration(void);
void Program_Flash_HalfWord(unsigned int address,unsigned short data);
unsigned short Program_Flash_ReadHalfWord(unsigned int address);
void FlashAllErase(void);
void FlashProgram(void);

#endif

