#include "stm32f10x.h"
#include "IERG3810_Buzzer.h"
#include "IERG3810_KEY.h"
#include "IERG3810_LED.h"
#include "IERG3810_TFTLCD.h"
#include "IERG3810_Clock.h"
#include "IERG3810_USART.h"
#include "Counter.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>

// SYSTICK variable
u8 costtime = 0;
u8 enemy_sponse_time = 0;
u8 attack_time = 0;

int lose_live = 0;
int stage = 0;
u8 lives = 3; 
int killnumber = 0;
int buff_arr = 999;
int buff_element = 999;
int buff_operator_ID = 999;
int cost = 15;
int operator_can_place = 5;
int sponse_number = 0;

//for numpad
u32 timeout = 10000;
u32 ps2key = 0;
u32 ps2count = 0;
u8 ps2dataReady = 0;
int confirm_mode = 0;
int finish = 0;

//data for operator
//id 0:platinum, 1:mudrock, 2:Texa, 3:Bagpipe, 4:skadi, 5:Eyjafjalla, 6:nightnigle
int max_blood[7] = { 1550, 3928, 1950, 2484, 3866, 1743, 1705 };
int blood[7] = { 1550, 3928, 1950, 2484, 3866, 1743, 1705 };
int damage[7] = { 1160, 882, 1100, 1150, 1200, 1100, 350 };
int operator_cost[7] = { 11, 32, 11, 11, 17, 19, 16 };
int idle[7] = { 1, 1, 1, 1, 1, 1, 1 }; // avoid repeat
int location[7][2] = { {-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1} }; // 1:x, 2:y

//data for map
int map_damage[3][7] = { {0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0} }; //1:y, 2:x
int map_heal[3][7] = { {0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0} }; //1:y, 2:x
int map_idle[3][7] = { {1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1} }; //1:y, 2:x

//data for enemy
int enemy_location[2] = { -1,-1 }; // 1:x, 2:y
int enemy_damage = 1500;
int enemy_blood = 3000;
int enemy_maxblood = 3000;

void U16toStr(char* st, int value)
{
	st[0] = value / 10000000 + 48;
	st[1] = value / 1000000 % 10 + 48;
	st[2] = value / 100000 % 10 + 48;
	st[3] = value / 10000 % 10 + 48;
	st[4] = value / 1000 % 10 + 48;
	st[5] = value / 100 % 10 + 48;
	st[6] = value / 10 % 10 + 48;
	st[7] = value % 10 + 48;
}

void Delay(u32 count)
{
	u32 i;
	for (i = 0; i < count; i++);
}

