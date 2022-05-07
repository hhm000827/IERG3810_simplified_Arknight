#include "stm32f10x.h"

void IERG3810_clock_tree_init(void) {
	u8 PLL=7;
	unsigned char temp=0;
	RCC->CFGR &= 0xF8FF0000;	
	
	RCC->CR &= 0xFEF6FFFF;
	RCC->CR |= 0x00010000;
	while(!(RCC->CR>>17));
	//RCC->CFGR = 0x00000000; //APB2
	RCC->CFGR = 0x00000400; //APB1 ,APB2 no need divide by 2 , so APB2 keep 0 is ok
	RCC->CFGR |= PLL<<18;
	RCC->CFGR |= 1<<16;
	
	FLASH->ACR |= 0x30;
	RCC->CR |= 0x01000000;
	while(!(RCC->CR>>25));
	RCC->CFGR |= 0x00000002;
	while (temp != 0x02){
		temp = RCC->CFGR>>2;
		temp &= 0x03;
	}
}