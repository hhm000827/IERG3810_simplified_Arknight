#include "stm32f10x.h"
#include "IERG3810_TFTLCD.h"
#include "IERG3810_Clock.h"
#include "IERG3810_USART.h"
#include "FONT.H"
#include "CFONT.H"
#include "Image.h"
#include "Map.h"

typedef struct {
	u16 LCD_REG;
	u16 LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE   ((u32) (0x6C000000 | 0x000007FE))
#define LCD        ((LCD_TypeDef  *) LCD_BASE)

void IERG3810_TFTLCD_WrReg(u16 regval) {
	LCD->LCD_REG = regval;
}

void IERG3810_TFTLCD_WrData(u16 data) {
	LCD->LCD_RAM = data;
}

void IERG3810_TFTLCD_SetParameter(void) {
	IERG3810_TFTLCD_WrReg(0X01);
	IERG3810_TFTLCD_WrReg(0X11);

	IERG3810_TFTLCD_WrReg(0X3A);
	IERG3810_TFTLCD_WrData(0X55);

	IERG3810_TFTLCD_WrReg(0X29);

	IERG3810_TFTLCD_WrReg(0X36);
	IERG3810_TFTLCD_WrData(0XCA);
}

void IERG3810_TFTLCD_Init(void) {
	RCC->AHBENR |= 1 << 8;
	RCC->APB2ENR |= 1 << 3;
	RCC->APB2ENR |= 1 << 5;
	RCC->APB2ENR |= 1 << 6;
	RCC->APB2ENR |= 1 << 8;
	GPIOB->CRL &= 0xFFFFFFF0; //PB0
	GPIOB->CRL |= 0x00000003;

	//PORTD
	GPIOD->CRH &= 0x00FFF000;
	GPIOD->CRH |= 0xBB000BBB;
	GPIOD->CRL &= 0xFF00FF00;
	GPIOD->CRL |= 0x00BB00BB;

	//PORTE :
	GPIOE->CRH &= 0x00000000;
	GPIOE->CRH |= 0xBBBBBBBB;
	GPIOE->CRL &= 0x0FFFFFFF;
	GPIOE->CRL |= 0xB0000000;

	//PORTG12
	GPIOG->CRH &= 0XFFF0FFFF;
	GPIOG->CRH |= 0x000B0000;
	GPIOG->CRL &= 0XFFFFFFF0;
	GPIOG->CRL |= 0x0000000B;

	FSMC_Bank1->BTCR[6] = 0x00000000;
	FSMC_Bank1->BTCR[7] = 0x00000000;
	FSMC_Bank1E->BWTR[6] = 0x00000000;
	FSMC_Bank1->BTCR[6] |= 1 << 12;
	FSMC_Bank1->BTCR[6] |= 1 << 14;
	FSMC_Bank1->BTCR[6] |= 1 << 4;
	FSMC_Bank1->BTCR[7] |= 0 << 28;
	FSMC_Bank1->BTCR[7] |= 1 << 0;
	FSMC_Bank1->BTCR[7] |= 0xF << 8;
	FSMC_Bank1E->BWTR[6] |= 0 << 28;
	FSMC_Bank1E->BWTR[6] |= 0 << 0;
	FSMC_Bank1E->BWTR[6] |= 3 << 8;
	FSMC_Bank1->BTCR[6] |= 1 << 0;
	IERG3810_TFTLCD_SetParameter();
	GPIOB->BSRR = 1 << 0; //LCD light on
}

// screen size is 320*240 ; (0,0) is botton left corner
void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color) {
	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData(0x01);
	IERG3810_TFTLCD_WrData(0x3F);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData(0x01);
	IERG3810_TFTLCD_WrData(0xDF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color
	IERG3810_TFTLCD_WrData(color);
}

void IERG3810_TFTLCD_FillRectangular(u16 color, u16 start_x, u16 length_x, u16 start_y, u16 length_y) {
	u32 index = 0;
	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(start_x >> 8);
	IERG3810_TFTLCD_WrData(start_x & 0xFF);
	IERG3810_TFTLCD_WrData((length_x + start_x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length_x + start_x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(start_y >> 8);
	IERG3810_TFTLCD_WrData(start_y & 0xFF);
	IERG3810_TFTLCD_WrData((length_y + start_y - 1) >> 8);
	IERG3810_TFTLCD_WrData((length_y + start_y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color
	for (index = 0; index < length_x * length_y; index++) {
		IERG3810_TFTLCD_WrData(color);
	}
}

void IERG3810_TFTLCD_SevenSegment(u16 color, u16 start_x, u16 start_y, u8 digit) {
	if (digit == 0) {
		//a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10);
		//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);
		//c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);
		//d
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y, 10);
		//e
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 10, 65);
		//f
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 85, 65);
	}
	else if (digit == 1) {
		//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);
		//c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);
	}
	else if (digit == 2) {
		//a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10);
		//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);
		//d
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y, 10);
		//e
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 10, 65);
		//g
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 75, 10);
	}
	else if (digit == 3) {
		//a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10);
		//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);
		//c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);
		//d
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y, 10);
		//g
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 75, 10);
	}
	else if (digit == 4) {
		//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);
		//c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);
		//f
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 85, 65);
		//g
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 75, 10);
	}
	else if (digit == 5) {
		//a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10);
		//c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);
		//d
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y, 10);

		//f
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 85, 65);
		//g
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 75, 10);
	}
	else if (digit == 6) {
		//a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10);
		//c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);
		//d
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y, 10);
		//e
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 10, 65);
		//f
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 85, 65);
		//g
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 75, 10);
	}
	else if (digit == 7) {
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10);//a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);//c
	}
	else if (digit == 8) {
		//a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10);
		//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);
		//c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65);
		//d
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y, 10);
		//e
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 10, 65);
		//f
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 85, 65);
		//g
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 75, 10);
	}
	else if (digit == 9) {
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 150, 10); //a
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 85, 65);//b
		IERG3810_TFTLCD_FillRectangular(color, start_x + 70, 10, start_y + 10, 65); //c
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y, 10); //d
		IERG3810_TFTLCD_FillRectangular(color, start_x, 10, start_y + 85, 65); //f
		IERG3810_TFTLCD_FillRectangular(color, start_x + 10, 60, start_y + 75, 10); //g
	}
}

