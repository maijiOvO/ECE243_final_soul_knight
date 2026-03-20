#include "player.h"
#include "../hardware/vga.h" // 需要用到画图函数
#include "map.h"

// 初始化玩家
void Player_Init(Player *p, int start_x, int start_y) {
    p->x = start_x;
    p->y = start_y;
    p->speed = 3;        // 每帧移动 3 个像素
    p->dir = 0;
    p->height = 32;
    p->width = 32;

    p->current_frame = 0;
    p->anim_tick = 0;
    p->anim_speed = 6;   // 60Hz 下，每 6 帧切一张图 (10 FPS 动画)
    
    p->is_moving = 0;
}

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