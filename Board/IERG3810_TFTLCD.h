#include "stm32f10x.h"
#include "global.h"

void IERG3810_TFTLCD_WrReg(u16 regval);
void IERG3810_TFTLCD_WrData(u16 data);
void IERG3810_TFTLCD_SetParameter(void);
void IERG3810_TFTLCD_Init(void);
void IERG3810_TFTLCD_DrawDot(u16 x, u16 y, u16 color);
void IERG3810_TFTLCD_FillRectangular(u16 color, u16 start_x, u16 length_x, u16 start_y, u16 length_y);
void IERG3810_TFTLCD_SevenSegment(u16 color, u16 start_x, u16 start_y, u8 digit);
void IERG3810_TFTLCD_ShowChar(u16 x, u16 y, u8 ascii, u16 color, u16 bgcolor);
void IERG3810_TFTLCD_ShowChinChar(u16 x, u16 y, u8 ascii, u16 color, u16 bgcolor);

// map resources
void IERG3810_TFTLCD_ShowMap(u16 x, u16 y, u16 arr, u16 element);
void IERG3810_TFTLCD_ShowLive(u16 x, u16 y);
void IERG3810_TFTLCD_ShowStage(u16 x, u16 y);
void  ShowMenu(u16 x, u16 y);

// object
void IERG3810_TFTLCD_Showoperator(u16 x, u16 y, u8 id);
void IERG3810_TFTLCD_ShowEnemy(u16 x, u16 y, u8 id);
void IERG3810_TFTLCD_ShowRange(u16 x, u16 y, u8 id);
void Put_Operator(u16 x, u16 y, u8 id);

//show different things
void showlive(u8 live);
void ShowOperator(u8 num_of_operators);
void ShowEnemy(u8 num);
void ShowMap(u8 xgap, u8 ymapstarting);
void ShowKillnumber(int total_number, int killnumber);
void ShowStats(int buff_operator_ID);
void ShowCost(int cost);
void ShowRange(int buff_operator_ID);
void ShowOperatorcanuse(int operator_can_place);
void Intro();
void Change_location(void);

//calculate map damage
void Put_Enemy(u16 x, u16 y);
void Change_map_damage(int buff_operator_ID, int buff_arr, int buff_element);
void remove_map_damage(int buff_operator_ID, int buff_arr, int buff_element);
void Repair_Map(u16 x, u16 y);