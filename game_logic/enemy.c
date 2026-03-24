#include "enemy.h"
#include <stdlib.h>                 // 为了使用 rand()
#include "map.h"                    // 为了使用 can_move_to() 碰撞检测
#include "../hardware/vga.h"        // 为了使用画图函数
#include "../sources/ghost_anim.h"  // 包含你转换好的幽灵图片数组 (马上要建的)

// 1. 初始化幽灵 (随机生成位置)
void Enemy_Init_Random(Enemy *e) {
    // 随机出生在地图中间区域，避免直接卡在墙里
    // 假设网格是 20x20，我们让它生在 (2~13) 的网格里
    e->x = (rand() % 12 + 2) * 20; 
    e->y = (rand() % 8 + 2) * 20;  

    // 初始化双缓冲擦除坐标
    e->prev_x[0] = e->x; e->prev_x[1] = e->x;
    e->prev_y[0] = e->y; e->prev_y[1] = e->y;

    e->width = 16;
    e->height = 16;
    e->speed = 1;        // 幽灵飘得慢一点
    
    e->current_frame = 0;
    e->anim_tick = 0;
    e->anim_speed = 10;  // 动画速度 (数字越大，切图越慢)
    
    e->alive = 1;        // 默认存活

    e->dir_x = 0;
    e->dir_y = 0;
    e->ai_timer = 0;
    e->face_dir = 1;     // 默认面朝右
}

// 2. 幽灵的状态更新 (移动与动画)
void Enemy_Update(Enemy *e) {
    if (!e->alive) return;

    // --- 【大脑逻辑：随机漫步 AI】 ---
    e->ai_timer++;
    // 在 60Hz 刷新率下，120 约等于每 2 秒换一次方向
    if (e->ai_timer >= 120) { 
        e->dir_x = (rand() % 3) - 1; // 产生 -1, 0, 1
        e->dir_y = (rand() % 3) - 1; // 产生 -1, 0, 1
        e->ai_timer = 0;             // 重新计时
    }

    // 根据 X 方向决定脸朝向 (0 的时候保持原来朝向)
    if (e->dir_x > 0) {
        e->face_dir = 1; // 朝右
    } else if (e->dir_x < 0) {
        e->face_dir = 2; // 朝左
    }

    // --- 【移动与碰撞检测】 ---
    // 复用你在 map.c 里写好的极品碰撞检测函数！
    int next_x = e->x + e->dir_x * e->speed;
    int next_y = e->y + e->dir_y * e->speed;

    if (e->dir_x != 0) {
        if (can_move_to(next_x, e->y, e->width, e->height)) {
            e->x = next_x;
        }
    }
    if (e->dir_y != 0) {
        if (can_move_to(e->x, next_y, e->width, e->height)) {
            e->y = next_y;
        }
    }

    // --- 【动画播放逻辑】 ---
    e->anim_tick++;
    if (e->anim_tick >= e->anim_speed) {
        e->current_frame++;
        if (e->current_frame > 3) { // 幽灵只有 4 帧 (0, 1, 2, 3)
            e->current_frame = 0;
        }
        e->anim_tick = 0;
    }
}

// 3. 绘制幽灵
void Enemy_Draw(Enemy *e) {
    if (!e->alive) return;

    // 从我们马上要建的 ghost_anim.h 里拿当前帧的图片
    const unsigned char *current_image = ghost_anim[e->current_frame];

    // 判断朝向，决定是否水平翻转
    if (e->face_dir == 2) {
        // 朝左，调用翻转画图函数
        Draw_Transparent_Image_horizontal_flip(e->x, e->y, current_image, e->width, e->height);
    } else {
        // 朝右，正常画
        Draw_Transparent_Image(e->x, e->y, current_image, e->width, e->height);
    }
}