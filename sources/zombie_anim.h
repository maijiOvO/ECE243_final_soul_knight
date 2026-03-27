#ifndef ZOMBIE_ANIM_H
#define ZOMBIE_ANIM_H

#include <stdint.h>

// 8 帧僵尸动画，每帧 16x16 像素 = 512 字节
extern const unsigned char zombie_00[2048];
extern const unsigned char zombie_01[2048];
extern const unsigned char zombie_02[2048];
extern const unsigned char zombie_03[2048];
extern const unsigned char zombie_04[2048];
extern const unsigned char zombie_05[2048];
extern const unsigned char zombie_06[2048];
extern const unsigned char zombie_07[2048];

extern const unsigned char * const zombie_anim[];

#endif