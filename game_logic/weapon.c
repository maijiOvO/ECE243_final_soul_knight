#include "weapon.h"
#include "../hardware/vga.h"
#include "../sources/knight_anim.h" // 确保能访问到 weapons 数组
#include "map.h" // 需要调用 can_move_to
#include "../hardware/ps2.h"
#include "../sources/weapon_texture.h"
#include "player.h"

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

static int get_dir_index_16(int dx, int dy) {
    int adx = (dx < 0) ? -dx : dx;
    int ady = (dy < 0) ? -dy : dy;

    if (adx == 0 && ady == 0) return 0;

    // 用斜率近似分成每个象限 4 段，共 16 方向
    // 比例阈值大致对应 22.5° 和 67.5°
    int t1 = adx * 2;
    int t2 = ady * 5;

    if (dx >= 0 && dy <= 0) { // 右上象限
        if (t2 <= adx) return 0;       // 右
        if (ady <= adx) return 1;      // 右上偏右
        if (t1 <= ady) return 3;       // 上
        return 2;                      // 右上偏上
    }
    else if (dx < 0 && dy <= 0) { // 左上象限
        if (t2 <= adx) return 8;       // 左
        if (ady <= adx) return 7;      // 左上偏左
        if (t1 <= ady) return 5;       // 上
        return 6;                      // 左上偏上
    }
    else if (dx < 0 && dy > 0) { // 左下象限
        if (t2 <= adx) return 8;       // 左
        if (ady <= adx) return 9;      // 左下偏左
        if (t1 <= ady) return 11;      // 下
        return 10;                     // 左下偏下
    }
    else { // dx >= 0 && dy > 0, 右下象限
        if (t2 <= adx) return 0;       // 右
        if (ady <= adx) return 15;     // 右下偏右
        if (t1 <= ady) return 13;      // 下
        return 14;                     // 右下偏下
    }
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
    w->current_dir_index = get_dir_index_16(dx, dy);

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
                        
                       if (enemies[e].type == ENEMY_TYPE_NECRO) {
                            // --- 处理 Boss ---
                            if (enemies[e].hp <= 0) {
                                // 1. Boss 死亡：不设为 alive = false，而是切状态 6
                                if (enemies[e].state != 6) {
                                    enemies[e].state = 6;
                                    enemies[e].current_frame = 0;
                                    enemies[e].anim_tick = 0;
                                    enemies[e].dir_x = 0; // 停下脚步
                                    enemies[e].dir_y = 0;
                                }
                            } else {
                                // 2. Boss 受伤：还没死，切状态 5
                                if (enemies[e].state != 6 && enemies[e].state != 5) {
                                    enemies[e].state = 5;
                                    enemies[e].current_frame = 0;
                                    enemies[e].anim_tick = 0;
                                    enemies[e].dir_x = 0; // 停下脚步挨打
                                    enemies[e].dir_y = 0;
                                }
                            }
                        } else {
                            // --- 处理其他普通怪物 ---
                            if (enemies[e].hp <= 0) {
                                // 普通怪物血扣光了直接死亡消失
                                enemies[e].alive = false;
                            }
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