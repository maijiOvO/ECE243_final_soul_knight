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
    
    // 1. 简单的软件延时，等待鼠标硬件上电准备就绪 (约几百毫秒)
    for(volatile int i = 0; i < 1000000; i++); 
    
    // 2. 强制发送 0xF4 给鼠标通道，不需要用 while 检查保留位
    *mouse_ptr = 0xF4; 
    
    // 3. 再给一点时间让鼠标响应这个命令
    for(volatile int i = 0; i < 500000; i++); 
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
}