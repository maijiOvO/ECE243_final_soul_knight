#include <stdint.h>
//#include"weapon.h"
//#include"map.h"
//#include"../hardware/ps2.h"
//#include"../hardware/timer.h"
#include"../hardware/vga.h"
#include"../sources/knight_anim.h"

// 1. 定义洋红色的 RGB565 值
#define COLOR_MAGENTA_565  0xF81F  

// 2. CPUlator DE1-SoC 默认的 VGA 显存基地址 (请根据你实际 BSP 手册确认，通常是这个)


// 假设你的屏幕分辨率是 320x240

short int Buffer0[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer1[240][512];
int current_buffer = 0;         // can be 0 or 1 corresponding to current buffer



int main(void){

    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    *(pixel_ctrl_ptr + 1) = (int) &Buffer0;

    vga_wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                             // clean up two buffers, make sure all black
    
    int knight_w = 32;
    int knight_h = 32;
	
	int knight_frame_count = 0;
    int render_frame_count = 0;
	while(1){
		
		vga_draw_rect(116, 116, 32, 32, 0xF81F);
		
		Draw_Transparent_Image(100, 100, knight_anim[knight_frame_count], knight_w, knight_h);
        render_frame_count ++;
        if(render_frame_count >5){
            knight_frame_count ++ ;
            render_frame_count = 0;
        }

		if(knight_frame_count > 9){knight_frame_count = 0;}
            current_buffer = 1 - current_buffer;

        vga_wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
	}
    return 0; 
}