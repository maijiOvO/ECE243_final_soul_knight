#ifndef WEAPON_H
#define WEAPON_H

#include <stdint.h>
#include <stdbool.h>
#include "player.h"
#include "../hardware/ps2.h"
#include <math.h>
#include "enemy.h"

#define MAX_BULLETS 15
#define PI 3.14159265f

#define MAX_BULLETS 15
#define FP_SHIFT 8    // 定点数放大倍数 (2^8 = 256)

typedef struct {
    int x_fp;         // X坐标 * 256
    int y_fp;         // Y坐标 * 256
    int vx_fp;        // X速度 * 256
    int vy_fp;        // Y速度 * 256
    bool active;

    int damage;
    // prev用于双缓冲擦除
    int prev_x[2];    
    int prev_y[2];
} Bullet;

// 武器结构体定义
typedef struct {
    int id;                 // 对应 weapons[] 数组中的索引
    int fire_rate;          // 射速 (单位：帧，例如每 15 帧发一发)
    int damage;             // 单发伤害值
    bool is_laser;          // 是否为激光
    bool is_bouncy;         // 是否会反弹
    int bullet_w;           // 子弹宽度
    int bullet_h;           // 子弹高度
    int bullet_speed;       // 子弹移动速度
    
    int cooldown_timer;     // 内部冷却计时器
    int width;              // 武器贴图宽度
    int height;             // 武器贴图高度
    int current_dir_index;  // 当前旋转角度索引
    Bullet bullets[MAX_BULLETS];
} Weapon;



// 外部贴图声明 (声明在 anim_night.c 或相应文件中)
extern const unsigned char * const weapons[1];

// 函数声明
void Weapon_Init(Weapon *w, int id);
void Weapon_Draw(Weapon *w, Player *p);
bool Weapon_Can_Shoot(Weapon *w); // 检查是否可以射击

void Weapon_Update(Weapon *w, Player *p, MouseState *m, Enemy *enemies, int enemy_count);
void Bullet_Draw(Weapon *w, int buffer_id); // 画出所有活动子弹
void Bullet_Erase(Weapon *w, int buffer_id);

#endif