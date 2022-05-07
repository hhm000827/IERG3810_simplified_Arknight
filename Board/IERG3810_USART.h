#include "stm32f10x.h"

void IERG3810_USART1_init(u32 pclkl,u32 bound);
void IERG3810_USART2_init(u32 pclkl,u32 bound);
void USART_print(u8 USARTport, char *st);