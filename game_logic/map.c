#include "map.h"
#include <stdlib.h>
#include "../hardware/vga.h"
#include "../hardware/ps2.h"
#include "../sources/knight_anim.h"
#include "../sources/map_texture.h"
#include "player.h"
#define PORTAL_ANIM_SPEED 6

#include "enemy.h"

int portal_frame = 0;
int portal_tick = 0;
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

// 检查玩家是否正在尝试与宝箱交互
void handle_chest_interaction(Player *p) {
    // 1. 只在宝箱房生效
    Room *cur_room = &world_map[current_room_x][current_room_y];
    if (cur_room->type != 'C') return;

    // 2. 计算玩家中心与房间中心(宝箱位置)的距离
    // 假设宝箱画在 (150, 110)
    int chest_center_x = 150 + 10; 
    int chest_center_y = 110 + 10;
    int player_center_x = p->x + (p->width / 2);
    int player_center_y = p->y + (p->height / 2);

    int dist_x = abs(player_center_x - chest_center_x);
    int dist_y = abs(player_center_y - chest_center_y);

    // 3. 判定范围：如果距离宝箱小于 35 像素
    if (dist_x < 35 && dist_y < 35) {
        
        // 如果宝箱还没开，显示交互提示
        if (!cur_room->is_opened) {
            // 在玩家头顶画一个绿色的感叹号 “!” 提示玩家按 J (替换掉原来的 F 形状)
            vga_draw_rect(p->x + (p->width/2) - 1, p->y - 20, 2, 8, Green); // 感叹号的上半部分
            vga_draw_rect(p->x + (p->width/2) - 1, p->y - 10, 2, 2, Green); // 感叹号的点

            // 4. 🌟 修改：检测按键，按下 J 键瞬间开启 (替换了 key_F)
            if (key_J) {
                cur_room->is_opened = 1;
                // 可以在这里加入即时反馈，比如满血
                p->current_hp = p->max_hp;
            }
        }
    }
}

void draw_room_objects() {
    Room cur = world_map[current_room_x][current_room_y];

    if (cur.type == 'C') {
        // 宝箱位置固定在 (160, 120)
        if (cur.is_opened) {
            // 画开启后的贴图 box[0]
            Draw_Transparent_Image(150, 110, box[0], 20, 20);
        } else {
            // 画关闭的贴图 box[1]
            Draw_Transparent_Image(150, 110, box[1], 20, 20);
        }
    }
}

void init_world_map() {
    // 1. 彻底清空（防止 Restart 后的脏数据）
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            world_map[i][j].exists = 0;
            world_map[i][j].is_cleared = 0;
            world_map[i][j].type = ' ';
        }
    }

    // 2. 准备坐标记录表（数组大小要大于等于 max_rooms）
    int rooms_x[16], rooms_y[16]; 
    int count = 0;
    max_rooms = 7; // 建议设为 7-8 比较好玩

    // 3. 设置起点
    current_room_x = 6;
    current_room_y = 6;
    world_map[6][6].exists = 1;
    world_map[6][6].is_cleared = 1;
    world_map[6][6].type = 'I'; // Initial
    rooms_x[count] = 6; 
    rooms_y[count] = 6;
    count++;

    // 4. 随机漫步
    int rx = 6, ry = 6;
    while(count < max_rooms) {
        int dir = rand() % 4;
        if      (dir == 0) ry--;
        else if (dir == 1) ry++;
        else if (dir == 2) rx--;
        else               rx++;

        // 边界保护
        if (rx < 0 || rx >= 13 || ry < 0 || ry >= 13) {
            rx = 6; ry = 6; continue;
        }

        if (world_map[rx][ry].exists == 0) {
            world_map[rx][ry].exists = 1;
            world_map[rx][ry].is_cleared = 1; // 调试期全开，后期改为 0
            world_map[rx][ry].type = 'M';    // Monster
            
            // 为新房间生成种子
            RoomSeeds *s = &world_map[rx][ry].seeds;
            s->num_clusters = rand() % 3 + 2;
            s->current_inner_texture = rand() % 5;
            for (int k = 0; k < 4; k++) {
                s->shape[k] = rand() % 5;
                s->cx[k] = rand() % 3 + 2;
                s->cy[k] = rand() % 4 + 3;
            }

            // 记录位置用于计算 Boss 房
            rooms_x[count] = rx;
            rooms_y[count] = ry;
            count++;
        }
    }

    // 5. 寻找最远点作为 Boss 房
    int max_dist = -1, boss_idx = -1;
    for (int i = 1; i < count; i++) {
        int dist = abs(rooms_x[i] - 6) + abs(rooms_y[i] - 6);
        if (dist > max_dist) {
            max_dist = dist;
            boss_idx = i;
        }
    }
    if (boss_idx != -1) world_map[rooms_x[boss_idx]][rooms_y[boss_idx]].type = 'B';

    // 6. 随机选一个不是起点也不是 Boss 的房间作为宝箱房
    if (count > 2) {
        int c_idx;
        do { c_idx = rand() % count; } while (c_idx == 0 || c_idx == boss_idx);
        world_map[rooms_x[c_idx]][rooms_y[c_idx]].type = 'C';
        world_map[rooms_x[c_idx]][rooms_y[c_idx]].is_cleared = 1;
        world_map[rooms_x[c_idx]][rooms_y[c_idx]].seeds.num_clusters = 0;
    }
}

