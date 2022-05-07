#include "stm32f10x.h"

void IERG3810_LED_Init() {
		//enable PB5 (light 1)
		RCC ->APB2ENR |= 1 << 3; //To enable PB,shift 1 to IOPB(3)	
		GPIOB->CRL &= 0xFF0FFFFF; //To enable PB5, we found that it is in CRL and area 3, set area 3 to 0 , others are F(no need enable)
		GPIOB->CRL |= 0x00300000;	//find and set the mode and turn it to decimal(0011->3)
		GPIOB->ODR |= 1<<5;
	
		//enable PE5 (light 2)
		RCC ->APB2ENR |= 1 << 6; //To enable PE,shift 1 to IOPE(6)
		GPIOE->CRL &= 0xFF0FFFFF; //To enable PE5, we found that it is in CRL and area 5, set area 5 to 0 , others are F(no need enable)
		GPIOE->CRL |= 0x00300000;	//find and set the mode and turn it to decimal(0011->3)
		GPIOE->ODR |= 1<<5;
}
