#ifndef VGA_H
#define VGA_H

#include <stdbool.h>
#include <stdint.h>

#define COLOR_MAGENTA_565  0xF81F 
#define SCREEN_W 320
#define SCREEN_H 240

#define Blue 0x001F
#define Black 0
#define Red 0xF800
#define Pink 0xF81F
#define Green 0x07E0

//void vga_init();
void plot_pixel(int x, int y, short int color);
void clear_screen();
void vga_draw_rect(int x, int y, int width, int height, short color);
void vga_wait_for_vsync(); 
void Draw_Transparent_Image(int start_x, int start_y, const unsigned char *img_data, int width, int height);
void Draw_Transparent_Image_horizontal_flip(int start_x, int start_y, const unsigned char *img_data, int width, int height);
extern volatile int pixel_buffer_start; 

#endif