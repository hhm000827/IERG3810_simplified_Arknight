#include "stm32f10x.h"
//#include "IERG3810_TouchScreen.h"
//#include "Board.h"
#include "IERG3810_USART.h"
#include "IERG3810_TFTLCD.h"
#include "global.h"

void IERG3810_NVIC_SetPriorityGroup(u8 prigroup) {
	u32 temp, temp1;
	temp1 = prigroup & 0x00000007;
	temp1 <<= 8;
	temp = SCB->AIRCR;
	temp &= 0X0000F8FF;
	temp |= 0x05FA0000;
	temp |= temp1;
	SCB->AIRCR = temp;
}

void IERG3810_PS2key_ExtiInit(void) {
	//PC11(PS/2 CLK),EXTI-11,IRQ#40 and PC10(PS/2 Data),EXTI-10,IRQ#40
	RCC->APB2ENR |= 1 << 4;	//enable port C
	GPIOC->CRH &= 0XFFFFF0FF; //enable PC10
	GPIOC->CRH |= 0X00000800;
	GPIOC->CRH &= 0XFFFF0FFF; //enable PC11
	GPIOC->CRH |= 0X00008000;
	RCC->APB2ENR |= 0x01;	//enable AFIO clock
	AFIO->EXTICR[2] &= 0XFFFFF0FF; //enable EXTI-10
	AFIO->EXTICR[2] |= 0x00000200;  //enable EXTI-10 PC10
	AFIO->EXTICR[2] &= 0XFFFF0FFF; //enable EXTI-11
	AFIO->EXTICR[2] |= 0x00002000;  //enable EXTI-11 PC11
	EXTI->IMR |= 1 << 11; //edge trigger (use CLK as an interrupt)
	EXTI->FTSR |= 1 << 11;	//falling edge
	//EXTI->RTSR |= 1 << 0; // rising edge

	NVIC->IP[40] = 0x20;	//set priority
	NVIC->ISER[1] &= ~(1 << 8);	//set NVIC 'SET ENABLE REGISTER'

	NVIC->ISER[1] |= (1 << 8);	//IRQ40
}

void EXTI15_10_IRQHandler(void) {
	int i;
	if (ps2count > 0 && ps2count < 9)
	{
		ps2key |= ((GPIOC->IDR & 1 << 10) >> 10) << (ps2count - 1);
	}
	ps2count++;
	for (i = 0; i < 10; i++);
	EXTI->PR = 1 << 11;
}

u16 Get_ADC(u8 ch) {
	ADC1->SQR3 &= 0xFFFFFFE0;
	ADC1->SQR3 |= ch;
	ADC1->CR2 |= 1 << 22;
	while (!(ADC1->SR & 1 << 1)); // wait until, EOC bi t is set return ADC1->DR;
	return ADC1->DR;
}

void IERG3810_ADC_Init(void)
{
	RCC->APB2ENR |= 1 << 2;
	GPIOA->CRL &= 0xFFFFFFF0;
	RCC->APB2ENR |= 1 << 9;
	RCC->APB2RSTR = 1 << 9;
	RCC->APB2RSTR &= ~(1 << 9);
	RCC->CFGR &= ~(3 << 14);
	RCC->CFGR |= 2 << 14;
	ADC1->CR1 &= 0xF0FFFF;
	ADC1->CR1 &= ~(1 << 8);
	ADC1->CR2 &= ~(1 << 1);
	ADC1->CR2 &= ~(7 << 17);
	ADC1->CR2 |= 7 << 17;
	ADC1->CR2 |= 1 << 20;
	ADC1->CR2 &= ~(1 << 11);
	ADC1->SQR1 &= ~(0xF << 20);
	ADC1->SQR1 |= 0 << 20;
	ADC1->SMPR2 &= ~(7 << 0);
	ADC1->SMPR2 |= 7 << 0;
	ADC1->CR2 |= 1 << 0;
	ADC1->CR2 |= 1 << 3;

	while (ADC1->CR2 & 1 << 2);
}