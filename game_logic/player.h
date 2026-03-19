#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "../sources/knight_anim.h" // 包含图片数据声明

// 定义玩家结构体
typedef struct {

    int x;          // current position
    int y;
    int speed;      // 移动速度
    int dir;        // 0 = idle, i = left, 2 = right
    int height;
    int width;

    // 2. 动画属性 (独立解耦计时)
    int current_frame; // 当前播放到第几帧 (0~9)
    int anim_tick;     // 内部计时器 (记录经历了几个 VSync)
    int anim_speed;    // 动画阈值 (比如设为 6，代表 6 个 VSync 切一帧)
    
    // 3. 状态标记
    int is_moving;     // 0: 静止, 1: 正在跑动
} Player;

// 声明对外接口
void Player_Init(Player *p, int start_x, int start_y);
void Player_Update(Player *p, int move_dx, int move_dy);
void Player_Draw(Player *p);

#endif