void apply_doors_to_walls() {       // the position of walls

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
        init_walls();
        apply_doors_to_walls();
        clear_screen(); 
        vga_wait_for_vsync(); 
        clear_screen(); 
        
        // 在这里刷怪！把刚刚算好的新房间坐标传进去
        Spawn_Enemies_For_Room(current_room_x, current_room_y);
    }

    return teleported;
}

void draw_world_map() {
    int min_i = 13, min_j = 13;
    bool any_room = false;

    // 1. 第一遍扫描：动态寻找当前所有房间的最左上角坐标
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            if (world_map[i][j].exists) {
                if (i < min_i) min_i = i;
                if (j < min_j) min_j = j;
                any_room = true;
            }
        }
    }
    if (!any_room) return;

    // 2. 绘图参数
    int map_offset_x = 15; 
    int map_offset_y = 15;
    int room_size = 6;
    int spacing = 8;

    // 3. 第二遍扫描：绘制
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            if (world_map[i][j].exists) {
                int draw_x = map_offset_x + (i - min_i) * spacing;
                int draw_y = map_offset_y + (j - min_j) * spacing;

                short int color;
                // 玩家当前所在位置（最高优先级）
                if (i == current_room_x && j == current_room_y) {
                    color = Green;
                } 
                // 其他房间
                else {
                    char type = world_map[i][j].type;
                    if (type == 'B') color = Pink;       // Boss 房
                    else if (type == 'C') color = 0xFFE0; // 宝箱房 (黄)
                    else color = Blue;                    // 已清空/普通房
                }
                
                // 保护性绘图：确保不越界
                if (draw_x >= 0 && draw_y >= 0) {
                    vga_draw_rect(draw_x, draw_y, room_size, room_size, color);
                }
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
    for(int i = 1; i < GRID_W - 1; i ++){
        for(int j = 1; j < GRID_H - 1; j ++){
            int cur_x = 20 * i;
            int cur_y = 20 * j;
            if(walls[i][j] == 1){
                if(current_inner_texture == 3){Draw_Transparent_Image(cur_x, cur_y, wall_texture[current_inner_texture], 20, 20);}
                else{Draw_Opaque_Image(cur_x, cur_y, wall_texture[current_inner_texture], 20, 20);}
            }
        }
    }
}

void draw_portals() {
    // 1. 更新动画帧索引
    portal_tick++;
    if (portal_tick >= PORTAL_ANIM_SPEED) {
        portal_frame = (portal_frame + 1) % 8;
        portal_tick = 0;
    }

    const unsigned char* current_portal_img = portal[portal_frame];

    for (int i = 0; i < GRID_W; i++) {
        for (int j = 0; j < GRID_H; j++) {
            if (walls[i][j] == 2) {
                int cur_x = 20 * i;
                int cur_y = 20 * j;

                // --- 情况 A：顶部门 (walls[7][0], walls[8][0]) ---
                if (j == 0 && i == 7) {
                    Draw_Transparent_Image_CCW90(cur_x, cur_y, current_portal_img, 20, 40);
                }

                // --- 情况 B：底部门 (walls[7][11], walls[8][11]) ---
                else if (j == GRID_H - 1 && i == 7) {
                    Draw_Transparent_Image_CW90(cur_x, cur_y, current_portal_img, 20, 40);
                }

                // --- 情况 C：左侧门 (walls[0][5], walls[0][6]) ---
                else if (i == 0 && j == 5) {
                    Draw_Transparent_Image_horizontal_flip(cur_x, cur_y, current_portal_img, 20, 40);
                }

                // --- 情况 D：右侧门 (walls[15][5], walls[15][6]) ---
                else if (i == GRID_W - 1 && j == 5) {
                    Draw_Transparent_Image(cur_x, cur_y, current_portal_img, 20, 40);
                }
            }
        }
    }

    // ==========================================
    // 🌟 核心修改：在画完传送门后，顺便更新并画出怪物
    // 因为 main.c 每一帧都会调用 draw_portals，
    // 所以这里的怪物也会跟着每一帧更新和渲染！
    // ==========================================
   
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