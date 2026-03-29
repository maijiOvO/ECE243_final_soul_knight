#include "enemy.h"

#include <stdlib.h>
#include <math.h>           // 🌟 新增：用于计算宝箱怪的距离
#include "map.h"
#include "player.h"  // 🌟 新增：让系统认识骑士，为了后面扣血
#include "../hardware/vga.h"
#include "../sources/zombie_anim.h" 
#include "../sources/frankie_anim.h" 
#include "../sources/goblin_bomber_anim.h" 
#include "../sources/mimic_anim.h"  
#include "../sources/boss_anim.h" 
#include "../sources/boss_attack_anim.h" 
#include "../sources/map_texture.h"

// ==========================================
// 🌟 1. 魔法特效应有尽有的定义区
// ==========================================
typedef enum {
    FX_TYPE_SMEAR = 0,
    FX_TYPE_TORNADO,
    FX_TYPE_FIRE
} BossFXType;

typedef struct {
    int active;      
    int x, y;        
    int vx, vy;      
    int width, height;
    int life_timer;  
    int anim_frame;
    int anim_tick;
    BossFXType type;
} BossFX;

#define MAX_BOSS_FX 50
BossFX boss_fx_list[MAX_BOSS_FX]; 

int boss_timer_tornado = 600; 
int boss_timer_ultimate = 900;

void Spawn_Boss_FX(int start_x, int start_y, int speed_x, int speed_y, BossFXType type, int life_time, int w, int h) {
    for (int i = 0; i < MAX_BOSS_FX; i++) {
        if (!boss_fx_list[i].active) {
            boss_fx_list[i].active = 1;
            boss_fx_list[i].x = start_x;
            boss_fx_list[i].y = start_y;
            boss_fx_list[i].vx = speed_x;
            boss_fx_list[i].vy = speed_y;
            boss_fx_list[i].type = type;
            boss_fx_list[i].life_timer = life_time;
            boss_fx_list[i].width = w;   
            boss_fx_list[i].height = h;  
            boss_fx_list[i].anim_frame = 0;
            boss_fx_list[i].anim_tick = 0;
            break;
        }
    }
}

// --- 🌟 新增：探索进度记事本 ---
int room_assigned_type[15][15] = {0}; 
int rooms_explored_count = 0;         

// 定义一个全局僵尸
Enemy enemies[MAX_ENEMIES]; 
int num_enemies = 0;        

void Enemy_Init_Random(Enemy *e) {
    int valid_spawn = 0;
    int attempts = 0;
    
    while (valid_spawn == 0 && attempts < 100) {
        e->x = (rand() % 8 + 4) * 20; 
        e->y = (rand() % 5 + 3) * 20;  
        
        if (can_move_to(e->x, e->y, 20, 20)) {
            valid_spawn = 1; 
        }
        attempts++;
    }

    e->prev_x[0] = e->x; e->prev_x[1] = e->x;
    e->prev_y[0] = e->y; e->prev_y[1] = e->y;

    e->width = 32;   
    e->height = 32;
    e->speed = 1;        

    e->max_hp = 3;  
    e->hp = e->max_hp;
    e->alive = 1;
    
    e->current_frame = 0;
    e->anim_tick = 0;
    e->anim_speed = 8; 
    
    e->alive = 1;        
    e->dir_x = 0;
    e->dir_y = 0;
    e->ai_timer = 0;
    e->face_dir = 1; 

    e->type = (rand() % 2 == 0) ? ENEMY_TYPE_ZOMBIE : ENEMY_TYPE_FRANKIE;

    if (e->type == ENEMY_TYPE_FRANKIE) {
        e->speed = 1;         
        e->max_hp = 6;        
        e->anim_speed = 25;   
    } else {
        e->speed = (rand() % 2) + 1; 
        e->max_hp = 3;
        e->anim_speed = 8;
    }
    
    e->hp = e->max_hp;
}

