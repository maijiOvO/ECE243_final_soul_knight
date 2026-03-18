#include "vga.h"

// 默认的前台地址是 0x08000000
// 我们在 SDRAM 里随便挑一块没用的地方作为后台，比如 0x08040000 (偏移了 256KB)
volatile int * pixel_ctrl_ptr = (volatile int *)0xFF203020;
volatile int * pixel_status_ptr = (volatile int *)0xFF20302C;
volatile int * pixel_back_buffer_ptr = (volatile int *)0xFF203024; 

// 一个全局变量，用来记住我们当前正在哪块内存上画画
int current_buffer_addr = 0x08000000;

void vga_init() {
    // 告诉硬件：后台缓冲区的地址是 0x08040000
    *pixel_back_buffer_ptr = 0x08040000;
    // 初始状态，我们在后台画画
    current_buffer_addr = 0x08040000; 
}

// 你的核心 wait 函数 (也就是执行交换)
void vga_wait_for_vsync() {
    *pixel_ctrl_ptr = 1; // 请求交换前后缓冲
    while (*pixel_status_ptr & 0x01); // 等待垂直同步完成
    
    // 交换完成后，我们的画笔也要换到另一块内存去
    if (current_buffer_addr == 0x08000000) {
        current_buffer_addr = 0x08040000;
    } else {
        current_buffer_addr = 0x08000000;
    }
}

// 修改后的画点函数（注意地址变成了 current_buffer_addr）
void vga_draw_pixel(int x, int y, short color) {
    if (x < 0 || x >= 320 || y < 0 || y >= 240) return; 
    volatile short *pixel_ptr = (volatile short *)(current_buffer_addr + (y << 10) + (x << 1));
    *pixel_ptr = color;
}

// ... clear_screen 和 draw_rect 保持不变，它们会自动调用新的 vga_draw_pixel