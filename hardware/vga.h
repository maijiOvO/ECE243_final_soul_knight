// vga.h
#ifndef VGA_H
#define VGA_H

#include <stdbool.h>

// color def
#define COLOR_BLACK  0x0000
#define COLOR_WHITE  0xFFFF
#define COLOR_RED    0xF800
#define COLOR_GREEN  0x07E0
#define COLOR_BLUE   0x001F
#define COLOR_YELLOW 0xFFE0

void vga_init();
void vga_draw_pixel(int x, int y, short color);
void vga_clear_screen();
void vga_draw_rect(int x, int y, int width, int height, short color);
void vga_wait_for_vsync(); 

#endif