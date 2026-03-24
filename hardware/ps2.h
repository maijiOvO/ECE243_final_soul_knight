#ifndef PS2_H
#define PS2_H

// 暴露给外界的 4 个按键状态 (1=按下, 0=松开)
extern int key_W;
extern int key_A;
extern int key_S;

typedef struct {
    int x, y;           // 准星在屏幕上的绝对坐标
    int left_click;     // 左键状态 (1=按下, 0=松开)
} MouseState;

extern MouseState mouse;
extern int key_D;

// 初始化与更新函数
void PS2_Init(void);
void PS2_Update(void);

#endif