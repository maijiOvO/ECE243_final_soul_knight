#ifndef BOSS_ANIM_H
#define BOSS_ANIM_H

// 🌟 防撞护盾：宏定义的名字也建议跟着改一下，保持规范
#ifndef BOSS_FRAME_STRUCT_DEFINED
#define BOSS_FRAME_STRUCT_DEFINED
typedef struct {
    const unsigned char* data;
    int w;
    int h;
} BossFrame; // 👉 第一处修改：把 MonsterFrame 改成 BossFrame
#endif

// 声明本体动作
// 👉 第二处修改：把下面所有的 MonsterFrame 都替换成 BossFrame
extern const BossFrame boss_idle_frames[];
extern const BossFrame boss_walk_frames[];
extern const BossFrame boss_attack_frames[]; 
extern const BossFrame boss_ultimate_frames[];
extern const BossFrame boss_hurt_frames[];
extern const BossFrame boss_death_frames[];

#endif