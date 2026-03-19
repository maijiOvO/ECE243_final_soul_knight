#include "ps2.h"

#define PS2_BASE 0xFF200100

int key_W = 0;
int key_A = 0;
int key_S = 0;
int key_D = 0;

static int is_break = 0; // 内部状态，不需要暴露给外界

void PS2_Init(void) {
    key_W = key_A = key_S = key_D = 0;
    is_break = 0;
}

void PS2_Update(void) {
    volatile int *ps2_ptr = (int *)PS2_BASE;
    int ps2_data, rvalid;

    // 【极其关键：FIFO 队列排空】
    // 玩家的手速可能很快，一帧(16ms)内可能按下了好几个键。
    // 硬件会把这些按键排成队伍(FIFO)。我们必须用 while 循环一次性把队伍读空！
    // 否则按键会有严重的延迟感（按了半天才反应）。
    while (1) {
        ps2_data = *ps2_ptr;
        rvalid = ps2_data & 0x8000;

        if (!rvalid) {
            break; // 队伍空了，没有新按键了，跳出循环
        }

        unsigned char byte = ps2_data & 0xFF;

        if (byte == 0xF0) {
            is_break = 1;
        } else {
            int is_pressed = (is_break == 1) ? 0 : 1;
            
            if (byte == 0x1D) key_W = is_pressed;
            if (byte == 0x1C) key_A = is_pressed;
            if (byte == 0x1B) key_S = is_pressed;
            if (byte == 0x23) key_D = is_pressed;
            
            is_break = 0;
        }
    }
}