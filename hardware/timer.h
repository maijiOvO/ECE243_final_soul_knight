#ifndef VGA_H
#define VGA_H

// 颜色宏定义保持不变...
#define COLOR_BLACK  0x0000
#define COLOR_RED    0xF800

void vga_init();
void vga_draw_pixel(int x, int y, short color);
void vga_draw_rect(int x, int y, int width, int height, short color);
void vga_clear_screen();
void vga_wait_for_vsync();

#endif