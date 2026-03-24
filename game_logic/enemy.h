#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>
#include "player.h" // 如果未来需要幽灵追踪玩家，可以包含这个

// 定义幽灵结构体
typedef struct {
    int x;
    int y;
    int prev_x[2];      // 用于双缓冲擦除的上一帧 X 坐标
    int prev_y[2];      // 用于双缓冲擦除的上一帧 Y 坐标
    
    int width;          // 幽灵宽度 (16)
    int height;         // 幽灵高度 (16)
    int speed;          // 移动速度

    int current_frame;  // 当前动画帧 (0~3)
    int anim_tick;      // 动画计时器
    int anim_speed;     // 动画播放速度

    int alive;          // 是否存活：1=活，0=死（不显示，无碰撞）

    // --- 随机 AI 与朝向控制 ---
    int dir_x;          // 当前 X 轴移动意图 (-1左, 0停, 1右)
    int dir_y;          // 当前 Y 轴移动意图 (-1上, 0停, 1下)
    int ai_timer;       // 随机漫步的大脑计时器
    int face_dir;       // 画面朝向：1=朝右，2=朝左
} Enemy;

// 声明外部接口
void Enemy_Init_Random(Enemy *e);
void Enemy_Update(Enemy *e);
void Enemy_Draw(Enemy *e);

#endif