void IERG3810_TFTLCD_ShowChar(u16 x, u16 y, u8 ascii, u16 color, u16 bgcolor) {
	u8 i, j;
	u8 index;
	u8 height = 16, length = 8;
	if (ascii < 32 || ascii >127) return;
	ascii -= 32;
	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < height / 8; j++) {
		for (i = 0; i < height / 2; i++) {
			for (index = 0; index < length; index++) {
				if ((asc2_1608[ascii][index * 2 + 1 - j] >> i) & 0x01) IERG3810_TFTLCD_WrData(color);
				else IERG3810_TFTLCD_WrData(bgcolor);
			}
		}
	}
}

void IERG3810_TFTLCD_ShowChinChar(u16 x, u16 y, u8 ascii, u16 color, u16 bgcolor) {
	u8 i, j;
	u8 index;
	u8 height = 16, length = 16;
	if (ascii < 0 || ascii >10) return;
	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < height / 8; j++) {
		for (i = 0; i < height / 2; i++) {
			for (index = 0; index < length; index++) {
				if ((chi_1616[ascii][index * 2 + 1 - j] >> i) & 0x01) IERG3810_TFTLCD_WrData(color);
				else IERG3810_TFTLCD_WrData(bgcolor);
			}
		}
	}
}
// used for printing
void IERG3810_TFTLCD_ShowRange(u16 x, u16 y, u8 id) {
	u32 j;
	u32 height = Image2.height, length = Image2.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		IERG3810_TFTLCD_WrData(range[id][j]);
	}
}

void IERG3810_TFTLCD_Showoperator(u16 x, u16 y, u8 id) {
	u32 j;
	u32 height = Image.height, length = Image.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		IERG3810_TFTLCD_WrData(image_data_Image[id][j]);
	}
}

void IERG3810_TFTLCD_ShowEnemy(u16 x, u16 y, u8 id) {
	u32 j;
	u32 height = Image.height, length = Image.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		IERG3810_TFTLCD_WrData(enemies[id][j]);
	}
}

void IERG3810_TFTLCD_ShowMap(u16 x, u16 y, u16 arr, u16 element) {
	u32 j;
	u32 height = map.height, length = map.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	if (mapid[arr][element] == 100) {
		for (j = 0; j < (length * height); j++) {
			IERG3810_TFTLCD_WrData(red[j]);
		}
	}

	if (mapid[arr][element] == 50) {
		for (j = 0; j < (length * height); j++) {
			IERG3810_TFTLCD_WrData(blue[j]);
		}
	}

	if (mapid[arr][element] == 0) {
		for (j = 0; j < (length * height); j++) {
			IERG3810_TFTLCD_WrData(road[j]);
		}
	}
}

