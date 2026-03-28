#include "ps2.h"

#define PS2_KBD_BASE   0xFF200100
#define PS2_MOUSE_BASE 0xFF200108

int key_W = 0, key_A = 0, key_S = 0, key_D = 0;
int key_J = 0;

MouseState mouse = {160, 120, 0}; // 初始位置屏幕中央

static int kbd_break = 0;
static int mouse_phase = 0;
static unsigned char mouse_packet[3];

void PS2_Init(void) {
    volatile int *mouse_ptr = (int *)PS2_MOUSE_BASE;
    // 发送使能指令 0xF4 让鼠标开始工作
    *mouse_ptr = 0xF4; 
}

void PS2_Update(void) {
    // --- 1. 处理键盘 (Port 0) ---
    volatile int *kbd_ptr = (int *)PS2_KBD_BASE;
    while (1) {
        int data = *kbd_ptr;
        if (!(data & 0x8000)) break;
        unsigned char b = data & 0xFF;
        if (b == 0xF0) kbd_break = 1;
        else {
            int state = kbd_break ? 0 : 1;
            if (b == 0x1D) key_W = state;
            if (b == 0x1C) key_A = state;
            if (b == 0x1B) key_S = state;
            if (b == 0x23) key_D = state;
            if (b == 0x3B) key_J = state;
            kbd_break = 0;
        }
    }

    // --- 2. 处理鼠标 (Port 1) ---
    volatile int *mouse_ptr = (int *)PS2_MOUSE_BASE;
    while (1) {
        int data = *mouse_ptr;
        if (!(data & 0x8000)) break;
        unsigned char b = data & 0xFF;

        // 同步判断：第一包的 Bit 3 必须为 1
        if (mouse_phase == 0 && !(b & 0x08)) continue;

        mouse_packet[mouse_phase++] = b;

        if (mouse_phase == 3) {
            mouse_phase = 0;
            mouse.left_click = mouse_packet[0] & 0x01;
            // 补码转换位移
            signed char dx = (signed char)mouse_packet[1];
            signed char dy = (signed char)mouse_packet[2];
            mouse.x += dx;
            mouse.y += dy; // 屏幕 Y 轴向下，PS/2 向上为正

            // 边界限制
            if (mouse.x < 0) mouse.x = 0; if (mouse.x > 319) mouse.x = 319;
            if (mouse.y < 0) mouse.y = 0; if (mouse.y > 239) mouse.y = 239;
        }
    }
}