typedef struct {
	u16 LCD_REG;
	u16 LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE   ((u32) (0x6C000000 | 0x000007FE))
#define LCD        ((LCD_TypeDef  *) LCD_BASE)

void Confirm_Location(void) {
	if (ps2count >= 11) {
		if (((ps2key == 0x79 && ps2dataReady != 0xF0) || (ps2key == 0x79 && ps2dataReady != 0x79)) && operator_can_place > 0) {
			if (confirm_mode == 0) {
				confirm_mode = 1;
				USART_print(1, "operator ok\n");
			}
			else if (confirm_mode == 1) {
				confirm_mode = 2;
				USART_print(1, "arr ok\n");
			}
			else if (confirm_mode == 2) {
				confirm_mode = 0;
				USART_print(1, "element ok\n");
				finish = 1;
				if (buff_operator_ID != 999 && buff_arr != 999 && buff_element != 999 && finish == 1)
					if ((map_idle[buff_arr][buff_element] == 1) && (cost >= operator_cost[buff_operator_ID]) && (idle[buff_operator_ID] == 1))
						Change_location();
			}
			Delay(50000);
		}
		//choose operator ID 0-6
		if (confirm_mode == 0) {
			if (ps2key == 0x70) {
				USART_print(1, "\nplatinum\n");
				buff_operator_ID = 0;
				ShowStats(buff_operator_ID);
				ShowRange(buff_operator_ID);
			}
			else if (ps2key == 0x69) {
				USART_print(1, "\nmudrock\n");
				buff_operator_ID = 1;
				ShowStats(buff_operator_ID);
				ShowRange(buff_operator_ID);
			}
			else if (ps2key == 0x72) {
				USART_print(1, "\nTexa\n");
				buff_operator_ID = 2;
				ShowStats(buff_operator_ID);
				ShowRange(buff_operator_ID);
			}
			else if (ps2key == 0x7A) {
				USART_print(1, "\nBagpipe\n");
				buff_operator_ID = 3;
				ShowStats(buff_operator_ID);
				ShowRange(buff_operator_ID);
			}
			else if (ps2key == 0x6B) {
				USART_print(1, "\nskadi\n");
				buff_operator_ID = 4;
				ShowStats(buff_operator_ID);
				ShowRange(buff_operator_ID);
			}
			else if (ps2key == 0x73) {
				USART_print(1, "\nEyjafjalla\n");
				buff_operator_ID = 5;
				ShowStats(buff_operator_ID);
				ShowRange(buff_operator_ID);
			}
			else if (ps2key == 0x74) {
				USART_print(1, "\nnightnigle\n");
				buff_operator_ID = 6;
				ShowStats(buff_operator_ID);
				ShowRange(buff_operator_ID);
			}
			Delay(50000);
		}

		//choose map arr ID
		else if (confirm_mode == 1) {
			if (ps2key == 0x70) {
				buff_arr = 0;
				USART_print(1, "\nx = 0\n");
			}
			else if (ps2key == 0x69) {
				buff_arr = 1;
				USART_print(1, "\nx = 1\n");
			}
			else if (ps2key == 0x72) {
				buff_arr = 2;
				USART_print(1, "\nx = 2\n");
			}
			Delay(50000);
		}

		//choose map element ID
		else if (confirm_mode == 2) {
			if (ps2key == 0x69) {
				buff_element = 1;
				USART_print(1, "\ny = 1\n");
			}
			else if (ps2key == 0x72) {
				buff_element = 2;
				USART_print(1, "\ny = 2\n");
			}
			else if (ps2key == 0x7A) {
				buff_element = 3;
				USART_print(1, "\ny = 3\n");
			}
			else if (ps2key == 0x6B) {
				buff_element = 4;
				USART_print(1, "\ny = 4\n");
			}
			else if (ps2key == 0x73) {
				buff_element = 5;
				USART_print(1, "\ny = 5\n");
			}
			Delay(50000);
		}
		ps2dataReady = ps2key;
		ps2count = 0;
		ps2key = 0;
		EXTI->PR = 1 << 11;
	}
}

void IERG3810_SYSTICK_Init10ms(void) {
	SysTick->CTRL = 0;
	SysTick->LOAD = 89999; // (72MHz/8)*10ms-1
	SysTick->CTRL |= 0x00000003; // TICKINT = 1, ENABLE = 1
}

void Init(void) {
	int j, i;
	// SYSTICK variable
	costtime = 0;
	enemy_sponse_time = 0;
	attack_time = 0;

	lose_live = 0;
	lives = 3;
	killnumber = 0;
	buff_arr = 999;
	buff_element = 999;
	buff_operator_ID = 999;
	cost = 15;
	operator_can_place = 5;
	sponse_number = 0;

	//for numpad
	timeout = 10000;
	ps2key = 0;
	ps2count = 0;
	ps2dataReady = 0;
	confirm_mode = 0;
	finish = 0;

	//data for operator
	//id 0:platinum, 1:mudrock, 2:Texa, 3:Bagpipe, 4:skadi, 5:Eyjafjalla, 6:nightnigle
	for (i = 0; i < 7; i++) {
		blood[i] = max_blood[i];
		idle[i] = 1;
	}

	for (i = 0; i < 7; i++) {
		for (j = 0; j < 2; j++) {
			location[i][j] = -1;
		}
	}

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 7; j++) {
			//data for map
			map_damage[i][j] = 0; //1:y, 2:x
			map_heal[i][j] = 0; //1:y, 2:x
			map_idle[i][j] = 1; //1:y, 2:x
		}
	}

	//data for enemy
	enemy_location[0] = -1; // x
	enemy_location[1] = -1; // y
	enemy_damage = 1500;
	enemy_blood = 3000;
	enemy_maxblood = 3000;
}