void ShowMenu(u16 x, u16 y) {
	u32 j;
	u32 height = intros.height, length = intros.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		IERG3810_TFTLCD_WrData(Menu[j]);
	}
}

void IERG3810_TFTLCD_ShowStage(u16 x, u16 y) {
	u32 j;
	u32 height, length;

	if (lose_live == 3) {
		height = fails.height;
		length = fails.width;
	}
	else {
		height = wins.height;
		length = wins.width;
	}

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		if (lose_live == 3) IERG3810_TFTLCD_WrData(fail[j]);
		else IERG3810_TFTLCD_WrData(Complete[j]);
	}
}

void IERG3810_TFTLCD_ShowLive(u16 x, u16 y) {
	u32 j;
	u32 height = map.height, length = map.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		IERG3810_TFTLCD_WrData(live[j]);
	}
}

//used for showing
void showlive(u8 live) {
	int x = 144, y = 280;
	int i;
	int gap = 32;
	u16 bgcolor = 0x0000, color = 0xFFFF;
	for (i = 0; i <= 9; i++) {
		if (i == live)IERG3810_TFTLCD_ShowChar(x + gap, y, 48 + i, color, bgcolor);
	}
	IERG3810_TFTLCD_ShowLive(x, y);
}

void ShowOperator(u8 num_of_operators) {
	u8 x, gap = 32;

	for (x = 0; x < num_of_operators; x++) {
		IERG3810_TFTLCD_Showoperator(x * gap, 0, x);
	}
}

void ShowEnemy(u8 num) {
	u8 x, gap = 32;

	for (x = 0; x < num; x++) {
		IERG3810_TFTLCD_ShowEnemy(x * gap, 280, x);
	}
}

void ShowMap(u8 xgap, u8 ymapstarting) {
	u8 i, j, x = 32;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 7; j++) {
			if (i == 0) {
				IERG3810_TFTLCD_ShowMap(j * x + xgap, ymapstarting + i * x, i, j);
			}
			else if (i == 1) {
				IERG3810_TFTLCD_ShowMap(j * x + xgap, ymapstarting + i * x, i, j);
			}
			else if (i == 2) {
				IERG3810_TFTLCD_ShowMap(j * x + xgap, ymapstarting + i * x, i, j);
			}
		}
	}
}

void ShowKillnumber(int total_number, int killnumber) {
	int x = 50, y = 290;
	int gap = 8;
	u16 bgcolor = 0x0000, color = 0xFFFF;
	int i;

	int first_number = total_number / 10; //e.g 1 in "15"
	int latter_number = total_number % 10; //e.g 5 in "15"

	int kill_first_number = killnumber / 10;
	int kill_latter_number = killnumber % 10;

	//show "kill:", kill number and total number
	if (killnumber <= total_number) {
		for (i = 0; i <= 9; i++) {
			IERG3810_TFTLCD_ShowChar(x, y, 75, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + gap, y, 105, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 2 * gap, y, 108, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 3 * gap, y, 108, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 4 * gap, y, 58, color, bgcolor);
			if (kill_first_number == i) IERG3810_TFTLCD_ShowChar(x + 5 * gap, y, 48 + i, color, bgcolor);
			if (kill_latter_number == i) IERG3810_TFTLCD_ShowChar(x + 6 * gap, y, 48 + i, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 7 * gap, y, 47, color, bgcolor); //show "/"
			if (first_number == i) IERG3810_TFTLCD_ShowChar(x + 8 * gap, y, 48 + i, color, bgcolor);
			if (latter_number == i) IERG3810_TFTLCD_ShowChar(x + 9 * gap, y, 48 + i, color, bgcolor);
		}
	}
}

