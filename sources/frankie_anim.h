#ifndef FRANKIE_ANIM_H
#define FRANKIE_ANIM_H

#include <stdint.h>

// 🌟 必须改回 unsigned char [2048]，因为你的 .c 文件现在每帧是 2048 字节
extern const unsigned char Frankie_01[2048];
extern const unsigned char Frankie_02[2048];
extern const unsigned char Frankie_03[2048];
extern const unsigned char Frankie_04[2048];
extern const unsigned char Frankie_05[2048];
extern const unsigned char Frankie_06[2048];
extern const unsigned char Frankie_07[2048];
extern const unsigned char Frankie_08[2048];

// 🌟 动画帧数组的类型也要对应修改为 const unsigned char *
extern const unsigned char * const frankie_anim_frames[];

#endif