#include "weapon.h"
#include "../hardware/vga.h"
#include "../sources/knight_anim.h" // 确保能访问到 weapons 数组
#include "map.h" // 需要调用 can_move_to
#include "../hardware/ps2.h"
#include "player.h"

void Bullet_Erase(Weapon *w, int buffer_id) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (w->bullets[i].active) {
            // 用黑色方块覆盖该子弹在这个 Buffer 上的旧位置
            // 假设子弹大小是 4x4
            vga_draw_rect(w->bullets[i].prev_x[buffer_id], 
                          w->bullets[i].prev_y[buffer_id], 
                          10, 10, Black);
        }
    }
}
void Weapon_Update(Weapon *w, Player *p, MouseState *m) {
    // 1. 处理射击冷却
    if (w->cooldown_timer > 0) w->cooldown_timer--;

    // 2. 尝试发射子弹 (玩家按下左键且冷却完毕)
    if (m->left_click && w->cooldown_timer == 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!w->bullets[i].active) {
                w->bullets[i].active = true;
                if(p->dir == 2){
                    w->bullets[i].x = p->x + 0; // 从角色中心发射
                    w->bullets[i].y = p->y + 12;
                    w->bullets[i].vx = -w->bullet_speed;
                    w->bullets[i].vy = 0;
                }
                else{
                    w->bullets[i].x = p->x + 32; // 从角色中心发射
                    w->bullets[i].y = p->y + 12;
                    w->bullets[i].vx = w->bullet_speed;
                    w->bullets[i].vy = 0;
                }

                w->cooldown_timer = w->fire_rate;
                break;
            }
        }
    }

    // 3. 更新所有子弹位置
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (w->bullets[i].active) {
            int nx = w->bullets[i].x + w->bullets[i].vx;
            int ny = w->bullets[i].y + w->bullets[i].vy;

            // 撞墙检测：子弹通常比玩家小，这里假设 4x4
            if (can_move_to(nx, ny, 4, 4)) {
                w->bullets[i].x = nx;
                w->bullets[i].y = ny;
                // 飞出屏幕检测
                if (nx < 0 || nx > 320 || ny < 0 || ny > 240) w->bullets[i].active = false;
            } else {
                w->bullets[i].active = false; // 撞墙消失
            }
        }
    }
}
// 初始化武器属性
void Weapon_Init(Weapon *w, int id) {
    w->id = id;
    w->fire_rate = 10;      // 默认每 0.25 秒一发 (60fps/15)
    w->damage = 3;
    w->is_laser = false;
    w->is_bouncy = false;
    w->bullet_w = 8;
    w->bullet_h = 2;
    w->bullet_speed = 6;
    
    w->cooldown_timer = 0;
    w->width = 32;          // 假设武器贴图也是 32x32
    w->height = 32;
}
// 绘制武器
void Weapon_Draw(Weapon *w, Player *p) {
    // 设置武器相对于玩家坐标中心的偏移
    // 这里的数字需要根据你的贴图实际效果微调
    int offset_x = 4; 
    int offset_y = 0; 

    if (p->dir == 2) {
        // 玩家面朝左 (dir == 2)
        // 武器也要翻转，并出现在玩家坐标的左侧
        Draw_Transparent_Image_horizontal_flip(
            p->x - offset_x, 
            p->y + offset_y, 
            weapons[w->id], 
            w->width, 
            w->height
        );
    } else {
        // 默认/面朝右 (dir == 1)
        Draw_Transparent_Image(
            p->x + offset_x, 
            p->y + offset_y, 
            weapons[w->id], 
            w->width, 
            w->height
        );
    }
}
void Bullet_Draw(Weapon *w, int buffer_id) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (w->bullets[i].active) {
            // 画出当前位置的黄色子弹
            vga_draw_rect(w->bullets[i].x, w->bullets[i].y, 6, 3, 0xFFE0);
            
            // 【重要】：立即记录在这个 Buffer 上的位置，供下次循环回到此 Buffer 时擦除
            w->bullets[i].prev_x[buffer_id] = w->bullets[i].x;
            w->bullets[i].prev_y[buffer_id] = w->bullets[i].y;
        }
    }
}
// 简单的开火冷却逻辑
bool Weapon_Can_Shoot(Weapon *w) {
    if (w->cooldown_timer > 0) {
        w->cooldown_timer--;
        return false;
    }
    return true;
}