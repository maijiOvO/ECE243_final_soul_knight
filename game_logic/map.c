#include "map.h"
#include <stdlib.h>
#include "../hardware/vga.h"
#include "../sources/knight_anim.h"
#include "player.h"

int current_inner_texture = 0;
int max_rooms = 0;
int walls[16][12] = {0};
extern int walls[GRID_W][GRID_H];

Room world_map[world_map_max][world_map_max];     //13 * 13 room space
Room seed_map[world_map_max][world_map_max];
int init_x = 6;
int init_y = 6;
int current_room_x = 6;
int current_room_y = 6;

void init_world_map(){
    for (int i = 0; i < world_map_max; i ++){
        for (int j = 0; j < world_map_max; j ++){
            world_map[i][j].exists = 0;
        }
    }

    current_room_x = 6;
    current_room_y = 6;

    int count = 1;
    max_rooms = rand()%2 +5;

    // 确保起始房间默认是清空的，否则一出生就被怪围住
    world_map[current_room_x][current_room_y].is_cleared = 1; 
    world_map[current_room_x][current_room_y].exists = 1;
    int rx = 6, ry = 6;
    while(count < max_rooms) {
        int dir = rand() % 4;
        if(dir == 0) ry--;      // up
        else if(dir == 1) ry++; // down
        else if(dir == 2) rx--; // left
        else rx++;              // right

        if (world_map[rx][ry].exists == 0) {
            world_map[rx][ry].exists = 1;
        
            RoomSeeds *s = &world_map[rx][ry].seeds;
            s->num_clusters = rand() % 3 + 2;
            s->current_inner_texture = rand() % 5;
        
            for (int i = 0; i < 4; i++) {
                s->shape[i] = rand() % 5;
                s->cx[i] = rand() % 3 + 2;
                s->cy[i] = rand() % 4 + 3;
            }
        count++;
        world_map[rx][ry].is_cleared = 1;
        }
        else{continue;}
    }
}

void apply_doors_to_walls() {       // the position of walls
    // 只有房间清空了才执行开门逻辑
    if (world_map[current_room_x][current_room_y].is_cleared == 0) {
        return; 
    }

    // 检查上方：如果北边有房间，挖开顶部中间
    if (world_map[current_room_x][current_room_y - 1].exists) {
        walls[7][0] = 2; walls[8][0] = 2;
    }
    // 检查下方：如果南边有房间，挖开底部中间
    if (world_map[current_room_x][current_room_y + 1].exists) {
        walls[7][11] = 2; walls[8][11] = 2;
    }
    // 检查左方：如果西边有房间，挖开左侧中间
    if (world_map[current_room_x - 1][current_room_y].exists) {
        walls[0][5] = 2; walls[0][6] = 2;
    }
    // 检查右方：如果东边有房间，挖开右侧中间
    if (world_map[current_room_x + 1][current_room_y].exists) {
        walls[15][5] = 2; walls[15][6] = 2;
    }
}

int handle_room_transition(Player *p) { // return 1 if jumped
    int teleported = 0;

    // 检测四个边缘
    if (p->y < -5) {
        current_room_y--; p->y = 204; teleported = 1;
    } else if (p->y > 205) {
        current_room_y++; p->y = 10;  teleported = 1;
    } else if (p->x < -5) {
        current_room_x--; p->x = 285; teleported = 1;
    } else if (p->x > 286) {
        current_room_x++; p->x = 10;  teleported = 1;
    }

    if (teleported) {
        // 1. 重新生成地形（掩体随机变化）
        init_walls();
        
        // 2. 根据逻辑打开新房间的大门
        // 如果新房间之前打过了，门是开的；如果没打过，门是关的
        apply_doors_to_walls();

        // 3. 彻底重置双缓冲，防止旧房间残影（这是最简洁的处理方式）
        clear_screen(); 
        vga_wait_for_vsync(); // 等待同步
        clear_screen(); 
        
        // 4. (预留) 在这里可以根据 is_cleared 决定是否刷怪
    }

    return teleported;
}

void draw_world_map() {
    int min_i = 4, min_j = 4; // 初始设为最大值

    // 2. 第二遍扫描：绘图
    int map_offset_x = 10; // 小地图距离屏幕左边的边距
    int map_offset_y = 10; // 小地图距离屏幕顶部的边距
    int room_size = 6;     // 每个小房间方块的大小
    int spacing = 8;      // 房间中心之间的间距 (留2像素空隙)

    //vga_draw_rect(0, 0, room_size * 7, room_size * 8, Grey);

    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            if (world_map[i][j].exists) {
                // 计算在屏幕上的绝对像素坐标
                // (i - min_i) 确保最左边的房间从 0 开始算
                int draw_x = map_offset_x + (i - min_i) * spacing;
                int draw_y = map_offset_y + (j - min_j) * spacing;

                // 3. 区分颜色显示
                short int color;
                if (i == current_room_x && j == current_room_y) {
                    color = Green; // 玩家当前所在房间用绿色
                } else if (world_map[i][j].is_cleared) {
                    color = Blue;  // 已清空的房间用蓝色
                } else {
                    color = Red;   // 有怪的房间用红色
                }

                // 绘制小方块 (可以使用你之前的画矩形函数)
                vga_draw_rect(draw_x, draw_y, room_size, room_size, color);
            }
        }
    }
}

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

    RoomSeeds *s = &world_map[current_room_x][current_room_y].seeds;
    current_inner_texture = s->current_inner_texture; 

    for(int c = 0; c < s->num_clusters; c++) {
        int shape = s->shape[c];
        int cx = s->cx[c];
        int cy = s->cy[c];

        // using the parameters in RoomSeeds
        if (shape == 0) { 
            walls[cx][cy] = 1;
        } 
        else if (shape == 1) { 
            walls[cx][cy] = 1;   walls[cx+1][cy] = 1;
            walls[cx][cy+1] = 1; walls[cx+1][cy+1] = 1;
        } 
        else if (shape == 2) { 
            walls[cx][cy] = 1; walls[cx+1][cy] = 1; walls[cx+2][cy] = 1;
        } 
        else if (shape == 3) { 
            walls[cx][cy] = 1; walls[cx][cy+1] = 1; walls[cx][cy+2] = 1;
        } 
        else if (shape == 4) { 
            walls[cx][cy] = 1; 
            walls[cx][cy+1] = 1; 
            walls[cx+1][cy+1] = 1;
        }
    }
    // to make sure the edge areas are passable
    for(int i = 0; i < GRID_W; i++ ){
        walls[i][2] = 0;
        walls[i][GRID_H - 3] = 0;
    }
    for(int i = 0; i < GRID_H; i++ ){
        walls[2][i] = 0;
        walls[GRID_W - 3][i] = 0;
    }

    //duplicate the left areas to the right
    for(int x = 1; x <= 7; x++) {
        for (int y = 1; y <= 10; y++) {
            // 中心对称公式
            walls[15 - x][11 - y] = walls[x][y]; 
        }
    }
    apply_doors_to_walls();
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
            if(walls[i][j] == 2){
                vga_draw_rect(cur_x, cur_y, 20, 20, Black);
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