#include "enemy.h"
#include <stdlib.h>
#include "map.h"
#include "../hardware/vga.h"
#include "../sources/zombie_anim.h" // 确保名字和你保存僵尸数组的头文件一致！
#include "../sources/frankie_anim.h" // 🌟 新增：Frankie 的动画数据

// --- 🌟 新增：探索进度记事本 ---
int room_assigned_type[15][15] = {0}; // 记录每个坐标的房间分配了什么怪 (0:没分配, 1:Zombie, 2:Frankie, 3:空)
int rooms_explored_count = 0;         // 记录玩家累计探索了几个新房间
// ------------------------------

// 定义一个全局僵尸
Enemy enemies[MAX_ENEMIES]; // 数组，宿舍建好了
int num_enemies = 0;        // 计数器，记录现在住了多少个僵尸

// 初始化僵尸 (随机位置)
void Enemy_Init_Random(Enemy *e) {
    int valid_spawn = 0;
    int attempts = 0;
    
    // 不断尝试生成坐标，直到不卡墙为止（最多尝试 100 次防止死循环）
    while (valid_spawn == 0 && attempts < 100) {
        e->x = (rand() % 8 + 4) * 20; 
        e->y = (rand() % 5 + 3) * 20;  
        
        // 用你现成的函数检查这个坐标有没有石头
        if (can_move_to(e->x, e->y, 20, 20)) {
            valid_spawn = 1; // 坐标安全，退出循环
        }
        attempts++;
    }

    e->prev_x[0] = e->x; e->prev_x[1] = e->x;
    e->prev_y[0] = e->y; e->prev_y[1] = e->y;

    e->width = 32;   // 🌟 你的僵尸图是 32x32 的
    e->height = 32;
    e->speed = 1;        

    e->max_hp = 3;  // 🌟 比如设定僵尸有 3 点血
    e->hp = e->max_hp;
    e->alive = 1;
    
    e->current_frame = 0;
    e->anim_tick = 0;
    e->anim_speed = 8; // 数字越大，僵尸动画播放越慢
    
    e->alive = 1;        
    e->dir_x = 0;
    e->dir_y = 0;
    e->ai_timer = 0;
    e->face_dir = 1; 

    // 🌟 1. 随机决定类型
    e->type = (rand() % 2 == 0) ? ENEMY_TYPE_ZOMBIE : ENEMY_TYPE_FRANKIE;

    // 🌟 2. 根据类型设置不同的属性
    if (e->type == ENEMY_TYPE_FRANKIE) {
        e->speed = 1;         // Frankie 走得慢
        e->max_hp = 6;        // 但是血厚
        e->anim_speed = 25;   // 动画稍微慢一点，显重
    } else {
        e->speed = (rand() % 2) + 1; // 绿怪速度 1~2
        e->max_hp = 3;
        e->anim_speed = 8;
    }
    
    e->hp = e->max_hp;
}



// 僵尸状态更新
// 🌟 核心改动：参数里增加了 player_x 和 player_y
void Enemy_Update(Enemy *e, int player_x, int player_y) {
    if (!e->alive) return;

    // --- 🌟 AI 修改：从随机改为追踪 ---
    e->ai_timer++;
    if (e->ai_timer >= 15) { // 每 15 帧重新判断一下玩家位置，防止走得太死板
        
        // 如果玩家在右边，僵尸往右走 (dir_x = 1)
        if (e->x < player_x) e->dir_x = 1;
        // 如果玩家在左边，僵尸往左走 (dir_x = -1)
        else if (e->x > player_x) e->dir_x = -1;
        else e->dir_x = 0;

        // 同理处理 Y 轴
        if (e->y < player_y) e->dir_y = 1;
        else if (e->y > player_y) e->dir_y = -1;
        else e->dir_y = 0;

        e->ai_timer = 0;             
    }
    // ---------------------------------

    // 下面这些原本的逻辑全部【保持不变】
    if (e->dir_x > 0) e->face_dir = 1; 
    else if (e->dir_x < 0) e->face_dir = 2; 

    e->prev_x[1] = e->prev_x[0]; e->prev_y[1] = e->prev_y[0];
    e->prev_x[0] = e->x; e->prev_y[0] = e->y;

 // --- 坐标更新与“滑动”逻辑 ---
    int actual_speed = e->speed;
    
    // 只有 Frankie 并且在偶数帧时，速度才变为 0（普通僵尸不受影响）
    if (e->type == ENEMY_TYPE_FRANKIE && e->anim_tick % 2 == 0) {
        actual_speed = 0; 
    }

    int next_x = e->x + e->dir_x * actual_speed;
    int next_y = e->y + e->dir_y * actual_speed;

    // 1. 处理 X 轴
    // 1. 处理 X 轴
    if (e->dir_x != 0) {
        // 🌟 建议：这里传 20 而不是 e->width (32)，让僵尸在逻辑上变瘦
        if (can_move_to(next_x, e->y, 20, 20)) { 
            e->x = next_x;
        } else {
            // 🌟 提高到 40%，且 nudge 直接给 +/- 2 或 4
            if (rand() % 100 < 40) {
                int nudge_y = (rand() % 2 == 0) ? 4 : -4; 
                if (can_move_to(e->x, e->y + nudge_y, 20, 20)) e->y += nudge_y;
            }
        }
    }

    // 2. 处理 Y 轴
    if (e->dir_y != 0) {
        if (can_move_to(e->x, next_y, 20, 20)) {
            e->y = next_y;
        } else {
            if (rand() % 100 < 40) {
                int nudge_x = (rand() % 2 == 0) ? 4 : -4;
                if (can_move_to(e->x + nudge_x, e->y, 20, 20)) e->x += nudge_x;
            }
        }
    }

    e->anim_tick++;
    if (e->anim_tick >= e->anim_speed) {
        e->current_frame = (e->current_frame + 1) % 8; // 在 0-7 帧之间循环
        e->anim_tick = 0;
    }
}