void print_stats(void) {
	USART_print(1, "\n\r[Amiya] Doctor, we don't have a lot of time. Try your best to escape here! \n\r");
	USART_print(1, "\n\r[Amiya] You need to keep your health not too low, and kill the enemies as much as you can! \n\r");
	USART_print(1, "\n\r~~~~~~Gameplay~~~~~~ \n\r");
	USART_print(1, "\n\r1. Choose your Operators to fight (ID: 0 - 6)\n\r");
	USART_print(1, "\n\r2. Choose the Row you wanna put on (0 - 2)\n\r");
	USART_print(1, "\n\r3. Choose the Column you wanna put on (1 - 5)\n\r");
	USART_print(1, "\n\r** Press '+' for every steps you've complete! ** \n\r");
	USART_print(1, "\n\r+=====[Stats]======+ \n\r");
	USART_print(1, "\n\r{0} Platinum    [11] \n\r");
	USART_print(1, "\n\r{1} Mudrock     [32] \n\r");
	USART_print(1, "\n\r{2} Texa        [11] \n\r");
	USART_print(1, "\n\r{3} Wind        [11] \n\r");
	USART_print(1, "\n\r{4} Skadi       [17] \n\r");
	USART_print(1, "\n\r{5} Eyjafjalla  [19] \n\r");
	USART_print(1, "\n\r{6} Nightnigle  [16] \n\r");
	USART_print(1, "\n\r+==================+ \n\r");
	USART_print(1, "\n\r[Amiya] There's still lots of work that needs to be done, Doctor. You can't rest now! \n\r");
}

