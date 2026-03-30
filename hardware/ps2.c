#include "ps2.h"

// 在 ps2.c 或 ps2.h 的顶部修改：

#define PS2_KBD_BASE   0xFF200100
// 🚨 临时修改：因为直接插板子，硬件把鼠标认成了 Port 0
#define PS2_MOUSE_BASE 0xFF200108  // <--- 把末尾的 8 改成 0


int key_W = 0, key_A = 0, key_S = 0, key_D = 0;
int key_J = 0;

MouseState mouse = {160, 120, 0}; // 初始位置屏幕中央

static int kbd_break = 0;
static int mouse_phase = 0;
static unsigned char mouse_packet[3];

void PS2_Init(void) {
    volatile int *mouse_ptr = (int *)PS2_MOUSE_BASE;
    int timeout = 100000; // 设置一个超时计数器
    
    // 如果一直没有写空间，尝试一段时间后就放弃，防止卡死
    while (((*(mouse_ptr + 1)) & 0xFFFF0000) == 0 && timeout > 0) {
        timeout--;
    }
    
    if (timeout > 0) {
        *mouse_ptr = 0xF4; 
    }
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
            
            signed char dx = (signed char)mouse_packet[1];
            signed char dy = (signed char)mouse_packet[2];
            
            mouse.x += dx;
            // 🌟 修改这里：把 += 改成 -=
            // 因为屏幕的 Y 轴是往下增长的，而物理鼠标往前推 dy 是正数
            mouse.y += dy; 

            if (mouse.x < 0) { mouse.x = 0; }
            if (mouse.x > 319) { mouse.x = 319; }
            
            if (mouse.y < 0) { mouse.y = 0; }
            if (mouse.y > 239) { mouse.y = 239; }
        }
    }
}