// ==========================================
// 🌟 核心：更新逻辑 (AI与动画)
// ==========================================
void Enemy_Update(Enemy *e, int player_x, int player_y, int is_j_pressed) {
    if (!e->alive) return;

    int dist_x = abs(e->x - player_x);
    int dist_y = abs(e->y - player_y);
    int distance = dist_x + dist_y; 

    // ==========================================
    // 🌟 1. AI 决策逻辑
    // ==========================================
    if (e->type == ENEMY_TYPE_MIMIC) {
        switch (e->state) {
            case MIMIC_STATE_STATIONARY:
                e->dir_x = 0; e->dir_y = 0;
                if (distance < 80 && is_j_pressed) { 
                    e->state = MIMIC_STATE_TRANSFORMING;
                    e->current_frame = 0;
                    e->anim_tick = 0;
                }
                break;
            case MIMIC_STATE_TRANSFORMING:
                e->dir_x = 0; e->dir_y = 0; 
                break;
            case MIMIC_STATE_EVADING:
                e->ai_timer++;
                if (e->ai_timer >= 20) { 
                    e->dir_x = (rand() % 3) - 1; 
                    e->dir_y = (rand() % 3) - 1;
                    e->ai_timer = 0;
                }
                if (distance < 200) { 
                    e->state = MIMIC_STATE_CHASING;
                }
                break;
            case MIMIC_STATE_CHASING:
                e->ai_timer++;
                if (e->ai_timer >= 10) { 
                    if (e->x < player_x) e->dir_x = 1; else if (e->x > player_x) e->dir_x = -1; else e->dir_x = 0;
                    if (e->y < player_y) e->dir_y = 1; else if (e->y > player_y) e->dir_y = -1; else e->dir_y = 0;
                    e->ai_timer = 0;             
                }
                if (dist_y < 15 && dist_x < 55) { 
                    e->state = MIMIC_STATE_ATTACKING;
                    e->current_frame = 0;
                    e->anim_tick = 0;
                    e->dir_x = 0; e->dir_y = 0; 
                }
                break;
            case MIMIC_STATE_ATTACKING:
                e->dir_x = 0; e->dir_y = 0;
                break;
        }
    }
    // 👇👇👇 🌟 亡灵法师 Boss 大脑逻辑 🌟 👇👇👇
    else if (e->type == ENEMY_TYPE_NECRO) {
        e->ai_timer++;
        if (e->ai_timer >= 25) { 
            if (e->x < player_x) e->dir_x = 1; else if (e->x > player_x) e->dir_x = -1; else e->dir_x = 0;
            if (e->y < player_y) e->dir_y = 1; else if (e->y > player_y) e->dir_y = -1; else e->dir_y = 0;
            e->ai_timer = 0;             
        }

        boss_timer_tornado--;
        boss_timer_ultimate--;

        if (e->state == 0 || e->state == 1) { 
            if (boss_timer_ultimate <= 0) {
                e->state = 4; e->current_frame = 0; e->anim_tick = 0; e->dir_x = 0; e->dir_y = 0; 
                boss_timer_ultimate = 900;  
                Spawn_Boss_FX(e->x, e->y, 0, -3, FX_TYPE_FIRE, 240, 32, 32); 
                Spawn_Boss_FX(e->x, e->y, 0, 3,  FX_TYPE_FIRE, 240, 32, 32); 
                Spawn_Boss_FX(e->x, e->y, -3, 0, FX_TYPE_FIRE, 240, 32, 32); 
                Spawn_Boss_FX(e->x, e->y, 3, 0,  FX_TYPE_FIRE, 240, 32, 32); 
            }
            else if (boss_timer_tornado <= 0) {
                e->state = 2; e->current_frame = 0; e->anim_tick = 0; e->dir_x = 0; e->dir_y = 0;
                boss_timer_tornado = 600;   
                Spawn_Boss_FX(e->x, e->y, 3, -3, FX_TYPE_TORNADO, 300, 32, 32); 
                Spawn_Boss_FX(e->x, e->y, 3, 3,  FX_TYPE_TORNADO, 300, 32, 32); 
                Spawn_Boss_FX(e->x, e->y, -3, -3, FX_TYPE_TORNADO, 300, 32, 32); 
                Spawn_Boss_FX(e->x, e->y, -3, 3,  FX_TYPE_TORNADO, 300, 32, 32); 
            }
            else if (distance < 150 && (rand() % 100 < 3)) { 
                e->state = 3; e->current_frame = 0; e->anim_tick = 0; e->dir_x = 0; e->dir_y = 0;
                int speed_x = 0, speed_y = 0;
                if (abs(dist_x) > abs(dist_y)) speed_x = (player_x > e->x) ? 6 : -6;
                else speed_y = (player_y > e->y) ? 6 : -6;
                Spawn_Boss_FX(e->x, e->y, speed_x, speed_y, FX_TYPE_SMEAR, 9999, 32, 32); 
            }
            else { 
                e->state = (e->dir_x == 0 && e->dir_y == 0) ? 0 : 1; 
            }
        } else { 
            e->dir_x = 0; e->dir_y = 0; 
        }
    }
    else if (e->type == ENEMY_TYPE_GOBLIN_BOMBER && e->state == 0) {
        if (dist_x < GOBLIN_EXPLODE_PROXIMITY && dist_y < GOBLIN_EXPLODE_PROXIMITY) {
            e->state = 1;         
            e->current_frame = 8; 
            e->anim_tick = 0;
            e->dir_x = 0;         
            e->dir_y = 0;
        } else {
            e->ai_timer++;
            if (e->ai_timer >= 15) { 
                if (e->x < player_x) e->dir_x = 1; else if (e->x > player_x) e->dir_x = -1; else e->dir_x = 0;
                if (e->y < player_y) e->dir_y = 1; else if (e->y > player_y) e->dir_y = -1; else e->dir_y = 0;
                e->ai_timer = 0;             
            }
        }
    } 
    else if (e->type == ENEMY_TYPE_GOBLIN_BOMBER && e->state == 1) {
        e->dir_x = 0; e->dir_y = 0;
    } 
    else {
        e->ai_timer++;
        if (e->ai_timer >= 15) { 
            if (e->x < player_x) e->dir_x = 1; else if (e->x > player_x) e->dir_x = -1; else e->dir_x = 0;
            if (e->y < player_y) e->dir_y = 1; else if (e->y > player_y) e->dir_y = -1; else e->dir_y = 0;
            e->ai_timer = 0;             
        }
    }

    // ==========================================
    // 2. 移动与碰撞检测
    // ==========================================
    if (e->dir_x > 0) e->face_dir = 1; 
    else if (e->dir_x < 0) e->face_dir = 2; 

    e->prev_x[1] = e->prev_x[0]; e->prev_y[1] = e->prev_y[0];
    e->prev_x[0] = e->x; e->prev_y[0] = e->y;

    int actual_speed = e->speed;
    if (e->type == ENEMY_TYPE_FRANKIE && e->anim_tick % 2 == 0) {
        actual_speed = 0; 
    }
    if (e->type == ENEMY_TYPE_MIMIC) {
        if (e->state == MIMIC_STATE_TRANSFORMING || e->state == MIMIC_STATE_ATTACKING) {
            actual_speed = 0;
        }
    }

    int next_x = e->x + e->dir_x * actual_speed;
    int next_y = e->y + e->dir_y * actual_speed;

    if (e->dir_x != 0) {
        if (can_move_to(next_x, e->y, 20, 20)) { 
            e->x = next_x;
        } else {
            if (rand() % 100 < 40) {
                int nudge_y = (rand() % 2 == 0) ? 4 : -4; 
                if (can_move_to(e->x, e->y + nudge_y, 20, 20)) e->y += nudge_y;
            }
        }
    }
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

    // ==========================================
    // 🌟 3. 动画切帧逻辑
    // ==========================================
    e->anim_tick++;
    
    int current_play_speed = e->anim_speed; 
    
    if (e->type == ENEMY_TYPE_MIMIC) {
        if (e->state == MIMIC_STATE_ATTACKING) current_play_speed = 3;  
        else if (e->state == MIMIC_STATE_STATIONARY) current_play_speed = 15; 
        else if (e->state == MIMIC_STATE_TRANSFORMING) current_play_speed = 8;  
        else current_play_speed = 6;  
    }
    else if (e->type == ENEMY_TYPE_NECRO) {
        current_play_speed = 5; 
    }

    if (e->anim_tick >= current_play_speed) {
        e->anim_tick = 0;

        if (e->type == ENEMY_TYPE_MIMIC) {
            if (e->state == MIMIC_STATE_STATIONARY) {
                e->current_frame = (e->current_frame + 1) % 18; 
            } 
            else if (e->state == MIMIC_STATE_TRANSFORMING) {
                e->current_frame++;
                if (e->current_frame >= 9) { 
                    e->state = MIMIC_STATE_EVADING;
                    e->current_frame = 0;
                    e->speed = 3; 
                }
            } 
            else if (e->state == MIMIC_STATE_EVADING || e->state == MIMIC_STATE_CHASING) {
                e->current_frame = (e->current_frame + 1) % 6; 
            } 
            else if (e->state == MIMIC_STATE_ATTACKING) {
                e->current_frame++;
                if (e->current_frame >= 14) { 
                    e->state = MIMIC_STATE_CHASING;
                    e->current_frame = 0;
                    e->speed = 3; 
                }
            }
        } 
        else if (e->type == ENEMY_TYPE_NECRO) {
            e->current_frame++;
            if (e->state == 0) e->current_frame %= 8; 
            else if (e->state == 1) e->current_frame %= 8; 
            else if (e->state == 2 && e->current_frame >= 13) { e->state = 0; e->current_frame = 0; } 
            else if (e->state == 3 && e->current_frame >= 13) { e->state = 0; e->current_frame = 0; } 
            else if (e->state == 4 && e->current_frame >= 17) { e->state = 0; e->current_frame = 0; } 
        }
        else if (e->type == ENEMY_TYPE_GOBLIN_BOMBER && e->state == 1) {
            e->current_frame++; 
            if (e->current_frame > 15) e->alive = 0; 
        } 
        else {
            e->current_frame = (e->current_frame + 1) % 8; 
        }
    }
}

