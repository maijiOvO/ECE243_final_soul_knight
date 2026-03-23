#ifndef GHOST_ANIM_H
#define GHOST_ANIM_H

#include <stdint.h>

// 4个16x16的小幽灵，每个像素点占用2个字节，共512字节
extern const unsigned char ghost_00[512];
extern const unsigned char ghost_01[512];
extern const unsigned char ghost_02[512];
extern const unsigned char ghost_03[512];

extern const unsigned char * const ghost_anim[];

#endif
