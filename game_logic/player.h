#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "../sources/knight_anim.h" // 包含图片数据声明

// 定义玩家结构体
typedef struct Player {
    int x;
    int y;
    int speed;
    int dir;
    int height;
    int width;
    
    int max_hp;
    int current_hp;

    int invincibility_timer; // 🌟 新增：受击后的无敌时间倒计时

    int current_frame;
    int anim_tick;
    int anim_speed;

    int is_moving;
} Player;

// 声明对外接口
void Player_Init(Player *p, int start_x, int start_y);
void Player_Update(Player *p, int move_dx, int move_dy);
void Player_Draw(Player *p);

#endif