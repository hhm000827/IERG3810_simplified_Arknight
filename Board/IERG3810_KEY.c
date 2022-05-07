#include "stm32f10x.h"

void IERG3810_KEY_Init(){
		//enable PA0 (key up)
		RCC ->APB2ENR |= 1 << 2; //To enable PA,shift 1 to IOPA(2)
		GPIOA->CRL &= 0xFFFFFFF0; //To enable PA0, we found that it is in CRL and area 0, set area 0 to 0 , others are F(no need enable)
		GPIOA->CRL |= 0x00000008; //find and set the mode and turn it to decimal(1000->8)	

		//enable PE4 (key0)
	  RCC ->APB2ENR |= 1 << 6; //To enable PE,shift 1 to IOPE(6)
		GPIOE->CRL &= 0xFFF0FFFF; //To enable PE4, we found that it is in CRL and area 4, set area 4 to 0 , others are F(no need enable)
		GPIOE->CRL |= 0x00080000; //find and set the mode and turn it to decimal(1000->8)
		GPIOE->ODR |= 1<<4;
	
		//enable PE3 (key 1)
		RCC ->APB2ENR |= 1 << 6; //To enable PE,shift 1 to IOPE(6)
		GPIOE->CRL &= 0xFFFF0FFF; //To enable PE3, we found that it is in CRL and area 3, set area 3 to 0 , others are F(no need enable)
		GPIOE->CRL |= 0x00008000; //find and set the mode and turn it to decimal(1000->8)
		GPIOE->ODR |= 1<<3;	

		//enable PE2 (key2)
	  RCC ->APB2ENR |= 1 << 6; //To enable PE,shift 1 to IOPE(6)
		GPIOE->CRL &= 0xFFFFF0FF; //To enable PE2, we found that it is in CRL and area 6, set area 6 to 0 , others are F(no need enable)
		GPIOE->CRL |= 0x00000800; //find and set the mode and turn it to decimal(1000->8)
		GPIOE->ODR |= 1<<2;
}
