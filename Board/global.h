#ifndef __GLOBAL
#define __GLOBAL
#include"stm32f10x.h"

// SYSTICK variable
extern u8 costtime;
extern u8 enemy_sponse_time;
extern u8 attack_time;

extern int lose_live;
extern int stage;
extern u8 lives;
extern int killnumber;

extern int buff_operator_ID;
extern int buff_arr;
extern int buff_element;
extern int cost;
extern int operator_can_place;
extern int sponse_number;

//for numpad
extern u32 timeout;
extern u32 ps2key;
extern u32 ps2count;
extern u8 ps2dataReady;
extern int confirm_mode;
extern int finish;

//data for operator
extern int damage[7];
extern int blood[7];
extern int max_blood[7];
extern int operator_cost[7];
extern int idle[7]; // avoid repeat
extern int location[7][2];

//data for map
extern int map_damage[3][7];
extern int map_heal[3][7];
extern int map_idle[3][7];

//data for enemy
extern int enemy_damage;
extern int enemy_blood;
extern int enemy_maxblood;
extern int enemy_location[2];
#endif
