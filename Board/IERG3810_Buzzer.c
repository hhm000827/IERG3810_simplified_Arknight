#include "stm32f10x.h"

void IERG3810_Buzzer_Init() {
		//enable PB8 (buzzer)
		RCC ->APB2ENR |= 1 << 3; //To enable PB,shift 1 to IOPB(3)	
		GPIOB->CRH &= 0xFFFFFFF0; //To enable PB8, we found that it is in CRL and area 8, set area 8 to 0 , others are F(no need enable)
		GPIOB->CRH |= 0x00000003;	//find and set the mode and turn it to decimal(0011->3)	
}
