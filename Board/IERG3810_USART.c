#include "stm32f10x.h"


void IERG3810_USART1_init(u32 pclkl,u32 bound){
	float temp;
	u16 mantissa;
	u16 fraction;
	temp=(float) (pclkl*1000000)/(bound*16);
	mantissa =temp;
	fraction = (temp-mantissa)*16;
	mantissa <<= 4;
	mantissa += fraction;
	RCC->APB2ENR |= 1<<2;
	RCC->APB2ENR |= 1<<14;
	GPIOA->CRH &= 0xFFFFF00F;
	GPIOA->CRH |= 0x000008B0;
	RCC->APB2RSTR |= 1<<14;
	RCC->APB2RSTR &= ~(1<<14);
	USART1->BRR = mantissa;
	USART1->CR1 |= 0x2008;
}

void IERG3810_USART2_init(u32 pclkl,u32 bound){
	float temp;
	u16 mantissa;
	u16 fraction;
	temp=(float) (pclkl*1000000)/(bound*16);
	mantissa =temp;
	fraction = (temp-mantissa)*16;
	mantissa <<= 4;
	mantissa += fraction;
	RCC->APB2ENR |= 1<<2;
	RCC->APB1ENR |= 1<<17;
	GPIOA->CRL &= 0xFFFF00FF;
	GPIOA->CRL |= 0x00008B00;
	RCC->APB2RSTR |= 1<<17;
	RCC->APB2RSTR &= ~(1<<17);
	USART2->BRR = mantissa;
	USART2->CR1 |= 0x2008;
}

void USART_print(u8 USARTport, char *st){
	u8 i=0;
	while (st[i] != 0x00){
		//if (USARTport == 1) USART1 -> DR = st[i];
		//if (USARTport == 2) USART2 -> DR = st[i];
		//Delay(50000);
		
		if (USARTport == 1) {
			USART1 -> DR = st[i];
			while((USART1->SR & 1<<7) ==0);
		}
		if (USARTport == 2)		{
			USART2 -> DR = st[i];
			while((USART2->SR & 1<<7) ==0);
		}
		if(i==252) break;
		i++;
	}
}