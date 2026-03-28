#ifndef ENEMY_H
#define ENEMY_H

#define MAX_ENEMIES 30  // 保证同屏最多能有 30 个怪物

// 如果哥布林和骑士的 X 和 Y 距离都小于 40，就会引爆
#define GOBLIN_EXPLODE_PROXIMITY 20

typedef enum {
    ENEMY_TYPE_ZOMBIE,        // 默认绿怪
    ENEMY_TYPE_FRANKIE,       // 新皮肤 Frankie
    ENEMY_TYPE_GOBLIN_BOMBER, // 新增哥布林炸弹客类型
    ENEMY_TYPE_MIMIC,         // 宝箱怪
    ENEMY_TYPE_NECRO          // 🌟 新增：在这里给亡灵法师 Boss 注册身份！
} EnemyType;

// 宝箱怪的专属状态枚举
typedef enum {
    MIMIC_STATE_STATIONARY = 0,    // 0: 伪装中
    MIMIC_STATE_TRANSFORMING = 1,  // 1: 变身中
    MIMIC_STATE_EVADING = 2,       // 2: 变身后随机快速闪避
    MIMIC_STATE_CHASING = 3,       // 3: 狂暴追逐玩家
    MIMIC_STATE_ATTACKING = 4      // 4: 攻击玩家
} MimicState;

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

    int state; // 状态机 (例如 Boss 的 0=Idle, 1=Walk, 2=Tornado, 等等)
} Enemy;

// 声明外部数组（让 main.c 也能访问到怪物数组）
extern Enemy enemies[MAX_ENEMIES];
extern int num_enemies;

// 声明大管家及更新渲染函数
void Spawn_Enemies_For_Room(int rx, int ry);
void Draw_All_Enemies();
void Update_All_Enemies(int player_x, int player_y, int is_j_pressed);
void Update_Boss_FX(int knight_x, int knight_y, int knight_w, int knight_h);
void Draw_Boss_FX();

#endif