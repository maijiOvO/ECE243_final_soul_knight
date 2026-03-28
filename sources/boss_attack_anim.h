#ifndef BOSS_ATTACK_ANIM_H
#define BOSS_ATTACK_ANIM_H

// 🌟 防撞护盾：如果前面已经定义过了，这块代码就会自动隐身！
#ifndef MONSTER_FRAME_STRUCT_DEFINED
#define MONSTER_FRAME_STRUCT_DEFINED
typedef struct {
    const unsigned char* data;
    int w;
    int h;
} MonsterFrame;
#endif

// 声明特效动作
extern const MonsterFrame tornado_frames[];
extern const MonsterFrame smear_frames[];
extern const MonsterFrame fire_frames[];

#endif