// 绘制所有怪物 (🌟 中心点对齐版)
void Enemy_Draw(Enemy *e) {
    if (!e || !e->alive) return;

    const unsigned char *current_image = NULL;
    
    int draw_width = e->width;   
    int draw_height = e->height; 
    
    if (e->type == ENEMY_TYPE_FRANKIE) {
        current_image = frankie_anim_frames[e->current_frame];
    } 
    else if (e->type == ENEMY_TYPE_GOBLIN_BOMBER) {
        current_image = goblin_anim[e->current_frame]; 
    }
    else if (e->type == ENEMY_TYPE_MIMIC) {
        switch (e->state) {
            case MIMIC_STATE_STATIONARY:
                current_image = box[1];
                draw_width = 20;
                draw_height = 20;
                break;
            case MIMIC_STATE_TRANSFORMING:
                current_image = mimic_transformation_frames[e->current_frame].data;
                draw_width = mimic_transformation_frames[e->current_frame].w;
                draw_height = mimic_transformation_frames[e->current_frame].h;
                break;
            case MIMIC_STATE_EVADING:
            case MIMIC_STATE_CHASING:
                current_image = mimic_walking_frames[e->current_frame].data;
                draw_width = mimic_walking_frames[e->current_frame].w;
                draw_height = mimic_walking_frames[e->current_frame].h;
                break;
            case MIMIC_STATE_ATTACKING:
                current_image = mimic_attack_frames[e->current_frame].data;
                draw_width = mimic_attack_frames[e->current_frame].w;
                draw_height = mimic_attack_frames[e->current_frame].h;
                break;
            default:
                break;
        }
    }
    // 补齐：亡灵法师图片数据挂载
    else if (e->type == ENEMY_TYPE_NECRO) {
        const BossFrame *f = NULL;
        
        if (e->state == 0) { current_image = boss_idle_frames[e->current_frame].data; draw_width = boss_idle_frames[e->current_frame].w; draw_height = boss_idle_frames[e->current_frame].h; }
        else if (e->state == 1) { current_image = boss_walk_frames[e->current_frame].data; draw_width = boss_walk_frames[e->current_frame].w; draw_height = boss_walk_frames[e->current_frame].h; }
        else if (e->state == 2) { current_image = boss_attack_frames[e->current_frame].data; draw_width = boss_attack_frames[e->current_frame].w; draw_height = boss_attack_frames[e->current_frame].h; }
        else if (e->state == 3) { current_image = boss_attack_frames[e->current_frame].data; draw_width = boss_attack_frames[e->current_frame].w; draw_height = boss_attack_frames[e->current_frame].h; }
        else if (e->state == 4) { current_image = boss_ultimate_frames[e->current_frame].data; draw_width = boss_ultimate_frames[e->current_frame].w; draw_height = boss_ultimate_frames[e->current_frame].h; }
    }
    else {
        current_image = zombie_anim[e->current_frame];
    }

    if (current_image == NULL) return;

    int center_x = e->x + 16;
    int center_y = e->y + 16;
    int draw_x = center_x - (draw_width / 2);
    int draw_y = center_y - (draw_height / 2);

    if (e->face_dir == 2) {
        Draw_Transparent_Image_horizontal_flip(draw_x, draw_y, current_image, draw_width, draw_height);
    } else {
        Draw_Transparent_Image(draw_x, draw_y, current_image, draw_width, draw_height);
    }
}