void print_name(u16 textcolor, u16 bgcolor) {
	u16 textcolor2 = 0xCE79;
	// 1155142373
	IERG3810_TFTLCD_ShowChar(60, 2, 49, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 1, 2, 49, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 2, 2, 53, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 3, 2, 53, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 4, 2, 49, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 5, 2, 52, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 6, 2, 50, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 7, 2, 51, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 8, 2, 55, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 9, 2, 51, textcolor, bgcolor);

	// 1155144098
	IERG3810_TFTLCD_ShowChar(60, 20, 49, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 1, 20, 49, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 2, 20, 53, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 3, 20, 53, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 4, 20, 49, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 5, 20, 52, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 6, 20, 52, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 7, 20, 48, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 8, 20, 57, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(60 + 8 * 9, 20, 56, textcolor, bgcolor);

	//my name
	IERG3810_TFTLCD_ShowChinChar(1, 2, 0, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChinChar(20, 2, 1, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChinChar(40, 2, 2, textcolor, bgcolor);
	//partner name
	IERG3810_TFTLCD_ShowChinChar(1, 22, 3, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChinChar(20, 22, 4, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChinChar(40, 22, 2, textcolor, bgcolor);

	//> Press KEY0 to continue <
	IERG3810_TFTLCD_ShowChar(16, 77, 62, textcolor2, bgcolor);

	IERG3810_TFTLCD_ShowChar(16 + 8 * 2, 77, 80, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 3, 77, 114, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 4, 77, 101, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 5, 77, 115, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 6, 77, 115, textcolor2, bgcolor);

	IERG3810_TFTLCD_ShowChar(16 + 8 * 8, 77, 75, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 9, 77, 69, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 10, 77, 89, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 11, 77, 48, textcolor2, bgcolor);

	IERG3810_TFTLCD_ShowChar(16 + 8 * 13, 77, 116, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 14, 77, 111, textcolor2, bgcolor);

	IERG3810_TFTLCD_ShowChar(16 + 8 * 16, 77, 99, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 17, 77, 111, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 18, 77, 110, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 19, 77, 116, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 20, 77, 105, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 21, 77, 110, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 22, 77, 117, textcolor2, bgcolor);
	IERG3810_TFTLCD_ShowChar(16 + 8 * 23, 77, 101, textcolor2, bgcolor);

	IERG3810_TFTLCD_ShowChar(16 + 8 * 25, 77, 60, textcolor2, bgcolor);

	// GROUP 10 (Y = 300)
	IERG3810_TFTLCD_ShowChar(225 - 8 * 7, 300, 71, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(225 - 8 * 6, 300, 82, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(225 - 8 * 5, 300, 79, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(225 - 8 * 4, 300, 85, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(225 - 8 * 3, 300, 80, textcolor, bgcolor);

	IERG3810_TFTLCD_ShowChar(225 - 8, 300, 49, textcolor, bgcolor);
	IERG3810_TFTLCD_ShowChar(225, 300, 48, textcolor, bgcolor);
}

int main(void)
{
	int a;
	int i, j;
	int x, y;
	u16 value = 0;
	int voltage = 0;
	u8 x_gap;
	u8 y_mapstarting;
	u16 bgcolor = 0x0000;
	int total_number;
	u16 textcolor = 0xFFFF;

	char volt[8] = "-------\0";

	IERG3810_TFTLCD_Init();
	IERG3810_Buzzer_Init();
	IERG3810_clock_tree_init();
	IERG3810_LED_Init();
	IERG3810_USART1_init(72, 9600);
	IERG3810_ADC_Init();

	while (1) {
		if (stage == 0) {
			IERG3810_KEY_Init();
			Delay(1000000);
			IERG3810_TFTLCD_FillRectangular(bgcolor, 0, 240, 0, 320); // background black

			// print intro pics (TFTLCD L.383)
			ShowMenu(0, 95); // 77 + 16 = 93
			print_name(textcolor, bgcolor);
			value = Get_ADC(0);
			voltage = (value * 3.3 / 4096);

			U16toStr(volt, voltage);
			USART_print(1, volt);
			USART_print(1, "\n\r");

			while (1) {
				if ((GPIOE->IDR & 1 << 4) == 0) {
					stage = 1;
					Init();
					x = 10;
					y = 130;
					x_gap = 10;
					y_mapstarting = 130;
					total_number = 5;
					GPIOE->CRL &= 0xFFF4FFFF;
					break;
				}
			}
		}

		if (stage == 1) {
			IERG3810_NVIC_SetPriorityGroup(5);
			IERG3810_PS2key_ExtiInit();
			IERG3810_SYSTICK_Init10ms();
			//Delay(1000000);
			print_stats();
			IERG3810_TFTLCD_FillRectangular(bgcolor, 0, 240, 0, 320); //general background
			IERG3810_TFTLCD_FillRectangular(0xC618, 0, 240, 35, 1); //upper seperate line for operator
			ShowCost(cost);
			ShowMap(x_gap, y_mapstarting);
			ShowOperator(7);
			Intro();

			while (1) {
				Confirm_Location();
				// timeout for numpad
				timeout--;
				if (timeout == 0) {
					timeout = 20000;
					ps2key = 0;
					ps2count = 0;
				}

				showlive(lives);
				ShowKillnumber(total_number, killnumber);
				ShowCost(cost);
				ShowOperatorcanuse(operator_can_place);

				//for producing cost
				if (costtime >= 70) {
					costtime = 0;
					if (cost <= 98) cost++;
				}

				//for sponsing enemy
				if (sponse_number <= total_number && lives >= 0) {
					if (enemy_sponse_time >= 250) {
						enemy_sponse_time = 0;
						a = rand() % 3;

						//for collide
						if (enemy_location[0] != -1 && enemy_location[1] != -1) {
							for (i = 0; i < 7; i++) {
								if ((location[i][0] == (enemy_location[0] - 1)) && (location[i][1] == (enemy_location[1] - 1))) {
									if (blood[i] <= enemy_damage) {
										remove_map_damage(i, location[i][0], location[i][1]);
										map_idle[location[i][1]][location[i][0]] = 1;

										location[i][0] = -1;
										location[i][1] = -1;
										blood[i] = 0;
									}
									else {
										enemy_blood = 0;
										blood[i] -= enemy_damage;
									}
								}
							}
						}

						//check enemy blood
						if (enemy_blood <= 0 && enemy_location[0] != 0) {
							if (map_idle[enemy_location[1]][enemy_location[0]] == 1)IERG3810_TFTLCD_ShowMap(x_gap + 32 * enemy_location[0], y_mapstarting + 32 * enemy_location[1], enemy_location[1], enemy_location[0]);
							killnumber++;
							enemy_location[0] = -1;
							enemy_location[1] = -1;
						}

						//logic for enemy movement
						if ((enemy_location[0] <= 6) && (enemy_location[0] > 0)) enemy_location[0]--;
						else if (enemy_location[0] == 0) {
							lives--;
							GPIOB->BSRR = 1 << 21;
							GPIOB->BSRR = 1 << 8;
							Delay(100000);
							GPIOB->BSRR = 1 << 5;
							GPIOB->BSRR = 1 << 24;
							lose_live++;
							IERG3810_TFTLCD_ShowMap(x_gap + 32 * (enemy_location[0] + 1), y_mapstarting + 32 * enemy_location[1], enemy_location[1], enemy_location[0] + 1);
							enemy_location[0] = 6;
							enemy_location[1] = a; // y
							sponse_number++;
							enemy_blood = enemy_maxblood;
						}
						else {
							enemy_location[0] = 6; // x
							enemy_location[1] = a; // y
							sponse_number++;
							enemy_blood = enemy_maxblood;
						}

						//locate x,y
						if (enemy_location[0] > 0 && enemy_location[0] < 7) {
							for (i = 0; i < 3; i++) {
								if (i == enemy_location[1]) break;
								else y += 32;
							}

							for (i = 0; i < 7; i++) {
								if (i == enemy_location[0]) break;
								else x += 32;
							}
							Put_Enemy(x, y);
							x = 10;
							y = 130;
						}
					}
				}
				//for attack and heal 
				//change difficulty (10->30)
				if (attack_time >= 30) {
					attack_time = 0;
					if ((map_damage[enemy_location[1]][enemy_location[0]] > 0) && (enemy_location[1] != -1 && enemy_location[0] != -1)) {
						if ((enemy_blood - map_damage[enemy_location[1]][enemy_location[0]]) <= 0) enemy_blood = 0;
						else enemy_blood -= map_damage[enemy_location[1]][enemy_location[0]];
					}

					for (i = 0; i < 7; i++) {
						if (location[i][0] != -1 && location[i][1] != -1) {
							if (map_heal[location[i][1]][location[i][0]] > 0) {
								if ((map_heal[location[i][1]][location[i][0]] + blood[i]) >= max_blood[i]) blood[i] = max_blood[i];
								else blood[i] += map_heal[location[i][1]][location[i][0]];
							}
						}
					}
				}
				if (lose_live == 3 || (lose_live + killnumber == total_number)) {
					ShowKillnumber(total_number, killnumber);
					if (lose_live == 3)showlive(0);
					stage = 2;
					break;
				}
			}
		}

		if (stage == 2) {
			IERG3810_KEY_Init();
			if (lose_live == 3) {
				IERG3810_TFTLCD_FillRectangular(bgcolor, 0, 240, 0, 320); //general background
				IERG3810_TFTLCD_ShowStage(0, 100);
			}
			else IERG3810_TFTLCD_ShowStage(0, 134);

			while (1) {
				if ((GPIOE->IDR & 1 << 4) == 0) {
					stage = 0;
					GPIOE->ODR |= 1 << 4;
					break;
				}
			}
		}
	}
}