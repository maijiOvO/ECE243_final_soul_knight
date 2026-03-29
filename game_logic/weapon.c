#include "weapon.h"
#include "../hardware/vga.h"
#include "../sources/knight_anim.h" // 确保能访问到 weapons 数组
#include "map.h" // 需要调用 can_move_to
#include "../hardware/ps2.h"
#include "../sources/weapon_texture.h"
#include "player.h"
#include <math.h> // 仅用于 atan2f 决定武器贴图

//  quick sqrt
uint32_t isqrt(uint32_t n) {
    uint32_t root = 0, bit = 1 << 30;
    while (bit > n) bit >>= 2;
    while (bit != 0) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else root >>= 1;
        bit >>= 2;
    }
    return root;
}

void Bullet_Erase(Weapon *w, int buffer_id) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        // 只要上次在这个 buffer 画过，就得擦掉
        vga_draw_rect(w->bullets[i].prev_x[buffer_id], 
                      w->bullets[i].prev_y[buffer_id], 
                      10, 10, Black);
    }
}

bool check_overlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    if (x1 + w1 <= x2) return false;
    if (x2 + w2 <= x1) return false;
    if (y1 + h1 <= y2) return false;
    if (y2 + h2 <= y1) return false;
    return true;
}

void Weapon_Update(Weapon *w, Player *p, MouseState *m, Enemy *enemies, int enemy_count) {
    // 1. 视觉效果：决定武器贴图索引 (每帧执行一次 atan2f 性能开销极小)
    int dx = m->x - (p->x + 16);
    int dy = m->y - (p->y + 16);
    float angle = atan2f((float)dy, (float)dx);
    if (angle < 0) angle += 6.283185f; // 2*PI
    w->current_dir_index = (int)((angle / 6.283185f) * 16.0f + 0.5f) % 16;

    // 2. 射击逻辑：高精度向量计算
    if (w->cooldown_timer > 0) w->cooldown_timer--;
    
    if (m->left_click && w->cooldown_timer == 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!w->bullets[i].active) {
                // 计算距离 L
                uint32_t dist_sq = dx*dx + dy*dy;
                if (dist_sq == 0) dist_sq = 1;
                uint32_t L = isqrt(dist_sq);

                w->bullets[i].damage = w->damage;// give the damage to bullet

                // 发射子弹：Vx = (speed * dx * 256) / L
                w->bullets[i].active = true;
                w->bullets[i].vx_fp = (w->bullet_speed * dx << FP_SHIFT) / (int)L;
                w->bullets[i].vy_fp = (w->bullet_speed * dy << FP_SHIFT) / (int)L;
                
                // 初始位置设定 (玩家中心)
                w->bullets[i].x_fp = (p->x + 16) << FP_SHIFT;
                w->bullets[i].y_fp = (p->y + 16) << FP_SHIFT;
                
                w->cooldown_timer = w->fire_rate;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (w->bullets[i].active) {
            w->bullets[i].x_fp += w->bullets[i].vx_fp;
            w->bullets[i].y_fp += w->bullets[i].vy_fp;

            // 定点数转回真实像素坐标
            int real_x = w->bullets[i].x_fp >> FP_SHIFT;
            int real_y = w->bullets[i].y_fp >> FP_SHIFT;

            // A. 查环境（撞墙检测）
            // 假设子弹是一个 4x4 的小方块
            if (!can_move_to(real_x, real_y, 4, 4) || 
                real_x <= 0 || real_x >= 320 || real_y <= 0 || real_y >= 240) {
                w->bullets[i].active = false; // 撞墙碎裂
                continue; // 这颗子弹没了，直接跳过后面的怪物检测
            }

            // B. 查实体（打怪检测）
            bool hit_enemy = false;
            for (int e = 0; e < enemy_count; e++) {
                if (enemies[e].alive) {
                    // 假设怪物的物理碰撞箱稍微比贴图小一点，手感更好
                    // 这里假设怪物也是 32x32，碰撞箱取中心 24x24
                    int enemy_hit_x = enemies[e].x + 4;
                    int enemy_hit_y = enemies[e].y + 4;
                    int enemy_hit_w = 24;
                    int enemy_hit_h = 24;

                    // AABB 碰撞判定！
                    if (check_overlap(real_x, real_y, 4, 4, enemy_hit_x, enemy_hit_y, enemy_hit_w, enemy_hit_h)) {
                        // 命中！
                        hit_enemy = true;
                        
                        //伤害结算
                        enemies[e].hp -= w->bullets[i].damage;
                        
                        //受击反馈标记 (让怪物在绘制时闪烁白光或红光)
                        //enemies[e].hit_timer = 5; // 闪烁 5 帧
                        
                        //死亡判定
                        if (enemies[e].hp <= 0) {
                            enemies[e].alive = false;
                            // 这里未来可以触发金币掉落或爆炸特效
                        }
                        
                        break; // 假设子弹不能穿透，打中一只怪就停止遍历
                    }
                }
            }

            // 如果打中怪了，子弹也要销毁
            if (hit_enemy) {
                w->bullets[i].active = false;
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
    // 1. 获取鼠标相对于玩家中心的偏移量，判断在左还是在右
    // 注意：这里的 mouse 是 ps2.h 里的全局变量
    int dx = mouse.x - (p->x + 16); 

    if (dx < 0) {
        // --- 情况 A：鼠标在左侧 ---
        // 1. 计算右侧对应的镜像贴图索引
        int mirrored_index = (8 - w->current_dir_index + 16) % 16;
        
        // 2. 调用水平翻转函数
        // 注意：如果你发现位置有轻微偏移，可以在 p->x 基础上加减 1-2 像素微调
        Draw_Transparent_Image_horizontal_flip(p->x, p->y, rotated_weapons[mirrored_index], 32, 32);
    } 
    else {
        // --- 情况 B：鼠标在右侧 ---
        // 直接按当前索引正常绘制
        Draw_Transparent_Image(p->x, p->y, rotated_weapons[w->current_dir_index], 32, 32);
    }
}
void Bullet_Draw(Weapon *w, int buffer_id) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (w->bullets[i].active) {
            int rx = w->bullets[i].x_fp >> FP_SHIFT;
            int ry = w->bullets[i].y_fp >> FP_SHIFT;
            
            // 画出黄色 3x3 子弹
            vga_draw_rect(rx, ry, 3, 3, 0xFFE0);
            
            // 记录当前坐标供双缓冲擦除使用
            w->bullets[i].prev_x[buffer_id] = rx;
            w->bullets[i].prev_y[buffer_id] = ry;
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