void Spawn_Enemies_For_Room(int rx, int ry) {
    num_enemies = 0; 

    // 1. 绝对安全区：出生点和宝箱房不刷怪
    if (rx == 6 && ry == 6) return; 
    if (world_map[rx][ry].type == 'C' && world_map[rx][ry].is_mimic == 0) return; 
    // 只有当房间被清理过，并且宝箱/宝箱怪被触发过，才彻底不刷怪
    if (world_map[rx][ry].is_cleared == 1 || world_map[rx][ry].is_opened == 1) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            enemies[i].alive = 0; 
        }
        return; 
    }

    // 2. 房间分配逻辑 确保boss只刷在boss房里
    if (room_assigned_type[rx][ry] == 0) {
        if (world_map[rx][ry].type == 'B') {
            // 如果大地图标记这里是 Boss 房，无视探索顺序，强行盖上 Boss 印章 (6)
            room_assigned_type[rx][ry] = 6; 
        } 
        else {
            // 否则就是普通怪物房 (type == 'M')，按照玩家探索的顺序循序渐进地给压力
            rooms_explored_count++; 
            
            if (rooms_explored_count == 1) room_assigned_type[rx][ry] = 1;      // 第1间：僵尸
            else if (rooms_explored_count == 2) room_assigned_type[rx][ry] = 2; // 第2间：弗兰肯斯坦
            else if (rooms_explored_count == 3) room_assigned_type[rx][ry] = 4; // 第3间：哥布林炸弹人
            else if (rooms_explored_count == 4){
                room_assigned_type[rx][ry] = 5;
                world_map[rx][ry].is_mimic = 1;     // 标记为伪装
                world_map[rx][ry].type = 'C';       // 小地图图标变成宝箱
                world_map[rx][ry].is_cleared = 1;   // 强制设为已清空，让大门保持敞开！
                world_map[rx][ry].is_opened = 0;    // 宝箱处于未开启状态
                world_map[rx][ry].seeds.num_clusters = 0; // 强制清空掩体，变成空旷大厅
            } // 第4间：宝箱怪 (伪装)
            
            else room_assigned_type[rx][ry] = 3; // 第5间及以后：空房间
        }
    }

    int type = room_assigned_type[rx][ry];
    int count = 0;
    
    if (type == 1) count = 10; 
    else if (type == 2) count = 6;  
    else if (type == 4) count = 30; 
    else if (type == 5) count = 1;  
    else if (type == 6) count = 1;  //Boss 只有一个
    else if (type == 3) count = 0;  

    for (int i = 0; i < count; i++) {
        if (num_enemies < MAX_ENEMIES) {
            Enemy_Init_Random(&enemies[i]); 

            if (type == 1) {
                enemies[i].type = ENEMY_TYPE_ZOMBIE;
                enemies[i].max_hp = 3;
                enemies[i].speed = 1;
                enemies[i].anim_speed = 8; 
                enemies[i].state = 0; 
            } 
            else if (type == 2) {
                enemies[i].type = ENEMY_TYPE_FRANKIE;
                enemies[i].max_hp = 6;      
                enemies[i].speed = 1;      
                enemies[i].anim_speed = 30; 
                enemies[i].state = 0; 
            }
            else if (type == 4) {
                enemies[i].type = ENEMY_TYPE_GOBLIN_BOMBER;
                enemies[i].max_hp = 4;
                enemies[i].speed = 2;       
                enemies[i].anim_speed = 6;  
                enemies[i].current_frame = 0;
                enemies[i].state = 0;       
            }
            else if (type == 5) {
                enemies[i].type = ENEMY_TYPE_MIMIC;
                enemies[i].max_hp = 50;           
                enemies[i].speed = 0;             
                enemies[i].anim_speed = 10;       
                enemies[i].current_frame = 0;
                enemies[i].state = MIMIC_STATE_STATIONARY; 
                enemies[i].x = 160; 
                enemies[i].y = 120; 
                enemies[i].prev_x[0] = enemies[i].x; enemies[i].prev_x[1] = enemies[i].x;
                enemies[i].prev_y[0] = enemies[i].y; enemies[i].prev_y[1] = enemies[i].y;
            }
            // 👇👇👇 🌟 补齐：生成法师属性 🌟 👇👇👇
            else if (type == 6) {
                enemies[i].type = ENEMY_TYPE_NECRO;
                enemies[i].max_hp = 200;          
                enemies[i].speed = 1;             
                enemies[i].anim_speed = 6;        
                enemies[i].current_frame = 0;
                enemies[i].state = 0; 
                enemies[i].x = 160; enemies[i].y = 120; 
                enemies[i].prev_x[0] = enemies[i].x; enemies[i].prev_x[1] = enemies[i].x;
                enemies[i].prev_y[0] = enemies[i].y; enemies[i].prev_y[1] = enemies[i].y;
            }
            // 👆👆👆 🌟 补齐结束 🌟 👆👆👆
            
            enemies[i].hp = enemies[i].max_hp;
            num_enemies++; 
        }
    }
}