// 绘制僵尸
// 绘制僵尸
void Enemy_Draw(Enemy *e) {
    if (!e || !e->alive) return;

    // 1. 定义一个简单的 1 级指针，用来存放最终要画的那一帧数据的首地址
    const unsigned char *current_image = NULL;
    
    // 2. 分开获取指针，不要进行 (unsigned char**) 强制转换
    if (e->type == ENEMY_TYPE_FRANKIE) {
        // frankie_anim_frames 是指针数组，直接取
        current_image = frankie_anim_frames[e->current_frame];
    } else {
        // zombie_anim 如果是二维数组或类似结构，这样取是最稳妥的
        current_image = zombie_anim[e->current_frame];
    }

    // 3. 安全校验：如果没取到数据则不画，防止程序崩溃
    if (current_image == NULL) return;

    // 4. 绘图逻辑
    if (e->face_dir == 2) {
        // 朝左跑，水平翻转
        Draw_Transparent_Image_horizontal_flip(e->x, e->y, current_image, e->width, e->height);
    } else {
        // 朝右跑，正常画
        Draw_Transparent_Image(e->x, e->y, current_image, e->width, e->height);
    }
}

// =====================================
// 管家系统：给外部调用
// =====================================
// 管家系统：给外部调用
void Spawn_Enemies_For_Room(int rx, int ry) {
    num_enemies = 0; // 进新房间先清零

    if (rx == 6 && ry == 6) return; // 出生点绝对不刷怪

    // --- 🌟 核心控制区：按探索顺序给房间“盖章” ---
    // 如果这个房间还是未分配的 (0)，说明是第一次进来
    if (room_assigned_type[rx][ry] == 0) {
        rooms_explored_count++; // 发现新房间，总数 +1
        
        // 按照探索顺序，永久给这个房间盖个“印章”
        if (rooms_explored_count == 1) {
            room_assigned_type[rx][ry] = 1; // 第一次进的房间：永久盖上 Zombie 印章
        } 
        else if (rooms_explored_count == 2) {
            room_assigned_type[rx][ry] = 2; // 第二次进的房间：永久盖上 Frankie 印章
        } 
        else {
            room_assigned_type[rx][ry] = 3; // 第三次及以后进的房间：永久盖上“空房间”印章
        }
    }

    // ----------------------------------------------------
    // 读取这个房间的“印章”
    int type = room_assigned_type[rx][ry];
    int count = 0;
    
    // 根据印章决定数量
    if (type == 1) {
        count = 10; // Zombie 房间刷 10 个
    } else if (type == 2) {
        count = 6;  // Frankie 房间刷 6 个
    } else if (type == 3) {
        count = 0;  // 空房间 0 个
    }

    // 开始真正生成怪物
    for (int i = 0; i < count; i++) {
        if (num_enemies < MAX_ENEMIES) {
            // 先去随机找个不卡墙的坐标
            Enemy_Init_Random(&enemies[i]); 

            // 根据印章，强制设置怪物属性
            if (type == 1) {
                enemies[i].type = ENEMY_TYPE_ZOMBIE;
                enemies[i].max_hp = 3;
                enemies[i].speed = 1;
                enemies[i].anim_speed = 8; 
            } 
            else if (type == 2) {
                enemies[i].type = ENEMY_TYPE_FRANKIE;
                enemies[i].max_hp = 6;      
                enemies[i].speed = 1;      
                enemies[i].anim_speed = 30; // 笨重的动作速度
            }
            
            enemies[i].hp = enemies[i].max_hp;
            num_enemies++; 
        }
    }
}

// 更新所有僵尸
// 🌟 增加参数：接收玩家当前的坐标
void Update_All_Enemies(int player_x, int player_y) { 
    for (int i = 0; i < num_enemies; i++) {
        // 🌟 把坐标传给每一个单独的僵尸更新函数
        Enemy_Update(&enemies[i], player_x, player_y); 
    }
}

// 绘制所有僵尸
void Draw_All_Enemies() { 
    for (int i = 0; i < num_enemies; i++) {
        Enemy_Draw(&enemies[i]); 
    }
}