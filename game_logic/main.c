#include <stdint.h>
#include <stdlib.h>
//#include"weapon.h"
#include"map.h"
#include"player.h"
#include"../hardware/ps2.h"
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

int prev_knight_x[2] = {100, 100};
int prev_knight_y[2] = {100, 100};

int main(void){

    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    *(pixel_ctrl_ptr + 1) = (int) &Buffer0;

    vga_wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
    Draw_Transparent_Image(0, 0, background_ptr, 320, 240);

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                            
    Draw_Transparent_Image(0, 0, background_ptr, 320, 240);     //write the opening image to the buffers

    Player knight_entity;
    
    Player_Init(&knight_entity, 144, 104);
    PS2_Init();             // init the keys WASD to 0;
    Player* knight = & knight_entity;

    unsigned int random_seed = 0; // 种子计数器

    // 这里其实可以画一行字："Press WASD to Start"
    // 但就算不画，玩家进来肯定会习惯性按一下键盘

    while (1) {
        random_seed++; // 以 CPU 全速疯狂自增 (每秒几百万次)
        
        PS2_Update();
        // 只要玩家按下了 W, A, S, D 中的任意一个键
        if (key_W || key_A || key_S || key_D) {
            break; // 瞬间打破循环！
        }
    }
    vga_wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                             // clean up two buffers, make sure all black

    // 注入灵魂：用玩家反应时间这一瞬的数字，初始化随机数发生器！
    srand(random_seed); 

    init_world_map();
    init_walls();
    draw_walls();

	while(1){
        PS2_Update();

        int dx = 0, dy = 0;
        if (key_W) dy -= 1;
        if (key_S) dy += 1;
        if (key_A) dx -= 1;
        if (key_D) dx += 1;

        Player_Update(knight, dx, dy);
        if (handle_room_transition(knight)) {
            // 如果发生了传送，必须重置 prev 坐标
            // 否则 vga_draw_rect 会去屏幕另一头擦除一个“幽灵残影”
            prev_knight_x[0] = prev_knight_x[1] = knight->x;
            prev_knight_y[0] = prev_knight_y[1] = knight->y;
        }
        vga_draw_rect(prev_knight_x[current_buffer], prev_knight_y[current_buffer], knight->width, knight->height, Black);

        draw_walls();
        draw_world_map();

        Player_Draw(knight);
        prev_knight_x[current_buffer] = knight->x;
        prev_knight_y[current_buffer] = knight->y;

        vga_wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        current_buffer = 1 - current_buffer;
	}
    return 0; 
}