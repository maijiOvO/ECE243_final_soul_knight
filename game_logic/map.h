#ifndef MAP_H                           //if define before
#define MAP_H                           //then define

#define SCREEN_W 320                    //screen wdith 320 pixels
#define SCREEN_H 240                    //screen height 240 pixels

#define BLACK 0x0000
#define RED 0xF800

#define min_space 16
#define GRID_W 16
#define GRID_H 12

typedef struct {
    int exists;     // 0: 无房间, 1: 有房间
    int is_cleared; // 0: 战斗中/未进入, 1: 已清空
} Room;

void init_walls();
void draw_walls();
void draw_world_map();
void init_world_map();
int can_move_to(int new_x, int new_y, int player_width, int player_height);            //Checking if player can move the (new_x, new_y) location

#endif
