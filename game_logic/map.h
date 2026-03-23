#ifndef MAP_H
#define MAP_H

#define SCREEN_W 320
#define SCREEN_H 240

#define BLACK 0x0000
#define RED 0xF800
#define Brown 0xa3c0
#define Grey 0x5aeb

#define min_space 16
#define GRID_W 16
#define GRID_H 12
#define world_map_max 13

typedef struct Player Player;

extern int current_room_x;
extern int current_room_y;

typedef struct {
    int num_clusters;
    int current_inner_texture;
    int shape[4];
    int cx[4];
    int cy[4];
} RoomSeeds;

typedef struct {
    int exists;
    int is_cleared;
    RoomSeeds seeds;
} Room;

void init_walls();
void draw_walls();
void draw_world_map();
void init_world_map();
int can_move_to(int new_x, int new_y, int player_width, int player_height);
int handle_room_transition(Player *p);
void apply_doors_to_walls();

#endif