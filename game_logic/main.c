#include <stdint.h>
#include "random.h"
#include "weapon.h"
#include "map.h"
#include "player.h"
#include "../hardware/ps2.h"
#include "../hardware/vga.h"
#include "../sources/knight_anim.h"
#include "../sources/map_texture.h"
#include "../sources/status_screen.h"
#include "enemy.h"
#include "skill.h"  // 🌟 新增：让 main 认识你的技能图标

#define COLOR_MAGENTA_565  0xF81F 

short int Buffer0[240][512]; 
short int Buffer1[240][512];
int current_buffer = 0;

int prev_knight_x[2] = {100, 100};
int prev_knight_y[2] = {100, 100};
int prev_mouse_x[2] = {160, 160};
int prev_mouse_y[2] = {120, 120};

Player knight_entity;
Weapon gun_entity;

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
    
    Player_Init(&knight_entity, 144, 104);
    Weapon_Init(&gun_entity, 0);
    //PS2_Init();
    
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
        vga_wait_for_vsync();
    }
    my_srand(random_seed); 

    // 4. 地图初始化
    init_world_map();
    current_room_x = 6;
    current_room_y = 6;
    init_walls();

    Spawn_Enemies_For_Room(current_room_x, current_room_y);

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
        Weapon_Update(init_weapon, knight, &mouse, enemies, num_enemies);

        // 把 0 换成真实的键盘状态 key_J
        Update_All_Enemies(knight->x, knight->y, key_J);     
        
        // 👇👇👇 🌟 新增：游戏失败 (Game Over) 判定 🌟 👇👇👇
        if (knight->current_hp <= 0) {
            Draw_Opaque_Image(0, 0, game_over_ptr, 320, 240); // 画出Game Over图片
            vga_wait_for_vsync(); 
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);       // 翻转缓冲区，显示画面
            while(1); // 死循环卡在这里，游戏结束，不再接收输入和画图
        }

        // [C] 房间切换处理
        if (handle_room_transition(knight)) {
            // 切换房间后，强制同步两个Buffer的历史坐标，防止擦除错误
            prev_knight_x[0] = prev_knight_x[1] = knight->x;
            prev_knight_y[0] = prev_knight_y[1] = knight->y;
            prev_mouse_x[0] = prev_mouse_x[1] = mouse.x;
            prev_mouse_y[0] = prev_mouse_y[1] = mouse.y;
        }

        Draw_Opaque_Image(0, 0, dungeon, 320, 240);

        if (world_map[current_room_x][current_room_y].is_cleared == 0) {
            
            int alive_count = 0;//calculate current number of alive enemies
            for (int i = 0; i < num_enemies; i++) {
                if (enemies[i].alive) {
                    alive_count++;
                }
            }
            if (alive_count == 0) {
                world_map[current_room_x][current_room_y].is_cleared = 1;
                apply_doors_to_walls(); 

                // 👇👇👇 🌟 新增：游戏胜利 (Victory) 判定 🌟 👇👇👇
                // 假设你的 Boss 房间类型标识符是 'B' 
                if (world_map[current_room_x][current_room_y].type == 'B') {
                    Draw_Opaque_Image(0, 0, victory_ptr, 320, 240); // 画出胜利图片
                    vga_wait_for_vsync(); 
                    pixel_buffer_start = *(pixel_ctrl_ptr + 1);     // 翻转缓冲区，显示画面
                    while(1); // 死循环卡在这里，游戏胜利，定格画面
                }

                // 👇👇👇 🌟 新增：房间清理完毕，生成 3 个不重复的随机技能 🌟 👇👇👇
                // 只在普通怪物房 ('M') 生成，打完 Boss ('B') 或在宝箱房 ('C') 绝对不刷技能！
                if (world_map[current_room_x][current_room_y].type == 'M') {
                    int pool[4] = {0, 1, 2, 3}; // 技能池：0移速, 1回血, 2伤害, 3射速
                    
                    // 洗牌算法：把数组打乱
                    for (int k = 0; k < 4; k++) {
                        int swap_idx = my_rand() % 4;
                        int temp = pool[k];
                        pool[k] = pool[swap_idx];
                        pool[swap_idx] = temp;
                    }
                    
                    // 取洗牌后的前 3 个作为本房间的奖励
                    world_map[current_room_x][current_room_y].spawned_skills[0] = pool[0];
                    world_map[current_room_x][current_room_y].spawned_skills[1] = pool[1];
                    world_map[current_room_x][current_room_y].spawned_skills[2] = pool[2];
                    world_map[current_room_x][current_room_y].skills_picked = 0; // 标记为未拾取
                }
            }
        }

        // [E] 绘制世界
        draw_walls();
        draw_portals();
        draw_world_map();

        handle_chest_interaction(knight);
        draw_room_objects();

        // ==========================================
        // 👇👇 🌟 新增：技能绘制与拾取判定 🌟 👇👇
        // ==========================================
        Room *cur_room = &world_map[current_room_x][current_room_y];
        
        // 如果是在普通怪物房，已经被清空了，并且技能还没被捡走
        if (cur_room->type == 'M' && cur_room->is_cleared && cur_room->skills_picked == 0) {
            
            // 三个技能分别放在屏幕中间靠左、正中、靠右的位置 (你可以微调这几个坐标)
            int pos_x[3] = {100, 144, 188}; 
            int pos_y = 104; 

            for (int i = 0; i < 3; i++) {
                int skill_id = cur_room->spawned_skills[i];
                
                // 1. 把技能图标画出来 (假设技能是 32x32，如果大小不对自己改)
                Draw_Transparent_Image(pos_x[i], pos_y, skill_anim[skill_id], 32, 32);

                // 2. 检测骑士是否碰到了这个技能（AABB碰撞检测）
                if (knight->x < pos_x[i] + 32 && knight->x + knight->width > pos_x[i] &&
                    knight->y < pos_y + 32 && knight->y + knight->height > pos_y) {
                    
                    // —— 拾取成功！分配对应的能力强化 ——
                    if (skill_id == 0) {
                        knight->speed += 1; // 技能0：速度变快
                    } 
                    else if (skill_id == 1) {
                        knight->current_hp = knight->max_hp; // 技能1：满血复活
                    } 
                    else if (skill_id == 2) {
                        init_weapon->damage += 2; // 技能2：子弹伤害增加
                    } 
                    else if (skill_id == 3) {
                        init_weapon->fire_rate -= 2; // 技能3：射击频率变高 (数值越小射得越快)
                        if (init_weapon->fire_rate < 2) init_weapon->fire_rate = 2; // 防止射速变负数
                    }

                    // 标记为已拾取！剩下的两个技能立刻消失 (三选一机制生效)
                    cur_room->skills_picked = 1; 
                    break; 
                }
            }
        }

        // [F] 绘制实体
        Player_Draw(knight);
        Weapon_Draw(init_weapon, knight);
        Bullet_Draw(init_weapon, current_buffer);

        Draw_All_Enemies();

        Update_Boss_FX(knight->x, knight->y, knight->width, knight->height); 
        Draw_Boss_FX();

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

        //if(knight -> current_hp > 10){knight -> current_hp --;}// for testing whether draws correct color

        // [H] 硬件同步与缓冲区翻转
        vga_wait_for_vsync(); 
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
        current_buffer = 1 - current_buffer;
    }

    return 0; 
}