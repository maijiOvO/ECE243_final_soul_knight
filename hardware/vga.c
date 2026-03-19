#include"vga.h"

#define COLOR_MAGENTA_565  0xF81F 

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

void Draw_Transparent_Image(int start_x, int start_y, const unsigned char *img_data, int width, int height) 
{
    // 【核心秘诀】：把 8位的 char 指针，强制当成 16位的 short 指针来用
    uint16_t *pixels = (uint16_t *)img_data; 
    
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            // 从数组中读取当前像素的 16 位颜色
            uint16_t current_color = pixels[y * width + x];
            
            // 【抠图逻辑】：如果颜色不是洋红色，才把它画到屏幕上
            if (current_color != COLOR_MAGENTA_565) 
            {
                plot_pixel(start_x + x, start_y + y, current_color);
            }
            // 如果是洋红色，什么都不做，这就形成了透明！
        }
    }
}