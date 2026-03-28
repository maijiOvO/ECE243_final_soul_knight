#ifndef BOSS_ANIM_H
#define BOSS_ANIM_H

// 🌟 防撞护盾：如果没定义过，才定义它
#ifndef MONSTER_FRAME_STRUCT_DEFINED
#define MONSTER_FRAME_STRUCT_DEFINED
typedef struct {
    const unsigned char* data;
    int w;
    int h;
} MonsterFrame;
#endif

// 声明本体动作
extern const MonsterFrame boss_idle_frames[];
extern const MonsterFrame boss_walk_frames[];
extern const MonsterFrame boss_attack_frames[]; // 这里的 attack 是指 Boss 挥法杖的物理动作
extern const MonsterFrame boss_ultimate_frames[];
extern const MonsterFrame boss_hurt_frames[];
extern const MonsterFrame boss_death_frames[];

#endif