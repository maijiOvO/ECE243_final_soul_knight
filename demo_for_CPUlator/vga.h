#ifndef VGA_H
#define VGA_H

#include <stdbool.h>
#include <stdint.h>

//void vga_init();
void plot_pixel(int x, int y, short int color);
void clear_screen();
void vga_draw_rect(int x, int y, int width, int height, short color);
void vga_wait_for_vsync(); 
void Draw_Transparent_Image(int start_x, int start_y, const unsigned char *img_data, int width, int height);
extern volatile int pixel_buffer_start; 

#endif