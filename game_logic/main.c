#include <stdint.h>
#include <stdlib.h>
#include"weapon.h"
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

int prev_mouse_x[2] = {160, 160};   // for erasing cursors
int prev_mouse_y[2] = {120, 120};

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
    Weapon gun_entity;
    
    Player_Init(&knight_entity, 144, 104);
    Weapon_Init(&gun_entity, 0);
    PS2_Init();             // init the keys WASD to 0;
    Player* knight = & knight_entity;
    Weapon* init_weapon = &gun_entity;

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
    current_room_x = 6;
    current_room_y = 6;
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
        if (handle_room_transition(knight)) {               // reset the prev array after teleproting
            prev_knight_x[0] = prev_knight_x[1] = knight->x;
            prev_knight_y[0] = prev_knight_y[1] = knight->y;
        } 
        
        // the numbers means the offset due to ewapons, need to clear a greater area
        vga_draw_rect(prev_knight_x[current_buffer] - 5, prev_knight_y[current_buffer] - 5, knight->width + 10, knight->height + 10, Black);
        vga_draw_rect(prev_mouse_x[current_buffer], prev_mouse_y[current_buffer], 5, 5, Black); // erase old cursor
        Bullet_Erase(init_weapon, current_buffer);                                              // erase old bullet

        Weapon_Update(init_weapon, knight, &mouse);

        draw_walls();
        draw_world_map();

        Player_Draw(knight);
        Weapon_Draw(init_weapon, knight);
        Bullet_Draw(init_weapon, current_buffer);

        vga_draw_rect(mouse.x, mouse.y + 2, 5, 1, Green);// draw the cursor
        vga_draw_rect(mouse.x + 2, mouse.y, 1, 5, Green);// draw the cursor

        // update prev corrds
        prev_knight_x[current_buffer] = knight->x;
        prev_knight_y[current_buffer] = knight->y;

        prev_mouse_x[current_buffer] = mouse.x;
        prev_mouse_y[current_buffer] = mouse.y;

        vga_wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        current_buffer = 1 - current_buffer;
	}
    return 0; 
}