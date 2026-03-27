#include <stdint.h>
#include <stdlib.h>
#include "weapon.h"
#include "map.h"
#include "player.h"
#include "../hardware/ps2.h"
#include "../hardware/vga.h"
#include "../sources/knight_anim.h"
#include "../sources/map_texture.h"

// 颜色定义
#define COLOR_MAGENTA_565  0xF81F 

// 显存缓冲区
short int Buffer0[240][512]; 
short int Buffer1[240][512];
int current_buffer = 0;

// 历史位置记录（用于局部擦除或逻辑同步）
int prev_knight_x[2] = {100, 100};
int prev_knight_y[2] = {100, 100};
int prev_mouse_x[2] = {160, 160};
int prev_mouse_y[2] = {120, 120};

int main(void) {
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    // 1. 硬件初始化：设置双缓冲并初次清屏
    *(pixel_ctrl_ptr + 1) = (int) &Buffer0;
    vga_wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
    // 如果有背景图，在此绘制
    if(background_ptr) Draw_Opaque_Image(0, 0, background_ptr, 320, 240);

    *(pixel_ctrl_ptr + 1) = (int) &Buffer1;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();                            
    if(background_ptr) Draw_Opaque_Image(0, 0, background_ptr, 320, 240);

    // 2. 初始化玩家与武器实体
    Player knight_entity;
    Weapon gun_entity;
    
    Player_Init(&knight_entity, 144, 104);
    Weapon_Init(&gun_entity, 0);
    PS2_Init();
    
    Player* knight = &knight_entity;
    Weapon* init_weapon = &gun_entity;

    // 3. 随机种子获取流程
    unsigned int random_seed = 0; 
    while (1) {
        random_seed++; 
        PS2_Update();
        if (key_W || key_A || key_S || key_D) {
            break; 
        }
    }
    srand(random_seed); 

    // 4. 地图初始化
    init_world_map();
    current_room_x = 6;
    current_room_y = 6;
    init_walls();

    //游戏正式主循环
    while(1) {
        // [A] 读取输入
        PS2_Update();

        int dx = 0, dy = 0;
        if (key_W) dy -= 1;
        if (key_S) dy += 1;
        if (key_A) dx -= 1;
        if (key_D) dx += 1;

        // [B] 逻辑更新
        Player_Update(knight, dx, dy);
        Weapon_Update(init_weapon, knight, &mouse);

        // [C] 房间切换处理
        if (handle_room_transition(knight)) {
            // 切换房间后，强制同步两个Buffer的历史坐标，防止擦除错误
            prev_knight_x[0] = prev_knight_x[1] = knight->x;
            prev_knight_y[0] = prev_knight_y[1] = knight->y;
            prev_mouse_x[0] = prev_mouse_x[1] = mouse.x;
            prev_mouse_y[0] = prev_mouse_y[1] = mouse.y;
        }

        // [D] 渲染准备：清屏
        // 注意：性能建议——如果 draw_walls 覆盖全图，不需要 clear_screen
        // 但由于你有黑色背景，clear_screen() 是最稳妥的防残影方法
        Draw_Opaque_Image(0, 0, dungeon, 320, 240);

        // [E] 绘制世界
        draw_walls();
        draw_portals();
        draw_world_map();

        // [F] 绘制实体
        Player_Draw(knight);
        Weapon_Draw(init_weapon, knight);
        Bullet_Draw(init_weapon, current_buffer);

        // 绘制准星 (绿色十字)
        vga_draw_rect(mouse.x, mouse.y + 2, 5, 1, Green);
        vga_draw_rect(mouse.x + 2, mouse.y, 1, 5, Green);

        int bar_width = 30;
        int bar_height = 4;
        int bar_x = knight->x + (knight->width - bar_width) / 2; // 居中
        int bar_y = knight->y - bar_height - 4;                  // 头顶上方留 4 像素间隙
        
        vga_draw_health_bar(bar_x, bar_y, knight->current_hp, knight->max_hp, bar_width, bar_height);

        // [G] 记录当前位置，留给下一帧擦除（如果不用全屏清屏的话）
        prev_knight_x[current_buffer] = knight->x;
        prev_knight_y[current_buffer] = knight->y;
        prev_mouse_x[current_buffer] = mouse.x;
        prev_mouse_y[current_buffer] = mouse.y;

        if(knight -> current_hp > 10){knight -> current_hp --;}// for testing whether draws correct color

        // [H] 硬件同步与缓冲区翻转
        vga_wait_for_vsync(); 
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
        current_buffer = 1 - current_buffer;
    }

    return 0; 
}