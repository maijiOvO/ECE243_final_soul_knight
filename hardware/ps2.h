#ifndef PS2_H
#define PS2_H

#include <stdbool.h>

// 暴露出四个按键的状态，供 main.c 或 player.c 读取
extern bool is_W_pressed;
extern bool is_A_pressed;
extern bool is_S_pressed;
extern bool is_D_pressed;

void ps2_init();
void ps2_update_keys();     // called before the start of new frames

#endif // PS2_H