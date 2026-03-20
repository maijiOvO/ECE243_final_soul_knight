#include "map.h"
#include <stdlib.h>
#include "../hardware/vga.h"
#include "../sources/knight_anim.h"
#define min_space 16

int current_inner_texture = 0;
int walls[16][12] = {0};

#define GRID_W 16
#define GRID_H 12

extern int walls[GRID_W][GRID_H];

void init_walls() {
    // 1. 初始化清空数组 (确保每次重新生成都是一张白纸)
    for(int x = 0; x < GRID_W; x++) {
        for(int y = 0; y < GRID_H; y++) {
            walls[x][y] = 0;
        }
    }

    // 2. 生成外围的绝对边界
    for(int i = 0; i < GRID_W; i++ ){
        walls[i][0] = 1;           // 顶边
        walls[i][GRID_H - 1] = 1;  // 底边
    }
    for(int i = 0; i < GRID_H; i++ ){
        walls[0][i] = 1;           // 左边
        walls[GRID_W - 1][i] = 1;  // 右边
    }


    int num_clusters = rand() % 3 + 2; // 随机丢 2 到 4 个掩体堆
    current_inner_texture = rand()%5;

    for(int c = 0; c < num_clusters; c++) {
        int shape = rand() % 5;      // 随机抽取 5 种掩体形状之一
        int cx = rand() % 3 + 2;     // 掩体的基准 X 坐标 (2, 3, 4)
        int cy = rand() % 4 + 3;     // 掩体的基准 Y 坐标 (2~7)

        // 像盖印章一样，把形状“盖”进二维数组里
        if (shape == 0) { 
            // 形状 0：1x1 孤立的柱子
            walls[cx][cy] = 1;
        } 
        else if (shape == 1) { 
            // 形状 1：2x2 坚固的大方块
            walls[cx][cy] = 1;   walls[cx+1][cy] = 1;
            walls[cx][cy+1] = 1; walls[cx+1][cy+1] = 1;
        } 
        else if (shape == 2) { 
            // 形状 2：3x1 横向战壕
            walls[cx][cy] = 1; walls[cx+1][cy] = 1; walls[cx+2][cy] = 1;
        } 
        else if (shape == 3) { 
            // 形状 3：1x3 纵向掩体
            walls[cx][cy] = 1; walls[cx][cy+1] = 1; walls[cx][cy+2] = 1;
        } 
        else if (shape == 4) { 
            // 形状 4：L 型转角墙 (极其适合躲子弹)
            walls[cx][cy] = 1; 
            walls[cx][cy+1] = 1; 
            walls[cx+1][cy+1] = 1;
        }
    }

    // ==========================================
    // 4. 见证奇迹：中心对称复制到右半边
    // ==========================================
    // 遍历左半边的可活动区域 (X: 1~7, Y: 1~10)
    for(int x = 1; x <= 7; x++) {
        for (int y = 1; y <= 10; y++) {
            // 中心对称公式
            walls[15 - x][11 - y] = walls[x][y]; 
        }
    }
}
void draw_walls(){
    for(int i = 0; i < GRID_W; i ++){
        for(int j = 0; j < GRID_H; j ++){
            int cur_x = 20 * i;
            int cur_y = 20 * j;
            if(i == 0 || i == GRID_W - 1 || j == 0 || j == GRID_H - 1){
                Draw_Transparent_Image(cur_x, cur_y, wall_texture[5], 20, 20);
            }
            else if(walls[i][j] == 1){
                Draw_Transparent_Image(cur_x, cur_y, wall_texture[current_inner_texture], 20, 20);
            }
        }
    }
}

int can_move_to(int new_x, int new_y, int player_width, int player_height) {

    int x_offset = 2;
    int y_offset = 1;// since the animation does not excactly = 32*32, need the hitbox is smaller

    int hit_x = new_x + x_offset;
    int hit_y = new_y - y_offset;
    int hit_width = player_width - 2 * x_offset;
    int hit_height = player_height + y_offset;

    int left   = hit_x;
    int right  = hit_x + hit_width - 1;
    int top    = hit_y;
    int bottom = hit_y + hit_height - 1;

    int start_col = left / 20;
    int end_col   = right / 20;
    int start_row = top / 20;
    int end_row   = bottom / 20;

    if (start_col < 0 || end_col >= 16 || start_row < 0 || end_row >= 12) {
        return 0; //edge detection
    }
    for (int i = start_col; i <= end_col; i++) {
        for (int j = start_row; j <= end_row; j++) {
            if (walls[i][j] == 1) {
                return 0; // return 0 if the next move hit the wall
            }
        }
    }

    return 1; // move when no collision
}