void ShowStats(int buff_operator_ID) {
	u16 bgcolor = 0x0000, color = 0xFFFF;
	int xgap = 8, ygap = 16;
	int x = 5, y = 45;
	if (buff_operator_ID != 999) {
		IERG3810_TFTLCD_FillRectangular(bgcolor, 0, 130, 37, 80);

		// print "Class:"
		IERG3810_TFTLCD_ShowChar(x, y, 67, color, bgcolor);
		IERG3810_TFTLCD_ShowChar(x + xgap, y, 108, color, bgcolor);
		IERG3810_TFTLCD_ShowChar(x + 2 * xgap, y, 97, color, bgcolor);
		IERG3810_TFTLCD_ShowChar(x + 3 * xgap, y, 115, color, bgcolor);
		IERG3810_TFTLCD_ShowChar(x + 4 * xgap, y, 115, color, bgcolor);
		IERG3810_TFTLCD_ShowChar(x + 5 * xgap, y, 58, color, bgcolor);

		// 0: sniper 1: defender 2, 3: vanguard 4: guard 5: caster 6: medic
		if (buff_operator_ID == 0) {
			IERG3810_TFTLCD_ShowChar(x + 7 * xgap, y, 83, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 8 * xgap, y, 110, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 9 * xgap, y, 105, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 10 * xgap, y, 112, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 11 * xgap, y, 101, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 12 * xgap, y, 114, color, bgcolor);
		}
		else if (buff_operator_ID == 1) {
			IERG3810_TFTLCD_ShowChar(x + 7 * xgap, y, 68, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 8 * xgap, y, 101, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 9 * xgap, y, 102, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 10 * xgap, y, 101, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 11 * xgap, y, 110, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 12 * xgap, y, 100, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 13 * xgap, y, 101, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 14 * xgap, y, 114, color, bgcolor);
		}
		else if ((buff_operator_ID == 2) || (buff_operator_ID == 3)) {
			IERG3810_TFTLCD_ShowChar(x + 7 * xgap, y, 86, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 8 * xgap, y, 97, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 9 * xgap, y, 110, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 10 * xgap, y, 103, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 11 * xgap, y, 117, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 12 * xgap, y, 97, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 13 * xgap, y, 114, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 14 * xgap, y, 100, color, bgcolor);
		}
		else if (buff_operator_ID == 4) {
			IERG3810_TFTLCD_ShowChar(x + 7 * xgap, y, 71, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 8 * xgap, y, 117, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 9 * xgap, y, 97, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 10 * xgap, y, 114, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 11 * xgap, y, 100, color, bgcolor);
		}
		else if (buff_operator_ID == 5) {
			IERG3810_TFTLCD_ShowChar(x + 7 * xgap, y, 67, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 8 * xgap, y, 97, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 9 * xgap, y, 115, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 10 * xgap, y, 116, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 11 * xgap, y, 101, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 12 * xgap, y, 114, color, bgcolor);
		}
		else if (buff_operator_ID == 6) {
			IERG3810_TFTLCD_ShowChar(x + 7 * xgap, y, 77, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 8 * xgap, y, 101, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 9 * xgap, y, 100, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 10 * xgap, y, 105, color, bgcolor);
			IERG3810_TFTLCD_ShowChar(x + 11 * xgap, y, 99, color, bgcolor);
		}
	}
}

void ShowRange(int buff_operator_ID) {
	// x, y, id
	// id 0, 1, 2, 3
	//range 0:platium, 1:Eyjafjalla, 2:nightnigle, 3:other
	if (buff_operator_ID == 0) IERG3810_TFTLCD_ShowRange(5, 61, 0);
	else if (buff_operator_ID == 5) IERG3810_TFTLCD_ShowRange(5, 61, 1);
	else if (buff_operator_ID == 6) IERG3810_TFTLCD_ShowRange(5, 61, 2);
	else IERG3810_TFTLCD_ShowRange(5, 61, 3);
}

void ShowCost(int cost) {
	int x = 170, y = 45;
	int gap = 8;
	u16 bgcolor = 0x0000, color = 0xFFFF;
	int i;

	int first_number = cost / 10; //e.g 1 in "15"
	int latter_number = cost % 10; //e.g 5 in "15"

	//print "cost:"
	IERG3810_TFTLCD_ShowChar(x, y, 67, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + gap, y, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 2 * gap, y, 115, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 3 * gap, y, 116, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 4 * gap, y, 58, color, bgcolor);

	for (i = 0; i <= 9; i++) {
		if (first_number == i) IERG3810_TFTLCD_ShowChar(x + 5 * gap, y, 48 + i, color, bgcolor);
		if (latter_number == i) IERG3810_TFTLCD_ShowChar(x + 6 * gap, y, 48 + i, color, bgcolor);
	}
}

