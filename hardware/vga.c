#include"vga.h"
#include"address_map.h"



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

// 专门用于画墙、地板等不透明物体
void Draw_Opaque_Image(int start_x, int start_y, const unsigned char *img_data, int width, int height) {
    uint16_t *pixels = (uint16_t *)img_data;
    for (int y = 0; y < height; y++) {
        // 直接定位到该行的起始显存地址
        volatile uint16_t *dest = (uint16_t *)(pixel_buffer_start + ((start_y + y) << 10) + (start_x << 1));
        for (int x = 0; x < width; x++) {
            *dest++ = pixels[y * width + x]; // 无须判断，直接覆盖
        }
    }
}

void Draw_Transparent_Image(int start_x, int start_y, const unsigned char *img_data, int width, int height) {
    uint16_t *pixels = (uint16_t *)img_data; 
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t current_color = pixels[y * width + x];
            if (current_color != COLOR_MAGENTA_565) {
                plot_pixel(start_x + x, start_y + y, current_color);
            }
        }
    }
}

void Draw_Transparent_Image_horizontal_flip(int start_x, int start_y, const unsigned char *img_data, int width, int height) {
    uint16_t *pixels = (uint16_t *)img_data; 
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t current_color = pixels[y * width + x];
            if (current_color != COLOR_MAGENTA_565) {
                plot_pixel(start_x + width - 1 - x, start_y + y, current_color);
            }
        }
    }
}


void Draw_Transparent_Image_CW90(int start_x, int start_y, const unsigned char *img_data, int width, int height) {
    uint16_t *pixels = (uint16_t *)img_data;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t current_color = pixels[y * width + x];
            
            // 只有非背景色才绘制
            if (current_color != 0xF81F) {
                // 坐标映射：新的 X 是由 Y 决定的，新的 Y 是由 X 决定的
                int new_x = start_x + (height - 1 - y);
                int new_y = start_y + x;
                plot_pixel(new_x, new_y, current_color);
            }
        }
    }
}

void Draw_Transparent_Image_CCW90(int start_x, int start_y, const unsigned char *img_data, int width, int height) {
    uint16_t *pixels = (uint16_t *)img_data;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t current_color = pixels[y * width + x];
            
            if (current_color != 0xF81F) {
                // 坐标映射
                int new_x = start_x + y;
                int new_y = start_y + (width - 1 - x);
                plot_pixel(new_x, new_y, current_color);
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

void vga_draw_rect(int x, int y, int width, int height, short color){

    for(int i = x; i <= x + width; i++){
        for(int j = y; j <= y + height; j++){
            int cur_x = i;
            int cur_y = j;
            if((cur_x >= 0) && (cur_x < 320) && (cur_y >= 0)&&(cur_y < 240)){
                plot_pixel(cur_x, cur_y, color);
            }
        }
    }
}
