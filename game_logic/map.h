#ifndef MAP_H                           //if define before
#define MAP_H                           //then define

#define SCREEN_W 320                    //screen wdith 320 pixels
#define SCREEN_H 240                    //screen height 240 pixels

#define BLACK 0x0000
#define RED 0xF800
#define Brown 0xa3c0
#define Grey 0x5aeb

#define min_space 16
#define GRID_W 16
#define GRID_H 12
#define world_map_max 13
#include"player.h"
extern int current_room_x;
extern int current_room_y;

typedef struct {
    int exists;     // 0: 无房间, 1: 有房间
    int is_cleared; // 0: 战斗中/未进入, 1: 已清空
} Room;

void init_walls();
void draw_walls();
void draw_world_map();
void init_world_map();
int can_move_to(int new_x, int new_y, int player_width, int player_height);            //Checking if player can move the (new_x, new_y) location
int handle_room_transition(Player *p);
void apply_doors_to_walls();

#endif