void ShowOperatorcanuse(int operator_can_place) {
	int x = 170, y = 61;
	int gap = 8;
	u16 bgcolor = 0x0000, color = 0xFFFF;
	int i;

	int number = operator_can_place % 10;

	IERG3810_TFTLCD_ShowChinChar(x, y, 5, color, bgcolor);
	IERG3810_TFTLCD_ShowChinChar(x + 16, y, 6, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 16 * 2, y, 58, color, bgcolor); //":"

	for (i = 0; i <= 9; i++) {
		if (number == i) IERG3810_TFTLCD_ShowChar(x + 16 * 2 + 8, y, 48 + i, color, bgcolor);
	}
}

// rule
void Intro() {
	int x = 5, y = 264;
	int gap = 8;
	u16 bgcolor = 0x0000, color = 0xCE79;

	// 1. Choose Operator (0 - 5)
	IERG3810_TFTLCD_ShowChar(x, y, 49, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + gap, y, 46, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 3 * gap, y, 67, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 4 * gap, y, 104, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 5 * gap, y, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 6 * gap, y, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 7 * gap, y, 115, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 8 * gap, y, 101, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 10 * gap, y, 79, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 11 * gap, y, 112, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 12 * gap, y, 101, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 13 * gap, y, 114, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 14 * gap, y, 97, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 15 * gap, y, 116, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 16 * gap, y, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 17 * gap, y, 114, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 19 * gap, y, 40, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 20 * gap, y, 48, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 22 * gap, y, 45, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 24 * gap, y, 54, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 25 * gap, y, 41, color, bgcolor);

	// IERG3810_TFTLCD_ShowChar(x + 26 * gap, y, 44, color, bgcolor);
	// IERG3810_TFTLCD_ShowChar(x + 28 * gap, y, 43, color, bgcolor);

	// 2. Choose Row (0 - 2)
	IERG3810_TFTLCD_ShowChar(x, y - 16, 50, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + gap, y - 16, 46, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 3 * gap, y - 16, 67, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 4 * gap, y - 16, 104, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 5 * gap, y - 16, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 6 * gap, y - 16, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 7 * gap, y - 16, 115, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 8 * gap, y - 16, 101, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 10 * gap, y - 16, 82, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 11 * gap, y - 16, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 12 * gap, y - 16, 119, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 14 * gap, y - 16, 40, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 15 * gap, y - 16, 48, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 17 * gap, y - 16, 45, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 19 * gap, y - 16, 50, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 20 * gap, y - 16, 41, color, bgcolor);

	// IERG3810_TFTLCD_ShowChar(x + 21 * gap, y - 16, 44, color, bgcolor);
	// IERG3810_TFTLCD_ShowChar(x + 23 * gap, y - 16, 43, color, bgcolor);

	// 3. Choose Column (1 - 5)
	IERG3810_TFTLCD_ShowChar(x, y - 16 * 2, 51, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + gap, y - 16 * 2, 46, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 3 * gap, y - 16 * 2, 67, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 4 * gap, y - 16 * 2, 104, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 5 * gap, y - 16 * 2, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 6 * gap, y - 16 * 2, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 7 * gap, y - 16 * 2, 115, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 8 * gap, y - 16 * 2, 101, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 10 * gap, y - 16 * 2, 67, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 11 * gap, y - 16 * 2, 111, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 12 * gap, y - 16 * 2, 108, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 13 * gap, y - 16 * 2, 117, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 14 * gap, y - 16 * 2, 109, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 15 * gap, y - 16 * 2, 110, color, bgcolor);

	IERG3810_TFTLCD_ShowChar(x + 17 * gap, y - 16 * 2, 40, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 18 * gap, y - 16 * 2, 49, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 20 * gap, y - 16 * 2, 45, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 22 * gap, y - 16 * 2, 53, color, bgcolor);
	IERG3810_TFTLCD_ShowChar(x + 23 * gap, y - 16 * 2, 41, color, bgcolor);

	// IERG3810_TFTLCD_ShowChar(x + 24 * gap, y - 16 * 2, 44, color, bgcolor);
	// IERG3810_TFTLCD_ShowChar(x + 26 * gap, y - 16 * 2, 43, color, bgcolor);
}

