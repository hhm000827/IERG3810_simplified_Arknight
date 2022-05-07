#include "stm32f10x.h"
#include "global.h"

void IERG3810_NVIC_SetPriorityGroup(u8 prigroup);
void IERG3810_PS2key_ExtiInit(void);
u16 Get_ADC(u8 ch);
void IERG3810_ADC_Init(void);
