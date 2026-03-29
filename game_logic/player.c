#include "player.h"
#include "../hardware/vga.h" // 需要用到画图函数
#include "map.h"
#include "enemy.h" // 🌟 新增：必须引入敌人的头文件，不然系统不认识怪物

// 初始化玩家
void Player_Init(Player *p, int start_x, int start_y) {
    p->x = start_x;
    p->y = start_y;
    p->speed = 3;        // 每帧移动 3 个像素
    p->dir = 0;
    p->height = 32;
    p->width = 32;

    p->max_hp = 100;      // 总血量 100
    p->current_hp = 100;  // 初始满血

    p->invincibility_timer = 0; // 🌟 新增：刚出生时没有无敌状态

    p->current_frame = 0;
    p->anim_tick = 0;
    p->anim_speed = 6;   // 60Hz 下，每 6 帧切一张图 (10 FPS 动画)
    
    p->is_moving = 0;
}

// 玩家状态更新 (这个函数被 main 函数以严格的 60Hz 频率调用)
// 玩家状态更新 (这个函数被 main 函数以严格的 60Hz 频率调用)
void Player_Update(Player *p, int move_dx, int move_dy) {

    int next_x = p->x + move_dx * p->speed;
    int next_y = p->y + move_dy * p->speed;

    // 分别检测 X 轴和 Y 轴 (实现丝滑贴墙滑动)
    if (move_dx != 0) {
        // 如果在 X 轴移动后，不会撞墙，才真正修改 p->x
        if (can_move_to(next_x, p->y, p->width, p->height)) {
            p->x = next_x;
        }
    }

    if (move_dy != 0) {
        // 如果在 Y 轴移动后，不会撞墙，才真正修改 p->y
        if (can_move_to(p->x, next_y, p->width, p->height)) {
            p->y = next_y;
        }
    }
    // 2. 记忆朝向 (假设 1 是右，2 是左)
    if (move_dx < 0) {
        p->dir = 2; // 面朝左
    } else if (move_dx > 0) {
        p->dir = 1; // 面朝右
    }

    // 3. 【状态机核心】：记录上一帧的状态，用来检测“状态切换”
    int was_moving = p->is_moving; 

    // 判断当前是否在移动
    if (move_dx != 0 || move_dy != 0) {
        p->is_moving = 1;
    } else {
        p->is_moving = 0;
    }

    // logic for character animations
    //===================================
    // 4. 【动画状态切换逻辑】
    // 如果这一帧的状态和上一帧不一样 (也就是刚起步，或者刚刹车)
    if (was_moving != p->is_moving) {
        p->current_frame = 0; // 动画立刻从第 0 帧重新开始
        p->anim_tick = 0;     // 计时器清零
    }

    // 5. 【动画播放逻辑】(现在无论是跑还是停，都在播放动画！)
    p->anim_tick++;

    if (p->is_moving) {
        // --- 正在跑动时的动画播放 ---
        p->anim_speed = 6; // 跑得快，每 6 个 VSync 切一张
        if (p->anim_tick >= p->anim_speed) {
            p->current_frame++;
            if (p->current_frame > 9) { // 跑动有 10 帧 (0~9)
                p->current_frame = 0;
            }
            p->anim_tick = 0;
        }
    } else {
        // --- 正在待机(Idle)时的动画播放 ---
        p->anim_speed = 12; // 待机呼吸慢，每 12 个 VSync 切一张
        if (p->anim_tick >= p->anim_speed) {
            p->current_frame++;
            // 【注意】：这里写你的待机动画总帧数 - 1。假设 idle 有 4 帧就是 > 3
            if (p->current_frame > 3) { 
                p->current_frame = 0;
            }
            p->anim_tick = 0;
        }
    }
    

    if (p->invincibility_timer > 0) {
        p->invincibility_timer--; // 如果还在无敌状态，计时器递减，期间不扣血
    } 
    else {
        // 声明外部的怪物数组 (从 enemy.c 借过来用)
        extern Enemy enemies[];
        extern int num_enemies;

        // 遍历所有活着的怪物，看看有没有碰到玩家
        for (int i = 0; i < num_enemies; i++) {
            if (enemies[i].alive) {
                // AABB 矩形碰撞检测 (判断玩家边缘和怪物边缘是否重合)
                int hit = (p->x < enemies[i].x + enemies[i].width &&
                           p->x + p->width > enemies[i].x &&
                           p->y < enemies[i].y + enemies[i].height &&
                           p->y + p->height > enemies[i].y);

                if (hit) {
                    // 🩸 碰到怪物了！根据怪物种类扣除不同的血量：
                    if (enemies[i].type == ENEMY_TYPE_FRANKIE) {
                        p->current_hp -= 15; // 大块头撞人很痛，扣 15 滴
                    } 
                    else if (enemies[i].type == ENEMY_TYPE_MIMIC) {
                        p->current_hp -= 20; // 宝箱怪咬人极痛，扣 20 滴
                    } 
                    else if (enemies[i].type == ENEMY_TYPE_NECRO) {
                        p->current_hp -= 10; // 碰到 Boss 身体，扣 10 滴
                    } 
                    else {
                        p->current_hp -= 5;  // 默认（比如普通僵尸、哥布林），扣 5 滴
                    }

                    // 🛡️ 触发无敌帧保护！设为 60 帧 (即 1 秒内不会再次受伤)
                    p->invincibility_timer = 60; 
                    
                    // 被打了一下立刻跳出循环，防止同时被几只怪叠在一起瞬间秒杀
                    break; 
                }
            }
        }
    }
    //
}

void Player_Draw(Player *p) {
    const unsigned char *current_image; // 用一个临时指针存当前该用的图片

    // 1. 智能选图：跑动还是待机？
    if (p->is_moving) {
        current_image = knight_anim[p->current_frame];
    } else {
        current_image = knight_idle[p->current_frame];
    }

    // 2. 智能翻转：朝左还是朝右？(假设 dir=2 是朝左)
    if (p->dir == 2) {
        // 面朝左，水平翻转
        Draw_Transparent_Image_horizontal_flip(p->x, p->y, current_image, p->width, p->height);
    } else {
        // 面朝右，正常画
        Draw_Transparent_Image(p->x, p->y, current_image, p->width, p->height);
    }
}