void Put_Operator(u16 x, u16 y, u8 id) {
	u32 j;
	u32 height = Image.height, length = Image.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		if (image_data_Image[id][j] == 0x0000)  IERG3810_TFTLCD_WrData(road[j]);
		else IERG3810_TFTLCD_WrData(image_data_Image[id][j]);
	}
}

void Change_location(void) {
	int i, j;
	int x = 10, y = 130;

	for (i = 0; i < 3; i++) {
		if (i == buff_arr) break;
		else y += 32;
	}

	for (i = 0; i < 7; i++) {
		if (i == buff_element) break;
		else x += 32;
	}

	Put_Operator(x, y, buff_operator_ID);
	location[buff_operator_ID][0] = buff_arr;
	location[buff_operator_ID][1] = buff_element;

	operator_can_place--;

	cost -= operator_cost[buff_operator_ID];
	if (buff_operator_ID == 2) {
		cost += 12;
		if (cost > 99) cost = 99;
	}
	else if (buff_operator_ID == 3) {
		cost += 25;
		if (cost > 99) cost = 99;
	}

	Change_map_damage(buff_operator_ID, buff_arr, buff_element);

	finish = 0;
	idle[buff_operator_ID] = 0;
	map_idle[buff_arr][buff_element] = 0;
	buff_arr = 999;
	buff_element = 999;
	buff_operator_ID = 999;
}

