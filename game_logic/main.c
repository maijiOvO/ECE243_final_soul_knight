
#include<stdlib.h>
#include"map.h"
#include"player.h"
#include"weapon.h"
#include"../hardware/ps2.h"
#include"../hardware/vga.h"
#define Blue 0x001F
#define Black 0
#define Red 0xF800
#define Pink 0xF81F
#define Green 0x07E0
#define Width 5
#define Height 5

volatile int pixel_buffer_start = 0x08000000; // global variable
short int Buffer0[240][512]; // 240 rows, 512 (320 + padding) columns
short int Buffer1[240][512];
int current_buffer = 0;         // can be 0 or 1 corresponding to current buffer

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    *(pixel_ctrl_ptr + 1) = (int) &Buffer0;

    wait();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                             // clean up two buffers, make sure all black

    int knight_w = 32;
    int knight_h = 32;
	
	int frame_count = 0;

    while (1){
        // the main loop

        vga_draw_rect(116, 116, 32, 32, 0x0000);
		
		Draw_Transparent_Image(100, 100, knight_anim[frame_count], knight_w, knight_h);
		frame_count ++ ;
		
		if(frame_count > 9){frame_count = 0;}
		
		for(int j = 0; j < 12; j ++){
			vga_wait_for_vsync();
		}

        // the main loop
        current_buffer = 1 - current_buffer;

        wait(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}