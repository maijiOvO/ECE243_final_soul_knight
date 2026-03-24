#include <stdint.h>
#include <stdlib.h>
#include"weapon.h"
#include"map.h"
#include"player.h"
#include"../hardware/ps2.h"
//#include"../hardware/timer.h"
#include"../hardware/vga.h"
#include"../sources/knight_anim.h"

#include "map.h"
#include "player.h"
#include "enemy.h"             // 引入幽灵
#include "../hardware/ps2.h"
#include "../hardware/vga.h"
#include "../sources/knight_anim.h"

#define ENEMY_COUNT 8  // ✨ 新增：定义怪物数量为 8
Enemy ghosts[ENEMY_COUNT]; // ✨ 新增：创建 8 只幽灵

#define COLOR_MAGENTA_565  0xF81F  

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

    // 1. 设置双缓冲并初次清屏
    *(pixel_ctrl_ptr + 1) = (int) &Buffer0;
    vga_wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
    Draw_Transparent_Image(0, 0, background_ptr, 320, 240);

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                            
    Draw_Transparent_Image(0, 0, background_ptr, 320, 240);     // 写入开机画面

    // 2. 初始化实体
    Player knight_entity;
    Weapon gun_entity;
    
    Player_Init(&knight_entity, 144, 104);
    Weapon_Init(&gun_entity, 0);
    PS2_Init();             // init the keys WASD to 0;
    Player* knight = & knight_entity;
    Weapon* init_weapon = &gun_entity;

    Enemy ghost;            // ✨ 定义幽灵

    unsigned int random_seed = 0; // 种子计数器

    // 🚦 菜单等待画面：全速自增随机种子
    while (1) {
        random_seed++; 
        PS2_Update();
        if (key_W || key_A || key_S || key_D) {
            break; // 玩家按下 WASD，开始游戏
        }
    }

    vga_wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen(); // 双缓冲全黑清屏

    // 🎲 注入灵魂：随机数发生器
    srand(random_seed); 

    init_world_map();
    current_room_x = 6;
    current_room_y = 6;
    init_walls();
    draw_walls();

    // 👻 召唤第一只幽灵
    Enemy_Init_Random(&ghost);
    
// 🎮 游戏正式主循环
    while(1){
        PS2_Update();

        int dx = 0, dy = 0;
        if (key_W) dy -= 1;
        if (key_S) dy += 1;
        if (key_A) dx -= 1;
        if (key_D) dx += 1;

        Player_Update(knight, dx, dy);
        
        // the numbers means the offset due to ewapons, need to clear a greater area
        vga_draw_rect(prev_knight_x[current_buffer] - 5, prev_knight_y[current_buffer] - 5, knight->width + 10, knight->height + 10, Black);
        vga_draw_rect(prev_mouse_x[current_buffer], prev_mouse_y[current_buffer], 5, 5, Black); // erase old cursor
        Bullet_Erase(init_weapon, current_buffer);                                              // erase old bullet

        Weapon_Update(init_weapon, knight, &mouse);

        // --- 🤖 1. 更新所有幽灵 ---
        for (int i = 0; i < ENEMY_COUNT; i++) {
            Enemy_Update(&ghosts[i]); 
        }

        // 🚪 地图切换房间逻辑
        if (handle_room_transition(knight)) {
            prev_knight_x[0] = prev_knight_x[1] = knight->x;
            prev_knight_y[0] = prev_knight_y[1] = knight->y;

            // ✨【关键判断】：假设我们只让地图坐标为 (6, 7) 的房间刷鬼
            // 你可以改成任意你喜欢的房间组合，或者随机决定
            if (current_room_x == 6 && current_room_y == 7) {
                for (int i = 0; i < ENEMY_COUNT; i++) {
                    Enemy_Init_Random(&ghosts[i]); // 在这个房间刷出 8 只鬼
                }
            } else {
                // 如果不是这个房间，把所有的鬼“杀掉”（alive = 0），它们就不会显示和移动了
                for (int i = 0; i < ENEMY_COUNT; i++) {
                    ghosts[i].alive = 0; 
                }
            }
        }

        // 🪠 2. 擦除上一帧旧图像（骑士和 8 只幽灵）
        vga_draw_rect(prev_knight_x[current_buffer], prev_knight_y[current_buffer], knight->width, knight->height, Black);
        
        for (int i = 0; i < ENEMY_COUNT; i++) {
            // 只要它上一帧活着，或者现在还活着，就得擦除它在当前 buffer 留下的残影
            vga_draw_rect(ghosts[i].prev_x[current_buffer], ghosts[i].prev_y[current_buffer], ghosts[i].width, ghosts[i].height, Black);
        }

        // 🧱 3. 渲染地图
        draw_walls();
        draw_world_map();

        // 🎨 4. 渲染实体（骑士和 8 只幽灵）
        Player_Draw(knight);
        Weapon_Draw(init_weapon, knight);
        Bullet_Draw(init_weapon, current_buffer);

        vga_draw_rect(mouse.x, mouse.y + 2, 5, 1, Green);// draw the cursor
        vga_draw_rect(mouse.x + 2, mouse.y, 1, 5, Green);// draw the cursor

        // update prev corrds
        
        for (int i = 0; i < ENEMY_COUNT; i++) {
            if (ghosts[i].alive) {
                Enemy_Draw(&ghosts[i]); // 只画活着的鬼
            }
        }

        // 📝 5. 记录当前位置，留给下一帧在当前 buffer 擦除
        prev_knight_x[current_buffer] = knight->x;
        prev_knight_y[current_buffer] = knight->y;
        
        for (int i = 0; i < ENEMY_COUNT; i++) {
            ghosts[i].prev_x[current_buffer] = ghosts[i].x;
            ghosts[i].prev_y[current_buffer] = ghosts[i].y;
        }

        prev_mouse_x[current_buffer] = mouse.x;
        prev_mouse_y[current_buffer] = mouse.y;

        vga_wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
        current_buffer = 1 - current_buffer;
    }

    return 0; 
}