void Update_All_Enemies(int player_x, int player_y, int is_j_pressed) { 
    for (int i = 0; i < num_enemies; i++) {
        Enemy_Update(&enemies[i], player_x, player_y, is_j_pressed); 
    }
}

void Draw_All_Enemies() { 
    for (int i = 0; i < num_enemies; i++) {
        Enemy_Draw(&enemies[i]); 
    }
}

void Update_Boss_FX(int knight_x, int knight_y, int knight_w, int knight_h) {
    for (int i = 0; i < MAX_BOSS_FX; i++) {
        if (!boss_fx_list[i].active) continue;
        BossFX *fx = &boss_fx_list[i];

        fx->life_timer--;
        if (fx->life_timer <= 0) { fx->active = 0; continue; }

        int next_x = fx->x + fx->vx; int next_y = fx->y + fx->vy;

        if (!can_move_to(next_x, next_y, fx->width, fx->height)) {
            if (fx->type == FX_TYPE_TORNADO) {
                if (!can_move_to(next_x, fx->y, fx->width, fx->height)) fx->vx = -fx->vx; 
                if (!can_move_to(fx->x, next_y, fx->width, fx->height)) fx->vy = -fx->vy; 
            } else if (fx->type == FX_TYPE_FIRE) { fx->vx = 0; fx->vy = 0; } 
            else if (fx->type == FX_TYPE_SMEAR) { fx->active = 0; continue; }
        } else { fx->x = next_x; fx->y = next_y; }

        int hit_knight = (fx->x < knight_x + knight_w && fx->x + fx->width > knight_x && fx->y < knight_y + knight_h && fx->y + fx->height > knight_y);
        if (hit_knight) {
            // TODO: 这里可以加上骑士扣血的代码！
            if (fx->type != FX_TYPE_FIRE) fx->active = 0; 
        }

        fx->anim_tick++;
        if (fx->anim_tick > 4) { fx->anim_tick = 0; fx->anim_frame = (fx->anim_frame + 1) % 4; }
    }
}

void Draw_Boss_FX() {
    for (int i = 0; i < MAX_BOSS_FX; i++) {
        if (!boss_fx_list[i].active) continue;
        BossFX *fx = &boss_fx_list[i];
        
        const BossFrame *f = NULL; // 🌟 像画 Boss 一样，先用指针定位！

        // 1. 根据魔法类型，找到对应的动画帧数据
        if      (fx->type == FX_TYPE_TORNADO) f = &boss_tornado_frames[fx->anim_frame];
        else if (fx->type == FX_TYPE_FIRE)    f = &boss_fire_frames[fx->anim_frame];
        else if (fx->type == FX_TYPE_SMEAR)   f = &boss_smear_frames[fx->anim_frame];
        
        // 2. 只要找到了，就提取 data 并画出来
        if (f != NULL) {
            // 如果你的魔法特效每一帧大小都不一样，你甚至可以把 fx->width 换成 f->w
            Draw_Transparent_Image(fx->x, fx->y, f->data, fx->width, fx->height);
        }
    }
}