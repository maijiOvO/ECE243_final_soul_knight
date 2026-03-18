#include"vga.h"
#include"address_map.h"

volatile int * pixel_ctrl_ptr = (volatile int *)0xFF203020;
volatile int * pixel_status_ptr = (volatile int *)0xFF20302C;
volatile int * pixel_back_buffer_ptr = (volatile int *)0xFF203024; 

int current_buffer_addr = 0x08000000;

void vga_init() {
    *pixel_back_buffer_ptr = 0x08040000;
    current_buffer_addr = 0x08040000; 
}

void vga_draw_pixel(int x, int y, short color) {
    // edge check
    if (x < 0 || x >= 320 || y < 0 || y >= 240) {
        return; 
    }
    volatile short *pixel_ptr = (volatile short *)(FPGA_PIXEL_BUF_BASE + (y << 10) + (x << 1));
    *pixel_ptr = color;
}

void vga_clear_screen() {
    int y, x;
    for (x = 0; x < 320; x++){
        for (y = 0; y < 240; y++){
            plot_pixel (x, y, 0); // Black
        }
    }
}

void vga_draw_rect(int x, int y, int width, int height, short color){
    int half_w = width >> 1;
    int half_h = height >> 1;
    for(int i = (x - half_w); i <= (x + half_w); i++){
        for(int j = (y - half_h); j <= (y + half_h); j++){
            int cur_x = i;
            int cur_y = j;
            if((cur_x >= 0) && (cur_x < 320) && (cur_y >= 0)&&(cur_y < 240)){
                plot_pixel(cur_x, cur_y, color);
            }
        }
    }
}

void vga_wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020; // Buffer Register
    volatile int * pixel_status_ptr = (int *)0xFF20302C; // Status Register
    *pixel_ctrl_ptr = 1;
    while (*pixel_status_ptr & 0x01);
}