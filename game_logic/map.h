#ifndef MAP_H                           //if define before
#define MAP_H                           //then define

#define SCREEN_W 320                    //screen wdith 320 pixels
#define SCREEN_H 240                    //screen height 240 pixels

#define BORDER_THICKNESS 8              //edge thickness

#define MIN_OBS 3                       //Minimum number of obstacle present
#define MAX_OBS 7                        //Maximum number of obstalce present

#define MIN_OBS_LEN 30                  //Minimum length of obstacle
#define MAX_OBS_LEN 80                  //Maximum length of obstalce

#define MIN_OBS_THICK 8                 //Minimum thickness of obstacle
#define MAX_OBS_THICK 16                //Maximum thickness of obstacle 

#define BLACK 0x0000
#define RED 0xF800

typedef struct{                         //New data type called Rect
    int x;                              //x - coordinate
    int y;                              //y - coordinate
    int w;                              //width
    int h;                              //height
} Rect;

extern Rect obstacle[MAX_OBS];          //
extern int num_obstacles;               //It represent number of obstacles

void init_obstacles(void);              //Generates obstacles
void draw_rect(int x, int y, int w, int h, short int color);        //draw a rectangle on the screen
void draw_border(void);                 //draw red border around the screen
void draw_obstacles(void);              //draw all obstacles
void draw_map(void);                    //draw the whole map

int overlap(Rect a, Rect b);            //check if a nad b are overalap
int can_move_to(int new_x, int new_y, int player_size);             //Checking if player can move the (new_x, new_y) location



