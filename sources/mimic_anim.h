#ifndef MIMIC_ANIM_H
#define MIMIC_ANIM_H

// 1. 结构体定义 (整个项目只定义一次)
typedef struct {
    const unsigned char* data;
    int w;
    int h;
} MonsterFrame;

// --- 宝箱怪全状态动画列表 ---

// 1. 伪装与变身状态
extern const MonsterFrame mimic_stationary_frames[];     // 25x21 (伪装成木箱)
extern const MonsterFrame mimic_transformation_frames[]; // 64x64 (变身动作)

// 2. 日常活动状态
extern const MonsterFrame mimic_transformed_frames[];    // 64x64 (变身后待机)
extern const MonsterFrame mimic_walking_frames[];        // 64x64 (追逐玩家行走)

// 3. 战斗与反馈状态
extern const MonsterFrame mimic_attack_frames[];         // 64x64 (攻击动画)
extern const MonsterFrame mimic_damaged_frames[];        // 64x64 (受到伤害反馈)
extern const MonsterFrame mimic_death_frames[];          // 64x64 (彻底死亡散架)

#endif