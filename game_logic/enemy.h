#ifndef ENEMY_H
#define ENEMY_H

#define MAX_ENEMIES 20  // 把这个数字改大，比如从 5 改到 20

typedef enum {
    ENEMY_TYPE_ZOMBIE,  // 默认绿怪
    ENEMY_TYPE_FRANKIE  // 新皮肤 Frankie
} EnemyType;

typedef struct {
    int x, y;
    int prev_x[2], prev_y[2]; // 用于双缓冲历史位置
    int width, height;
    int speed;

    EnemyType type; // 记录怪物种类
    
    int current_frame;
    int anim_tick;
    int anim_speed;
    
    int alive;
    int hp;       
    int max_hp;   
    int dir_x, dir_y;
    int ai_timer;
    int face_dir; // 1为朝右，2为朝左
} Enemy;

// 声明外部数组（让 main.c 也能访问到怪物数组）
extern Enemy enemies[MAX_ENEMIES];
extern int num_enemies;

// 声明房间大管家函数
void Spawn_Enemies_For_Room(int rx, int ry);
void Draw_All_Enemies();
void Update_All_Enemies(int player_x, int player_y);

#endif