void remove_map_damage(int buff_operator_ID, int buff_arr, int buff_element) {
	int buff_range;
	int j, i;

	if (buff_operator_ID == 0) {
		buff_range = 3;
		if (buff_arr == 0) {
			for (i = 0; i < 2; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_damage[i][buff_element + j] -= damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 1) {
			for (i = 0; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_damage[i][buff_element + j] -= damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 2) {
			for (i = 1; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_damage[i][buff_element + j] -= damage[buff_operator_ID];
				}
			}
		}
	}
	else if (buff_operator_ID == 5) {
		if (buff_arr == 0) {
			for (j = 0; j <= 3; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[0][buff_element + j] -= damage[buff_operator_ID];
			}

			for (j = 0; j <= 2; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[1][buff_element + j] -= damage[buff_operator_ID];
			}
		}
		if (buff_arr == 1) {
			for (j = 0; j <= 3; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[1][buff_element + j] -= damage[buff_operator_ID];
			}

			for (j = 0; j <= 2; j++) {
				if ((buff_element + j) == 6) break;
				else {
					map_damage[0][buff_element + j] -= damage[buff_operator_ID];
					map_damage[2][buff_element + j] -= damage[buff_operator_ID];
				}
			}
		}

		if (buff_arr == 2) {
			for (j = 0; j <= 3; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[2][buff_element + j] -= damage[buff_operator_ID];
			}

			for (j = 0; j <= 2; j++) {
				if ((buff_element + j) == 6) break;
				else  map_damage[1][buff_element + j] -= damage[buff_operator_ID];
			}
		}
	}

	else if (buff_operator_ID == 6) {
		buff_range = 3;
		if (buff_arr == 0) {
			for (i = 0; i < 2; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_heal[i][buff_element + j - 1] -= damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 1) {
			for (i = 0; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_heal[i][buff_element + j - 1] -= damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 2) {
			for (i = 1; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_heal[i][buff_element + j - 1] -= damage[buff_operator_ID];
				}
			}
		}
	}
	else {
		map_damage[buff_arr][buff_element] += damage[buff_operator_ID];
		if ((buff_element + 1) != 6) map_damage[buff_arr][buff_element + 1] -= damage[buff_operator_ID];
	}
}

//calculate map damage
void Change_map_damage(int buff_operator_ID, int buff_arr, int buff_element) {
	int buff_range;
	int j, i;

	if (buff_operator_ID == 0) {
		buff_range = 3;
		if (buff_arr == 0) {
			for (i = 0; i < 2; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_damage[i][buff_element + j] += damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 1) {
			for (i = 0; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_damage[i][buff_element + j] += damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 2) {
			for (i = 1; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_damage[i][buff_element + j] += damage[buff_operator_ID];
				}
			}
		}
	}
	else if (buff_operator_ID == 5) {
		if (buff_arr == 0) {
			for (j = 0; j <= 3; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[0][buff_element + j] += damage[buff_operator_ID];
			}

			for (j = 0; j <= 2; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[1][buff_element + j] += damage[buff_operator_ID];
			}
		}
		if (buff_arr == 1) {
			for (j = 0; j <= 3; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[1][buff_element + j] += damage[buff_operator_ID];
			}

			for (j = 0; j <= 2; j++) {
				if ((buff_element + j) == 6) break;
				else {
					map_damage[0][buff_element + j] += damage[buff_operator_ID];
					map_damage[2][buff_element + j] += damage[buff_operator_ID];
				}
			}
		}

		if (buff_arr == 2) {
			for (j = 0; j <= 3; j++) {
				if ((buff_element + j) == 6) break;
				else map_damage[2][buff_element + j] += damage[buff_operator_ID];
			}

			for (j = 0; j <= 2; j++) {
				if ((buff_element + j) == 6) break;
				else  map_damage[1][buff_element + j] += damage[buff_operator_ID];
			}
		}
	}

	else if (buff_operator_ID == 6) {
		buff_range = 3;
		if (buff_arr == 0) {
			for (i = 0; i < 2; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_heal[i][buff_element + j - 1] += damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 1) {
			for (i = 0; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_heal[i][buff_element + j - 1] += damage[buff_operator_ID];
				}
			}
		}
		if (buff_arr == 2) {
			for (i = 1; i < 3; i++) {
				for (j = 0; j <= buff_range; j++) {
					if ((buff_element + j) == 6) break;
					else map_heal[i][buff_element + j - 1] += damage[buff_operator_ID];
				}
			}
		}
	}
	else {
		map_damage[buff_arr][buff_element] += damage[buff_operator_ID];
		if ((buff_element + 1) != 6) map_damage[buff_arr][buff_element + 1] += damage[buff_operator_ID];
	}
}

// function for enemy
void Repair_Map(u16 x, u16 y) {
	u32 j;
	u32 height = Image.height, length = Image.width;

	if ((enemy_location[0] + 1 >= 1) && (enemy_location[0] + 1 <= 6)) {
		IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
		IERG3810_TFTLCD_WrData((x + 32) >> 8);
		IERG3810_TFTLCD_WrData((x + 32) & 0xFF);
		IERG3810_TFTLCD_WrData((length + x + 32 - 1) >> 8);
		IERG3810_TFTLCD_WrData((length + x + 32 - 1) & 0xFF);
		IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
		IERG3810_TFTLCD_WrData(y >> 8);
		IERG3810_TFTLCD_WrData(y & 0xFF);
		IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
		IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
		IERG3810_TFTLCD_WrReg(0x2C); //set color

		for (j = 0; j < (length * height); j++) {
			if ((enemy_location[0] + 1) == 6) {
				IERG3810_TFTLCD_WrData(red[j]);
			}
			if ((enemy_location[0] + 1 >= 1) && (enemy_location[0] + 1 <= 5)) {
				IERG3810_TFTLCD_WrData(road[j]);
			}
		}
	}
}

void Put_Enemy(u16 x, u16 y) {
	u32 j;
	u32 height = Image.height, length = Image.width;

	IERG3810_TFTLCD_WrReg(0x2A); //set x pos.
	IERG3810_TFTLCD_WrData(x >> 8);
	IERG3810_TFTLCD_WrData(x & 0xFF);
	IERG3810_TFTLCD_WrData((length + x - 1) >> 8);
	IERG3810_TFTLCD_WrData((length + x - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2B); //set y pos.
	IERG3810_TFTLCD_WrData(y >> 8);
	IERG3810_TFTLCD_WrData(y & 0xFF);
	IERG3810_TFTLCD_WrData((height + y - 1) >> 8);
	IERG3810_TFTLCD_WrData((height + y - 1) & 0xFF);
	IERG3810_TFTLCD_WrReg(0x2C); //set color

	for (j = 0; j < (length * height); j++) {
		if (enemy_location[0] == 6) {
			if (enemies[0][j] == 0x0000) IERG3810_TFTLCD_WrData(red[j]);
			else IERG3810_TFTLCD_WrData(enemies[0][j]);
		}
		else if (enemy_location[0] >= 1 && enemy_location[0] <= 5) {
			if (enemies[0][j] == 0x0000) IERG3810_TFTLCD_WrData(road[j]);
			else IERG3810_TFTLCD_WrData(enemies[0][j]);
		}
	}
	Repair_Map(x, y);
}