#include"vga.h"
#include"address_map.h"

#define COLOR_MAGENTA_565  0xF81F 
#define SCREEN_W 320
#define SCREEN_H 240

#define Blue 0x001F
#define Black 0
#define Red 0xF800
#define Pink 0xF81F
#define Green 0x07E0

volatile int * pixel_ctrl_ptr = (volatile int *)0xFF203020;
volatile int * pixel_status_ptr = (volatile int *)0xFF20302C;
volatile int * pixel_back_buffer_ptr = (volatile int *)0xFF203024; 
volatile int pixel_buffer_start;


// code not shown for clear_screen() and draw_line() subroutines
void clear_screen() {
    int y, x;
    for (x = 0; x < 320; x++){
        for (y = 0; y < 240; y++){
            plot_pixel (x, y, Black); // Black
        }
    }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


void plot_pixel(int x, int y, short int line_color)
{
    // 1. 边界保护（极为重要！这能拦截掉 90% 的 CPUlator 越界报错）
    if (x < 0 || x >= 320 || y < 0 || y >= 240) {
        return; 
    }

    // 2. 使用纯正的 int 整数计算最终的物理地址（彻底粉碎 C 语言指针瞎乘倍数的企图）
    // 注意：这里的 pixel_buffer_start 必须是普通的 int 类型！
    int pixel_address = pixel_buffer_start + (y << 10) + (x << 1);

    // 3. 将算好的纯整数地址，强制转换为 16位指针，并写入颜色
    *(volatile short int *)pixel_address = line_color;
}

void draw_line(int x0, int y0, int x1, int y1, int color) {
    int delta_y = (y1 - y0 > 0) ? (y1 - y0) : (y0 - y1);
    int delta_x = (x1 - x0 > 0) ? (x1 - x0) : (x0 - x1);
    int is_steep = delta_y > delta_x;
    
    if (is_steep) {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }

    if (x0 > x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    delta_x = x1 - x0;
    delta_y = ((y1 - y0) > 0 ? (y1 - y0) : (y0 - y1));
    int error = -(delta_x / 2);
    int y_step = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, color);
        } else {
            plot_pixel(x, y, color);
        }

        error = error + delta_y;
        if (error > 0) {
            y = y + y_step;
            error = error - delta_x;
        }
    }
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

void vga_wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020; // Buffer Register
    volatile int * pixel_status_ptr = (int *)0xFF20302C; // Status Register
    *pixel_ctrl_ptr = 1;
    while (*pixel_status_ptr & 0x01);
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
